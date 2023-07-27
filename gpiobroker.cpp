#include "gpiobroker.h"

#include "aliseconstants.h"
#include "avtukccu.h"

#include <QDebug>
#include <QRandomGenerator>
#include <config.h>
#include <cstdlib>
#include <gen/datamanager/datamanager.h>
#include <gen/datatypes.h>
#include <gen/error.h>
#include <sys/reboot.h>
#include <unistd.h>

constexpr GpioBroker::GpioPin PowerStatusPin0 { 0, 5 };
constexpr GpioBroker::GpioPin PowerStatusPin1 { 3, 17 };
constexpr GpioBroker::GpioPin LedPin { 1, 31 };
constexpr GpioBroker::GpioPin ResetPin { 2, 6 };

GpioBroker::GpioBroker(QObject *parent) : Broker(parent)
{
    qDebug() << "[GPIO] GPIO Broker created";
    m_resetTimer.setInterval(AliseConstants::ResetCheckPeriod());
    m_gpioTimer.setInterval(AliseConstants::GpioBlinkCheckPeriod());
    QObject::connect(&m_resetTimer, &QTimer::timeout, this, &GpioBroker::reset);
    QObject::connect(&m_gpioTimer, &QTimer::timeout, this, &GpioBroker::blink);
    m_resetTimer.start();
    m_gpioTimer.start();

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
        line.request({ PROGNAME, ::gpiod::line_request::DIRECTION_INPUT, 0 });
    }
    chip3.open(std::to_string(3));
    {
        auto line = chip3.get_line(PowerStatusPin1.offset);
        line.request({ PROGNAME, ::gpiod::line_request::DIRECTION_INPUT, 0 });
    }
    chip1.get_line(LedPin.offset).set_value(blinkStatus);
}

void GpioBroker::checkPowerUnit()
{
    QMutexLocker locker(&_mutex);
    auto status1 = chip0.get_line(PowerStatusPin0.offset).get_value();
    auto status2 = chip3.get_line(PowerStatusPin1.offset).get_value();
    qDebug() << "[GPIO] PWR status 1: " << status1 << ", PWR status 2: " << status2;
    DataTypes::BlockStruct blk;
    blk.data.resize(sizeof(AVTUK_CCU::Main));
    AVTUK_CCU::Main str;

    str.PWRIN = status2 | (status1 << 1);
    qDebug() << "[GPIO] PWRIN: " << str.PWRIN;
    str.resetReq = false;
    std::memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
    blk.ID = AVTUK_CCU::MainBlock;
    DataManager::GetInstance().addSignalToOutList(blk);
}

void GpioBroker::setIndication()
{
    QMutexLocker locker(&_mutex);
    m_gpioTimer.setInterval(m_currentBlinkingPeriod);
}

void GpioBroker::setTime(timespec time)
{
    Q_UNUSED(time)
}

void GpioBroker::getTime()
{
}

void GpioBroker::rebootMyself()
{
    qDebug() << "[GPIO] Rebooting...";
    m_gpioTimer.stop();
    m_resetTimer.stop();
    chip1.get_line(LedPin.offset).set_value(false);
    reboot(RB_AUTOBOOT);
}

void GpioBroker::reset()
{
    QMutexLocker locker(&_mutex);
    bool value = !chip2.get_line(ResetPin.offset).get_value();

    if (value) // button pushed, counting seconds...
    {
        resetCounter += value;
        return;
    }
    else if (resetCounter == 0) // normal mode, there's no any button push
        return;

    // soft reset (only reboot)
    if ((resetCounter > (AliseConstants::SecondsToHardReset() / 2)) && (resetCounter <= AliseConstants::SecondsToHardReset()))
    {
        qDebug() << "[GPIO] Reboot only";
        rebootMyself();
        return;
    }
    // hard reset
    if (resetCounter > AliseConstants::SecondsToHardReset())
    {
        qDebug() << "[GPIO] Reset interface settings...";

        auto status1 = chip0.get_line(PowerStatusPin0.offset).get_value();
        auto status2 = chip3.get_line(PowerStatusPin1.offset).get_value();

        DataTypes::BlockStruct blk;
        blk.data.resize(sizeof(AVTUK_CCU::Main));
        AVTUK_CCU::Main str;

        str.PWRIN = status2 | (status1 << 1);
        str.resetReq = true;
        std::memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
        blk.ID = AVTUK_CCU::MainBlock;
        DataManager::GetInstance().addSignalToOutList(blk);
    }
    resetCounter = 0;
}

void GpioBroker::blink()
{
    chip1.get_line(LedPin.offset).set_value(blinkStatus);
    blinkStatus = !blinkStatus;
}
