#include "main-widget.h"
#include "ui_main-widget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    ui->ReceiveSendSplitter->setSizes(QList<int>{350, 60});
}

MainWidget::~MainWidget()
{
    delete ui;
}
