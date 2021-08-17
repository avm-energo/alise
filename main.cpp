
#include "../comaversion/comaversion.h"
#include "../gen/logger.h"
#include "../gen/stdfunc.h"
#include "controller.h"

#include <QCoreApplication>
#include <config.h>

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

    StdFunc::Init();
    Logging::writeStart();
    qInstallMessageHandler(Logging::messageHandler);
    Controller controller;
    if (!controller.launch())
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
