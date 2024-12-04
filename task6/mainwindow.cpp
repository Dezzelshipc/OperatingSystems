#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Network/client.h"
#include "Network/clientmanager.h"

#include "Widgets/aboutwidget.h"
#include "Widgets/changeserverwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), translator(new QTranslator)
{
    mainInstance = this;
    connect(this, &MainWindow::languageChanged, this, &MainWindow::retranslate);

    ui->setupUi(this);
    changeLanguage("ru_RU");

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

