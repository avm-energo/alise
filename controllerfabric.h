#ifndef CONTROLLERFABRIC_H
#define CONTROLLERFABRIC_H

#include "controller.h"

#if defined(AVTUK_STM)
#include "stmbroker.h"
#elif defined(AVTUK_NO_STM)
#include "gpiobroker.h"
#endif

#include "recovery.h"
#include "zerorunner.h"

#include <QObject>

// clang-format off
#if defined(AVTUK_STM)
    using deviceType = StmBroker;
#elif defined(AVTUK_NO_STM)
    using deviceType = GpioBroker;
#else
    using deviceType = void;
#endif
// clang-format on

class ControllerFabric : public QObject
{
public:
    ControllerFabric(QObject *parent = nullptr);
    bool createController(Controller::ContrTypes ofType, int port);
    bool getStatus();

private:
    ZeroRunner *m_runner;
    Broker *m_broker;
    bool m_status;
    RecoveryEngine m_recoveryEngine;
    UniquePointer<DataTypesProxy> proxyBS;
};

#endif // CONTROLLERFABRIC_H
