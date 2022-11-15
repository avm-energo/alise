
#include "../comaversion/comaversion.h"
#include "aliseconstants.h"
#include "controller.h"

#include <QCoreApplication>
#include <config.h>
#include <gen/logger.h>
#include <iostream>
#include <memory>

#ifdef AVTUK_NO_STM
void listPins();
#endif

int main(int argc, char *argv[])
{
    std::cout << "Started " << std::endl;

    GitVersion gitVersion;
    QCoreApplication a(argc, argv);
    a.setApplicationVersion(QString(COMAVERSION) + "-" + gitVersion.getGitHash());

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

    QSettings settings("/root/sonica/alise/settings/settings.ini", QSettings::IniFormat);
    int logcounter = settings.value("Test/counter", "1").toInt();
    settings.setValue("Test/counter", ++logcounter);
    QString logFileName = settings.value("Logs/logfile", "/root/sonica/alise/logs/alise.log").toString();
    QString logLevel = settings.value("Logs/Loglevel", "Info").toString();
    int portCore = settings.value("Main/CorePort", "5555").toInt();
    int portBooter = settings.value("Main/BooterPort", "5556").toInt();
    AliseConstants::setFailureBlinkPeriod(settings.value("Timers/FailureBlink", "50").toInt());
    AliseConstants::setSonicaStartingBlinkPeriod(settings.value("Timers/StartingBlink", "250").toInt());
    AliseConstants::setSonicaNormalBlinkPeriod(settings.value("Timers/NormalBlink", "500").toInt());
    AliseConstants::setPowerCheckPeriod(settings.value("Timers/PowerCheckPeriod", "1000").toInt());
    AliseConstants::setResetCheckPeriod(settings.value("Timers/ResetCheckPeriod", "1000").toInt());
    AliseConstants::setHealthQueryPeriod(settings.value("Timers/HealthQueryPeriod", "4000").toInt());
    AliseConstants::setGpioBlinkPeriod(settings.value("Timers/GpioBlinkPeriod", "50").toInt());
    Logger::writeStart(logFileName);
    Logger::setLogLevel(logLevel);
    qInstallMessageHandler(Logger::messageHandler);
    qInfo() << "Reading settings from: " << settings.fileName();
    auto devBroker = std::unique_ptr<deviceType>(new deviceType);
    Controller booterController(devBroker.get()), coreController(devBroker.get());
    if (!booterController.launch(portBooter))
        return 13;
    if (!coreController.launch(portCore))
        return 13;
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
