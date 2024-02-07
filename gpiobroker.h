#ifndef GPIOBROKER_H
#define GPIOBROKER_H

#include "broker.h"
#include "protos/protos.pb.h"

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <gpiod.hpp>

constexpr uint16_t c_maxBlinks = 0xFFFF;

class GpioBroker : public Broker
{
public:
    struct GpioPin
    {
        int chip;
        int offset;
    };

    enum BlinkMode
    {
        ONEBLINK,
        TWOBLINKS
    };

    GpioBroker(QObject *parent = nullptr);
    bool connect() override;

public slots:
    void currentIndicationReceived(const QVariant &) override {};
    void checkIndication() override {};
    void checkPowerUnit() override;
    void setIndication(const AVTUK_CCU::Indication &indication) override;
    void setTime(timespec time) override;
    void getTime() override;
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

    ::gpiod::chip chip0, chip1, chip2, chip3;

private slots:
    void blink();
};

#endif // GPIOBROKER_H
