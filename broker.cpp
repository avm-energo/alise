#include "broker.h"

#include "aliseconstants.h"

#include <QDebug>

Broker::Broker(QObject *parent) : QObject(parent)
{
    m_clientTimeoutTimer.setInterval(
        AliseConstants::HealthQueryPeriod() * 3); // 3 times of Health Query period without replies
    QObject::connect(&m_clientTimeoutTimer, &QTimer::timeout, [&] {
        qDebug() << "Health Query Timeout";
        setIndication(AliseConstants::FailureIndication);
    });

    m_clientTimeoutTimer.start();
}

// Health code received
//
// code bits: 0 - there's some errors
// 1 - adminja working normally
// 2 - core working normally
// 3 - ninja working normally
// 4 - vasya working normally
// 5 - petya working normally
// 8-10 - adminja status:
//     000 - not working (none)
//     001 - not installed
//     010 - starting
//     011 - need to merge settings
//     100 - unknown status
//     101 - stopping
//     110 - stopped
//     111 - unknown status
// 11-13 - core status
// 14-16 - ninja status
// 17-19 - vasya status
// 20-22 - petya status

void Broker::healthReceived(uint32_t code)
{
    uint8_t processStatus;
    uint8_t healthCode = code & 0x000000FF;
    uint8_t worstProcessNumber = 0;
    uint8_t worstProcessStatus = NORMAL; // 0 - no errors, 1 - starting/stopping, 2 - stopped, 3 - error
    AVTUK_CCU::Indication indic;
    qDebug() << "Setting indication: " << code;
    m_clientTimeoutTimer.start();
    if (!(healthCode & 0x01)) // 0 is bad situation
        indic = AliseConstants::FailureIndication;
    else
    {
        for (int i = 0; i < numberOfProcesses; ++i)
        {
            if (!(healthCode & (0x02 << i))) // is process in failed state?
            {
                // check three bits in corresponding position
                int move = i * 3;
                processStatus = (code & (0x00000700 << move) >> (8 + move));
                switch (processStatus)
                {
                // any failure causes failed indication
                case AliseConstants::NOTINSTALLED:
                    break; // that's normal for now, checking remains
                case AliseConstants::NOTWORKING:
                case AliseConstants::UNKNOWN:
                case AliseConstants::UNKNOWN2:
                    worstProcessNumber = i;
                    worstProcessStatus = FAILED;
                    break;
                case AliseConstants::STARTING:
                case AliseConstants::STOPPING:
                    if ((worstProcessStatus != FAILED) && (worstProcessStatus != STOPPED))
                    {
                        worstProcessNumber = i;
                        worstProcessStatus = STARTING;
                    }
                    break;
                case AliseConstants::NEEDTOMERGE:
                case AliseConstants::STOPPED:
                    if (worstProcessStatus != FAILED)
                    {
                        worstProcessNumber = i;
                        worstProcessStatus = STOPPED;
                    }
                    break;
                }
            }
        }
        if ((worstProcessStatus == NORMAL) || (worstProcessNumber == 0)) // all is good
            indic = AliseConstants::NormalIndication;
        else
        {
            indic.PulseCnt1 = firstPulsesCount;       // first count is for showing status
            indic.PulseCnt2 = worstProcessNumber + 1; // second count is the number of process
            indic.PulseFreq2 = AliseConstants::ProcessNormalBlink();
            switch (worstProcessStatus)
            {
            case STARTING:
                indic.PulseFreq1 = AliseConstants::ProcessStartingBlink();
                break;
            case STOPPED:
                indic.PulseFreq1 = AliseConstants::ProcessStoppedBlink();
                break;
            case FAILED:
                indic.PulseFreq1 = AliseConstants::ProcessFailedBlink();
                break;
            default:
                indic.PulseFreq1 = AliseConstants::ProcessNormalBlink();
                break;
            }
        }
    }
    setIndication(indic);
}
