#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
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
    void on_udp_ready_read();

    void on_ConnectPushButton_clicked();
    void on_SendPushButton_clicked();
    void on_ResetCounterPushButton_clicked();

private:
    Ui::MainWidget *ui;
    QUdpSocket *udpSocket;

    bool bConnected;
};

#endif
