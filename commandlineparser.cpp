#include "commandlineparser.h"

#include <QCommandLineParser>
#include <QEventLoop>
#include <QTimer>
#include <gen/stdfunc.h>
#include <gpiod.hpp>
#include <iostream>

CommandLineParser::CommandLineParser(QObject *parent) : QObject(parent)
{
}

bool CommandLineParser::parseCommandLine(AliseSettings &settings)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("Avtuk LInux SErver");

#ifndef AVTUK_STM
    QCommandLineOption showGpio("g", "List all gpios");
    parser.addOption(showGpio);
#endif

    QCommandLineOption serialNumber({ "m", "serial" }, "Sets module serial number", "serial");
    QCommandLineOption serialNumberB({ "b", "serialb" }, "Sets board serial number", "serialb");
    QCommandLineOption hardware({ "w", "hardware" }, "Sets module hardware version (format: mv.lv-sv)", "hardware");
    QCommandLineOption software({ "s", "software" }, "Sets module software version (format: mv.lv-sv)", "software");
    QCommandLineOption listversion({ "l", "list" }, "Lists module versions and numbers");
    parser.addOption(serialNumber);
    parser.addOption(serialNumberB);
    parser.addOption(hardware);
    parser.addOption(software);
    parser.addOption(listversion);
    parser.addHelpOption();
    parser.addVersionOption();
    if (QCoreApplication::arguments().size() > 1)
    {
        parser.process(QCoreApplication::arguments());

#ifdef AVTUK_STM
        m_broker = new StmBroker;
        if (!m_broker->connect(settings))
        {
            std::cout << "Error connecting to broker";
            return false;
        }
        if (!m_broker->BSIReady())
            waitForBSIOrTimeout();
#else
        bool showPins = parser.isSet(showGpio);
        if (showPins)
        {
            listPins();
        }
#endif
        if (parser.isSet(listversion))
        {
            std::cout << "Module serial number: " << settings.serialNum << "\n";
            std::cout << "Board serial number: " << settings.serialNumB << "\n";
            std::cout << "Hardware version: " << StdFunc::VerToStr(settings.hwVersion).toStdString() << "\n";
            std::cout << "Software version: " << StdFunc::VerToStr(settings.swVersion).toStdString() << "\n";
            return false;
        }
        if (parser.isSet(serialNumber))
            settings.serialNum = parser.value("serial").toUInt();
        if (parser.isSet(serialNumberB))
            settings.serialNumB = parser.value("serialb").toUInt();
        if (parser.isSet(hardware))
            settings.hwVersion = StdFunc::StrToVer(parser.value("hardware"));
        if (parser.isSet(software))
            settings.swVersion = StdFunc::StrToVer(parser.value("software"));
        settings.writeSettings();
#ifdef AVTUK_STM
        writeHiddenBlock();
#endif
        return false;
    }
    return true;
}

#ifdef AVTUK_STM

void CommandLineParser::waitForBSIOrTimeout()
{
    QTimer *timer = new QTimer;
    timer->setInterval(5000); // 5 secs to wait for BSI reception
    QEventLoop *loop = new QEventLoop;
    connect(timer, &QTimer::timeout, loop, &QEventLoop::quit);
    connect(m_broker, &StmBroker::ModuleInfoFilled, loop, &QEventLoop::quit);
    timer->start();
    loop->exec(QEventLoop::AllEvents);
}

void CommandLineParser::writeHiddenBlock()
{
    QTimer *timer = new QTimer;
    timer->setInterval(1000); // 1 secs to wait for result
    QEventLoop *loop = new QEventLoop;
    connect(timer, &QTimer::timeout, loop, &QEventLoop::quit);
    connect(m_broker, &StmBroker::operationCompleted, loop, &QEventLoop::quit);
    timer->start();
    m_broker->writeHiddenBlock();
    loop->exec(QEventLoop::AllEvents);
}

#else

void CommandLineParser::listPins()
{

    try
    {
        for (const auto &entry : ::std::filesystem::directory_iterator("/dev/"))
        {
            if (::gpiod::is_gpiochip_device(entry.path()))
            {
                ::gpiod::chip chip(entry.path());
                ::gpiod::chip_info info = chip.get_info();
                for (int i = 0; i < info.num_lines(); ++i)
                {
                    ::gpiod::line_info line = chip.get_line_info(i);
                    std::cout << "\tline ";
                    std::cout.width(3);
                    std::cout << line.offset() << ": ";

                    std::cout.width(12);
                    std::cout << line.name();
                    std::cout << " ";

                    std::cout.width(12);
                    std::cout << line.consumer();
                    std::cout << " ";

                    std::cout.width(8);
                    std::cout << (line.direction() == ::gpiod::line::direction::INPUT ? "input" : "output");
                    std::cout << " ";

                    std::cout.width(10);
                    std::cout << (line.active_low() ? "active-low" : "active-high");

                    std::cout << ::std::endl;
                }
            }
        }
    } catch (std::exception e)
    {
        std::cout << "Exception was raised: " << e.what() << std::endl;
    }
}
#endif
