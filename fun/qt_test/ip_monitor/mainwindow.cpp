#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ip_lines.clear();

    scrollArea = new QScrollArea( this );
    scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    scrollArea->setWidgetResizable( true );
    scrollArea->setGeometry( 10, 10, 271, 401 );

    QWidget *widget = new QWidget();
    scrollArea->setWidget( widget );

    scrollLyout = new QVBoxLayout();
    scrollLyout->setAlignment(Qt::AlignTop);

    widget->setLayout(scrollLyout);
}

MainWindow::~MainWindow()
{
    delete ui;
    ip_lines.clear();
}

void MainWindow::ipline_create()
{
    static int x_offset = 10;
    static int qline_count = 0;

    QLineEdit *ipline = new QLineEdit(scrollArea);
    ipline->setGeometry(10,x_offset,146,26);
    ipline->setText(QString::number(qline_count));
    scrollLyout->addWidget(ipline);
    ip_lines << ipline;
    ipline->show();

    x_offset += 30;
    qline_count++;
}

void MainWindow::on_pushButton_clicked()
{
    ipline_create();
    t.start();
}
