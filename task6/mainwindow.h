#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QTranslator>
#include <QPointer>

#include <Network/client.h>
#include <Network/clientmanager.h>

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

    void changeLanguage(const QString& lang);
    void retranslate();


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
