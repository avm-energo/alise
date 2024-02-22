#include "controllerfabric.h"

ControllerFabric::ControllerFabric(QObject *parent) : QObject(parent)
{
}

bool ControllerFabric::createController(Controller::ContrTypes ofType, int port, Broker *broker, TimeSyncronizer *tm)
{
    Controller *controller = new Controller(this);
    return controller             //
        ->withBroker(broker)      //
        ->ofType(ofType)          //
        ->withTimeSynchonizer(tm) //
        ->launchOnPort(port);     //
}
