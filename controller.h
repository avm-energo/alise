#pragma once

#include "broker.h"
#include "gen/stdfunc.h"
#include "recovery.h"
#include "timesyncronizer.h"
#include "zerorunner.h"

#include <QObject>
#include <QThread>

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
    bool launch();
    void shutdown();
    void syncTime(const timespec &);
    void ofType(ContrTypes type);

private:
    bool isIncorrectType();
    void adminjaSetup();
    void booterSetup();
    void coreSetup();

    ContrTypes m_type;
    ZeroRunner *m_runner;
    Broker *m_deviceBroker;
    QTimer *m_pingTimer;
    TimeSyncronizer m_timeSynchronizer;
    RecoveryEngine m_recoveryEngine;
    int syncCounter = 0;
};
