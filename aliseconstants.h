#ifndef ALISECONSTANTS_H
#define ALISECONSTANTS_H

#include "avtukccu.h"

#include <cstdint>

class AliseConstants
{
public:
    enum ProcessStatuses
    {
        NOTWORKING = 0,
        NOTINSTALLED,
        STARTING,
        NEEDTOMERGE,
        UNKNOWN,
        STOPPING,
        STOPPED,
        UNKNOWN2
    };

    struct Timers
    {
        int ResetCheckPeriod;
        int PowerCheckPeriod;
        int GpioBlinkCheckPeriod;
        int HealthQueryPeriod;
    };

    struct Blinks
    {
        uint16_t FailureBlink;
        uint16_t ProcessStatusStartingBlink;
        uint16_t ProcessStatusNormalBlink;
        uint16_t ProcessStatusStoppedBlink;
        uint16_t ProcessStatusFailedBlink;
    };

    AliseConstants();
    static void setResetCheckPeriod(int period);
    static void setPowerCheckPeriod(int period);
    static void setHealthQueryPeriod(int period);
    static void setFailureBlinkFreq(uint16_t period);
    static void setProcessNormalBlinkFreq(uint16_t period);
    static void setProcessStartingBlinkFreq(uint16_t period);
    static void setProcessStoppedBlinkFreq(uint16_t period);
    static void setProcessFailedBlinkFreq(uint16_t period);
    static void setSecondsToHardReset(int seconds);
    static int ResetCheckPeriod();
    static int PowerCheckPeriod();
    static int HealthQueryPeriod();
    static int SecondsToHardReset();
    static uint16_t FailureBlink();
    static uint16_t ProcessStartingBlink();
    static uint16_t ProcessNormalBlink();
    static uint16_t ProcessStoppedBlink();
    static uint16_t ProcessFailedBlink();
    static AVTUK_CCU::Indication FailureIndication;
    static AVTUK_CCU::Indication NormalIndication;
    static Timers _timersConstants;
    static Blinks _blinksConstants;

private:
    static int s_SecondsToHardReset; // really it's ResetCheckPeriod times (seconds = ResetCheckPeriod / 1000 *
                                     // s_SecondsToHardReset)
    static uint16_t freqByPeriod(int period);
    static uint16_t periodByFreq(int freq);
};

#endif // ALISECONSTANTS_H
