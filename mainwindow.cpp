#include "mainwindow.h"
#include "ui_mainwindow.h"
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString qtstrApplicationPath = QApplication::applicationDirPath();
    // Загрузка габаритов материала
//    qtclMetFile.setFileName(qtstrApplicationPath + "/met.xml");

}
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------
