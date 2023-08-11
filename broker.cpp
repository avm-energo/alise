#include "broker.h"

#include "aliseconstants.h"

#include <QDebug>

Broker::Broker(QObject *parent) : QObject(parent)
{
    m_clientTimeoutTimer.setInterval(
        AliseConstants::HealthQueryPeriod() * 3); // 3 times of Health Query period without replies
    QObject::connect(&m_clientTimeoutTimer, &QTimer::timeout, [&] {
        qDebug() << "Health Query Timeout";
        criticalBlinking();
    });

    m_clientTimeoutTimer.start();
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
