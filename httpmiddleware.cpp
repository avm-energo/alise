#include "httpmiddleware.h"

#include "alisesettings.h"
#include "avtukccu.h"
#include "gitversion/gitversion.h"

#include <QDateTime>
#include <QString>
#include <config.h>

HttpMiddleware::HttpMiddleware(QObject *parent)
    : QObject{parent}
{
}

QJsonObject HttpMiddleware::helloReply()
{
    QJsonObject json;
    AliseSettings settings;
    GitVersion gitVersion;
    settings.init();
    settings.readSettings();
    json["AliseVersion"] = QString(ALISEVERSION) + "-" + gitVersion.getGitHash();
    json["SerialNum"] = QString::number(settings.serialNum);
    json["HwVersion"] = settings.versionStr(settings.hwVersion);
    json["SwVersion"] = settings.versionStr(settings.swVersion);
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
    json["NTPState"] = (m_ntpState) ? "1" : "0";
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

void HttpMiddleware::setBooterConnectionLost()
{
    emit booterConnectionIsLost();
}

void HttpMiddleware::setTimeStamp(const timespec &systemTime)
{
    m_time = systemTime;
}

void HttpMiddleware::setNtpState(bool state)
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
