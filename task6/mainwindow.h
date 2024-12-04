#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QTranslator>
#include <QPointer>

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

    void changeLanguage(const QString& lang);
    void retranslate();


private slots:
    void on_action_ChangeServer_triggered();
    void on_action_About_triggered();
    void on_actionEnglish_triggered();
    void on_actionRussian_triggered();
};
#endif // MAINWINDOW_H
