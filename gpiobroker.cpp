#include "gpiobroker.h"

#include "../gen/datamanager.h"
#include "../gen/error.h"
#include "gpiohelper.h"

#include <QDebug>
#include <QRandomGenerator>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct GpioPin
{
    int chip;
    int offset;
};

constexpr GpioPin PowerStatusPin0 { 0, 5 };
constexpr GpioPin PowerStatusPin1 { 3, 17 };
constexpr GpioPin LedPin { 1, 31 };
constexpr GpioPin ResetPin { 2, 6 };
enum GpioPowerStatus
{
    Pin1 = 5,
    Pin2 = 113
};

constexpr int GpioLedPin = 63;
constexpr int GpioResetPin = 70;
constexpr char prefix[] = "/sys/class/gpio/gpio";
enum LedTimeout
{
    Small = 125,
    Big = 500
};

GpioBroker::GpioBroker(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(1000);
    m_resetTimer.setInterval(100);
    m_gpioTimer.setInterval(50);
    QObject::connect(&m_timer, &QTimer::timeout, this, &GpioBroker::checkPowerUnit);
    QObject::connect(&m_gpioTimer, &QTimer::timeout, this, &GpioBroker::criticalBlinking);
    QObject::connect(&m_resetTimer, &QTimer::timeout, this, &GpioBroker::reset);
    m_timer.start();
    m_gpioTimer.start();
    m_resetTimer.start();
#ifdef TEST_INDICATOR
    QTimer *testTimer = new QTimer(this);
    testTimer->setInterval(70009);
    QObject::connect(testTimer, &QTimer::timeout, this, [this] {
        auto random = static_cast<alise::Health_Code>(QRandomGenerator::global()->bounded(0, 8));
        qDebug() << "Random:" << random;
        setIndication(random);
    });
    testTimer->start();
#endif
}
namespace detail
{
bool isdigit(char ch)
{
    return std::isdigit(static_cast<unsigned char>(ch));
}

}

void GpioBroker::checkPowerUnit()
{
    auto status1 = gpio_read(PowerStatusPin0.chip, PowerStatusPin0.offset);
    auto status2 = gpio_read(PowerStatusPin1.chip, PowerStatusPin1.offset);

    DataTypes::BlockStruct blk;
    blk.data.resize(sizeof(AVTUK_CCU::Main));
    AVTUK_CCU::Main str;

    // TODO Проверить то ли сдвигаем
    str.PWRIN = status1 ^ (status2 << 1);
    str.resetReq = false;
    std::memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
    blk.ID = AVTUK_CCU::MainBlock;
    DataManager::addSignalToOutList(DataTypes::SignalTypes::Block, blk);
}

void get_chip_info(void)
{
    // struct gpiochip_info info;
    //  int fd, rv;
    //  fd = open("/dev/gpiochip0", O_RDWR);
    // rv = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, info);
    // gpio_list("/dev/gpiochip0");
}

void GpioBroker::setIndication(alise::Health_Code code)
{
    m_gpioTimer.stop();
    shortBlink = blinkStatus = 0;
    QObject::disconnect(&m_gpioTimer, &QTimer::timeout, nullptr, nullptr);

    gpio_write(LedPin.chip, LedPin.offset, blinkStatus /*+ '0'*/);
    // setGpioValue(GpioLedPin, blinkStatus + '0');
    if (noBooter)
    {
        m_gpioTimer.setInterval(LedTimeout::Small);
        noBooter = !noBooter;
    }

    switch (code)
    {
    case alise::Health_Code_Startup:
    {
        m_gpioTimer.setInterval(LedTimeout::Small);
        QObject::connect(&m_gpioTimer, &QTimer::timeout, this, [&status = blinkStatus] {
            gpio_write(LedPin.chip, LedPin.offset, status /*+ '0'*/);
            //     setGpioValue(GpioLedPin, status + '0');
            status = !status;
        });
        break;
    }
    case alise::Health_Code_Work:
    {
        m_gpioTimer.setInterval(LedTimeout::Big);
        QObject::connect(&m_gpioTimer, &QTimer::timeout, this, [&status = blinkStatus] {
            gpio_write(LedPin.chip, LedPin.offset, status /*+ '0'*/);
            // setGpioValue(GpioLedPin, status + '0');
            status = !status;
        });
        break;
    }
    default:
    {
        m_gpioTimer.setInterval(LedTimeout::Small);
        currentMode = BlinkMode::big;
        QObject::connect(&m_gpioTimer, &QTimer::timeout, this, [=] { blinker(code); });
    }
    }
    m_gpioTimer.start();
}

void GpioBroker::rebootMyself()
{
    m_timer.stop();
    m_gpioTimer.stop();
    m_resetTimer.stop();
    gpio_write(LedPin.chip, LedPin.offset, false /*+ '0'*/);
    // setGpioValue(GpioLedPin, false + '0');
    sync();
    reboot(RB_AUTOBOOT);
}

