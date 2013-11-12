#include "mainwindow.h"
#include "SleeperThread.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->progressBar->setMaximum( 100 );
    ui->progressBar->setMinimum( 0 );
    ui->progressBar->setValue(0);

//  ReadHostNames("home/dmarsh/src/qt/NitroQt/hostnames.txt");
    ReadHostNames("hostnames.txt");

    connect( this, SIGNAL( signalProgress(int) ), ui->progressBar, SLOT( setValue(int) ) );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    QString ipAddress = ui->lineEdit_ipAddress->text();
    QStringList hostNames = GetHostNames();

    int count = hostNames.count();

    UpdateProgress(72);
}

void MainWindow::on_hiButton_clicked()
{
    //QMessageBox("Test", "Hola, buddy", NULL, 0, 0, 0);
    UpdateProgress(50);
}

void MainWindow::ReadHostNames(QString filename)  // read them in from file, line by line
{
    ui->listWidget_HostNames->clear();

//  QFile inputFile(":/input.txt");
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while ( !in.atEnd() )
        {
            QString line = in.readLine();
            ui->listWidget_HostNames->addItem(line);
        }
    }
    inputFile.close();
}

QStringList MainWindow::GetHostNames()
{
    QStringList sl;
    for (int row=0; row < ui->listWidget_HostNames->count(); row++)
    {
        QListWidgetItem *item = ui->listWidget_HostNames->item(row);
        QString s = item->text();
        sl.append(s);
    }
    return sl;
}

void MainWindow::UpdateProgress(int value)
{
    emit signalProgress(value);// how to use
    ui->jobCountLabel->setText(QString("%1 of %2 jobs").arg(value).arg(100));

    // example:
//    for (int i=0; i<=10; i++)
//    {
//       emit signalProgress(i*10);
//       SleeperThread::msleep(200);
//    }
}



