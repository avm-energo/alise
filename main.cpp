#include "alisesettings.h"
#include "commandlineparser.h"
#include "engine.h"
#include "gitversion/gitversion.h"

#include <QCoreApplication>
#include <avm-gen/logger.h>
#include <avm-gen/messagehandler.h>
#include <avm-gen/stdfunc.h>
#include <config.h>
#include <iostream>

constexpr char ethPathString[] = "/etc/network/interfaces.d/eth";
constexpr char ethResourcePathString[] = ":/network/eth";

int main(int argc, char *argv[])
{
    Engine *engine = new Engine;
    bool ok;
    AliseSettings settings;
    CommandLineParser parser;

    qRegisterMetaType<uint32_t>("uint32_t");

    GitVersion gitVersion;
    QCoreApplication a(argc, argv);
    a.setApplicationVersion(QString(ALISEVERSION) + "-" + gitVersion.getGitHash());
    StdFunc::Init();

    settings.init();
    settings.readSettings();
    settings.writeSettings(); // to set those settings which was not set (settings.ini is absent or is old enough)

    if (!parser.parseCommandLine(settings))
    {
        delete engine;
        return 0;
    }

    Logger logger;
    logger.writeStart(settings.logFilename);
    logger.setLogLevel(settings.m_logLevel);
    MessageHandler::setMessageHandlerFilename(settings.logFilename);
    MessageHandler::setLogLevel(Logger::s_logLevelsMap[settings.m_logLevel]);
    qInstallMessageHandler(MessageHandler::messageHandler);
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

    if (!engine->init(settings))
    {
        qCritical() << "Can't create engine, exiting";
        delete engine;
        return 11;
    }

    std::cout << "Enter the event loop" << std::endl;
    return a.exec();
}
