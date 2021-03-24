#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    lex();
    std::ifstream f("adjust.txt ", std::ios::in);
    display_file(f, 1);
    f.close();
    f.open("result.txt ", std::ios::in);
    display_file(f, 2);
    f.close();
}

void MainWindow::display_file(std::ifstream &in_file, int browser)
{
    char a = ' ';
    QString ss;
    switch(browser)
    {
    case 0:
        ui->textBrowser->clear();
        break;
    case 1:
        ui->textBrowser_2->clear();
        break;
    case 2:
        ui->textBrowser_3->clear();
        break;
    default:break;
    }
    while(1)
    {
        a = in_file.get();
        if(in_file.eof())
            break;
        ss = QString(QChar (a));
        switch(browser)
        {
        case 0:
            ui->textBrowser->insertPlainText(ss);
            break;
        case 1:
            ui->textBrowser_2->insertPlainText(ss);
            break;
        case 2:
            ui->textBrowser_3->insertPlainText(ss);
            break;
        default:break;
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    std::ifstream fs("a.txt ", std::ios::in);
    display_file(fs, 0);
    fs.close();
}
