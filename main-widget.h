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
    void on_TcpSocket_errorOccurred(QAbstractSocket::SocketError socketError);
    void on_TcpSocket_connected();
    void on_TcpSocket_readyRead();
    void on_TcpSocket_disconnected();
    void on_UdpSocket_readyRead();

    void on_ConnectPushButton_clicked();
    void on_SendPushButton_clicked();
    void on_ResetCounterPushButton_clicked();
    void on_ClearHistoryPushButton_clicked();
    void on_ProtocolComboBox_currentTextChanged(const QString &optString);

private:
    Ui::MainWidget *ui;
    QTcpSocket *tcp_sock;
    QUdpSocket *udp_sock;

    bool connected;
};

#endif
