#ifndef BROKER_H
#define BROKER_H

#include "aliseconstants.h"
#include "protos/protos.pb.h"

#include <QObject>
#include <QTimer>

constexpr int checkIndicationPeriod = 2000;
constexpr int numberOfProcesses = 7; // booter, adminja, core, alise, ninja, vasya, petya
constexpr int firstPulsesCount = 4;  // number of pulses to show status

class Broker : public QObject
{
    Q_OBJECT
public:
    explicit Broker(QObject *parent = nullptr);

    virtual bool connect() = 0;

    AVTUK_CCU::Indication m_currentIndication;

public slots:
    void healthReceived(uint32_t code);
    virtual void checkIndication() = 0;
    virtual void checkPowerUnit() = 0;
    virtual void setTime(timespec time) = 0;
    virtual void getTime() = 0;
    virtual void rebootMyself() = 0;
    virtual void setIndication(const AVTUK_CCU::Indication &indication) = 0;
    virtual void currentIndicationReceived(const QVariant &msg) = 0;

private:
    QTimer checkPowerTimer, m_clientTimeoutTimer;
    uint8_t m_worstProcessNumber;
    Alise::ProcessErrors m_worstProcessError;

    void setStartingProcessError(int index);
    void setStoppedProcessError(int index);
    void setFailedProcessError(int index);
    void setSemiWorkingProcessError(int index);

signals:
};

#endif // BROKER_H
