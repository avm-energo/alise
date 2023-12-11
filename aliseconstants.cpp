#include "aliseconstants.h"

AliseConstants::Timers AliseConstants::_timersConstants = { 1000, 1000, 50, 4000 };
AliseConstants::Blinks AliseConstants::_blinksConstants = { 25, 250, 500, 3000, 125 };
int AliseConstants::s_SecondsToHardReset = 4;
AVTUK_CCU::Indication AliseConstants::FailureIndication = { 1, AliseConstants::_blinksConstants.FailureBlink, 0, 0 };
AVTUK_CCU::Indication AliseConstants::NormalIndication
    = { 1, AliseConstants::_blinksConstants.ProcessStatusNormalBlink, 0, 0 };

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

void AliseConstants::setHealthQueryPeriod(int period)
{
    _timersConstants.HealthQueryPeriod = period;
}

void AliseConstants::setFailureBlinkFreq(uint16_t period)
{
    _blinksConstants.FailureBlink = freqByPeriod(period);
    FailureIndication = { 1, _blinksConstants.FailureBlink, 0, 0 };
}

void AliseConstants::setProcessNormalBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusNormalBlink = freqByPeriod(period);
    NormalIndication = { 1, _blinksConstants.ProcessStatusNormalBlink, 0, 0 };
}

void AliseConstants::setProcessStartingBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusStartingBlink = freqByPeriod(period);
}

void AliseConstants::setProcessStoppedBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusStoppedBlink = freqByPeriod(period);
}

void AliseConstants::setProcessFailedBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusFailedBlink = freqByPeriod(period);
}

void AliseConstants::setSecondsToHardReset(int seconds)
{
    s_SecondsToHardReset = seconds;
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

int AliseConstants::HealthQueryPeriod()
{
    return _timersConstants.HealthQueryPeriod;
}

int AliseConstants::SecondsToHardReset()
{
    return s_SecondsToHardReset;
}

uint16_t AliseConstants::FailureBlink()
{
    return _blinksConstants.FailureBlink;
}

uint16_t AliseConstants::ProcessStartingBlink()
{
    return _blinksConstants.ProcessStatusStartingBlink;
}

uint16_t AliseConstants::ProcessNormalBlink()
{
    return _blinksConstants.ProcessStatusNormalBlink;
}

uint16_t AliseConstants::ProcessStoppedBlink()
{
    return _blinksConstants.ProcessStatusStoppedBlink;
}

uint16_t AliseConstants::ProcessFailedBlink()
{
    return _blinksConstants.ProcessStatusFailedBlink;
}

uint16_t AliseConstants::freqByPeriod(int period)
{
    return 1000 / period * 1000;
}

uint16_t AliseConstants::periodByFreq(int freq)
{
    return 1000 / (freq / 1000);
}

int AliseConstants::SecondsToHardReset()
{
    return s_SecondsToHardReset;
}
