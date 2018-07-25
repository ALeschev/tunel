#ifndef TASK_H
#define TASK_H

#include <QTime>

class Task
{
public:
    Task();
    Task(QString, QTime, bool);

    void setID(QString ID);
    void setActive(bool ena);
    void setTime(QTime time);

    QString taskId();
    bool isActive();
    QTime ontime();

private:
    QString taskID;
    QTime time;
    bool active;
};

#endif // TASK_H
