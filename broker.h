#ifndef BROKER_H
#define BROKER_H

#include "protos/protos.pb.h"

#include <QObject>
#include <QTimer>

constexpr int checkIndicationPeriod = 2000;

class Broker : public QObject
{
    Q_OBJECT
public:
    explicit Broker(QObject *parent = nullptr);

    virtual bool connect() = 0;

    void criticalBlinking();
    int m_currentBlinkingPeriod;

public slots:
    void healthReceived(alise::Health_Code code);
    virtual void checkIndication() = 0;
    virtual void checkPowerUnit() = 0;
    virtual void setTime(timespec time) = 0;
    virtual void getTime() = 0;
    virtual void rebootMyself() = 0;
    virtual void setIndication() = 0;
    virtual void currentIndicationReceived(const QVariant &msg) = 0;

private:
    QTimer checkPowerTimer, m_clientTimeoutTimer;

signals:
};

#endif // BROKER_H
