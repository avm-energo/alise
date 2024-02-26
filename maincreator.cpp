#include "maincreator.h"

#include "aliseconstants.h"
#if defined(AVTUK_STM)
#include "stmbroker.h"
#elif defined(AVTUK_NO_STM)
#include "gpiobroker.h"
#endif

MainCreator::MainCreator(QObject *parent)
{
    Q_UNUSED(parent)
}

void MainCreator::init()
{
    m_timeSynchronizer = new TimeSyncronizer;
    m_timeSynchronizer->init();
}

Broker *MainCreator::create(bool &ok)
{
    Broker *broker;
#if defined(AVTUK_STM)
    broker = new StmBroker(this);
#elif defined(AVTUK_NO_STM)
    broker = new GpioBroker(this);
#endif
    ok = broker->connect();
    if (!ok)
        return nullptr;

    // check power status
    QTimer *checkPowerTimer = new QTimer(this);
    checkPowerTimer->setInterval(Alise::AliseConstants::PowerCheckPeriod());
    QObject::connect(checkPowerTimer, &QTimer::timeout, broker, &Broker::checkPowerUnit);
    checkPowerTimer->start();

    // check indication in
    QTimer *checkIndicationTimer = new QTimer(this);
    checkIndicationTimer->setInterval(checkIndicationPeriod);
    QObject::connect(checkIndicationTimer, &QTimer::timeout, broker, &Broker::checkIndication);
    checkIndicationTimer->start();

    // RecoveryEngine: rebooting and Power Status get from MCU
    connect(&m_recoveryEngine, &RecoveryEngine::rebootReq, broker, &Broker::rebootMyself);
    connect(broker, &Broker::receivedBlock, &m_recoveryEngine, &RecoveryEngine::receiveBlock);

#if defined(AVTUK_STM)
    connect(m_timeSynchronizer, &TimeSyncronizer::setTime, this, [&](const timespec &time) { broker->setTime(time); });
#endif
#ifdef __linux__
    auto connectionTimeSync = std::shared_ptr<QMetaObject::Connection>(new QMetaObject::Connection);
    *connectionTimeSync = connect(
        broker, &Broker::receivedTime, this,
        [=, &syncer = m_timeSynchronizer](const timespec &time) {
            QObject::disconnect(*connectionTimeSync);
            syncer->printAndSetSystemTime(time);
        },
        Qt::DirectConnection);
#endif

    return broker;
}

TimeSyncronizer *MainCreator::getTimeSynchronizer()
{
    return m_timeSynchronizer;
}
