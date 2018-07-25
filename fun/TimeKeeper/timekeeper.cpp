#include "timekeeper.h"
#include "ui_timekeeper.h"

#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

void TimeKeeper::addTaskView(Task *task)
{
    static int iterate_i = 0;
    int iterate_j = 0;

    QCheckBox *active = new QCheckBox();
    active->setCheckState(Qt::Checked);

    gridLayout->addWidget(new QLabel(task->taskId()), iterate_i,iterate_j++);
    gridLayout->addWidget(new QLabel(task->ontime().toString("hh:mm:ss")), iterate_i,iterate_j++);
    gridLayout->addWidget(active, iterate_i,iterate_j++);

    iterate_i++;
}

TimeKeeper::TimeKeeper(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TimeKeeper)
{
    ui->setupUi(this);

    gridLayout = new QGridLayout;
    widget = new QWidget;

    gridLayout->setAlignment(Qt::AlignTop);

    widget->setLayout(gridLayout);
    ui->scrollArea->setWidget(widget);
}

TimeKeeper::~TimeKeeper()
{
    delete ui;
}

void TimeKeeper::on_pushButtonAddTask_clicked()
{
    QTime time;
    Task task;

    time = time.fromString(ui->lineEditStartTime->text());

    task.setTime(time);
    task.setID(ui->lineEditTaskId->text());
    task.setActive(true);

    taskList.append(task);

    addTaskView(&task);

    ui->lineEditTaskId->clear();
    ui->lineEditStartTime->setText("00:00:00");
}
