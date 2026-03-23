#include "alisesettings.h"

#include "aliseconstants.h"

#include <QtDebug>
#include <gen/stdfunc.h>

AliseSettings::AliseSettings()
{
}

void AliseSettings::init()
{
    m_isValid = true;
    m_settings = new QSettings("/avtuk/settings/alise/settings/settings.ini", QSettings::IniFormat);
    logFilename = m_settings->value("Logs/logfile", "/avtuk/settings/alise/logs/alise.log").toString();
}

void AliseSettings::readSettings()
{
    Q_ASSERT(m_settings != nullptr);

    int logcounter = m_settings->value("Test/counter", "1").toInt();
    m_settings->setValue("Test/counter", ++logcounter);
    logLevel = m_settings->value("Logs/Loglevel", "Info").toString();
    httpPort = m_settings->value("Main/HttpPort", "5555").toInt();
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
    serialNum = m_settings->value("Module/SerialNumber", "4294967295").toString().toUInt(); // 0xffffffff
    serialNumB = m_settings->value("Module/BoardSerialNumber", "4294967295").toString().toUInt();
    hwVersion = m_settings->value("Module/HardwareVersion", "4294967295").toString().toUInt();
    swVersion = m_settings->value("Module/SoftwareVersion", "4294967295").toString().toUInt();
    m_gpioMap["Power1"] = parseGPIOSettings("Power1");
    m_gpioMap["Power2"] = parseGPIOSettings("Power2");
    m_gpioMap["ModeLed"] = parseGPIOSettings("ModeLed");
    m_gpioMap["Reset"] = parseGPIOSettings("Reset");
    gpioExceptionsAreOn = m_settings->value("Test/exceptions", "1").toBool();
    flush();
}

void AliseSettings::logSettings()
{
    Q_ASSERT(m_settings != nullptr);

    Logger::writeLog(Logger::All, "Reading settings from: " + m_settings->fileName());
    Logger::writeLog(Logger::All, "Startup information:");
    Logger::writeLog(Logger::All, "=========================");
    Logger::writeLog(Logger::All, "LogLevel: " + logLevel);
    Logger::writeLog(Logger::All, "HttpPort: " + QString::number(httpPort));
    Logger::writeLog(Logger::All, "--------- GPIOs ---------");
    Logger::writeLog(Logger::All,
        "Power1: " + QString::number(m_gpioMap["Power1"].pin) + ":" + QString::number(m_gpioMap["Power1"].offset));
    Logger::writeLog(Logger::All,
        "Power2: " + QString::number(m_gpioMap["Power2"].pin) + ":" + QString::number(m_gpioMap["Power2"].offset));
    Logger::writeLog(Logger::All,
        "ModeLed: " + QString::number(m_gpioMap["ModeLed"].pin) + ":" + QString::number(m_gpioMap["ModeLed"].offset));
    Logger::writeLog(Logger::All,
        "Reset: " + QString::number(m_gpioMap["Reset"].pin) + ":" + QString::number(m_gpioMap["Reset"].offset));
    Logger::writeLog(Logger::All, "-------- Blinks ---------");
    Logger::writeLog(Logger::All,
        "NormalBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::NORMAL)) + " ms");
    Logger::writeLog(Logger::All,
        "StartingBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::YELLOW)) + " ms");
    Logger::writeLog(Logger::All,
        "StoppedBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::ORANGE)) + " ms");
    Logger::writeLog(Logger::All,
        "SemiWorkingBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::VIOLET)) + " ms");
    Logger::writeLog(Logger::All,
        "ProcessFailedBlink period:" + QString::number(Alise::AliseConstants::ProcessBlink(Alise::RED)) + " ms");
    Logger::writeLog(
        Logger::All, "FailureBlink period:" + QString::number(Alise::AliseConstants::FailureBlink()) + " ms");
    Logger::writeLog(
        Logger::All, "Power check period:" + QString::number(Alise::AliseConstants::PowerCheckPeriod()) + " ms");
    Logger::writeLog(
        Logger::All, "Reset check period:" + QString::number(Alise::AliseConstants::ResetCheckPeriod()) + " ms");
    Logger::writeLog(
        Logger::All, "Time update period:" + QString::number(Alise::AliseConstants::UpdateTimePeriod()) + " ms");
    Logger::writeLog(
        Logger::All, "Health query period:" + QString::number(Alise::AliseConstants::HealthQueryPeriod()) + " ms");
    Logger::writeLog(
        Logger::All, "Reply timeout period:" + QString::number(Alise::AliseConstants::ReplyTimeoutPeriod()) + " ms");
    Logger::writeLog(Logger::All, "-------- Module ---------");
    Logger::writeLog(Logger::All, "Module serial: " + QString::number(serialNum));
    Logger::writeLog(Logger::All, "Board serial: " + QString::number(serialNumB));
    Logger::writeLog(Logger::All, "Board hardware: " + StdFunc::VerToStr(hwVersion));
    Logger::writeLog(Logger::All, "MCU software version: " + StdFunc::VerToStr(swVersion));
    const QString exceptionsMsg = (gpioExceptionsAreOn) ? "on" : "off";
    Logger::writeLog(Logger::All, "Log exceptions flag is " + exceptionsMsg);
}

void AliseSettings::writeSettings()
{
    Q_ASSERT(m_settings != nullptr);
    m_settings->setValue("Logs/Loglevel", logLevel);
    m_settings->setValue("Main/HttpPort", httpPort);
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
    m_settings->setValue("Module/SerialNumber", serialNum);
    m_settings->setValue("Module/BoardSerialNumber", serialNumB);
    m_settings->setValue("Module/HardwareVersion", hwVersion);
    m_settings->setValue("Module/SoftwareVersion", swVersion);
    m_settings->setValue("Test/exceptions", gpioExceptionsAreOn);
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
