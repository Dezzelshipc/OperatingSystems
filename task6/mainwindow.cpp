#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Network/client.h"
#include "Network/clientmanager.h"

#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPointer<Client> client = new Client;

    connect(client, &Client::recievedData, this, [=](QString str)
            {
                qDebug() << str << "!!!";
            });

    ClientManager::getLog(client, ClientManager::HOUR);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_action_ChangeServer_triggered()
{
    qDebug() << tr("asd");
}


void MainWindow::on_action_About_triggered()
{
    QPointer<QWidget> about;
}

