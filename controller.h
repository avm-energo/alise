#pragma once
#include "broker.h"
#include "gen/stdfunc.h"
#include "recovery.h"
#include "timesyncronizer.h"
#include "zerorunner.h"

#include <QObject>
#include <QThread>
#include <gen/datamanager/typesproxy.h>

class Controller : public QObject
{
public:
    enum ContrTypes
    {
        IS_BOOTER,
        IS_CORE,
        IS_ADMINJA,
        IS_INCORRECT_TYPE
    };

    explicit Controller(Broker *devBroker, ZeroRunner *runner, QObject *parent = nullptr) noexcept;

    ~Controller() override;

    bool launch();
    void shutdown();
    void syncTime(const timespec &);
    void ofType(ContrTypes type);
signals:

private:
    bool hasIncorrectType();
    ContrTypes m_type;
    ZeroRunner *m_runner;
    Broker *m_deviceBroker;
    QTimer *m_pingTimer;
    TimeSyncronizer m_timeSynchronizer;
    RecoveryEngine m_recoveryEngine;
    int syncCounter = 0;
    UniquePointer<DataTypesProxy> proxyBS, proxyTS;
};
