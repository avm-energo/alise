#ifndef ALISECONSTANTS_H
#define ALISECONSTANTS_H

class AliseConstants
{
public:
    struct Timers
    {
        int ResetCheckPeriod;
        int PowerCheckPeriod;
        int TestRandomHealthIndicatorModePeriod;
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
    static int ResetCheckPeriod();
    static int PowerCheckPeriod();
    static int GpioBlinkCheckPeriod();
    static int HealthQueryPeriod();
    static int FailureBlinkPeriod();
    static int SonicaStartingBlinkPeriod();
    static int SonicaNormalBlinkPeriod();

private:
    static Timers _timersConstants;
    static Blinks _blinksConstants;
};

#endif // ALISECONSTANTS_H
