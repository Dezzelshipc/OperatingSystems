#include "changeserverwidget.h"
#include "ui_changeserverwidget.h"

#include "Utility/config.h"

ChangeServerWidget::ChangeServerWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChangeServerWidget)
{
    ui->setupUi(this);

    ui->textEditHost->setText(Config::host_ip);
    ui->textEditPort->setText(QString::number(Config::host_port));
}

ChangeServerWidget::~ChangeServerWidget()
{
    delete ui;
}

void ChangeServerWidget::on_buttonBox_accepted()
{
    auto ip = ui->textEditHost->toPlainText();
    Config::host_ip = ip.isEmpty() ? Config::Default::host_ip : ip;

    auto port_s = ui->textEditPort->toPlainText();
    auto port = port_s.toShort();
    Config::host_port = port ? port : Config::Default::host_port;
}

