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
    bool resetReq;
};

struct Indication
{
    uint16_t PulseCnt1;
    uint16_t PulseFreq1;
    uint16_t PulseCnt2;
    uint16_t PulseFreq2;

    friend bool operator==(const Indication &lhs, const Indication &rhs)
    {
        return ((lhs.PulseCnt1 == rhs.PulseCnt1) && (lhs.PulseCnt2 == rhs.PulseCnt2)
            && (lhs.PulseFreq1 == rhs.PulseFreq1) && (lhs.PulseFreq2 == rhs.PulseFreq2));
    }
};
}

#endif // AVTUKCCU_H
