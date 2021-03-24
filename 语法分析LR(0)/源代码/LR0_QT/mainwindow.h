#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<iostream>
#include<fstream>
#include<vector>
#include<queue>
#include<map>
#include<set>
#include<stack>
#include <iomanip>
#include<string>

int lr();


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void display_file(std::ifstream &in_file, int browser);


private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
