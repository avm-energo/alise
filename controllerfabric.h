#ifndef CONTROLLERFABRIC_H
#define CONTROLLERFABRIC_H

#include "controller.h"
#include "gpiobroker.h"
#include "stmbroker.h"
#include "zerorunner.h"

#include <QObject>

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
};

#endif // CONTROLLERFABRIC_H
