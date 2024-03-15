#ifndef AVTUKCCU_H
#define AVTUKCCU_H

#include <cstdint>

/// AVTUK Central Controller Unit
namespace AVTUK_CCU
{
constexpr uint16_t MainBlock = 1;
constexpr uint16_t IndicationBlock = 2;

struct Main
{
    uint8_t PWRIN;
    uint8_t resetReq;
};

struct Indication
{
    uint16_t PulseCnt1;
    uint16_t PulseFreq1;
    uint16_t PulseCnt2;
    uint16_t PulseFreq2;

    Indication() : PulseCnt1(0), PulseFreq1(0), PulseCnt2(0), PulseFreq2(0)
    {
    }

    Indication(uint16_t pulseCnt1, uint16_t pulseFreq1, uint16_t pulseCnt2, uint16_t pulseFreq2)
        : PulseCnt1(pulseCnt1), PulseFreq1(pulseFreq1), PulseCnt2(pulseCnt2), PulseFreq2(pulseFreq2)
    {
    }

    friend bool operator==(const Indication &lhs, const Indication &rhs)
    {
        return ((lhs.PulseCnt1 == rhs.PulseCnt1) && (lhs.PulseCnt2 == rhs.PulseCnt2)
            && (lhs.PulseFreq1 == rhs.PulseFreq1) && (lhs.PulseFreq2 == rhs.PulseFreq2));
    }

    //    Indication operator=(const Indication &rhs)
    //    {
    //        Indication indic;
    //        indic.PulseCnt1 = rhs.PulseCnt1;
    //        indic.PulseCnt2 = rhs.PulseCnt2;
    //        indic.PulseFreq1 = rhs.PulseFreq1;
    //        indic.PulseFreq2 = rhs.PulseFreq2;
    //        return indic;
    //    }
};
}

#endif // AVTUKCCU_H
