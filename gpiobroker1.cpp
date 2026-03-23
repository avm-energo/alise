#include "gpiobroker1.h"

#include "avtukccu.h"

#include <QDebug>
#include <QRandomGenerator>
#include <config.h>
#include <cstdlib>
#include <gen/error.h>
#include <sys/reboot.h>
#include <unistd.h>

using namespace Alise;

GpioBroker::GpioBroker(QMap<QString, AliseSettings::GPIOInfo> &gpioMap, QObject *parent) : Broker(parent)
{
    qDebug() << "[GPIO] GPIO Broker created";
    m_blinkMode = BlinkMode::ONEBLINK;
    m_pinList = {
        { "Power1", gpioMap["Power1"].pin, gpioMap["Power1"].offset, PinDirections::INPUT },
        { "Power2", gpioMap["Power2"].pin, gpioMap["Power2"].offset, PinDirections::INPUT },
        { "ModeLed", gpioMap["ModeLed"].pin, gpioMap["ModeLed"].offset, PinDirections::OUTPUT },
        { "Reset", gpioMap["Reset"].pin, gpioMap["Reset"].offset, PinDirections::INPUT },
    };
}

GpioBroker::~GpioBroker()
{
}

bool GpioBroker::connect()
{
    QMap<std::string, struct gpiod_line *> lineMap; // pairs: <lineName, line> for each pin
    m_resetTimer.setInterval(AliseConstants::ResetCheckPeriod());
    setIndication(AliseConstants::FailureIndication);
    QObject::connect(&m_resetTimer, &QTimer::timeout, this, &GpioBroker::reset);
    QObject::connect(&m_gpioTimer, &QTimer::timeout, this, &GpioBroker::blink);
    m_resetTimer.start();
    m_gpioTimer.start();

#ifndef ALISE_LOCALDEBUG
    for (auto pin : m_pinList)
    {
        gpiod_chip *chip;
        const std::string chipstr = "/dev/gpiochip" + std::to_string(pin.chip);
        if (!m_chipMap.contains(pin.chip)) // if chip is not already opened
        {
            chip = gpiod_chip_open(chipstr.c_str());
            if (chip == NULL)
            {
                qCritical() << "cannot open chip " << chipstr.c_str() << "!";
                return false;
            }
            m_chipMap[pin.chip] = chip;
            qDebug() << "chip " << chipstr.c_str() << " has been opened";
        }
        else
            chip = m_chipMap[pin.chip];
        gpiod_line *line = gpiod_chip_get_line(chip, pin.offset);
        if (line == NULL)
        {
            qCritical() << "cannot get line " << chipstr.c_str() << ":" << pin.offset << "!";
            return false;
        }
        if (line == NULL)
        {
            qCritical() << "Line " << pin.name.c_str() << " is NULL!";
            return false;
        }
        lineMap[pin.name] = line;
        if (pin.direction == GpioBroker::PinDirections::OUTPUT)
        {
            int ret = gpiod_line_request_output(line, "gpio", 0);
            if (ret != 0)
            {
                qCritical() << "cannot request line " << chipstr.c_str() << ":" << pin.offset << " for output!";
                return false;
            }
        }
        else
        {
            int ret = gpiod_line_request_input(line, "gpio");
            if (ret != 0)
            {
                qCritical() << "cannot request line " << chipstr.c_str() << ":" << pin.offset << " for input!";
                return false;
            }
        }
        qDebug() << "line " << chipstr.c_str() << ":" << pin.offset << " has been opened";
    }
    m_modeLine = lineMap["ModeLed"];
    m_pwr1Line = lineMap["Power1"];
    m_pwr2Line = lineMap["Power2"];
    m_resetLine = lineMap["Reset"];
    gpiod_line_set_value(m_modeLine, 1);
#endif
    return true;
}

void GpioBroker::checkPowerUnit()
{
    QMutexLocker locker(&m_mutex);
#ifndef ALISE_LOCALDEBUG
    auto status1 = gpiod_line_get_value(m_pwr1Line);
    usleep(10000);
    auto status2 = gpiod_line_get_value(m_pwr2Line);
#else
    int status1 = 0;
    int status2 = 0;
#endif
    DataTypes::BlockStruct blk;
    blk.data.resize(sizeof(AVTUK_CCU::Main));
    AVTUK_CCU::Main str;

    str.PWRIN = status2 | (status1 << 1);
    qDebug() << "[GPIO] PWRIN: " << str.PWRIN;
    str.resetReq = false;
    memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
    blk.ID = AVTUK_CCU::MainBlock;
    emit receivedBlock(blk);
}

