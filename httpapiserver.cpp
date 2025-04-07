#include "httpapiserver.h"

#include <QJsonObject>
#include <QTcpServer>

HttpApiServer::HttpApiServer(HttpMiddleware *mw, QObject *parent) : QObject { parent }
{
    m_mw = mw;
    m_pingTimeoutTimer = new QTimer;
    connect(m_pingTimeoutTimer, &QTimer::timeout, this, &HttpApiServer::pingTimeoutTimerTimeout);
}

bool HttpApiServer::initServer(int port, int pingTimeout)
{
    m_pingTimeout = pingTimeout;
    setRoutes();
    auto tcpserver = new QTcpServer();
    if (!tcpserver->listen(QHostAddress::Any, port) || !m_server.bind(tcpserver))
    {
        delete tcpserver;
        return false;
    }
    m_pingTimeoutTimer->start(m_pingTimeout);
    return true;
}

void HttpApiServer::setRoutes()
{
    // GET requests

    m_server.route("/", []() { return "hello world"; });
    m_server.route("/hello/<arg>", QHttpServerRequest::Method::Get, [&](QString identity) {
        qDebug() << identity << " [GET] Hello";
        return QHttpServerResponse(m_mw->helloReply());
    });

    m_server.route("/gettimestamp/<arg>", QHttpServerRequest::Method::Get, [&](QString identity) {
        qDebug() << identity << " [GET] TimeStamp";
        return QHttpServerResponse(m_mw->timeStamp());
    });

    m_server.route("/ping/<arg>", QHttpServerRequest::Method::Get, [&](QString identity) {
        qDebug() << identity << " [GET] Ping";
        m_pingTimeoutTimer->start(m_pingTimeout);
        return QHttpServerResponse(m_mw->pingReply());
    });

    m_server.route("/getpowerstate/<arg>", QHttpServerRequest::Method::Get, [&](QString identity) {
        qDebug() << identity << " [GET] PowerState";
        return QHttpServerResponse(m_mw->powerStatus());
    });

    m_server.route("/getntpstate/<arg>", QHttpServerRequest::Method::Get, [&](QString identity) {
        qDebug() << identity << " [GET] NtpState";
        return QHttpServerResponse(m_mw->ntpStatus());
    });

    m_server.route("/gettimezone/<arg>", QHttpServerRequest::Method::Get, [&](QString identity) {
        qDebug() << identity << " [GET] NtpState";
        return QHttpServerResponse(m_mw->timeZone());
    });

    // POST requests
    m_server.route(
        "/sethealth/<arg>", QHttpServerRequest::Method::Post, [&](QString identity, const QHttpServerRequest &request) {
            qDebug() << identity << " [POST] SetHealth";
            m_mw->healthPost(request.body());
            return QHttpServerResponse(m_mw->pingReply());
        });

    m_server.route("/settimestamp/<arg>", QHttpServerRequest::Method::Post,
        [&](QString identity, const QHttpServerRequest &request) {
            qDebug() << identity << " [POST] SetTime";
            m_mw->timePost(request.body());
            return QHttpServerResponse(m_mw->pingReply());
        });

    m_server.route("/settimezone/<arg>", QHttpServerRequest::Method::Post,
        [&](QString identity, const QHttpServerRequest &request) {
            qDebug() << identity << " [POST] SetTimeZone";
            m_mw->timeZonePost(request.body());
            return QHttpServerResponse(m_mw->pingReply());
        });
}

void HttpApiServer::pingTimeoutTimerTimeout()
{
    m_mw->setBooterConnectionLost();
}
