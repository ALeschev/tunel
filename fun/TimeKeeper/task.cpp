#include "task.h"

Task::Task()
{

}

Task::Task(QString taskID, QTime time, bool active)
{
    this->taskID = taskID;
    this->time = time;
    this->active = active;
}

void Task::setID(QString ID)
{
    this->taskID = ID;
}

void Task::setActive(bool ena)
{
    this->active = ena;
}

void Task::setTime(QTime time)
{
    this->time = time;
}

QString Task::taskId()
{
    return this->taskID;
}

bool Task::isActive()
{
    return this->active;
}

QTime Task::ontime()
{
    return this->time;
}

