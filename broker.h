#pragma once

#include "aliseconstants.h"

#include <QObject>
#include <QTimer>
#include <avm-gen/datatypes.h>

constexpr int checkIndicationPeriod = 2000;
constexpr int numberOfProcesses = 7; // booter, adminja, core, alise, ninja, vasya, petya
constexpr int firstPulsesCount = 10; // number of pulses to show status
constexpr int numberFreq = 750;      // period (in ms) of showing process number

class Broker : public QObject
{
    Q_OBJECT
protected:
    AVTUK_CCU::Indication m_currentIndication;

public:
    explicit Broker(QObject *parent = nullptr);
    virtual bool connect() = 0;
    void clearOldCode();
    uint32_t m_oldCode;

public slots:
    void updateBlock(const DataTypes::BlockStruct &blk);
    void updateTime(const timespec &time);
    void healthReceived(uint32_t code);
    virtual void checkIndication() = 0;
    virtual void checkPowerUnit() = 0;
    virtual void setTime(const timespec &time) = 0;
    virtual void getTime() = 0;
    virtual void rebootMyself() = 0;
    virtual void setIndication(const AVTUK_CCU::Indication &indication) = 0;
    void setFailedIndication();

private:
    QTimer checkPowerTimer;
    uint8_t m_worstProcessNumber;
    Alise::ProcessErrors m_worstProcessError;

    void setStartingProcessError(int index);
    void setStoppedProcessError(int index);
    void setFailedProcessError(int index);
    void setSemiWorkingProcessError(int index);

signals:
    void receivedBlock(const DataTypes::BlockStruct &blk);
    void receivedTime(const timespec &time);
};
