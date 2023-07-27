#include "aliseconstants.h"

AliseConstants::Timers AliseConstants::_timersConstants = { 1000, 1000, 50, 4000 };
AliseConstants::Blinks AliseConstants::_blinksConstants = { 50, 250, 500 };
int AliseConstants::s_SecondsToHardReset = 4;

AliseConstants::AliseConstants()
{
}

void AliseConstants::setResetCheckPeriod(int period)
{
    _timersConstants.ResetCheckPeriod = period;
}

void AliseConstants::setPowerCheckPeriod(int period)
{
    _timersConstants.PowerCheckPeriod = period;
}

void AliseConstants::setGpioBlinkPeriod(int period)
{
    _timersConstants.GpioBlinkCheckPeriod = period;
}

void AliseConstants::setHealthQueryPeriod(int period)
{
    _timersConstants.HealthQueryPeriod = period;
}

void AliseConstants::setFailureBlinkPeriod(int period)
{
    _blinksConstants.FailureBlink = period;
}

void AliseConstants::setSonicaStartingBlinkPeriod(int period)
{
    _blinksConstants.SonicaStatusStartingBlink = period;
}

void AliseConstants::setSonicaNormalBlinkPeriod(int period)
{
    _blinksConstants.SonicaStatusNormalBlink = period;
}

void AliseConstants::setSecondsToHardReset(int seconds)
{
    s_SecondsToHardReset = seconds;
}

int AliseConstants::ResetCheckPeriod()
{
    return _timersConstants.ResetCheckPeriod;
}

int AliseConstants::PowerCheckPeriod()
{
    return _timersConstants.PowerCheckPeriod;
}

int AliseConstants::GpioBlinkCheckPeriod()
{
    return _timersConstants.GpioBlinkCheckPeriod;
}

int AliseConstants::HealthQueryPeriod()
{
    return _timersConstants.HealthQueryPeriod;
}

int AliseConstants::FailureBlinkPeriod()
{
    return _blinksConstants.FailureBlink;
}

int AliseConstants::SonicaStartingBlinkPeriod()
{
    return _blinksConstants.SonicaStatusStartingBlink;
}

int AliseConstants::SonicaNormalBlinkPeriod()
{
    return _blinksConstants.SonicaStatusNormalBlink;
}

int AliseConstants::SecondsToHardReset()
{
    return s_SecondsToHardReset;
}
