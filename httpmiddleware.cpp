#include "httpmiddleware.h"

#include "avtukccu.h"
#include "gitversion/gitversion.h"

#include <QDateTime>
#include <QString>
#include <config.h>
#include <gen/stdfunc.h>
#include <gen/timefunc.h>

HttpMiddleware::HttpMiddleware(const AliseSettings &settings, QObject *parent) : QObject { parent }
{
    GitVersion gitVersion;
    m_aliseVersion = QString(ALISEVERSION) + "-" + gitVersion.getGitHash();
    m_serialNum = QString::number(settings.serialNum);
    m_hwVersion = StdFunc::VerToStr(settings.hwVersion);
    m_swVersion = StdFunc::VerToStr(settings.swVersion);
    setNtpState(0);
}

QJsonObject HttpMiddleware::helloReply()
{
    QJsonObject json;
    json["AliseVersion"] = m_aliseVersion;
    json["SerialNum"] = m_serialNum;
    json["HwVersion"] = m_hwVersion;
    json["SwVersion"] = m_swVersion;
    return json;
}

QJsonObject HttpMiddleware::timeStamp()
{
    QJsonObject json;
    QDateTime dt = QDateTime::fromSecsSinceEpoch(m_time.tv_sec);
    json["Timestamp"] = dt.toString("yyyy-MM-dd hh:mm:ss");
    return json;
}

QJsonObject HttpMiddleware::pingReply()
{
    QJsonObject json;
    json["Result"] = "true";
    return json;
}

QJsonObject HttpMiddleware::powerStatus()
{
    QJsonObject json;
    json["PWRIN"] = QString::number(m_pwrIn);
    json["ResetReq"] = QString::number(m_resetReq);
    return json;
}

QJsonObject HttpMiddleware::ntpStatus()
{
    QJsonObject json;
    json["NTPState"] = QString::number(m_ntpState);
    return json;
}

QJsonObject HttpMiddleware::timeZone()
{
    QJsonObject json;
    int tz = TimeFunc::curTimeZone();
    json["Timezone"] = QString::number(tz); // inverting because of ugly timezone setting in Debian
    return json;
}

void HttpMiddleware::timePost(const QByteArray &arr)
{
    const QJsonObject json = byteArrayToJsonObject(arr);
    QString timeString = json["Timestamp"].toString();
    qDebug() << "Timestamp received: " << timeString;
    QDateTime dt = QDateTime::fromString(timeString, "yyyy-MM-dd hh:mm:ss");
    m_time.tv_sec = dt.toSecsSinceEpoch();
    qDebug() << "that is " << m_time.tv_sec << " ms from 01-01-1970";
    emit timeReceived(m_time);
}

void HttpMiddleware::healthPost(const QByteArray &arr)
{
    const QJsonObject json = byteArrayToJsonObject(arr);
    QString healthString = json["Health"].toString();
    qDebug() << "Health code received: " << healthString;
    bool ok;
    uint32_t code = healthString.toUInt(&ok, 16);
    emit healthReceived(code);
}

void HttpMiddleware::timeZonePost(const QByteArray &arr)
{
    const QJsonObject json = byteArrayToJsonObject(arr);
    QString timeZoneString = json["Timezone"].toString();
    qDebug() << "Timezone received: " << timeZoneString;
    bool ok;
    int8_t timeZone = timeZoneString.toInt(&ok);
    emit timeZoneReceived(timeZone);
}

void HttpMiddleware::setBooterConnectionLost()
{
    emit booterConnectionIsLost();
}

void HttpMiddleware::setTimeStamp(const timespec &systemTime)
{
    m_time = systemTime;
}

void HttpMiddleware::setNtpState(int state)
{
    m_ntpState = state;
}

void HttpMiddleware::setBlock(const DataTypes::BlockStruct &blk)
{
    if (blk.ID == AVTUK_CCU::MainBlock)
    {
        AVTUK_CCU::Main powerStatus;
        memcpy(&powerStatus, blk.data.data(), sizeof(powerStatus));
        m_pwrIn = powerStatus.PWRIN;
        m_resetReq = powerStatus.resetReq;
    }
}

QJsonObject HttpMiddleware::byteArrayToJsonObject(const QByteArray &arr)
{
    const auto json = QJsonDocument::fromJson(arr);
    return json.object();
}
