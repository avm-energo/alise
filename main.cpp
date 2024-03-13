#include "alisesettings.h"
#include "commandlineparser.h"
#include "controllerfabric.h"
#include "gitversion/gitversion.h"
#include "logger.h"
#include "maincreator.h"

#include <QCoreApplication>
#include <config.h>
#include <iostream>
#include <memory>

constexpr char ethPathString[] = "/etc/network/interfaces.d/eth";
constexpr char ethResourcePathString[] = ":/network/eth";

int main(int argc, char *argv[])
{
    MainCreator creator;
    bool ok;
    Broker *broker;
    TimeSyncronizer *tm;
    ControllerFabric fabric;
    AliseSettings settings;
    CommandLineParser parser;

    std::cout << "Started " << std::endl;

    qRegisterMetaType<uint32_t>("uint32_t");

    GitVersion gitVersion;
    QCoreApplication a(argc, argv);
    a.setApplicationVersion(QString(ALISEVERSION) + "-" + gitVersion.getGitHash());
    StdFunc::Init();

    settings.init();
    settings.readSettings();

    if (!parser.parseCommandLine(settings))
        return 0;

    Logger::writeStart(settings.logFilename);
    Logger::setLogLevel(settings.logLevel);
    qInstallMessageHandler(Logger::messageHandler);
    settings.logSettings();

    for (const QString ethLetter : { "0", "1", "2" })
    {
        QString ethPath = ethPathString + ethLetter;
        QString ethResourcePath = ethResourcePathString + ethLetter;
        if (!QFile::exists(ethResourcePath))
            qInfo() << "Recovery eth" << ethLetter << ": not found";
        else
            qInfo() << "Recovery eth" << ethLetter << ": found";
    }

    qInfo() << "=========================\n";

    creator.init();
    broker = creator.create(ok);
    if (!ok)
    {
        qCritical() << "Can't create broker, exiting";
        return 11;
    }
    tm = creator.getTimeSynchronizer();

    if (!fabric.createController(Controller::ContrTypes::IS_BOOTER, settings.portBooter, broker, tm))
    {
        qCritical() << "Booter controller was not created, exiting";
        return 12;
    }
    if (!fabric.createController(Controller::ContrTypes::IS_CORE, settings.portCore, broker, tm))
    {
        qCritical() << "Core controller was not created, exiting";
        return 13;
    }
    if (!fabric.createController(Controller::ContrTypes::IS_ADMINJA, settings.portAdminja, broker, tm))
    {
        qCritical() << "Core controller was not created, exiting";
        return 14;
    }

    std::cout << "Enter the event loop" << std::endl;
    return a.exec();
}
