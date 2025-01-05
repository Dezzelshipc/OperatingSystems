#ifndef CHANGESERVERWIDGET_H
#define CHANGESERVERWIDGET_H

#include <QDialog>

namespace Ui {
class ChangeServerWidget;
}

class ChangeServerWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeServerWidget(QWidget *parent = nullptr);
    ~ChangeServerWidget();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ChangeServerWidget *ui;
};

#endif // CHANGESERVERWIDGET_H
