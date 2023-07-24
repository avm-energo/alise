#ifndef BROKER_H
#define BROKER_H

#include "protos/protos.pb.h"

#include <QObject>
#include <QTimer>

class Broker : public QObject
{
    Q_OBJECT
public:
    explicit Broker(QObject *parent = nullptr);

    virtual void checkPowerUnit() = 0;
    virtual void setTime(timespec time) = 0;
    virtual void getTime() = 0;
    virtual void rebootMyself() = 0;
    virtual void setIndication() = 0;

    bool status();
    int m_currentBlinkingPeriod;
    bool m_status;

public slots:
    void healthReceived(alise::Health_Code code);

private:
    QTimer m_checkPowerTimer, m_clientTimeoutTimer;

    void criticalBlinking();
signals:
};

#endif // BROKER_H
