#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
//#include <QThread>

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow//, public QThread
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void signalProgress(int);

private slots:
    void on_startButton_clicked();
    void on_hiButton_clicked();

private:
    Ui::MainWindow *ui;

    void ReadHostNames(QString filename);
    QStringList GetHostNames();
    void UpdateProgress(int value);
};

#endif // MAINWINDOW_H
