#pragma once

#include <interfaces/types/usbhidsettings.h>
#include <variant>

struct ConnectStruct
{
    QString name;
    std::variant<UsbHidSettings> settings;
};
