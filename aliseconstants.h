#ifndef ALISECONSTANTS_H
#define ALISECONSTANTS_H

#include "avtukccu.h"

#include <cstdint>

namespace Alise
{

enum ProcessErrors
{
    NORMAL = 0,
    YELLOW = 1,
    ORANGE = 2,
    RED = 3,
    VIOLET = 4
};

enum ProcessStatuses
{
    CHECKHTHBIT = 0,
    NOTINSTALLED,
    STARTING,
    NEEDTOMERGE,
    BROKENHASH,
    STOPPING,
    STOPPED,
    UNKNOWN
};

class AliseConstants
{
public:
    struct Timers
    {
        int ResetCheckPeriod;
        int PowerCheckPeriod;
        int GpioBlinkCheckPeriod;
        int HealthQueryPeriod;
        int ReplyTimeoutPeriod;
    };

    struct Blinks
    {
        uint16_t FailureBlink;
        uint16_t ProcessStatusStartingBlink;
        uint16_t ProcessStatusNormalBlink;
        uint16_t ProcessStatusStoppedBlink;
        uint16_t ProcessStatusFailedBlink;
        uint16_t ProcessStatusSemiWorkingBlink;
    };

    AliseConstants();
    static void setResetCheckPeriod(int period);
    static void setPowerCheckPeriod(int period);
    static void setHealthQueryPeriod(int period);
    static void setReplyTimeoutPeriod(int period);
    static void setFailureBlinkFreq(uint16_t period);
    static void setProcessNormalBlinkFreq(uint16_t period);
    static void setProcessStartingBlinkFreq(uint16_t period);
    static void setProcessStoppedBlinkFreq(uint16_t period);
    static void setProcessFailedBlinkFreq(uint16_t period);
    static void setProcessSemiWorkingBlinkFreq(uint16_t period);
    static void setSecondsToHardReset(int seconds);
    static int ResetCheckPeriod();
    static int PowerCheckPeriod();
    static int HealthQueryPeriod();
    static int ReplyTimeoutPeriod();
    static int SecondsToHardReset();
    static uint16_t FailureBlink();
    static uint16_t ProcessBlink(ProcessErrors error);
    static AVTUK_CCU::Indication FailureIndication;
    static AVTUK_CCU::Indication NormalIndication;
    static Timers _timersConstants;
    static Blinks _blinksConstants;
    static uint16_t freqByPeriod(int period);
    static uint16_t periodByFreq(int freq);

private:
    static int s_SecondsToHardReset; // really it's ResetCheckPeriod times (seconds = ResetCheckPeriod / 1000 *
                                     // s_SecondsToHardReset)
};

}

#endif // ALISECONSTANTS_H
