#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>

#include "ui_main-widget.h"

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
    void on_TcpServer_newConnection();
    void on_TcpServer_client_readyRead();
    void on_TcpServer_client_disconnected();
    void on_UdpSocket_readyRead();

    void on_ConnectPushButton_clicked();
    void on_SendPushButton_clicked();
    void on_ResetCounterPushButton_clicked();
    void on_ClearHistoryPushButton_clicked();
    void on_ProtocolComboBox_currentTextChanged(const QString &optString);

private:
    Ui::MainWidget *ui;
    QTcpSocket *tcpSock;
    QTcpServer *tcpServer;
    QUdpSocket *udpSock;

    bool bConnected;
    QList<QTcpSocket *> clientList;

    void sendData() noexcept;

    inline void setProtocolInputDisabled(bool disable) const noexcept
    {
        ui->ProtocolLabel->setDisabled(disable);
        ui->ProtocolComboBox->setDisabled(disable);
    }

    inline void setLocalInputVisibility(bool visible) const noexcept
    {
        ui->LocalAddressLabel->setVisible(visible);
        ui->LocalAddressLineEdit->setVisible(visible);
        ui->LocalPortLabel->setVisible(visible);
        ui->LocalPortLineEdit->setVisible(visible);
    }

    inline void setLocalInputDisabled(bool disable) const noexcept
    {
        ui->LocalAddressLabel->setDisabled(disable);
        ui->LocalAddressLineEdit->setDisabled(disable);
        ui->LocalPortLabel->setDisabled(disable);
        ui->LocalPortLineEdit->setDisabled(disable);
    }

    inline void setRemoteInputVisibility(bool visible) const noexcept
    {
        ui->RemoteAddressLabel->setVisible(visible);
        ui->RemoteAddressLineEdit->setVisible(visible);
        ui->RemotePortLabel->setVisible(visible);
        ui->RemotePortLineEdit->setVisible(visible);
    }

    inline void setRemoteInputDisabled(bool disable) const noexcept
    {
        ui->RemoteAddressLabel->setDisabled(disable);
        ui->RemoteAddressLineEdit->setDisabled(disable);
        ui->RemotePortLabel->setDisabled(disable);
        ui->RemotePortLineEdit->setDisabled(disable);
    }

    inline void setConnectionListVisibility(bool visible) const noexcept
    {
        ui->ConnectionListLabel->setVisible(visible);
        ui->ConnectionListComboBox->setVisible(visible);
    }

    inline void setConnectionListDisabled(bool disable) const noexcept
    {
        ui->ConnectionListLabel->setDisabled(disable);
        ui->ConnectionListComboBox->setDisabled(disable);
    }
};

#endif
