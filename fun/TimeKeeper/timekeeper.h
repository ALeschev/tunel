#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <QMainWindow>
#include <QGridLayout>
#include "task.h"

//https://stackoverflow.com/questions/9660080/how-does-one-fill-a-qgridlayout-from-top-left-to-right

namespace Ui {
class TimeKeeper;
}

class TimeKeeper : public QMainWindow
{
    Q_OBJECT

public:
    explicit TimeKeeper(QWidget *parent = 0);
    ~TimeKeeper();

private slots:
    void on_pushButtonAddTask_clicked();

private:
    void addTaskView(Task *task);

    Ui::TimeKeeper *ui;
    QGridLayout *gridLayout;
    QWidget *widget;
    QList<Task> taskList;
};

#endif // TIMEKEEPER_H
