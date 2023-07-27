#include "broker.h"

#include "aliseconstants.h"

#include <QDebug>

Broker::Broker(QObject *parent) : QObject(parent)
{
    m_status = false;

    m_checkPowerTimer.setInterval(AliseConstants::PowerCheckPeriod());
    QObject::connect(&m_checkPowerTimer, &QTimer::timeout, this, &Broker::checkPowerUnit);

    m_clientTimeoutTimer.setInterval(
        AliseConstants::HealthQueryPeriod() * 3); // 3 times of Health Query period without replies
    QObject::connect(&m_clientTimeoutTimer, &QTimer::timeout, [&] {
        qDebug() << "Health Query Timeout";
        criticalBlinking();
    });

    m_checkPowerTimer.start();
    m_clientTimeoutTimer.start();
#ifdef TEST_INDICATOR
    QTimer *testTimer = new QTimer(this);
    testTimer->setInterval(AliseConstants::TestRandomHealthIndicatorModePeriod());
    QObject::connect(testTimer, &QTimer::timeout, this, [this] {
        auto random = static_cast<alise::Health_Code>(QRandomGenerator::global()->bounded(0, 8));
        qDebug() << "Random:" << random;
        setIndication(random);
    });
    testTimer->start();
#endif
}

bool Broker::status()
{
    return m_status;
}

void Broker::healthReceived(alise::Health_Code code)
{
    qDebug() << "Setting indication: " << code;
    m_clientTimeoutTimer.start();
    switch (code)
    {
    case alise::Health_Code_Startup:
    {
        if (m_currentBlinkingPeriod == AliseConstants::SonicaStartingBlinkPeriod())
            return;
        m_currentBlinkingPeriod = AliseConstants::SonicaStartingBlinkPeriod();
        break;
    }
    case alise::Health_Code_Work:
    {
        if (m_currentBlinkingPeriod == AliseConstants::SonicaNormalBlinkPeriod())
            return;
        m_currentBlinkingPeriod = AliseConstants::SonicaNormalBlinkPeriod();
        break;
    }
    default:
    {
        if (m_currentBlinkingPeriod == AliseConstants::FailureBlinkPeriod())
            return;
        m_currentBlinkingPeriod = AliseConstants::FailureBlinkPeriod();
        break;
    }
    }
    setIndication();
}

void Broker::criticalBlinking()
{
    m_currentBlinkingPeriod = AliseConstants::FailureBlinkPeriod();
    setIndication();
}
