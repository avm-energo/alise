#ifndef GPIOBROKER_H
#define GPIOBROKER_H

#include "protos.pb.h"

#include <QObject>
#include <QTimer>
#include <gpiod.hpp>

class GpioBroker : public QObject
{
    enum BlinkTimeout
    {
        verysmall = 50,
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
    bool blinkStatus = true;
    int resetCounter = 0;

    QTimer m_timer;
    QTimer m_gpioTimer;
    QTimer m_resetTimer, m_healthQueryTimeoutTimer;
    int m_currentBlinkingPeriod;
    void reset();
    void criticalBlinking();

    ::gpiod::chip chip0, chip1, chip2, chip3;

private slots:
    void blink();
};

#endif // GPIOBROKER_H
