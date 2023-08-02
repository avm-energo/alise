#include "controllerfabric.h"

ControllerFabric::ControllerFabric(QObject *parent) : QObject(parent)
{
#if defined(AVTUK_STM)
    m_broker = new StmBroker(this);
#elif defined(AVTUK_NO_STM)
    m_broker = new GpioBroker(this);
#endif
    m_status = m_broker->connect();
}

bool ControllerFabric::createController(Controller::ContrTypes ofType, int port)
{
    if (!m_status)
        return false;
    ZeroRunner *runner = new ZeroRunner(this);
    Controller *controller = new Controller(m_broker, runner, this);
    controller->ofType(ofType);
    runner->runServer(port);
    return controller->launch();
}

bool ControllerFabric::getStatus()
{
    return m_status;
}
