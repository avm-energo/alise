#include "alisesettings.h"

#include "aliseconstants.h"

#include <QtDebug>
#include <avm-gen/stdfunc.h>

AliseSettings::AliseSettings()
{
}

void AliseSettings::init()
{
    m_isValid = true;
    m_settings = new QSettings("/avtuk/settings/avm-alise/settings/settings.ini", QSettings::IniFormat);
    logFilename = m_settings->value("Logs/logfile", "/avtuk/settings/avm-alise/logs/avm-alise.log").toString();
    m_log.writeStart(logFilename);
}

void AliseSettings::readSettings()
{
    Q_ASSERT(m_settings != nullptr);

    int logcounter = m_settings->value("Test/counter", "1").toInt();
    m_settings->setValue("Test/counter", ++logcounter);
    m_logLevel = m_settings->value("Logs/Loglevel", "Info").toString();
    m_httpPort = m_settings->value("Main/HttpPort", "5555").toInt();
    Alise::AliseConstants::setFailureBlinkFreq(m_settings->value("Timers/FailureBlink", "50").toInt());
    Alise::AliseConstants::setProcessStartingBlinkFreq(m_settings->value("Timers/StartingBlink", "250").toInt());
    Alise::AliseConstants::setProcessSemiWorkingBlinkFreq(m_settings->value("Timers/SemiWorkingBlink", "1000").toInt());
    Alise::AliseConstants::setProcessNormalBlinkFreq(m_settings->value("Timers/NormalBlink", "500").toInt());
    Alise::AliseConstants::setProcessStoppedBlinkFreq(m_settings->value("Timers/StoppedBlink", "2000").toInt());
    Alise::AliseConstants::setProcessFailedBlinkFreq(m_settings->value("Timers/FailedBlink", "125").toInt());
    Alise::AliseConstants::setPowerCheckPeriod(m_settings->value("Timers/PowerCheckPeriod", "1000").toInt());
    Alise::AliseConstants::setResetCheckPeriod(m_settings->value("Timers/ResetCheckPeriod", "1000").toInt());
    Alise::AliseConstants::setUpdateTimePeriod(m_settings->value("Timers/UpdateTimePeriod", "3000").toInt());
    Alise::AliseConstants::setHealthQueryPeriod(m_settings->value("Timers/HealthQueryPeriod", "1500").toInt());
    Alise::AliseConstants::setReplyTimeoutPeriod(m_settings->value("Timers/ReplyTimeoutPeriod", "8000").toInt());
    Alise::AliseConstants::setSecondsToHardReset(m_settings->value("Reset/TimeToWaitForHardReset", "4").toInt());
    m_serialNum = m_settings->value("Module/SerialNumber", "4294967295").toString().toUInt(); // 0xffffffff
    m_serialNumB = m_settings->value("Module/BoardSerialNumber", "4294967295").toString().toUInt();
    m_hwVersion = m_settings->value("Module/HardwareVersion", "4294967295").toString().toUInt();
    m_swVersion = m_settings->value("Module/SoftwareVersion", "4294967295").toString().toUInt();
    m_gpioMap["Power1"] = parseGPIOSettings("Power1");
    m_gpioMap["Power2"] = parseGPIOSettings("Power2");
    m_gpioMap["ModeLed"] = parseGPIOSettings("ModeLed");
    m_gpioMap["Reset"] = parseGPIOSettings("Reset");
    m_gpioExceptionsAreOn = m_settings->value("Test/exceptions", "1").toBool();
    flush();
}

