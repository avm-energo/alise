#include "alisesettings.h"

#include "aliseconstants.h"

#include <QtDebug>

AliseSettings::AliseSettings()
{
}

void AliseSettings::init()
{
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
    flush();
}

void AliseSettings::logSettings()
{
    Q_ASSERT(m_settings != nullptr);

    qInfo() << "Reading settings from: " << m_settings->fileName();
    qInfo() << "Startup information:";
    qInfo() << "=========================";
    qInfo() << "LogLevel: " << logLevel;
    qInfo() << "HttpPort: " << httpPort;
    qInfo() << "NormalBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::NORMAL) << " ms";
    qInfo() << "StartingBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::YELLOW) << " ms";
    qInfo() << "StoppedBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::ORANGE) << " ms";
    qInfo() << "SemiWorkingBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::VIOLET) << " ms";
    qInfo() << "ProcessFailedBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::RED) << " ms";
    qInfo() << "FailureBlink period:" << Alise::AliseConstants::FailureBlink() << " ms";
    qInfo() << "Power check period:" << Alise::AliseConstants::PowerCheckPeriod() << " ms";
    qInfo() << "Reset check period:" << Alise::AliseConstants::ResetCheckPeriod() << " ms";
    qInfo() << "Time update period:" << Alise::AliseConstants::UpdateTimePeriod() << " ms";
    qInfo() << "Health query period:" << Alise::AliseConstants::HealthQueryPeriod() << " ms";
    qInfo() << "Reply timeout period:" << Alise::AliseConstants::ReplyTimeoutPeriod() << " ms";
    qInfo() << "Module serial: " << serialNum;
    qInfo() << "Board serial: " << serialNumB;
    qInfo() << "Board hardware: " << hwVersion;
    qInfo() << "MCU software version: " << swVersion;
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
    flush();
}

void AliseSettings::flush()
{
    m_settings->sync();
}
