#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Network/client.h"
#include "Network/clientmanager.h"

#include "Widgets/aboutwidget.h"
#include "Widgets/changeserverwidget.h"

#include "Utility/parser.h"

#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), translator(new QTranslator), client(new Client)
{
    mainInstance = this;
    connect(this, &MainWindow::languageChanged, this, &MainWindow::retranslate);

    ui->setupUi(this);
    changeLanguage("ru_RU");


    connect(client, &Client::recievedData, this, [=](const QString& str_recv)
            {
                auto str = str_recv.trimmed();
                auto parsed = Parser::ParseFromString(str);
                ui->scrollableText->setText(Parser::GetFromList(parsed));

                ui->tableWidget->clearContents();
                ui->tableWidget->setRowCount(parsed.size());
                ui->tableWidget->setColumnCount(2);
                ui->tableWidget->setColumnWidth(0, 200);
                for (int i = 0; i < parsed.size(); ++i)
                {
                    QTableWidgetItem* item_d = new QTableWidgetItem(parsed[i].datetime.toString("dd.MM.yyyy hh.mm.ss"));
                    ui->tableWidget->setItem(i, 0, item_d);

                    QTableWidgetItem* item_t = new QTableWidgetItem(QString::number( parsed[i].temp ));
                    ui->tableWidget->setItem(i, 1, item_t);
                }
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}

const MainWindow* MainWindow::instance()
{
    return mainInstance;
}

void MainWindow::changeLanguage(const QString& lang)
{
    if (!translator.isNull())
    {
        QApplication::removeTranslator(translator);
    }

    // ":/" path refers to embedded recources. see .qrc file and its inclusion in .pro
    Q_UNUSED(translator->load("task6_"+lang, ":/Localization"));
    QApplication::instance()->installTranslator(translator);

    emit languageChanged();
}

void MainWindow::retranslate()
{
    ui->retranslateUi(this);
}

void MainWindow::on_action_ChangeServer_triggered()
{
    QPointer<ChangeServerWidget> csw = new ChangeServerWidget;
    csw->show();
}


void MainWindow::on_action_About_triggered()
{
    QPointer<AboutWidget> about = new AboutWidget;
    about->show();
}

void MainWindow::on_actionEnglish_triggered()
{
    changeLanguage("en_US");
}


void MainWindow::on_actionRussian_triggered()
{
    changeLanguage("ru_RU");
}


void MainWindow::on_pushButton_log_sec_clicked()
{
    ClientManager::getLog(client, ClientManager::SEC);
}


void MainWindow::on_pushButton_log_hour_clicked()
{
    ClientManager::getLog(client, ClientManager::HOUR);
}


void MainWindow::on_pushButton_log_day_clicked()
{
    ClientManager::getLog(client, ClientManager::DAY);
}