void GpioBroker::reset()
{
    // bool status = false;
    bool value = !gpio_read(ResetPin.chip, ResetPin.offset);
    // bool value = !gpioStatus(GpioResetPin, &status);

    if (value)
    {
        resetCounter += value;
        return;
    }

    // soft reset (only reboot)
    if ((resetCounter > 20) && (resetCounter <= 40))
    {
        resetCounter = 0;
        rebootMyself();
        return;
    }
    // hard reset
    if (resetCounter > 40)
    {
        resetCounter = 0;
        // bool status = false;
        auto status1 = gpio_read(PowerStatusPin0.chip, PowerStatusPin0.offset);
        auto status2 = gpio_read(PowerStatusPin1.chip, PowerStatusPin1.offset);
        // auto status1 = gpioStatus(GpioPowerStatus::Pin1, &status);
        // auto status2 = gpioStatus(GpioPowerStatus::Pin2, &status);
        DataTypes::BlockStruct blk;
        blk.data.resize(sizeof(AVTUK_CCU::Main));
        AVTUK_CCU::Main str;

        // TODO Проверить то ли сдвигаем
        str.PWRIN = status1 ^ (status2 << 1);
        str.resetReq = true;
        std::memcpy(blk.data.data(), &str, sizeof(AVTUK_CCU::Main));
        blk.ID = AVTUK_CCU::MainBlock;
        DataManager::addSignalToOutList(DataTypes::SignalTypes::Block, blk);
    }
}

void GpioBroker::blinker(int code)
{

    switch (currentMode)
    {
    case BlinkMode::big:
    {

        if ((shortBlink / 2) == 1 && !(shortBlink % 2))
        {
            shortBlink = 0;
            m_gpioTimer.setInterval(LedTimeout::Small);
            currentMode = BlinkMode::small;
            break;
        }

        m_gpioTimer.setInterval(LedTimeout::Big);
        break;
    }
    case BlinkMode::small:
    {

        if ((shortBlink / 2) == code && !(shortBlink % 2))
        {
            shortBlink = 0;
            m_gpioTimer.setInterval(LedTimeout::Big);
            currentMode = BlinkMode::big;
            break;
        }

        m_gpioTimer.setInterval(LedTimeout::Small);
        break;
    }
    }
    ++shortBlink;
    gpio_write(LedPin.chip, LedPin.offset, blinkStatus /*+ '0'*/);
    //   setGpioValue(GpioLedPin, blinkStatus + '0');
    blinkStatus = !blinkStatus;
}

void GpioBroker::criticalBlinking()
{
    gpio_write(LedPin.chip, LedPin.offset, blinkStatus /*+ '0'*/);
    //  setGpioValue(GpioLedPin, blinkStatus + '0');
    blinkStatus = !blinkStatus;
}

bool GpioBroker::gpioExport(uint8_t gpio)
{
    auto fd = std::fopen("/sys/class/gpio/export", "w");
    if (!fd)
    {
        auto str = std::strerror(errno);
        std::cout << Error::FileOpenError << str << '\n';
        qCritical() << Error::FileOpenError << str;
        return false;
    }
    {
        auto rc = std::fputs(std::to_string(gpio).c_str(), fd);
        if (rc == EOF)
        {
            auto str = std::strerror(errno);
            std::cout << Error::FileWriteError << str << '\n';
            qCritical() << Error::FileWriteError << str;
            return false;
        }
    }
    std::fclose(fd);
    return true;
}

bool GpioBroker::gpioOpen(GpioBroker::Mode mode, const std::string &path)
{
    auto fd = std::fopen(path.c_str(), "w");
    if (!fd)
    {
        auto str = std::strerror(errno);
        std::cout << Error::FileOpenError << str << '\n';
        qCritical() << Error::FileOpenError << str;
        return false;
    }

    int rc = 0;
    switch (mode)
    {
    case Mode::in:
        rc = std::fputs("in", fd);
        break;
    case Mode::out:
        rc = std::fputs("out", fd);
        break;
    default:
        return false;
    }

    std::fclose(fd);
    if (rc == EOF)
    {
        auto str = std::strerror(errno);
        std::cout << Error::FileWriteError << str << '\n';
        qCritical() << Error::FileWriteError << str;
        return false;
    }
    return true;
}

uint8_t GpioBroker::gpioStatus(uint8_t gpio, bool *ok)
{
    if (ok)
        *ok = false;
    if (!gpioExport(gpio))
        return 0;

    std::string dir_path = prefix + std::to_string(gpio) + "/direction";

    if (!gpioOpen(Mode::in, dir_path))
        return 0;

    std::string val_path = prefix + std::to_string(gpio) + "/value";
    if (std::ifstream is { val_path, std::ios::binary })
    {
        char status = 0;
        is.get(status);
        //   qDebug() << status;
        is.close();
        if (!detail::isdigit(status))
            return 0;
        auto unsignedStatus = static_cast<unsigned char>(status);
        if (ok)
            *ok = true;
        return +(unsignedStatus - '0');
    }
    return 0;
}

bool GpioBroker::setGpioValue(uint8_t gpio, uint8_t value)
{

    if (!gpioExport(gpio))
        return false;

    std::string dir_path = prefix + std::to_string(gpio) + "/direction";

    if (!gpioOpen(Mode::out, dir_path))
        return false;
    std::string val_path = prefix + std::to_string(gpio) + "/value";
    if (std::ofstream os { val_path })
    {
        os.put(value);
        os.close();
        return true;
    }
    return false;
}
