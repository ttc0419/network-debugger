#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void on_tcp_client_error(QAbstractSocket::SocketError socketError);
    void on_tcp_client_connected();
    void on_tcp_client_ready_read();
    void on_tcp_client_disconnected_from_host();
    void on_udp_ready_read();

    void on_ConnectPushButton_clicked();
    void on_SendPushButton_clicked();
    void on_ResetCounterPushButton_clicked();
    void on_ClearHistoryPushButton_clicked();
    void on_ProtocolComboBox_currentTextChanged(const QString &optString);

private:
    Ui::MainWidget *ui;
    QTcpSocket *tcpSocket;
    QUdpSocket *udpSocket;

    bool bConnected;
};

#endif
