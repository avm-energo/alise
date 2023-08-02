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
    const QMap<Controller::ContrTypes, QString> map = { { Controller::ContrTypes::IS_BOOTER, "sb" },
        { Controller::ContrTypes::IS_CORE, "sc" }, { Controller::ContrTypes::IS_INCORRECT_TYPE, "unk" } };
    if (!m_status)
        return false;
    Q_ASSERT(map.contains(ofType));
    ZeroRunner *runner = new ZeroRunner(map[ofType], this);
    Controller *controller = new Controller(m_broker, runner, this);
    controller->ofType(ofType);
    runner->runServer(port);
    return controller->launch();
}

bool ControllerFabric::getStatus()
{
    return m_status;
}
