
#include "../comaversion/comaversion.h"
#include "../gen/logger.h"
#include "../gen/stdfunc.h"
#include "controller.h"

#ifdef AVTUK_NO_STM
#include "gpiohelper.h"
#endif

#include <QCoreApplication>
#include <config.h>

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
#endif
    parser.addOption(showGpio);
    parser.addHelpOption();
    parser.addVersionOption();
    if (QCoreApplication::arguments().size() > 1)
    {
        parser.process(QCoreApplication::arguments());
#ifdef AVTUK_NO_STM
        bool showPins = parser.isSet(showGpio);
        if (showPins)
        {
            gpio_list("/dev/gpiochip0");
            gpio_list("/dev/gpiochip1");
            gpio_list("/dev/gpiochip2");
            gpio_list("/dev/gpiochip3");
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
