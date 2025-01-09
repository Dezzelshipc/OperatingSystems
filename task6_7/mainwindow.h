#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QTranslator>
#include <QPointer>
#include <QTableWidgetItem>

#include <QChart>
#include <QChartView>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QStyleHints>

#include <Network/client.h>
#include <Network/clientmanager.h>

#include <Utility/parser.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    const MainWindow* instance();

signals:
    void languageChanged();

private:
    static inline QPointer<MainWindow> mainInstance;
    Ui::MainWindow *ui;
    QPointer<QTranslator> translator;
    QPointer<Client> client;
    ClientManager::LOG_TYPE last_log_type;

    QPointer<QChart> chart;
    QPointer<QDateTimeAxis> axisX;
    QPointer<QValueAxis> axisY;

    void changeLanguage(const QString& lang);
    void retranslate();

    void initSeries(const QList<Parser::DateTemp>& list);

    void changeEvent(QEvent *event) override;
    void changeChartTheme();

    void keyPressEvent(QKeyEvent *ev) override;
    void closeEvent(QCloseEvent  *ev) override;


private slots:
    void on_action_ChangeServer_triggered();
    void on_action_About_triggered();
    void on_actionEnglish_triggered();
    void on_actionRussian_triggered();
    void on_pushButton_log_sec_clicked();
    void on_pushButton_log_hour_clicked();
    void on_pushButton_log_day_clicked();
};
#endif // MAINWINDOW_H
