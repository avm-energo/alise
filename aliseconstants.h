#ifndef ALISECONSTANTS_H
#define ALISECONSTANTS_H

class AliseConstants
{
public:
    struct Timers
    {
        int ResetCheckPeriod;
        int PowerCheckPeriod;
        int GpioBlinkCheckPeriod;
        int HealthQueryPeriod;
    };

    struct Blinks
    {
        int FailureBlink;
        int SonicaStatusStartingBlink;
        int SonicaStatusNormalBlink;
    };

    AliseConstants();
    static void setResetCheckPeriod(int period);
    static void setPowerCheckPeriod(int period);
    static void setGpioBlinkPeriod(int period);
    static void setHealthQueryPeriod(int period);
    static void setFailureBlinkPeriod(int period);
    static void setSonicaStartingBlinkPeriod(int period);
    static void setSonicaNormalBlinkPeriod(int period);
    static void setSecondsToHardReset(int seconds);
    static int ResetCheckPeriod();
    static int PowerCheckPeriod();
    static int GpioBlinkCheckPeriod();
    static int HealthQueryPeriod();
    static int FailureBlinkPeriod();
    static int SonicaStartingBlinkPeriod();
    static int SonicaNormalBlinkPeriod();
    static int SecondsToHardReset();

private:
    static Timers _timersConstants;
    static Blinks _blinksConstants;
    static int s_SecondsToHardReset; // really it's ResetCheckPeriod times (seconds = ResetCheckPeriod / 1000 *
                                     // s_SecondsToHardReset)
};

#endif // ALISECONSTANTS_H
