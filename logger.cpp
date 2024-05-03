#include "logger.h"

#include <gen/files.h>
#include <gen/stdfunc.h>

struct MsgDescr
{
    Logger::LogLevels loglevel;
    const char *prefix;
};

const static QMap<QtMsgType, MsgDescr> msgTypes {
    { QtDebugMsg, { Logger::LogLevels::LOGLEVEL_DEBUG, "[DEBUG]" } },      //
    { QtWarningMsg, { Logger::LogLevels::LOGLEVEL_WARN, "[WARNING]" } },   //
    { QtCriticalMsg, { Logger::LogLevels::LOGLEVEL_CRIT, "[CRITICAL]" } }, //
    { QtFatalMsg, { Logger::LogLevels::LOGLEVEL_FATAL, "[FATAL]" } },      //
    { QtInfoMsg, { Logger::LogLevels::LOGLEVEL_INFO, "[INFO]" } }          //
};

const QMap<QString, Logger::LogLevels> _logLevelsMap = { { "Debug", Logger::LogLevels::LOGLEVEL_DEBUG },
    { "Info", Logger::LogLevels::LOGLEVEL_INFO }, { "Fatal", Logger::LogLevels::LOGLEVEL_FATAL },
    { "Warn", Logger::LogLevels::LOGLEVEL_WARN }, { "Error", Logger::LogLevels::LOGLEVEL_CRIT } };

static QString logFilename = "alise.log"; // имя по умолчанию
Logger::LogLevels Logger::_logLevel = Logger::LogLevels::LOGLEVEL_WARN;
QMutex Logger::_mutex;

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (_logLevel < msgTypes.value(type).loglevel)
        return;
    QMutexLocker locker(&_mutex);
    QFile logFile;
    QTextStream out;

    std::string function = context.function ? context.function : "";
    std::string rubbish(" __cdecl");
    StdFunc::RemoveSubstr(function, rubbish);

    logFile.setFileName(logFilename);
    out.setDevice(&logFile);
    if (logFile.open(QFile::ReadWrite | QFile::Text | QFile::Append))
    {
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz "); // Log datetime
        out << msgTypes.value(type).prefix << msg << "\n";
        out.flush(); // Flush buffer
        Files::checkNGzip(&logFile);
        logFile.close();
    }
}

void Logger::writeStart(const QString &filename)
{
    QMutexLocker locker(&_mutex);
    logFilename = filename;
    QFile logFile(logFilename);
    Files::makePath(logFile);
    QTextStream out;
    out.setDevice(&logFile);
    logFile.open(QFile::ReadWrite | QFile::Text | QFile::Append);
    out << "=====================================\nLog file started at "
        << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n"
        << QCoreApplication::applicationName() << " v." << QCoreApplication::applicationVersion() << "\n";
    out.flush();
    Files::checkNGzip(&logFile);
}

void Logger::setLogLevel(LogLevels level)
{
    _logLevel = level;
}

void Logger::setLogLevel(const QString &level)
{
    _logLevel = _logLevelsMap.value(level);
}
