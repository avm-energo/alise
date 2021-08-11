#ifndef GPIOBROKER_H
#define GPIOBROKER_H
#include "../gen/modules.h"
#include "protos.pb.h"

#include <QObject>
#include <QTimer>
class GpioBroker : public QObject
{
    enum Mode
    {
        in,
        out
    };
    enum BlinkMode
    {
        small,
        big
    };

public:
    GpioBroker(QObject *parent = nullptr);
    void checkPowerUnit();
    void setIndication(alise::Health_Code code);

    void setTime(timespec time);
    void getTime();
    void rebootMyself();

private:
    AVTUK_CCU::Indication transform(alise::Health_Code code) const;
    timespec transform(google::protobuf::Timestamp timestamp) const
    {
        timespec temp;
        temp.tv_nsec = timestamp.nanos();
        temp.tv_sec = timestamp.seconds();
        return temp;
    }

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
    bool gpioExport(uint8_t gpio);
    bool gpioOpen(Mode mode, const std::string &path);
    uint8_t gpioStatus(uint8_t gpio, bool *ok = nullptr);
    bool setGpioValue(uint8_t gpio, uint8_t value);
};

#endif // GPIOBROKER_H
