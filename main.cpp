
#include "aliseconstants.h"
#include "controllerfabric.h"
#include "gitversion/gitversion.h"

#include <QCoreApplication>
#include <config.h>
#include <gen/logger.h>
#include <iostream>
#include <memory>

#ifdef AVTUK_NO_STM
void listPins();
#endif

constexpr char ethPathString[] = "/etc/network/interfaces.d/eth";
constexpr char ethResourcePathString[] = ":/network/eth";

using namespace Alise;

int main(int argc, char *argv[])
{
    std::cout << "Started " << std::endl;

    qRegisterMetaType<uint32_t>("uint32_t");

    GitVersion gitVersion;
    QCoreApplication a(argc, argv);
    a.setApplicationVersion(QString(ALISEVERSION) + "-" + gitVersion.getGitHash());
    StdFunc::Init();

    QCommandLineParser parser;
    parser.setApplicationDescription("Avtuk LInux SErver");
#ifdef AVTUK_NO_STM
    QCommandLineOption showGpio("g", "List all gpios");
    parser.addOption(showGpio);
#endif
    parser.addHelpOption();
    parser.addVersionOption();
    if (QCoreApplication::arguments().size() > 1)
    {
        parser.process(QCoreApplication::arguments());
#ifdef AVTUK_NO_STM
        bool showPins = parser.isSet(showGpio);
        if (showPins)
        {
            listPins();
        }
#endif
        return 0;
    }

#ifdef ALISE_LOCALDEBUG
    QSettings settings("~/sonica/alise/settings/settings.ini", QSettings::IniFormat);
    QString logFileName = settings.value("Logs/logfile", "~/sonica/alise/logs/alise.log").toString();
#else
    QSettings settings("/avtuk/settings/alise/settings/settings.ini", QSettings::IniFormat);
    QString logFileName = settings.value("Logs/logfile", "/avtuk/settings/alise/logs/alise.log").toString();
#endif
    int logcounter = settings.value("Test/counter", "1").toInt();
    settings.setValue("Test/counter", ++logcounter);
    QString logLevel = settings.value("Logs/Loglevel", "Info").toString();
    int portCore = settings.value("Main/CorePort", "5555").toInt();
    int portBooter = settings.value("Main/BooterPort", "5556").toInt();
    int portAdminja = settings.value("Main/AdminjaPort", "5557").toInt();
    AliseConstants::setFailureBlinkFreq(settings.value("Timers/FailureBlink", "50").toInt());
    AliseConstants::setProcessStartingBlinkFreq(settings.value("Timers/StartingBlink", "250").toInt());
    AliseConstants::setProcessSemiWorkingBlinkFreq(settings.value("Timers/SemiWorkingBlink", "1000").toInt());
    AliseConstants::setProcessNormalBlinkFreq(settings.value("Timers/NormalBlink", "500").toInt());
    AliseConstants::setProcessStoppedBlinkFreq(settings.value("Timers/StoppedBlink", "2000").toInt());
    AliseConstants::setProcessFailedBlinkFreq(settings.value("Timers/FailedBlink", "125").toInt());
    AliseConstants::setPowerCheckPeriod(settings.value("Timers/PowerCheckPeriod", "1000").toInt());
    AliseConstants::setResetCheckPeriod(settings.value("Timers/ResetCheckPeriod", "1000").toInt());
    AliseConstants::setHealthQueryPeriod(settings.value("Timers/HealthQueryPeriod", "1500").toInt());
    AliseConstants::setReplyTimeoutPeriod(settings.value("Timers/ReplyTimeoutPeriod", "4000").toInt());
    AliseConstants::setSecondsToHardReset(settings.value("Reset/TimeToWaitForHardReset", "4").toInt());
    Logger::writeStart(logFileName);
    Logger::setLogLevel(logLevel);
    qInstallMessageHandler(Logger::messageHandler);

    qInfo() << "Reading settings from: " << settings.fileName();
    qInfo() << "Startup information:";
    qInfo() << "=========================";
    qInfo() << "LogLevel: " << Logger::logLevel();
    qInfo() << "CorePort: " << portCore;
    qInfo() << "BooterPort: " << portBooter;
    qInfo() << "AdminjaPort: " << portAdminja;
    qInfo() << "NormalBlink period:" << AliseConstants::ProcessBlink(Alise::NORMAL) << " ms";
    qInfo() << "StartingBlink period:" << AliseConstants::ProcessBlink(Alise::YELLOW) << " ms";
    qInfo() << "StoppedBlink period:" << AliseConstants::ProcessBlink(Alise::ORANGE) << " ms";
    qInfo() << "SemiWorkingBlink period:" << AliseConstants::ProcessBlink(Alise::VIOLET) << " ms";
    qInfo() << "ProcessFailedBlink period:" << AliseConstants::ProcessBlink(Alise::RED) << " ms";
    qInfo() << "FailureBlink period:" << AliseConstants::FailureBlink() << " ms";
    qInfo() << "Power check period:" << AliseConstants::PowerCheckPeriod() << " ms";
    qInfo() << "Reset check period:" << AliseConstants::ResetCheckPeriod() << " ms";
    qInfo() << "Health query period:" << AliseConstants::HealthQueryPeriod() << " ms";
    qInfo() << "Reply timeout period:" << AliseConstants::ReplyTimeoutPeriod() << " ms";

    for (const QString ethLetter : { "0", "1", "2" })
    {
        QString ethPath = ethPathString + ethLetter;
        QString ethResourcePath = ethResourcePathString + ethLetter;
        if (!QFile::exists(ethResourcePath))
            qInfo() << "Recovery eth" << ethLetter << ": not found";
        else
            qInfo() << "Recovery eth" << ethLetter << ": found";
    }

    qInfo() << "=========================";
    ControllerFabric fabric;
    if (!fabric.getStatus())
    {
        qCritical() << "Fabric was not created, exiting";
        return 11;
    }
    if (!fabric.createController(Controller::ContrTypes::IS_BOOTER, portBooter))
    {
        qCritical() << "Booter controller was not created, exiting";
        return 12;
    }
    if (!fabric.createController(Controller::ContrTypes::IS_CORE, portCore))
    {
        qCritical() << "Core controller was not created, exiting";
        return 13;
    }
    if (!fabric.createController(Controller::ContrTypes::IS_ADMINJA, portAdminja))
    {
        qCritical() << "Core controller was not created, exiting";
        return 14;
    }

    std::cout << "Enter the event loop" << std::endl;
    return a.exec();
}

#ifdef AVTUK_NO_STM
void listPins()
{
    for (auto &cit : ::gpiod::make_chip_iter())
    {
        std::cout << cit.name() << " - " << cit.num_lines() << " lines:" << ::std::endl;

        for (auto &lit : ::gpiod::line_iter(cit))
        {
            std::cout << "\tline ";
            std::cout.width(3);
            std::cout << lit.offset() << ": ";

            std::cout.width(12);
            std::cout << (lit.name().empty() ? "unnamed" : lit.name());
            std::cout << " ";

            std::cout.width(12);
            std::cout << (lit.consumer().empty() ? "unused" : lit.consumer());
            std::cout << " ";

            std::cout.width(8);
            std::cout << (lit.direction() == ::gpiod::line::DIRECTION_INPUT ? "input" : "output");
            std::cout << " ";

            std::cout.width(10);
            std::cout << (lit.active_state() == ::gpiod::line::ACTIVE_LOW ? "active-low" : "active-high");

            std::cout << ::std::endl;
        }
    }
}
#endif
