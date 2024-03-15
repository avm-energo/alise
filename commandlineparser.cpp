#include "commandlineparser.h"

#include "aliseconstants.h"

#include <QCommandLineParser>
#include <QEventLoop>
#include <QTimer>
#include <gpiod.hpp>
#include <iostream>

CommandLineParser::CommandLineParser(QObject *parent) : QObject(parent)
{
}

bool CommandLineParser::parseCommandLine(AliseSettings &settings)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("Avtuk LInux SErver");
#ifdef AVTUK_NO_STM
    QCommandLineOption showGpio("g", "List all gpios");
    parser.addOption(showGpio);
#endif
    QCommandLineOption serialNumber({ "m", "serial" }, "Sets module serial number", "serial");
    QCommandLineOption serialNumberB({ "b", "serialb" }, "Sets board serial number", "serialb");
    QCommandLineOption hardware({ "w", "hardware" }, "Sets module hardware version", "hardware");
    QCommandLineOption software({ "s", "software" }, "Sets module software version", "software");
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
#ifdef AVTUK_NO_STM
        bool showPins = parser.isSet(showGpio);
        if (showPins)
        {
            listPins();
        }
#endif
#ifdef AVTUK_STM
        m_broker = new StmBroker;
        if (!m_broker->connect(settings))
        {
            std::cout << "Error connecting to broker";
            return false;
        }
        if (!m_broker->BSIReady())
            waitForBSIOrTimeout();
        if (parser.isSet(listversion))
        {
            std::cout << "Module serial number: " << settings.serialNum << "\n";
            std::cout << "Board serial number: " << settings.serialNumB << "\n";
            std::cout << "Hardware version: " << settings.hwVersion << "\n";
            std::cout << "Software version: " << settings.swVersion << "\n";
            return false;
        }
#endif
        if (parser.isSet(serialNumber))
            setSerialNumber(parser.value("serial"));
        if (parser.isSet(serialNumberB))
            setSerialNumberB(parser.value("serialb"));
        if (parser.isSet(hardware))
            setHWVersion(parser.value("hardware"));
        if (parser.isSet(software))
            settings.swVersion = parser.value("software").toUInt();
        settings.writeSetting();
        return false;
    }
    return true;
}

#ifdef AVTUK_NO_STM

void CommandLineParser::setSerialNumber(const QString &serialNum)
{
    Alise::AliseConstants::s_moduleInfo.ModuleSerialNumber = serialNum.toUInt();
}

void CommandLineParser::setSerialNumberB(const QString &serialNum)
{
    Alise::AliseConstants::s_moduleInfo.SerialNumber = serialNum.toUInt();
}

void CommandLineParser::setHWVersion(const QString &hwversion)
{
    Alise::AliseConstants::s_moduleInfo.HWVersion = hwversion.toUInt();
}

#endif

#ifdef AVTUK_STM

void CommandLineParser::setSerialNumber(const QString &serialNum)
{
    waitForBSIOrTimeout();
    Alise::AliseConstants::s_moduleInfo.ModuleSerialNumber = serialNum.toUInt();
    std::cout << "Set module serial number: " << serialNum.toStdString();
    writeHiddenBlock();
}

void CommandLineParser::setSerialNumberB(const QString &serialNum)
{
    waitForBSIOrTimeout();
    Alise::AliseConstants::s_moduleInfo.SerialNumber = serialNum.toUInt();
    std::cout << "Set board serial number: " << serialNum.toStdString();
    writeHiddenBlock();
}

void CommandLineParser::setHWVersion(const QString &hwversion)
{
    waitForBSIOrTimeout();
    Alise::AliseConstants::s_moduleInfo.HWVersion = hwversion.toUInt();
    std::cout << "Set HW version: " << hwversion.toStdString();
    writeHiddenBlock();
}

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

#endif

#ifdef AVTUK_NO_STM
void CommandLineParser::listPins()
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
