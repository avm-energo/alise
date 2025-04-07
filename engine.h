#pragma once

#include "alisesettings.h"
#include "broker.h"
#include "httpapiserver.h"
#include "httpmiddleware.h"
#include "recovery.h"
#include "timesyncronizer.h"

#include <QObject>

class Engine : public QObject
{
public:
    Engine(QObject *parent = nullptr);

    /// \brief Инициализация движка
    bool init(AliseSettings &settings);

private:
    HttpMiddleware *m_mw;
    HttpApiServer *m_server;
    TimeSyncronizer *m_timeSynchronizer;
    RecoveryEngine m_recoveryEngine;
    Broker *m_broker;

    void createLocalConnections(); ///< создание локальных соединений между RecoveryEngine, Broker и TimeSynchronizer
    void createHttpConnections();  ///< создание соединений между Http слоем и остальными сущностями
};
