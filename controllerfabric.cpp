#include "controllerfabric.h"

ControllerFabric::ControllerFabric(QObject *parent) : QObject(parent)
{
}

bool ControllerFabric::createController(Controller::ContrTypes ofType, int port, Broker *broker, TimeSyncronizer *tm)
{
    Controller *controller = new Controller(this);
    return controller             //
        ->withBroker(broker)      //
        ->withTimeSynchonizer(tm) //
        ->ofType(ofType)          //
        ->launchOnPort(port);     //
}
