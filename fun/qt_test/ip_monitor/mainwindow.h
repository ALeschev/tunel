#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QVector>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QThread>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class Thread : public QThread
{
private:
    void run()
    {
        int i;

        while (i < 10)
        {
            Ui::on_pushButton_clicked();
        }
        qDebug()<<"From worker thread: "<<currentThreadId();
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    QScrollArea *scrollArea;
    QVBoxLayout *scrollLyout;

    Thread t;
    QVector<QLineEdit*> ip_lines;

    void ipline_create();
};

#endif // MAINWINDOW_H
