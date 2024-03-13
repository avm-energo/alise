#include "alisesettings.h"

#include "aliseconstants.h"

#include <QtDebug>

AliseSettings::AliseSettings()
{
}

void AliseSettings::init()
{
#ifdef ALISE_LOCALDEBUG
    m_settings = new QSettings("~/sonica/alise/settings/settings.ini", QSettings::IniFormat);
    logFilename = m_settings->value("Logs/logfile", "~/sonica/alise/logs/alise.log").toString();
#else
    m_settings = new QSettings("/avtuk/settings/alise/settings/settings.ini", QSettings::IniFormat);
    logFilename = m_settings->value("Logs/logfile", "/avtuk/settings/alise/logs/alise.log").toString();
#endif
}

void AliseSettings::readSettings()
{
    int logcounter = m_settings->value("Test/counter", "1").toInt();
    m_settings->setValue("Test/counter", ++logcounter);
    logLevel = m_settings->value("Logs/Loglevel", "Info").toString();
    portCore = m_settings->value("Main/CorePort", "5555").toInt();
    portBooter = m_settings->value("Main/BooterPort", "5556").toInt();
    portAdminja = m_settings->value("Main/AdminjaPort", "5557").toInt();
    Alise::AliseConstants::setFailureBlinkFreq(m_settings->value("Timers/FailureBlink", "50").toInt());
    Alise::AliseConstants::setProcessStartingBlinkFreq(m_settings->value("Timers/StartingBlink", "250").toInt());
    Alise::AliseConstants::setProcessSemiWorkingBlinkFreq(m_settings->value("Timers/SemiWorkingBlink", "1000").toInt());
    Alise::AliseConstants::setProcessNormalBlinkFreq(m_settings->value("Timers/NormalBlink", "500").toInt());
    Alise::AliseConstants::setProcessStoppedBlinkFreq(m_settings->value("Timers/StoppedBlink", "2000").toInt());
    Alise::AliseConstants::setProcessFailedBlinkFreq(m_settings->value("Timers/FailedBlink", "125").toInt());
    Alise::AliseConstants::setPowerCheckPeriod(m_settings->value("Timers/PowerCheckPeriod", "1000").toInt());
    Alise::AliseConstants::setResetCheckPeriod(m_settings->value("Timers/ResetCheckPeriod", "1000").toInt());
    Alise::AliseConstants::setHealthQueryPeriod(m_settings->value("Timers/HealthQueryPeriod", "1500").toInt());
    Alise::AliseConstants::setReplyTimeoutPeriod(m_settings->value("Timers/ReplyTimeoutPeriod", "4000").toInt());
    Alise::AliseConstants::setSecondsToHardReset(m_settings->value("Reset/TimeToWaitForHardReset", "4").toInt());
    serialNum = m_settings->value("Module/SerialNumber", "FFFFFFFF").toString().toUInt();
    serialNumB = m_settings->value("Module/BoardSerialNumber", "FFFFFFFF").toString().toUInt();
    hwVersion = m_settings->value("Module/HardwareVersion", "FFFFFFFF").toString().toUInt();
    swVersion = m_settings->value("Module/SoftwareVersion", "FFFFFFFF").toString().toUInt();
}

void AliseSettings::logSettings()
{
    Q_ASSERT(m_settings != nullptr);

    qInfo() << "Reading settings from: " << m_settings->fileName();
    qInfo() << "Startup information:";
    qInfo() << "=========================";
    qInfo() << "LogLevel: " << logLevel;
    qInfo() << "CorePort: " << portCore;
    qInfo() << "BooterPort: " << portBooter;
    qInfo() << "AdminjaPort: " << portAdminja;
    qInfo() << "NormalBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::NORMAL) << " ms";
    qInfo() << "StartingBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::YELLOW) << " ms";
    qInfo() << "StoppedBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::ORANGE) << " ms";
    qInfo() << "SemiWorkingBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::VIOLET) << " ms";
    qInfo() << "ProcessFailedBlink period:" << Alise::AliseConstants::ProcessBlink(Alise::RED) << " ms";
    qInfo() << "FailureBlink period:" << Alise::AliseConstants::FailureBlink() << " ms";
    qInfo() << "Power check period:" << Alise::AliseConstants::PowerCheckPeriod() << " ms";
    qInfo() << "Reset check period:" << Alise::AliseConstants::ResetCheckPeriod() << " ms";
    qInfo() << "Health query period:" << Alise::AliseConstants::HealthQueryPeriod() << " ms";
    qInfo() << "Reply timeout period:" << Alise::AliseConstants::ReplyTimeoutPeriod() << " ms";
    qInfo() << "Module serial: " << serialNum;
    qInfo() << "Board serial: " << serialNumB;
    qInfo() << "Board hardware: " << hwVersion;
    qInfo() << "MCU software version: " << swVersion;
}

void AliseSettings::writeSetting()
{
    Q_ASSERT(m_settings != nullptr);
    serialNum = Alise::AliseConstants::s_moduleInfo.ModuleSerialNumber;
    serialNumB = Alise::AliseConstants::s_moduleInfo.SerialNumber;
    hwVersion = Alise::AliseConstants::s_moduleInfo.HWVersion;
    m_settings->setValue("Module/SerialNumber", serialNum);
    m_settings->setValue("Module/BoardSerialNumber", serialNumB);
    m_settings->setValue("Module/HardwareVersion", hwVersion);
    m_settings->setValue("Module/SoftwareVersion", swVersion);
}

bool AliseSettings::isModuleInfoFilled()
{
    return ((serialNum != 0xFFFFFFFF) && (serialNumB != 0xFFFFFFFF) && (hwVersion != 0xFFFFFFFF)
        && (swVersion != 0xFFFFFFFF));
}

QString AliseSettings::versionStr(const std::uint32_t &version)
{
    std::uint8_t mv = version >> 24;
    std::uint8_t lv = (version & 0x00FF0000) >> 16;
    std::uint16_t sv = version & 0x0000FFFF;
    return QString::number(mv) + "." + QString::number(lv) + "-" + QString::number(sv);
}
