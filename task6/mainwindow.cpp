#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Network/client.h"
#include "Network/clientmanager.h"

#include "Widgets/aboutwidget.h"
#include "Widgets/changeserverwidget.h"

#include <QLineSeries>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    translator(new QTranslator),
    client(new Client),
    chart(new QChart)
{
    mainInstance = this;
    connect(this, &MainWindow::languageChanged, this, &MainWindow::retranslate);

    ui->setupUi(this);
    changeLanguage("ru_RU");
    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRubberBand(QChartView::RubberBand::HorizontalRubberBand); // Horizontal zoom (Hold LMB to zoom in, click RMB to zoom out)

    changeChartTheme();

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
                    QTableWidgetItem* item_d = new QTableWidgetItem(parsed[i].datetime.toString("dd.MM.yyyy hh:mm:ss"));
                    ui->tableWidget->setItem(i, 0, item_d);

                    QTableWidgetItem* item_t = new QTableWidgetItem(QString::number( parsed[i].temp ));
                    ui->tableWidget->setItem(i, 1, item_t);
                }

                initSeries(parsed);
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ThemeChange)
    {
        changeChartTheme();
    }
}

void MainWindow::changeChartTheme()
{
    switch (QGuiApplication::styleHints()->colorScheme()) {
    case Qt::ColorScheme::Light:
    case Qt::ColorScheme::Unknown:
        chart->setTheme(QChart::ChartThemeLight);
        break;
    case Qt::ColorScheme::Dark:
        chart->setTheme(QChart::ChartThemeDark);
        break;
    }
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

    // ":/" path refers to embedded recources. See .qrc file and its inclusion in .pro
    Q_UNUSED(translator->load("task6_"+lang, ":/Localization"));
    QApplication::instance()->installTranslator(translator);

    emit languageChanged();
}

void MainWindow::retranslate()
{
    ui->retranslateUi(this);
}

void MainWindow::initSeries(const QList<Parser::DateTemp>& list)
{
    chart->removeAllSeries();
    chart->legend()->hide();
    if (!axisX.isNull())
    {
        chart->removeAxis(axisX);
    }
    if (!axisX.isNull())
    {
        chart->removeAxis(axisY);
    }

    auto *series = new QLineSeries;

    for (int i = 0; i < list.size(); ++i)
    {
        series->append(list[i].datetime.toMSecsSinceEpoch(), list[i].temp);
    }

    chart->addSeries(series);

    QString format;
    switch (last_log_type)
    {
    case ClientManager::SEC:
    case ClientManager::HOUR:
        format = "dd.MM hh:mm:ss";
        break;

    case ClientManager::DAY:
        format = "dd.MM.yyyy";
        break;
    }

    axisX = new QDateTimeAxis;
    axisX->setTickCount(8);
    axisX->setFormat(format);
    axisX->setTitleText(tr("Date"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis;
    axisY->setLabelFormat("%.2f"); // float with 2 decimals
    axisY->setTickCount(9);
    axisY->setTitleText(tr("Trmperature"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
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
    last_log_type = ClientManager::SEC;
    ClientManager::getLog(client, ClientManager::SEC);
}


void MainWindow::on_pushButton_log_hour_clicked()
{
    last_log_type = ClientManager::HOUR;
    ClientManager::getLog(client, ClientManager::HOUR);
}


void MainWindow::on_pushButton_log_day_clicked()
{
    last_log_type = ClientManager::DAY;
    ClientManager::getLog(client, ClientManager::DAY);
}

