#include "main-widget.h"
#include "ui_main-widget.h"

#include <QMessageBox>
#include <QNetworkDatagram>
#include <QDateTime>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWidget),
    tcpSocket(new QTcpSocket(this)), udpSocket(new QUdpSocket(this)),
    bConnected(false)
{
    /* Setup Widgets */
    ui->setupUi(this);
    ui->ReceiveSendSplitter->setSizes(QList<int>{350, 60});
    ui->LocalAddressLabel->hide();
    ui->LocalAddressLineEdit->hide();
    ui->LocalPortLabel->hide();
    ui->LocalPortLineEdit->hide();

    /* TCP socket signals */
    connect(tcpSocket, &QTcpSocket::connected, this, &MainWidget::on_tcp_client_connected);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &MainWidget::on_tcp_client_error);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWidget::on_tcp_client_ready_read);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MainWidget::on_tcp_client_disconnected_from_host);

    /* UDP signals */
    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWidget::on_udp_ready_read);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete tcpSocket;
    delete udpSocket;
}

void MainWidget::on_tcp_client_error(QAbstractSocket::SocketError socketError)
{
    tcpSocket->disconnectFromHost();
    tcpSocket->close();

    ui->ProtocolLabel->setDisabled(false);
    ui->ProtocolComboBox->setDisabled(false);
    ui->RemoteAddressLabel->setDisabled(false);
    ui->RemoteAddressLineEdit->setDisabled(false);
    ui->RemotePortLabel->setDisabled(false);
    ui->RemotePortLineEdit->setDisabled(false);
    ui->ConnectPushButton->setText("Connect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(true);
    ui->StatusLabel->setText("Status: TCP Connection Failed, Code: " + QString::number(socketError));

    bConnected = false;
}

void MainWidget::on_tcp_client_connected()
{
    ui->ConnectPushButton->setText("Disconnect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(false);
    ui->StatusLabel->setText("Status: TCP Host Conected");

    bConnected = true;
}

void MainWidget::on_tcp_client_ready_read()
{
    QByteArray tcpData = tcpSocket->readAll();
    ui->ReceiveTextBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + tcpData.data());
    ui->RXValueLabel->setText(QString::number(ui->RXValueLabel->text().toULongLong() + tcpData.size()));
}

void MainWidget::on_tcp_client_disconnected_from_host()
{
    tcpSocket->close();

    ui->ProtocolLabel->setDisabled(false);
    ui->ProtocolComboBox->setDisabled(false);
    ui->RemoteAddressLabel->setDisabled(false);
    ui->RemoteAddressLineEdit->setDisabled(false);
    ui->RemotePortLabel->setDisabled(false);
    ui->RemotePortLineEdit->setDisabled(false);
    ui->ConnectPushButton->setText("Connect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(true);
    ui->StatusLabel->setText("Status: TCP Connection Disconnected by the Host");

    bConnected = false;
}

void MainWidget::on_udp_ready_read()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        ui->ReceiveTextBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + datagram.data());
        ui->RXValueLabel->setText(QString::number(ui->RXValueLabel->text().toULongLong() + datagram.data().size()));
    }
}