void AliseSettings::logSettings()
{
    Q_ASSERT(m_settings != nullptr);

    m_log.writeLog(m_log.All, "Reading settings from: " + m_settings->fileName());
    m_log.writeLog(m_log.All, "Startup information:");
    m_log.writeLog(m_log.All, "=========================");
    m_log.writeLog(m_log.All, "LogLevel: " + m_logLevel);
    m_log.writeLog(m_log.All, "HttpPort: " + QString::number(m_httpPort));
    m_log.writeLog(m_log.All, "--------- GPIOs ---------");
    m_log.writeLog(m_log.All,
        "Power1: " + QString::number(m_gpioMap["Power1"].pin) + ":" + QString::number(m_gpioMap["Power1"].offset));
    m_log.writeLog(m_log.All,
        "Power2: " + QString::number(m_gpioMap["Power2"].pin) + ":" + QString::number(m_gpioMap["Power2"].offset));
    m_log.writeLog(m_log.All,
        "ModeLed: " + QString::number(m_gpioMap["ModeLed"].pin) + ":" + QString::number(m_gpioMap["ModeLed"].offset));
    m_log.writeLog(m_log.All,
        "Reset: " + QString::number(m_gpioMap["Reset"].pin) + ":" + QString::number(m_gpioMap["Reset"].offset));
    m_log.writeLog(m_log.All, "-------- Blinks ---------");
    m_log.writeLog(
        m_log.All, "NormalBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::NORMAL)) + " ms");
    m_log.writeLog(m_log.All,
        "StartingBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::YELLOW)) + " ms");
    m_log.writeLog(m_log.All,
        "StoppedBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::ORANGE)) + " ms");
    m_log.writeLog(m_log.All,
        "SemiWorkingBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::VIOLET)) + " ms");
    m_log.writeLog(m_log.All,
        "ProcessFailedBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::RED)) + " ms");
    m_log.writeLog(m_log.All, "FailureBlink period:" + QString::number(Alise::AliseConstants::FailureBlink()) + " ms");
    m_log.writeLog(
        m_log.All, "Power check period:" + QString::number(Alise::AliseConstants::PowerCheckPeriod()) + " ms");
    m_log.writeLog(
        m_log.All, "Reset check period:" + QString::number(Alise::AliseConstants::ResetCheckPeriod()) + " ms");
    m_log.writeLog(
        m_log.All, "Time update period:" + QString::number(Alise::AliseConstants::UpdateTimePeriod()) + " ms");
    m_log.writeLog(
        m_log.All, "Health query period:" + QString::number(Alise::AliseConstants::HealthQueryPeriod()) + " ms");
    m_log.writeLog(
        m_log.All, "Reply timeout period:" + QString::number(Alise::AliseConstants::ReplyTimeoutPeriod()) + " ms");
    m_log.writeLog(m_log.All, "-------- Module ---------");
    m_log.writeLog(m_log.All, "Module serial: " + QString::number(m_serialNum));
    m_log.writeLog(m_log.All, "Board serial: " + QString::number(m_serialNumB));
    m_log.writeLog(m_log.All, "Board hardware: " + StdFunc::VerToStr(m_hwVersion));
    m_log.writeLog(m_log.All, "MCU software version: " + StdFunc::VerToStr(m_swVersion));
    const QString exceptionsMsg = (m_gpioExceptionsAreOn) ? "on" : "off";
    m_log.writeLog(m_log.All, "Log exceptions flag is " + exceptionsMsg);
}

void AliseSettings::writeSettings()
{
    Q_ASSERT(m_settings != nullptr);
    m_settings->setValue("Logs/Loglevel", m_logLevel);
    m_settings->setValue("Main/HttpPort", m_httpPort);
    m_settings->setValue("Timers/FailureBlink", Alise::AliseConstants::FailureBlink());
    m_settings->setValue("Timers/StartingBlink", Alise::AliseConstants::_blinksConstants.ProcessStatusStartingBlink);
    m_settings->setValue(
        "Timers/SemiWorkingBlink", Alise::AliseConstants::_blinksConstants.ProcessStatusSemiWorkingBlink);
    m_settings->setValue("Timers/NormalBlink", Alise::AliseConstants::_blinksConstants.ProcessStatusNormalBlink);
    m_settings->setValue("Timers/StoppedBlink", Alise::AliseConstants::_blinksConstants.ProcessStatusStoppedBlink);
    m_settings->setValue("Timers/FailedBlink", Alise::AliseConstants::_blinksConstants.ProcessStatusFailedBlink);
    m_settings->setValue("Timers/PowerCheckPeriod", Alise::AliseConstants::PowerCheckPeriod());
    m_settings->setValue("Timers/ResetCheckPeriod", Alise::AliseConstants::ResetCheckPeriod());
    m_settings->setValue("Timers/UpdateTimePeriod", Alise::AliseConstants::UpdateTimePeriod());
    m_settings->setValue("Timers/HealthQueryPeriod", Alise::AliseConstants::HealthQueryPeriod());
    m_settings->setValue("Timers/ReplyTimeoutPeriod", Alise::AliseConstants::ReplyTimeoutPeriod());
    m_settings->setValue("Reset/TimeToWaitForHardReset", Alise::AliseConstants::SecondsToHardReset());
    m_settings->setValue("Module/SerialNumber", m_serialNum);
    m_settings->setValue("Module/BoardSerialNumber", m_serialNumB);
    m_settings->setValue("Module/HardwareVersion", m_hwVersion);
    m_settings->setValue("Module/SoftwareVersion", m_swVersion);
    m_settings->setValue("Test/exceptions", m_gpioExceptionsAreOn);
    setGPIOValues();
    flush();
}

void AliseSettings::flush()
{
    m_settings->sync();
}

bool AliseSettings::isValid()
{
    return m_isValid;
}

AliseSettings::GPIOInfo AliseSettings::parseGPIOSettings(const QString &pinName)
{
    if (!m_isValid)
        return { -1, -1 };
    GPIOInfo pinInfo;
    bool ok;
    QString str = m_settings->value("GPIO/" + pinName, "3 5").toString();
    QStringList sl = str.split(" ");
    if (sl.size() > 1)
    {
        pinInfo.pin = sl.at(0).toInt(&ok);
        if (ok)
        {
            pinInfo.offset = sl.at(1).toInt(&ok);
            if (ok)
                return pinInfo;
        }
    }
    m_isValid = false;
    return { -1, -1 };
}

void AliseSettings::setGPIOValues()
{
    for (auto [key, value] : m_gpioMap.asKeyValueRange())
        m_settings->setValue("GPIO/" + key, QString::number(value.pin) + " " + QString::number(value.offset));
}
