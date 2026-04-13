#pragma once

#include <QObject>
#include <QString>

class GPIO : public QObject
{
public:
    enum PinDirections
    {
        INPUT,
        OUTPUT
    };

    enum PinOutputs
    {
        OFF = 0,
        ON = 1
    };

    struct GpioPin
    {
        int chip;                // gpiochip number
        int offset;              // gpiochip pin number
        PinDirections direction; // 0 for input, 1 for output
    };

    explicit GPIO(bool exceptionsAreOn, QObject *parent = nullptr);
    bool gpioGetLineValue(GpioPin pin);
    void gpioSetLineValue(GpioPin pin, bool value);

private:
    bool m_exceptionsAreOn;
    QString getChipName(int chipNum) const;
};
