#include "ocpp.h"
#include "ui_ocpp.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

void OCPP::start()
{
    if (!server.listen(QHostAddress("192.168.137.1"),8081))
    {
        qWarning() << server.errorString();
        return;
    }

    ui->textEdit->append("INFO:websockets.server:server listening on 192.168.137.1:8081");
    ui->textEdit->append("INFO:root:WebSocket Server Started");
}

void OCPP::quit()
{
    server.close();
}

void OCPP::newConnection()
{
    QTcpSocket* socket = server.nextPendingConnection();
    connect(socket, &QTcpSocket::disconnected, this, &OCPP::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &OCPP::readyRead);

    qInfo() << "Connected" << socket;
    ui->textEdit->append("INFO:websockets.server:connection open");
    ui->textEdit->append("INFO:root:Protocols Matched: ocpp2.0.1");
}

void OCPP::disconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    qInfo() << "Disconnected" << socket;
    qInfo() << "Parent" << socket->parent();

    socket->deleteLater();
}

void OCPP::readyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    qInfo() << "Ready Read" << socket;
    qInfo() << "Parent" << socket->readAll();
}

OCPP::OCPP(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::OCPP)
    , webSocketServer(new QWebSocketServer(QStringLiteral("OCPP Server"),
                                           QWebSocketServer::NonSecureMode, this))
{
    ui->setupUi(this);

    if (webSocketServer->listen(QHostAddress::Any, 8081)) {
        connect(webSocketServer, &QWebSocketServer::newConnection,
                this, &OCPP::onNewConnection);
        qInfo() << "WebSocket Server listening on port 8081";
        ui->textEdit->append("WebSocket Server listening on port 8081");
    } else {
        qWarning() << "WebSocket Server error:" << webSocketServer->errorString();
    }
}

OCPP::~OCPP()
{
    webSocketServer->close();
    qDeleteAll(clients.begin(), clients.end());
    delete ui;
}

void OCPP::on_connectBtn_clicked()
{
    if (!server.listen(QHostAddress::Any,8080))
    {
        qWarning() << server.errorString();
        return;
    }

    qInfo() << "Listening...";
    ui->textEdit->append("INFO:websockets.server:server listening on 192.168.137.1:8081");
    ui->textEdit->append("INFO:root:WebSocket Server Started");
}

void OCPP::onNewConnection()
{
    QWebSocket *pSocket = webSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &OCPP::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &OCPP::socketDisconnected);

    clients << pSocket;

    qDebug() << "New connection established";
    ui->textEdit->append("INFO:websockets.server:connection open");
    ui->textEdit->append("INFO:root:Protocols Matched: ocpp2.0.1");
}

void OCPP::processTextMessage(QString message)
{
    QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());

    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    if (!jsonDoc.isArray()) {
        qWarning() << "OCPP message hasn't the correct format. It should be a list.";
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    QString action = jsonArray.at(2).toString();
    QJsonDocument jsondoc(jsonArray);
    QString jsonString = jsondoc.toJson(QJsonDocument::Compact);
    qDebug() << "INFO: " << message;
    QJsonObject response;
    qDebug() << "Action: " << action;

    if (action == "BootNotification") {
        response["currentTime"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        response["interval"] = 10;
        response["status"] = "Accepted";
        ui->textEdit->append("INFO:" + jsonString);
        qDebug() << response;
    }
    else if (action == "Heartbeat") {
        response["currentTime"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        ui->textEdit->append("INFO: " + jsonString);
    } else if (action == "MeterValues") {
        QJsonObject customData;
        customData["evseId"] = jsonArray.at(3).toObject()["evse_id"];
        customData["vendorId"] = "mrctmgr99";
        response["customData"] = customData;
        ui->textEdit->append("INFO:" + jsonString);
    } else if (action == "StatusNotification") {
        QJsonObject customData;
        customData["connectorId"] = jsonArray.at(3).toObject()["connector_id"];
        customData["vendorId"] = "mrctmgr99";
        response["customData"] = customData;
        ui->textEdit->append("INFO:" + jsonString);
    } else if (action == "TransactionEvent") {
        QJsonObject customData;
        customData["vendor_id"] = "mrctmgr99";
        response["custom_data"] = customData;
        ui->textEdit->append("INFO:" + jsonString);
    } else if (action == "RequestStartTransaction") {
        response["status"] = "Accepted";
        ui->textEdit->append("INFO:" + jsonString);
    }

    QJsonArray responseArray;
    responseArray.append(3);
    responseArray.append(jsonArray.at(1));
    responseArray.append(response);

    QJsonDocument responseDoc(responseArray);
    QString responseString = responseDoc.toJson(QJsonDocument::Compact);
    pSender->sendTextMessage(responseString);
}

void OCPP::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        clients.removeAll(pClient);
        pClient->deleteLater();
    }
    qDebug() << "Client disconnected";
}

void OCPP::on_disconnectBtn_clicked()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    qInfo() << "Disconnected" << socket;
    qInfo() << "Parent" << socket->parent();

    ui->textEdit->append("Disconnected");

    socket->deleteLater();
}


void OCPP::on_readyReadBtn_clicked()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    qInfo() << "Ready Read" << socket;
    qInfo() << "Parent" << socket->readAll();
}

