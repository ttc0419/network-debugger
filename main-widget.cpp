#include "main-widget.h"
#include "ui_main-widget.h"

#include <QMessageBox>
#include <QNetworkDatagram>
#include <QDateTime>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWidget),
    udpSocket(new QUdpSocket(this)), bConnected(false)
{
    ui->setupUi(this);
    ui->ReceiveSendSplitter->setSizes(QList<int>{350, 60});

    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWidget::on_udp_ready_read);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete udpSocket;
}

void MainWidget::on_ConnectPushButton_clicked()
{
    if (!bConnected) {
        bool bPortValid;
        QHostAddress addr = QHostAddress(ui->LocalAddressLineEdit->text());
        quint16 port = ui->LocalPortLineEdit->text().toUShort(&bPortValid);

        /* If the address and port are valid, connect to the host or bind the address given */
        if (bPortValid && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            if (ui->ProtocolComboBox->currentText() == "UDP")
                udpSocket->bind(addr, port);

            /* Update Widgets */
            ui->ProtocolLabel->setDisabled(true);
            ui->ProtocolComboBox->setDisabled(true);
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
            ui->StatusLabel->setText("Status: Connected");

            /* Update bConnected */
            bConnected = true;
        /* Open an message box otherwise */
        } else {
            QMessageBox::warning(this, "Warning", "Local IPv4 address or the port is not valid!");
        }
    } else {
        if (ui->ProtocolComboBox->currentText() == "UDP")
            udpSocket->close();

        /* Update Widgets */
        ui->ProtocolLabel->setDisabled(false);
        ui->ProtocolComboBox->setDisabled(false);
        ui->LocalAddressLabel->setDisabled(false);
        ui->LocalAddressLineEdit->setDisabled(false);
        ui->LocalPortLabel->setDisabled(false);
        ui->LocalPortLineEdit->setDisabled(false);
        ui->RemoteAddressLabel->setDisabled(true);
        ui->RemoteAddressLineEdit->setDisabled(true);
        ui->RemotePortLabel->setDisabled(true);
        ui->RemotePortLineEdit->setDisabled(true);
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
        if (ui->ProtocolComboBox->currentText() == "UDP") {
            udpSocket->writeDatagram((const char *)(ui->SendTextEdit->toPlainText().toUtf8().data()),
                ui->SendTextEdit->toPlainText().size(), addr, port);
            ui->StatusLabel->setText("Status: UDP Message Sent");
        }
    } else {
        QMessageBox::warning(this, "Warning", "Remote IPv4 address or the port is not valid!");
    }
}

void MainWidget::on_udp_ready_read()
{
    while(udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        ui->ReceiveTextBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + datagram.data());
    }
}
