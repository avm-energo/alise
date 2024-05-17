#include "httpapiserver.h"

#include <QJsonObject>

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
    const auto rport = m_server.listen(QHostAddress::Any, port);
    if (!rport)
        return false;
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

    // POST requests
    m_server.route("/sethealth", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest &request) {
        m_mw->healthPost(request.body());
        return QHttpServerResponse(QHttpServerResponder::StatusCode::Created);
    });

    m_server.route("/settimestamp", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest &request) {
        m_mw->timePost(request.body());
        return QHttpServerResponse(QHttpServerResponder::StatusCode::Created);
    });
}

void HttpApiServer::pingTimeoutTimerTimeout()
{
    m_mw->setBooterConnectionLost();
}
