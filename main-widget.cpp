#include "main-widget.h"

#include <QMessageBox>
#include <QNetworkDatagram>
#include <QDateTime>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWidget),
    tcp_sock(new QTcpSocket(this)), tcp_server(new QTcpServer(this)),
    udp_sock(new QUdpSocket(this)), connected(false), client_list(QList<QTcpSocket *>{nullptr})
{
    /* Set object names to let QT connect slots automatically in setupUi() below */
    tcp_sock->setObjectName("TcpSocket");
    tcp_server->setObjectName("TcpServer");
    udp_sock->setObjectName("UdpSocket");

    /* Setup widgets */
    ui->setupUi(this);
    ui->ReceiveSendSplitter->setSizes(QList<int>{350, 60});
    setLocalInputVisibility(false);
    setConnectionListVisibility(false);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete tcp_sock;
    delete tcp_server;
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

void MainWidget::on_TcpServer_newConnection()
{
    while(tcp_server->hasPendingConnections()) {
        client_list.append(tcp_server->nextPendingConnection());

        connect(client_list.back(), &QTcpSocket::readyRead, this, &MainWidget::on_TcpServer_client_readyRead);
        connect(client_list.back(), &QTcpSocket::disconnected, this, &MainWidget::on_TcpServer_client_disconnected);

        QString conn = client_list.back()->peerAddress().toString() + ':' + QString::number(client_list.back()->peerPort());
        ui->ConnectionListComboBox->addItem(conn);
        ui->StatusLabel->setText("Status: " + conn + " Accepted");
    }
}

void MainWidget::on_TcpServer_client_readyRead()
{
    QTcpSocket *sock = qobject_cast<QTcpSocket *>(QObject::sender());
    QByteArray tcpData = sock->readAll();
    ui->ReceiveTextBrowser->append(QDateTime::currentDateTime().toString(
        "[" + sock->peerAddress().toString() + ':' + QString::number(sock->peerPort()) + " hh:mm:ss] ") + tcpData.data());
    ui->RXValueLabel->setText(QString::number(ui->RXValueLabel->text().toULongLong() + tcpData.size()));
}

void MainWidget::on_TcpServer_client_disconnected()
{
    QTcpSocket *sock = qobject_cast<QTcpSocket *>(QObject::sender());
    client_list.removeOne(sock);

    int i = 0;
    QString conn = sock->peerAddress().toString() + ':' + QString::number(sock->peerPort());

    while (ui->ConnectionListComboBox->itemText(i) != conn)
        i++;

    ui->ConnectionListComboBox->removeItem(i);
    ui->StatusLabel->setText("Status: " + conn + " Disconnected");
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
            } else if (ui->ProtocolComboBox->currentText() == "TCP Server") {
                tcp_server->listen(addr, port);

                /* Update TCP server specific widgets */
                setLocalInputDisabled(true);
                setConnectionListVisibility(true);
                setConnectionListDisabled(false);
                ui->SendPushButton->setEnabled(true);
                ui->ConnectPushButton->setText("Disconnect");
                ui->StatusLabel->setText("Status: TCP Server Started");

                connected = true;
            } else if (ui->ProtocolComboBox->currentText() == "UDP") {
                udp_sock->bind(addr, port);

                /* Update UDP specific widgets */
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
        } else if (ui->ProtocolComboBox->currentText() == "TCP Server") {
            while (client_list.back() != nullptr)
                client_list.back()->disconnectFromHost();

            tcp_server->close();
            ui->ConnectionListComboBox->clear();
            ui->ConnectionListComboBox->addItem("All Connections");
            setConnectionListDisabled(true);
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
        } else if (ui->ProtocolComboBox->currentText() == "TCP Server") {
            int index = ui->ConnectionListComboBox->currentIndex();
            if (index == 0) {
                for (qsizetype i = 1; i < client_list.size(); i++)
                    client_list[i]->write(
                        ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size());
            } else {
                client_list[index]->write(
                    ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size());
            }
        } else if (ui->ProtocolComboBox->currentText() == "UDP") {
            udp_sock->writeDatagram((const char *)(ui->SendTextEdit->toPlainText().toUtf8().data()),
                ui->SendTextEdit->toPlainText().size(), addr, port);
        }

        ui->StatusLabel->setText("Status: Data Sent");
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
        setRemoteInputVisibility(true);
        setRemoteInputDisabled(false);
        setConnectionListVisibility(false);
    } else if (optString == "TCP Server") {
        setLocalInputVisibility(true);
        setLocalInputDisabled(false);
        setRemoteInputVisibility(false);
        setConnectionListVisibility(true);
    } else if (optString == "UDP") {
        setLocalInputVisibility(true);
        setLocalInputDisabled(false);
        setRemoteInputVisibility(true);
        setRemoteInputDisabled(true);
        setConnectionListVisibility(false);
    }
}
