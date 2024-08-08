#ifndef OCPP_H
#define OCPP_H

#include <QMainWindow>
#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class OCPP;
}
QT_END_NAMESPACE

class OCPP : public QMainWindow
{
    Q_OBJECT

signals:

public slots:
    void start();
    void quit();
    void newConnection();
    void disconnected();
    void readyRead();
    void processTextMessage(QString message);
    void onNewConnection();
    void socketDisconnected();
public:
    OCPP(QWidget *parent = nullptr);
    ~OCPP();

private slots:
    void on_connectBtn_clicked();

    void on_disconnectBtn_clicked();

    void on_readyReadBtn_clicked();

private:
    Ui::OCPP *ui;
    QTcpServer server;
    QWebSocketServer *webSocketServer;
    QList<QWebSocket *> clients;
};
#endif // OCPP_H