void GpioBroker::setIndication(const AVTUK_CCU::Indication &indication)
{
    QMutexLocker locker(&m_mutex);
    AVTUK_CCU::Indication indic = indication;
    indic.PulseCnt1 *= 2; // one is on & one is off
    indic.PulseCnt2 *= 2; // the same
    if (m_currentIndication == indic)
        return;
    qDebug() << "new indication: PulseCnt1 = " << indic.PulseCnt1 << ", PulseFreq1 = " << indic.PulseFreq1
             << "PulseCnt2 = " << indic.PulseCnt2 << ", PulseFreq2 = " << indic.PulseFreq2;
    qDebug() << "old indication: PulseCnt1 = " << m_currentIndication.PulseCnt1
             << ", PulseFreq1 = " << m_currentIndication.PulseFreq1 << "PulseCnt2 = " << m_currentIndication.PulseCnt2
             << ", PulseFreq2 = " << m_currentIndication.PulseFreq2;
    m_currentIndication = indic;
    m_blinkCount = indic.PulseCnt1;
    m_blinkFreq = indic.PulseFreq1;
    if ((m_currentIndication.PulseCnt1 == 0) || (m_currentIndication.PulseFreq1 == 0))
    {
        m_blinkCount = c_maxBlinks;
        m_blinkFreq = indic.PulseFreq2;
        m_blinkMode = BlinkMode::ONEBLINK;
        restartBlinkTimer();
        return;
    }
    if ((m_currentIndication.PulseCnt2 == 0) || (m_currentIndication.PulseFreq2 == 0))
    {
        m_blinkCount = c_maxBlinks;
        m_blinkMode = BlinkMode::ONEBLINK;
        restartBlinkTimer();
        return;
    }
    else
        m_blinkMode = BlinkMode::TWOBLINKS;
    restartBlinkTimer();
}

void GpioBroker::rebootMyself()
{
    qDebug() << "[GPIO] Rebooting...";
    m_gpioTimer.stop();
    m_resetTimer.stop();
#ifndef ALISE_LOCALDEBUG
    gpiod_line_set_value(m_modeLine, PinOutputs::OFF);
#endif
    reboot(RB_AUTOBOOT);
}

void GpioBroker::reset()
{
    QMutexLocker locker(&m_mutex);
#ifndef ALISE_LOCALDEBUG
    bool value = !gpiod_line_get_value(m_resetLine);

    if (value) // button pushed, counting seconds...
    {
        m_resetCounter += value;
        return;
    }
    else if (m_resetCounter == 0) // normal mode, there's no any button push
        return;

    // soft reset (only reboot)
    if ((m_resetCounter > (AliseConstants::SecondsToHardReset() / 2))
        && (m_resetCounter <= AliseConstants::SecondsToHardReset()))
    {
        qDebug() << "[GPIO] Reboot only";
        rebootMyself();
        return;
    }
    // hard reset
    if (m_resetCounter > AliseConstants::SecondsToHardReset())
    {
        qDebug() << "[GPIO] Reset interface settings...";

        // set resetReq flag to RecoveryEngine to drop down eth settings to default
        DataTypes::BlockStruct blk;
        blk.data.resize(sizeof(AVTUK_CCU::Main));
        AVTUK_CCU::Main str;

        str.PWRIN = 0;
        str.resetReq = true;
        std::memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
        blk.ID = AVTUK_CCU::MainBlock;
        emit receivedBlock(blk);
    }
#endif
    m_resetCounter = 0;
}

void GpioBroker::restartBlinkTimer()
{
    m_gpioTimer.start(m_blinkFreq);
}

void GpioBroker::blink()
{
#ifndef ALISE_LOCALDEBUG
    --m_blinkCount;
    gpiod_line_set_value(m_modeLine, (m_blinkStatus) ? PinOutputs::ON : PinOutputs::OFF);
    m_blinkStatus = !m_blinkStatus;
    if (m_blinkCount <= 0)
    {
        if (m_blinkMode != BlinkMode::TWOBLINKS)
        {
            m_blinkCount = c_maxBlinks;
        }
        else if (m_blinkFreq == m_currentIndication.PulseFreq1)
        {
            m_blinkFreq = m_currentIndication.PulseFreq2;
            m_blinkCount = m_currentIndication.PulseCnt2;
            restartBlinkTimer();
        }
        else
        {
            m_blinkFreq = m_currentIndication.PulseFreq1;
            m_blinkCount = m_currentIndication.PulseCnt1;
            restartBlinkTimer();
        }
    }
#endif
}
