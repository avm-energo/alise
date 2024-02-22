#ifndef CONTROLLERFABRIC_H
#define CONTROLLERFABRIC_H

#include "controller.h"

#include <QObject>

class ControllerFabric : public QObject
{
public:
    ControllerFabric(QObject *parent = nullptr);
    bool createController(Controller::ContrTypes ofType, int port, Broker *broker, TimeSyncronizer *tm);

private:
};

#endif // CONTROLLERFABRIC_H
