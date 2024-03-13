#pragma once

#include <QtCore>

class Logger
{
public:
    enum LogLevels
    {
        LOGLEVEL_DEBUG = 4,
        LOGLEVEL_INFO = 3,
        LOGLEVEL_WARN = 2,
        LOGLEVEL_CRIT = 1,
        LOGLEVEL_FATAL = 0
    };

    void static messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    void static writeStart(const QString &filename);
    void static setLogLevel(LogLevels level);
    void static setLogLevel(const QString &level);

protected:
    Logger() = delete;

private:
    static LogLevels _logLevel;
    static QMutex _mutex;
};
