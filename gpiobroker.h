#ifndef GPIOBROKER_H
#define GPIOBROKER_H
#include "../gen/modules.h"
#include "protos.pb.h"

#include <QObject>
#include <QTimer>
#include <gpiod.hpp>
class GpioBroker : public QObject
{
    enum class BlinkMode
    {
        small,
        big
    };
    enum BlinkTimeout
    {
        small = 125,
        big = 500
    };

public:
    struct GpioPin
    {
        int chip;
        int offset;
    };
    GpioBroker(QObject *parent = nullptr);
    void checkPowerUnit();
    void setIndication(alise::Health_Code code);

    void setTime(timespec time);
    void getTime();
    void rebootMyself();

private:
    bool noBooter = true;
    bool blinkStatus = true;
    int shortBlink = 0;
    int resetCounter = 0;
    BlinkMode currentMode = BlinkMode::big;

    QTimer m_timer;
    QTimer m_gpioTimer;
    QTimer m_resetTimer;
    void reset();
    void blinker(int code);
    void criticalBlinking();

    ::gpiod::chip chip0, chip1, chip2, chip3;
};

#endif // GPIOBROKER_H
