#pragma once

#include "alisesettings.h"
#include "broker.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QTimer>

class GpioBroker : public Broker
{
public:
    static constexpr uint16_t c_maxBlinks = 0xFFFF;
    static constexpr auto mode = "ModeLed";
    static constexpr auto pwr1 = "Power1";
    static constexpr auto pwr2 = "Power2";
    static constexpr auto rst = "Reset";

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
        int chip;                // gpiochip number
        int offset;              // gpiochip pin number
        PinDirections direction; // 0 for input, 1 for output
    };

    enum BlinkMode
    {
        ONEBLINK,
        TWOBLINKS
    };

    GpioBroker(QMap<QString, AliseSettings::GPIOInfo> &gpioMap, QObject *parent = nullptr);
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
    QMap<QString, GpioPin> m_pinMap;
    int resetCounter = 0;
    QMutex _mutex;
    AliseSettings m_settings;
    QTimer m_gpioTimer, m_resetTimer;

    void reset();
    void restartBlinkTimer();
    bool gpioGetLineValue(GpioPin pin);
    void gpioSetLineValue(GpioPin pin, bool value);
    QString getChipName(int chipNum) const;

private slots:
    void blink();
};
