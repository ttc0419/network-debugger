#include "main-widget.h"

#include <QMessageBox>
#include <QNetworkDatagram>
#include <QDateTime>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWidget),
    tcpSock(new QTcpSocket(this)), tcpServer(new QTcpServer(this)), udpSock(new QUdpSocket(this)),
    bConnected(false), loopTimerId(0), clientList(QList<QTcpSocket *>{nullptr})
{
    /* Set object names to let QT connect slots automatically in setupUi() below */
    tcpSock->setObjectName("TcpSocket");
    tcpServer->setObjectName("TcpServer");
    udpSock->setObjectName("UdpSocket");

    /* Setup widgets */
    ui->setupUi(this);
    ui->ReceiveSendSplitter->setSizes(QList<int>{350, 60});
    setLocalInputVisibility(false);
    setConnectionListVisibility(false);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete tcpSock;
    delete tcpServer;
    delete udpSock;
}

void MainWidget::on_TcpSocket_errorOccurred(QAbstractSocket::SocketError socketError)
{
    tcpSock->disconnectFromHost();
    tcpSock->close();

    setProtocolInputDisabled(false);
    setRemoteInputDisabled(false);
    ui->ConnectPushButton->setText("Connect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(true);
    ui->StatusLabel->setText("Status: TCP Connection Failed, Code: " + QString::number(socketError));

    bConnected = false;
}

void MainWidget::on_TcpSocket_connected()
{
    ui->ConnectPushButton->setText("Disconnect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(false);
    ui->StatusLabel->setText("Status: TCP Host Conected");

    bConnected = true;
}

void MainWidget::on_TcpSocket_readyRead()
{
    QByteArray tcpData = tcpSock->readAll();
    ui->ReceiveTextBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + tcpData.data());
    ui->RXValueLabel->setText(QString::number(ui->RXValueLabel->text().toULongLong() + tcpData.size()));
}

void MainWidget::on_TcpSocket_disconnected()
{
    tcpSock->close();

    setProtocolInputDisabled(false);
    setRemoteInputDisabled(false);
    ui->ConnectPushButton->setText("Connect");
    ui->ConnectPushButton->setDisabled(false);
    ui->SendPushButton->setDisabled(true);
    ui->StatusLabel->setText("Status: TCP Connection Disconnected by the Host");

    bConnected = false;
}

void MainWidget::on_TcpServer_newConnection()
{
    while(tcpServer->hasPendingConnections()) {
        clientList.append(tcpServer->nextPendingConnection());

        connect(clientList.back(), &QTcpSocket::readyRead, this, &MainWidget::on_TcpServer_client_readyRead);
        connect(clientList.back(), &QTcpSocket::disconnected, this, &MainWidget::on_TcpServer_client_disconnected);

        QString conn = clientList.back()->peerAddress().toString() + ':' + QString::number(clientList.back()->peerPort());
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
    clientList.removeOne(sock);

    int i = 0;
    QString conn = sock->peerAddress().toString() + ':' + QString::number(sock->peerPort());

    while (ui->ConnectionListComboBox->itemText(i) != conn)
        i++;

    ui->ConnectionListComboBox->removeItem(i);
    ui->StatusLabel->setText("Status: " + conn + " Disconnected");
}

void MainWidget::on_UdpSocket_readyRead()
{
    while (udpSock->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSock->receiveDatagram();
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
                tcpSock->connectToHost(addr, port);

                /* Update TCP client specific widgets */
                setRemoteInputDisabled(true);
                ui->ConnectPushButton->setDisabled(true);
                ui->ConnectPushButton->setText("Connecting...");
                ui->StatusLabel->setText("Status: Connecting to the host...");
            } else if (ui->ProtocolComboBox->currentText() == "TCP Server") {
                tcpServer->listen(addr, port);

                /* Update TCP server specific widgets */
                setLocalInputDisabled(true);
                setConnectionListVisibility(true);
                setConnectionListDisabled(false);
                ui->SendPushButton->setEnabled(true);
                ui->ConnectPushButton->setText("Disconnect");
                ui->StatusLabel->setText("Status: TCP Server Started");

                bConnected = true;
            } else if (ui->ProtocolComboBox->currentText() == "UDP") {
                udpSock->bind(addr, port);

                /* Update UDP specific widgets */
                setLocalInputDisabled(true);
                setRemoteInputDisabled(false);
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
            tcpSock->disconnectFromHost();
            tcpSock->close();
            setRemoteInputDisabled(false);
        } else if (ui->ProtocolComboBox->currentText() == "TCP Server") {
            while (clientList.back() != nullptr)
                clientList.back()->disconnectFromHost();

            tcpServer->close();
            ui->ConnectionListComboBox->clear();
            ui->ConnectionListComboBox->addItem("All Connections");
            setConnectionListDisabled(true);
        } else if (ui->ProtocolComboBox->currentText() == "UDP") {
            udpSock->close();
            setRemoteInputDisabled(true);
        }

        /* Update Widgets */
        setProtocolInputDisabled(false);
        setLocalInputDisabled(false);
        ui->SendPushButton->setEnabled(false);
        ui->ConnectPushButton->setText("Connect");
        ui->StatusLabel->setText("Status: Disconnected");

        bConnected = false;
    }
}

void MainWidget::on_SendPushButton_clicked()
{
    if (loopTimerId) {
        killTimer(loopTimerId);
        loopTimerId = 0;
        ui->SendPushButton->setText("Send");
        ui->ConnectPushButton->setDisabled(false);
        ui->StatusLabel->setText("Status: Loop Sending Stopped");
    } else {
        bool bPortValid;
        QHostAddress addr = QHostAddress(ui->RemoteAddressLineEdit->text());
        ui->RemotePortLineEdit->text().toUShort(&bPortValid);

        if (bPortValid && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            if (ui->TXLoopCheckBox->isChecked()) {
                loopTimerId = startTimer(ui->TXLoopIntervalLineEdit->text().toInt(), Qt::TimerType::PreciseTimer);
                ui->SendPushButton->setText("Stop");
                ui->ConnectPushButton->setDisabled(true);
                ui->StatusLabel->setText("Status: Loop Sending Started");
            } else {
                sendData();
                ui->StatusLabel->setText("Status: Data Sent");
            }
        } else {
            QMessageBox::warning(this, "Warning", "Remote IPv4 address or the port is not valid!");
        }
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

void MainWidget::sendData() noexcept
{
    if (ui->ProtocolComboBox->currentText() == "TCP Client") {
        tcpSock->write(ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size());
    } else if (ui->ProtocolComboBox->currentText() == "TCP Server") {
        int index = ui->ConnectionListComboBox->currentIndex();
        if (index == 0) {
            for (qsizetype i = 1; i < clientList.size(); i++)
                clientList[i]->write(
                    ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size());
        } else {
            clientList[index]->write(
                ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size());
        }
    } else if (ui->ProtocolComboBox->currentText() == "UDP") {
        udpSock->writeDatagram(ui->SendTextEdit->toPlainText().toUtf8().data(), ui->SendTextEdit->toPlainText().size(),
            QHostAddress(ui->RemoteAddressLineEdit->text()), ui->RemotePortLineEdit->text().toUShort());
    }

    ui->TXValueLabel->setText(QString::number(ui->TXValueLabel->text().toULongLong() + ui->SendTextEdit->toPlainText().size()));
}