void MainWidget::on_ConnectPushButton_clicked()
{
    if (!bConnected) {
        bool bPortValid;
        QHostAddress addr = QHostAddress(ui->LocalAddressLineEdit->text());
        quint16 port = ui->LocalPortLineEdit->text().toUShort(&bPortValid);

        /* If the address and port are valid, connect to the host or bind the address given */
        if (bPortValid && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            if (ui->ProtocolComboBox->currentText() == "TCP Client") {
                tcpSocket->connectToHost(addr, port);

                /* Update TCP client specific widgets */
                ui->RemoteAddressLabel->setDisabled(true);
                ui->RemoteAddressLineEdit->setDisabled(true);
                ui->RemotePortLabel->setDisabled(true);
                ui->RemotePortLineEdit->setDisabled(true);
                ui->ConnectPushButton->setDisabled(true);
                ui->ConnectPushButton->setText("Connecting...");
                ui->StatusLabel->setText("Status: Connecting to the host...");
            } else if (ui->ProtocolComboBox->currentText() == "UDP") {
                udpSocket->bind(addr, port);

                /* Update widgets */
                ui->LocalAddressLabel->setDisabled(true);
                ui->LocalAddressLineEdit->setDisabled(true);
                ui->LocalPortLabel->setDisabled(true);
                ui->LocalPortLineEdit->setDisabled(true);
                ui->RemoteAddressLabel->setDisabled(false);
                ui->RemoteAddressLineEdit->setDisabled(false);
                ui->RemotePortLabel->setDisabled(false);
                ui->RemotePortLineEdit->setDisabled(false);
                ui->SendPushButton->setEnabled(true);
                ui->ConnectPushButton->setText("Disconnect");
                ui->StatusLabel->setText("Status: UDP Host Connected");

                bConnected = true;
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
            tcpSocket->disconnectFromHost();
            tcpSocket->close();

            ui->RemoteAddressLabel->setDisabled(false);
            ui->RemoteAddressLineEdit->setDisabled(false);
            ui->RemotePortLabel->setDisabled(false);
            ui->RemotePortLineEdit->setDisabled(false);
        } else if (ui->ProtocolComboBox->currentText() == "UDP") {
            udpSocket->close();

            ui->RemoteAddressLabel->setDisabled(true);
            ui->RemoteAddressLineEdit->setDisabled(true);
            ui->RemotePortLabel->setDisabled(true);
            ui->RemotePortLineEdit->setDisabled(true);
        }

        /* Update Widgets */
        ui->ProtocolLabel->setDisabled(false);
        ui->ProtocolComboBox->setDisabled(false);
        ui->LocalAddressLabel->setDisabled(false);
        ui->LocalAddressLineEdit->setDisabled(false);
        ui->LocalPortLabel->setDisabled(false);
        ui->LocalPortLineEdit->setDisabled(false);
        ui->SendPushButton->setEnabled(false);
        ui->ConnectPushButton->setText("Connect");
        ui->StatusLabel->setText("Status: Disconnected");

        /* Update bConnected */
        bConnected = false;
    }
}

void MainWidget::on_SendPushButton_clicked()
{
    bool bPortValid;
    QHostAddress addr = QHostAddress(ui->RemoteAddressLineEdit->text());
    quint16 port = ui->RemotePortLineEdit->text().toUShort(&bPortValid);

    if (bPortValid && addr.protocol() == QAbstractSocket::IPv4Protocol) {
        if (ui->ProtocolComboBox->currentText() == "TCP Client") {
            tcpSocket->write(ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size());
            ui->StatusLabel->setText("Status: TCP Message Sent");
        } else if (ui->ProtocolComboBox->currentText() == "UDP") {
            udpSocket->writeDatagram((const char *)(ui->SendTextEdit->toPlainText().toUtf8().data()),
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
        /* Hide and show widgets */
        ui->LocalAddressLabel->hide();
        ui->LocalAddressLineEdit->hide();
        ui->LocalPortLabel->hide();
        ui->LocalPortLineEdit->hide();

        /* Enable and disable widgets */
        ui->RemoteAddressLabel->setDisabled(false);
        ui->RemoteAddressLineEdit->setDisabled(false);
        ui->RemotePortLabel->setDisabled(false);
        ui->RemotePortLineEdit->setDisabled(false);
    } else if (optString == "UDP") {
        /* Show local network settings again */
        ui->LocalAddressLabel->show();
        ui->LocalAddressLineEdit->show();
        ui->LocalPortLabel->show();
        ui->LocalPortLineEdit->show();

        /* Enable and disable widgets */
        ui->RemoteAddressLabel->setDisabled(true);
        ui->RemoteAddressLineEdit->setDisabled(true);
        ui->RemotePortLabel->setDisabled(true);
        ui->RemotePortLineEdit->setDisabled(true);
    }
}
