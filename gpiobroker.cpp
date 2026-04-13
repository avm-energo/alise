#include "gpiobroker.h"

#include "avtukccu.h"

#include <QDebug>
#include <QRandomGenerator>
#include <avm-gen/error.h>
#include <config.h>
#include <sys/reboot.h>
#include <unistd.h>

using namespace Alise;

GpioBroker::GpioBroker(QMap<QString, AliseSettings::GPIOInfo> &gpioMap, bool gpioExceptionsAreOn, QObject *parent)
    : Broker(parent)
{
    m_gpio = new GPIO(gpioExceptionsAreOn, this);
    qDebug() << "[GPIO] GPIO Broker has been created";
    m_blinkMode = BlinkMode::ONEBLINK;
    m_pinMap[pwr1] = { gpioMap[pwr1].pin, gpioMap[pwr1].offset, GPIO::PinDirections::INPUT };
    m_pinMap[pwr2] = { gpioMap[pwr2].pin, gpioMap[pwr2].offset, GPIO::PinDirections::INPUT };
    m_pinMap[mode] = { gpioMap[mode].pin, gpioMap[mode].offset, GPIO::PinDirections::OUTPUT };
    m_pinMap[rst] = { gpioMap[rst].pin, gpioMap[rst].offset, GPIO::PinDirections::INPUT };
}

GpioBroker::~GpioBroker()
{
}

bool GpioBroker::connect()
{
    m_resetTimer.setInterval(AliseConstants::ResetCheckPeriod());
    setIndication(AliseConstants::FailureIndication);
    QObject::connect(&m_resetTimer, &QTimer::timeout, this, &GpioBroker::reset);
    QObject::connect(&m_gpioTimer, &QTimer::timeout, this, &GpioBroker::blink);
    m_resetTimer.start();
    m_gpioTimer.start();
    m_gpio->gpioSetLineValue(m_pinMap[mode], GPIO::PinOutputs::ON);
    return true;
}

void GpioBroker::checkPowerUnit()
{
    QMutexLocker locker(&_mutex);
#ifndef ALISE_LOCALDEBUG
    auto status1 = m_gpio->gpioGetLineValue(m_pinMap[pwr1]);
    auto status2 = m_gpio->gpioGetLineValue(m_pinMap[pwr2]);
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
    QMutexLocker locker(&_mutex);
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
        //        qDebug() << "1. restartBlinkTimer: blinkCount = " << m_blinkCount << ", blinkMode = " << m_blinkMode
        //                 << ", blinkFreq = " << m_blinkFreq;
        restartBlinkTimer();
        return;
    }
    if ((m_currentIndication.PulseCnt2 == 0) || (m_currentIndication.PulseFreq2 == 0))
    {
        m_blinkCount = c_maxBlinks;
        m_blinkMode = BlinkMode::ONEBLINK;
        //        qDebug() << "2. restartBlinkTimer: blinkCount = " << m_blinkCount << ", blinkMode = " << m_blinkMode
        //                 << ", blinkFreq = " << m_blinkFreq;
        restartBlinkTimer();
        return;
    }
    else
        m_blinkMode = BlinkMode::TWOBLINKS;
    //    qDebug() << "Two blinks mode";
    restartBlinkTimer();
}

void GpioBroker::rebootMyself()
{
    qDebug() << "[GPIO] Rebooting...";
    m_gpioTimer.stop();
    m_resetTimer.stop();
#ifndef ALISE_LOCALDEBUG
    m_gpio->gpioSetLineValue(m_pinMap[mode], GPIO::PinOutputs::OFF);
#endif
    reboot(RB_AUTOBOOT);
}

void GpioBroker::reset()
{
    QMutexLocker locker(&_mutex);
#ifndef ALISE_LOCALDEBUG
    bool value = !m_gpio->gpioGetLineValue(m_pinMap[rst]);

    if (value) // button pushed, counting seconds...
    {
        resetCounter += value;
        return;
    }
    else if (resetCounter == 0) // normal mode, there's no any button push
        return;

    // soft reset (only reboot)
    if ((resetCounter > (AliseConstants::SecondsToHardReset() / 2))
        && (resetCounter <= AliseConstants::SecondsToHardReset()))
    {
        qDebug() << "[GPIO] Reboot only";
        rebootMyself();
        return;
    }
    // hard reset
    if (resetCounter > AliseConstants::SecondsToHardReset())
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
    resetCounter = 0;
}

void GpioBroker::restartBlinkTimer()
{
    m_gpioTimer.start(m_blinkFreq);
}

void GpioBroker::blink()
{
#ifndef ALISE_LOCALDEBUG
    --m_blinkCount;
    m_gpio->gpioSetLineValue(m_pinMap[mode], (m_blinkStatus) ? GPIO::PinOutputs::ON : GPIO::PinOutputs::OFF);
    m_blinkStatus = !m_blinkStatus;
    if (m_blinkCount <= 0)
    {
        if (m_blinkMode != BlinkMode::TWOBLINKS)
        {
            //            qDebug() << "Setting maxBlinks";
            m_blinkCount = c_maxBlinks;
        }
        else if (m_blinkFreq == m_currentIndication.PulseFreq1)
        {
            m_blinkFreq = m_currentIndication.PulseFreq2;
            m_blinkCount = m_currentIndication.PulseCnt2;
            //            qDebug() << "Setting PulseFreq2 = " << m_blinkFreq << ", PulseCnt2 = " << m_blinkCount;
            restartBlinkTimer();
        }
        else
        {
            m_blinkFreq = m_currentIndication.PulseFreq1;
            m_blinkCount = m_currentIndication.PulseCnt1;
            //            qDebug() << "Setting PulseFreq1 = " << m_blinkFreq << ", PulseCnt1 = " << m_blinkCount;
            restartBlinkTimer();
        }
    }
#endif
}
