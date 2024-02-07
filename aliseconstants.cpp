#include "aliseconstants.h"

#include <QMap>

using namespace Alise;

AliseConstants::Timers AliseConstants::_timersConstants = { .ResetCheckPeriod = 1000,
    .PowerCheckPeriod = 1000,
    .GpioBlinkCheckPeriod = 1000,
    .HealthQueryPeriod = 4000,
    .ReplyTimeoutPeriod = 4000 };

AliseConstants::Blinks AliseConstants::_blinksConstants = { .FailureBlink = 25,
    .ProcessStatusStartingBlink = 250,
    .ProcessStatusNormalBlink = 500,
    .ProcessStatusStoppedBlink = 3000,
    .ProcessStatusFailedBlink = 125,
    .ProcessStatusSemiWorkingBlink = 1000 };

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

void AliseConstants::setReplyTimeoutPeriod(int period)
{
    _timersConstants.ReplyTimeoutPeriod = period;
}

void AliseConstants::setFailureBlinkFreq(uint16_t period)
{
    _blinksConstants.FailureBlink = period;
    FailureIndication = { 1, _blinksConstants.FailureBlink, 0, 0 };
}

void AliseConstants::setProcessNormalBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusNormalBlink = period;
    NormalIndication = { 1, _blinksConstants.ProcessStatusNormalBlink, 0, 0 };
}

void AliseConstants::setProcessStartingBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusStartingBlink = period;
}

void AliseConstants::setProcessStoppedBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusStoppedBlink = period;
}

void AliseConstants::setProcessFailedBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusFailedBlink = period;
}

void AliseConstants::setProcessSemiWorkingBlinkFreq(uint16_t period)
{
    _blinksConstants.ProcessStatusSemiWorkingBlink = period;
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

int AliseConstants::ReplyTimeoutPeriod()
{
    return _timersConstants.ReplyTimeoutPeriod;
}

int AliseConstants::SecondsToHardReset()
{
    return s_SecondsToHardReset;
}

uint16_t AliseConstants::FailureBlink()
{
    return _blinksConstants.FailureBlink;
}

uint16_t AliseConstants::ProcessBlink(Alise::ProcessErrors error)
{
    static const QMap<Alise::ProcessErrors, uint16_t> map = {
        { NORMAL, _blinksConstants.ProcessStatusNormalBlink },
        { YELLOW, _blinksConstants.ProcessStatusStartingBlink },
        { ORANGE, _blinksConstants.ProcessStatusStoppedBlink },
        { VIOLET, _blinksConstants.ProcessStatusSemiWorkingBlink },
        { RED, _blinksConstants.ProcessStatusFailedBlink },
    };
    return map.value(error, _blinksConstants.ProcessStatusFailedBlink);
}

uint16_t AliseConstants::freqByPeriod(int period)
{
    if (period == 0)
        return 0;
    return 1000 / period * 1000;
}

uint16_t AliseConstants::periodByFreq(int freq)
{
    if (freq == 0)
        return 0;
    return 1000 / (freq / 1000);
}
