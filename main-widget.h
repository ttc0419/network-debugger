#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>
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

    inline void setProtocolInputDisabled(bool disable) noexcept
    {
        ui->ProtocolLabel->setDisabled(disable);
        ui->ProtocolComboBox->setDisabled(disable);
    }

    inline void setLocalInputDisabled(bool disable) noexcept
    {
        ui->LocalAddressLabel->setDisabled(disable);
        ui->LocalAddressLineEdit->setDisabled(disable);
        ui->LocalPortLabel->setDisabled(disable);
        ui->LocalPortLineEdit->setDisabled(disable);
    }

    inline void setLocalInputVisibility(bool visible) noexcept
    {
        ui->LocalAddressLabel->setVisible(visible);
        ui->LocalAddressLineEdit->setVisible(visible);
        ui->LocalPortLabel->setVisible(visible);
        ui->LocalPortLineEdit->setVisible(visible);
    }

    inline void setRemoteInputDisabled(bool disable) noexcept
    {
        ui->RemoteAddressLabel->setDisabled(disable);
        ui->RemoteAddressLineEdit->setDisabled(disable);
        ui->RemotePortLabel->setDisabled(disable);
        ui->RemotePortLineEdit->setDisabled(disable);
    }

    inline void setRemoteInputVisibility(bool visible) noexcept
    {
        ui->RemoteAddressLabel->setVisible(visible);
        ui->RemoteAddressLineEdit->setVisible(visible);
        ui->RemotePortLabel->setVisible(visible);
        ui->RemotePortLineEdit->setVisible(visible);
    }
};

#endif
