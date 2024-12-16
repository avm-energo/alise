#ifndef GPIOBROKER_H
#define GPIOBROKER_H

#include "alisesettings.h"
#include "broker.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <gpiod.hpp>

constexpr uint16_t c_maxBlinks = 0xFFFF;

class GpioBroker : public Broker
{
public:
    enum PinDirections
    {
        INPUT,
        OUTPUT
    };

    enum PinOutputs
    {
        OFF = 0,
        ON = 1
    };

    struct GpioPin
    {
        std::string name;        // line name a-la "ModeLed"
        int chip;                // gpiochip number
        int offset;              // gpiochip pin number
        PinDirections direction; // 0 for input, 1 for output
    };

    enum BlinkMode
    {
        ONEBLINK,
        TWOBLINKS
    };

    GpioBroker(AliseSettings settings, QObject *parent = nullptr);
    ~GpioBroker();
    bool connect() override;

public slots:
    void checkIndication() override { };
    void checkPowerUnit() override;
    void setIndication(const AVTUK_CCU::Indication &indication) override;
    void setTime(const timespec &time) override { };
    void getTime() override { };
    void rebootMyself() override;

private:
    bool m_blinkStatus = true;
    int m_blinkCount, m_blinkFreq;
    BlinkMode m_blinkMode;
    int resetCounter = 0;
    QMutex _mutex;
    AliseSettings m_settings;

    GpioPin PowerStatusPin0 { "PWR1", 3, 5, PinDirections::INPUT };
    GpioPin PowerStatusPin1 { "PWR2", 2, 17, PinDirections::INPUT };
    GpioPin LedPin { "MODELED", 0, 31, PinDirections::OUTPUT };
    GpioPin ResetPin { "RESET", 1, 6, PinDirections::INPUT };
    const QList<GpioPin> c_pinList = { LedPin, ResetPin, PowerStatusPin0, PowerStatusPin1 };

    QTimer m_gpioTimer, m_resetTimer;
    void reset();
    void restartBlinkTimer();
    bool gpioGetLineValue(GpioPin pin);
    void gpioSetLineValue(GpioPin pin, bool value);

private slots:
    void blink();
};

#endif // GPIOBROKER_H
