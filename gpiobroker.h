#ifndef GPIOBROKER_H
#define GPIOBROKER_H

#include "broker.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <gpiod.h>

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

    GpioBroker(QObject *parent = nullptr);
    ~GpioBroker();
    bool connect() override;

public slots:
    void checkIndication() override {};
    void checkPowerUnit() override;
    void setIndication(const AVTUK_CCU::Indication &indication) override;
    void setTime(const timespec &time) override {};
    void getTime() override {};
    void rebootMyself() override;

private:
    bool m_blinkStatus = true;
    int m_blinkCount, m_blinkFreq;
    BlinkMode m_blinkMode;
    int resetCounter = 0;
    QMutex _mutex;

    QTimer m_gpioTimer, m_resetTimer;
    void reset();
    void restartBlinkTimer();

    QMap<int, struct gpiod_chip *> chipMap;                        // pairs: <chipNum, chip> for each gpiochipx
    QMap<std::string, struct gpiod_line *> lineMap;                // pairs: <lineName, line> for each pin
    struct gpiod_line *modeLine, *resetLine, *pwr1Line, *pwr2Line; // for each pin: led, reset, pwr1, pwr2

private slots:
    void blink();
};

#endif // GPIOBROKER_H
