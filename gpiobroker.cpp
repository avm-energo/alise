#include "gpiobroker.h"

#include "../gen/datamanager.h"
#include "../gen/error.h"

#include <QDebug>
#include <QRandomGenerator>
#include <config.h>
#include <cstdlib>
#include <sys/reboot.h>
#include <unistd.h>

constexpr GpioBroker::GpioPin PowerStatusPin0 { 0, 5 };
constexpr GpioBroker::GpioPin PowerStatusPin1 { 3, 17 };
constexpr GpioBroker::GpioPin LedPin { 1, 31 };
constexpr GpioBroker::GpioPin ResetPin { 2, 6 };

GpioBroker::GpioBroker(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(1000);
    m_resetTimer.setInterval(100);
    m_gpioTimer.setInterval(50);
    QObject::connect(&m_timer, &QTimer::timeout, this, &GpioBroker::checkPowerUnit);
    QObject::connect(&m_gpioTimer, &QTimer::timeout, this, &GpioBroker::criticalBlinking);
    QObject::connect(&m_resetTimer, &QTimer::timeout, this, &GpioBroker::reset);
    m_timer.start();
    m_gpioTimer.start();
    m_resetTimer.start();
#ifdef TEST_INDICATOR
    QTimer *testTimer = new QTimer(this);
    testTimer->setInterval(70009);
    QObject::connect(testTimer, &QTimer::timeout, this, [this] {
        auto random = static_cast<alise::Health_Code>(QRandomGenerator::global()->bounded(0, 8));
        qDebug() << "Random:" << random;
        setIndication(random);
    });
    testTimer->start();
#endif

    chip0.open(std::to_string(0));
    {
        auto line = chip0.get_line(PowerStatusPin0.offset);
        line.request({ PROGNAME, ::gpiod::line_request::DIRECTION_INPUT, 0 });
    }
    chip1.open(std::to_string(1));
    {
        auto line = chip1.get_line(LedPin.offset);
        line.request({ PROGNAME, ::gpiod::line_request::DIRECTION_OUTPUT, 0 });
    }
    chip2.open(std::to_string(2));
    {
        auto line = chip2.get_line(ResetPin.offset);
        line.request({ PROGNAME, ::gpiod::line_request::DIRECTION_OUTPUT, 0 });
    }
    chip3.open(std::to_string(3));
    {
        auto line = chip3.get_line(PowerStatusPin1.offset);
        line.request({ PROGNAME, ::gpiod::line_request::DIRECTION_INPUT, 0 });
    }
}

void GpioBroker::checkPowerUnit()
{
    auto status1 = chip0.get_line(PowerStatusPin0.offset).get_value();
    auto status2 = chip3.get_line(PowerStatusPin1.offset).get_value();
    DataTypes::BlockStruct blk;
    blk.data.resize(sizeof(AVTUK_CCU::Main));
    AVTUK_CCU::Main str;

    // TODO Проверить то ли сдвигаем
    str.PWRIN = status1 ^ (status2 << 1);
    str.resetReq = false;
    std::memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
    blk.ID = AVTUK_CCU::MainBlock;
    DataManager::addSignalToOutList(DataTypes::SignalTypes::Block, blk);
}

void GpioBroker::setIndication(alise::Health_Code code)
{
    m_gpioTimer.stop();
    shortBlink = blinkStatus = 0;
    QObject::disconnect(&m_gpioTimer, &QTimer::timeout, nullptr, nullptr);
    chip1.get_line(LedPin.offset).set_value(blinkStatus);

    if (noBooter)
    {
        m_gpioTimer.setInterval(BlinkTimeout::small);
        noBooter = !noBooter;
    }

    switch (code)
    {
    case alise::Health_Code_Startup:
    {
        m_gpioTimer.setInterval(BlinkTimeout::small);
        QObject::connect(&m_gpioTimer, &QTimer::timeout, this, [&status = blinkStatus, &chip = chip1] {
            chip.get_line(LedPin.offset).set_value(status);
            status = !status;
        });
        break;
    }
    case alise::Health_Code_Work:
    {
        m_gpioTimer.setInterval(BlinkTimeout::big);
        QObject::connect(&m_gpioTimer, &QTimer::timeout, this, [&status = blinkStatus, &chip = chip1] {
            chip.get_line(LedPin.offset).set_value(status);
            status = !status;
        });
        break;
    }
    default:
    {
        m_gpioTimer.setInterval(BlinkTimeout::small);
        currentMode = BlinkMode::big;
        QObject::connect(&m_gpioTimer, &QTimer::timeout, this, [=] { blinker(code); });
    }
    }
    m_gpioTimer.start();
}

void GpioBroker::rebootMyself()
{
    m_timer.stop();
    m_gpioTimer.stop();
    m_resetTimer.stop();
    chip1.get_line(LedPin.offset).set_value(false);
    sync();
    reboot(RB_AUTOBOOT);
}

void GpioBroker::reset()
{
    bool value = !chip2.get_line(ResetPin.offset).get_value();

    if (value)
    {
        resetCounter += value;
        return;
    }

    // soft reset (only reboot)
    if ((resetCounter > 20) && (resetCounter <= 40))
    {
        resetCounter = 0;
        rebootMyself();
        return;
    }
    // hard reset
    if (resetCounter > 40)
    {
        resetCounter = 0;

        auto status1 = chip0.get_line(PowerStatusPin0.offset).get_value();
        auto status2 = chip3.get_line(PowerStatusPin1.offset).get_value();

        DataTypes::BlockStruct blk;
        blk.data.resize(sizeof(AVTUK_CCU::Main));
        AVTUK_CCU::Main str;

        // TODO Проверить то ли сдвигаем
        str.PWRIN = status1 ^ (status2 << 1);
        str.resetReq = true;
        std::memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
        blk.ID = AVTUK_CCU::MainBlock;
        DataManager::addSignalToOutList(DataTypes::SignalTypes::Block, blk);
    }
}

void GpioBroker::blinker(int code)
{

    switch (currentMode)
    {
    case BlinkMode::big:
    {

        if ((shortBlink / 2) == 1 && !(shortBlink % 2))
        {
            shortBlink = 0;
            m_gpioTimer.setInterval(BlinkTimeout::small);
            currentMode = BlinkMode::small;
            break;
        }

        m_gpioTimer.setInterval(BlinkTimeout::big);
        break;
    }
    case BlinkMode::small:
    {

        if ((shortBlink / 2) == code && !(shortBlink % 2))
        {
            shortBlink = 0;
            m_gpioTimer.setInterval(BlinkTimeout::big);
            currentMode = BlinkMode::big;
            break;
        }

        m_gpioTimer.setInterval(BlinkTimeout::small);
        break;
    }
    }
    ++shortBlink;
    chip1.get_line(LedPin.offset).set_value(blinkStatus);

    blinkStatus = !blinkStatus;
}

void GpioBroker::criticalBlinking()
{
    auto line = chip1.get_line(LedPin.offset);
    line.set_value(blinkStatus);
    blinkStatus = !blinkStatus;
}
