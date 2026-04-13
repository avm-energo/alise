#include "gpio.h"

#include <QtDebug>
#include <gpiod.hpp>
GPIO::GPIO(bool exceptionsAreOn, QObject *parent) : m_exceptionsAreOn(exceptionsAreOn), QObject(parent)
{
}

bool GPIO::gpioGetLineValue(GpioPin pin)
{
    try
    {
        const ::std::filesystem::path chip_path(getChipName(pin.chip).toStdString());
#if (GPIO_TYPE == 1) // libgpiod v1.6
        gpiod_chip *chip = gpiod_chip_open(chip_path);
        if (chip == NULL)
        {
            qCritical() << "cannot open chip " << chippath.c_str() << "!";
            return false;
        }
        gpiod_line *line = gpiod_chip_get_line(chip, pin.offset);
        if (line == NULL)
        {
            qCritical() << "cannot get line " << chip_path.c_str() << ":" << pin.offset << "!";
            return false;
        }
        if (pin.direction == PinDirections::OUTPUT)
        {
            int ret = gpiod_line_request_output(line, "gpio", 0);
            if (ret != 0)
            {
                qCritical() << "cannot request line " << chip_path.c_str() << ":" << pin.offset << " for output!";
                return false;
            }
            return gpiod_line_get_value(line);
        }
        qCritical() << "wrong direction for line " << chip_path.c_str() << ":" << pin.offset;
        return false;
#elif (GPIO_TYPE == 2) // libgpiod v2
        auto request = ::gpiod::chip(chip_path)
                           .prepare_request()
                           .set_consumer("get-line-value")
                           .add_line_settings(
                               pin.offset, ::gpiod::line_settings().set_direction(::gpiod::line::direction::INPUT))
                           .do_request();
        return (request.get_value(pin.offset) == ::gpiod::line::value::ACTIVE);
#endif
    } catch (std::exception e)
    {
        if (m_exceptionsAreOn)
            qDebug() << "gpioGetLineValue exception: " << e.what();
    }
    return false;
}

void GPIO::gpioSetLineValue(GpioPin pin, bool value)
{
    try
    {
        const ::std::filesystem::path chip_path(getChipName(pin.chip).toStdString());
#if (GPIO_TYPE == 1) // libgpiod v1.6
        gpiod_chip *chip = gpiod_chip_open(chip_path);
        if (chip == NULL)
        {
            qCritical() << "cannot open chip " << chip_path.c_str() << "!";
            return false;
        }
        gpiod_line *line = gpiod_chip_get_line(chip, pin.offset);
        if (line == NULL)
        {
            qCritical() << "cannot get line " << chip_path.c_str() << ":" << pin.offset << "!";
            return false;
        }
        if (pin.direction == PinDirections::INPUT)
        {
            int ret = gpiod_line_request_input(line, "gpio");
            if (ret != 0)
            {
                qCritical() << "cannot request line " << chipstr.c_str() << ":" << pin.offset << " for input!";
                return false;
            }
            gpiod_line_set_value(m_modeLine, value);
        }
        qCritical() << "wrong direction for line " << chip_path.c_str() << ":" << pin.offset;
        return false;
#elif (GPIO_TYPE == 2) // libgpiod v2
        bool pinValue = gpioGetLineValue(pin);
        if (pinValue ^ value)
        {
            auto request = ::gpiod::chip(chip_path)
                               .prepare_request()
                               .set_consumer("toggle-line-value")
                               .add_line_settings(
                                   pin.offset, ::gpiod::line_settings().set_direction(::gpiod::line::direction::OUTPUT))
                               .do_request();
        }
#endif
    } catch (std::exception e)
    {
        if (m_exceptionsAreOn)
            qDebug() << "gpioSetLineValue exception: " << e.what();
    }
}

QString GPIO::getChipName(int chipNum) const
{
    return "/dev/gpiochip" + QString::number(chipNum);
}
