#include "main-widget.h"

#include <QMessageBox>
#include <QNetworkDatagram>
#include <QDateTime>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWidget),
    tcp_sock(new QTcpSocket(this)), udp_sock(new QUdpSocket(this)),
    connected(false)
{
    /* Set object names to let QT connect slots automatically in setupUi() below */
    tcp_sock->setObjectName("TcpSocket");
    udp_sock->setObjectName("UdpSocket");

    /* Setup widgets */
    ui->setupUi(this);
    ui->ReceiveSendSplitter->setSizes(QList<int>{350, 60});
    setLocalInputVisibility(false);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete tcp_sock;
    delete udp_sock;
}

void MainWidget::on_TcpSocket_errorOccurred(QAbstractSocket::SocketError socketError)
{
    tcp_sock->disconnectFromHost();
    tcp_sock->close();

    setProtocolInputDisabled(false);
    setRemoteInputDisabled(false);
    ui->ConnectPushButton->setText("Connect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(true);
    ui->StatusLabel->setText("Status: TCP Connection Failed, Code: " + QString::number(socketError));

    connected = false;
}

void MainWidget::on_TcpSocket_connected()
{
    ui->ConnectPushButton->setText("Disconnect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(false);
    ui->StatusLabel->setText("Status: TCP Host Conected");

    connected = true;
}

void MainWidget::on_TcpSocket_readyRead()
{
    QByteArray tcpData = tcp_sock->readAll();
    ui->ReceiveTextBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + tcpData.data());
    ui->RXValueLabel->setText(QString::number(ui->RXValueLabel->text().toULongLong() + tcpData.size()));
}

void MainWidget::on_TcpSocket_disconnected()
{
    tcp_sock->close();

    setProtocolInputDisabled(false);
    setRemoteInputDisabled(false);
    ui->ConnectPushButton->setText("Connect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(true);
    ui->StatusLabel->setText("Status: TCP Connection Disconnected by the Host");

    connected = false;
}

void MainWidget::on_UdpSocket_readyRead()
{
    while (udp_sock->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udp_sock->receiveDatagram();
        ui->ReceiveTextBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + datagram.data());
        ui->RXValueLabel->setText(QString::number(ui->RXValueLabel->text().toULongLong() + datagram.data().size()));
    }
}

void MainWidget::on_ConnectPushButton_clicked()
{
    if (!connected) {
        bool bPortValid;
        QHostAddress addr = QHostAddress(ui->LocalAddressLineEdit->text());
        quint16 port = ui->LocalPortLineEdit->text().toUShort(&bPortValid);

        /* If the address and port are valid, connect to the host or bind the address given */
        if (bPortValid && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            if (ui->ProtocolComboBox->currentText() == "TCP Client") {
                tcp_sock->connectToHost(addr, port);

                /* Update TCP client specific widgets */
                setRemoteInputDisabled(true);
                ui->ConnectPushButton->setDisabled(true);
                ui->ConnectPushButton->setText("Connecting...");
                ui->StatusLabel->setText("Status: Connecting to the host...");
            } else if (ui->ProtocolComboBox->currentText() == "UDP") {
                udp_sock->bind(addr, port);

                /* Update widgets */
                setLocalInputDisabled(true);
                setRemoteInputDisabled(false);
                ui->SendPushButton->setEnabled(true);
                ui->ConnectPushButton->setText("Disconnect");
                ui->StatusLabel->setText("Status: UDP Host Connected");

                connected = true;
            }

            /* Update common widgets */
            ui->ProtocolLabel->setDisabled(true);
            ui->ProtocolComboBox->setDisabled(true);
        /* Open a warning message box otherwise */
        } else {
            QMessageBox::warning(this, "Warning", "Local IPv4 address or the port is not valid!");
        }
    } else {
        if (ui->ProtocolComboBox->currentText() == "TCP Client") {
            tcp_sock->disconnectFromHost();
            tcp_sock->close();
            setRemoteInputDisabled(false);
        } else if (ui->ProtocolComboBox->currentText() == "UDP") {
            udp_sock->close();
            setRemoteInputDisabled(true);
        }

        /* Update Widgets */
        setProtocolInputDisabled(false);
        setLocalInputDisabled(false);
        ui->SendPushButton->setEnabled(false);
        ui->ConnectPushButton->setText("Connect");
        ui->StatusLabel->setText("Status: Disconnected");

        connected = false;
    }
}

void MainWidget::on_SendPushButton_clicked()
{
    bool bPortValid;
    QHostAddress addr = QHostAddress(ui->RemoteAddressLineEdit->text());
    quint16 port = ui->RemotePortLineEdit->text().toUShort(&bPortValid);

    if (bPortValid && addr.protocol() == QAbstractSocket::IPv4Protocol) {
        if (ui->ProtocolComboBox->currentText() == "TCP Client") {
            tcp_sock->write(ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size());
            ui->StatusLabel->setText("Status: TCP Message Sent");
        } else if (ui->ProtocolComboBox->currentText() == "UDP") {
            udp_sock->writeDatagram((const char *)(ui->SendTextEdit->toPlainText().toUtf8().data()),
                ui->SendTextEdit->toPlainText().size(), addr, port);
            ui->StatusLabel->setText("Status: UDP Message Sent");
        }
        ui->TXValueLabel->setText(QString::number(ui->TXValueLabel->text().toULongLong() + ui->SendTextEdit->toPlainText().size()));
    } else {
        QMessageBox::warning(this, "Warning", "Remote IPv4 address or the port is not valid!");
    }
}

void MainWidget::on_ResetCounterPushButton_clicked()
{
    ui->RXValueLabel->setText("0");
    ui->TXValueLabel->setText("0");
    ui->StatusLabel->setText("Status: RX and TX Counter Cleared");
}

void MainWidget::on_ClearHistoryPushButton_clicked()
{
    ui->ReceiveTextBrowser->clear();
    ui->StatusLabel->setText("Status: History Cleared");
}

void MainWidget::on_ProtocolComboBox_currentTextChanged(const QString &optString)
{
    if (optString == "TCP Client") {
        setLocalInputVisibility(false);
        setRemoteInputDisabled(false);
    } else if (optString == "UDP") {
        setLocalInputVisibility(true);
        setRemoteInputDisabled(true);
    }
}
