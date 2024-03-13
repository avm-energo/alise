#pragma once

#include "broker.h"
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

    explicit Controller(QObject *parent = nullptr) noexcept;
    void shutdown();
    void syncTime(const timespec &);
    Controller *withBroker(Broker *broker);
    Controller *withTimeSynchonizer(TimeSyncronizer *tm);
    Controller *ofType(ContrTypes type);
    bool launchOnPort(int port);

private:
    bool isIncorrectType();
    void adminjaSetup();
    void booterSetup();
    void coreSetup();

    ContrTypes m_type;
    ZeroRunner *m_runner;
    TimeSyncronizer *m_timeSynchronizer;
    Broker *m_deviceBroker;
    QTimer *m_pingTimer;
    int syncCounter = 0;
    const QMap<Controller::ContrTypes, QString> c_controllerMap
        = { { Controller::ContrTypes::IS_ADMINJA, "sa" }, { Controller::ContrTypes::IS_BOOTER, "sb" },
              { Controller::ContrTypes::IS_CORE, "sc" }, { Controller::ContrTypes::IS_INCORRECT_TYPE, "unk" } };
};
