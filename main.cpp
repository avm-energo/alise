#include "alisesettings.h"
#include "commandlineparser.h"
#include "engine.h"
#include "gitversion/gitversion.h"
#include "logger.h"

#include <QCoreApplication>
#include <config.h>
#include <gen/stdfunc.h>
#include <iostream>
#include <memory>

constexpr char ethPathString[] = "/etc/network/interfaces.d/eth";
constexpr char ethResourcePathString[] = ":/network/eth";

int main(int argc, char *argv[])
{
    Engine *engine = new Engine;
    bool ok;
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

    if (!engine->init(settings.httpPort))
    {
        qCritical() << "Can't create engine, exiting";
        return 11;
    }

    engine->start();

    std::cout << "Enter the event loop" << std::endl;
    return a.exec();
}
