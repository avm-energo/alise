#pragma once

#include <QObject>

class NtpStatus
{
public:
    enum NtpStatusEnum
    {
        NO_SYNC = 1,
        SYNC_EXT = 2,
        SYNC_LOCAL = 3,
        ERROR = 4
    };

    NtpStatus();
    int getNtpStatus();
};
