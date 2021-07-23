#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtXml>
#include <QTextStream>
#include <QMessageBox>

#define MET_W 286
#define MET_H 15

#define MET_MAX_W 700
#define MET_MAX_H 1000000
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString qtstrApplicationPath = QApplication::applicationDirPath();

    // Загрузка габаритов материала
    qtclMetFile.setFileName(qtstrApplicationPath + "/met.xml");
    ui32MetW = MET_W;
    ui32MetH = MET_H;
    if (!qtclMetFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Ошибка не удалось открыть xml-файл для чтения!" << endl;

        //сохраняем в xml
        MetToXml();

        ui->lineEdit_met_w_value->setText(QString("%1").arg(ui32MetW));
        ui->lineEdit_met_h_value->setText(QString("%1").arg(ui32MetH));
    }
    else
    {
        qDebug() << "Удалось открыть xml-файл для чтения!" << endl;
        QDomDocument docMet;
        if (docMet.setContent(&qtclMetFile))
        {
            QDomElement domEl = docMet.documentElement();
            QDomNode domNode = domEl.firstChild();
            QDomElement domGetEl = domNode.toElement();
            QString strTmp = "";
            if (!domGetEl.isNull())
            {
                qDebug() << "\tШирина: " << domGetEl.tagName() << domGetEl.text();
                qint32 i32Res = TestValue(domGetEl.text(), MET_MAX_W); //domGetEl.text().toUInt();
                if (i32Res != -1)
                    ui32MetW = i32Res;
            }
            domNode = domNode.nextSibling();
            domGetEl = domNode.toElement();
            if (!domGetEl.isNull())
            {
                qDebug() << "\tДлина: " << domGetEl.tagName() << domGetEl.text() << endl;
                qint32 i32Res = TestValue(domGetEl.text(), MET_MAX_H); //domGetEl.text().toUInt();
                if (i32Res != -1)
                    ui32MetH = i32Res;
            }
        }
        ui->lineEdit_met_w_value->setText(QString("%1").arg(ui32MetW));
        ui->lineEdit_met_h_value->setText(QString("%1").arg(ui32MetH));
        qtclMetFile.close();
    }
    qDebug() << "Ширина листа: " << ui32MetW << "мм. Длина листа: " << ui32MetH << " м." << endl;

    // Подключаем кнопки
    connect(ui->pushButton_met_set, SIGNAL(pressed()), SLOT(slotMetSet()));
    connect(ui->pushButton_rect_set, SIGNAL(pressed()), SLOT(slotRectSet()));
    connect(ui->pushButton_rects_load, SIGNAL(pressed()), SLOT(slotRectsLoad()));
    connect(ui->pushButton_rects_generate, SIGNAL(pressed()), SLOT(slotRectsGenerate()));
    connect(ui->pushButton_rects_clear, SIGNAL(pressed()), SLOT(slotRectsClear()));

    // Загружаем список деталей
    slotRectsLoad();
}
//------------------------------------------------------------------------------
qint32 MainWindow::TestValue(QString strValue, quint32 ui32Border)
{
    //Проверка корректности вводимого размера

    quint32 ui32Res = strValue.toUInt();

    if ((ui32Res)&&(ui32Res <= ui32Border))
        return ui32Res;
    else
    {
        qDebug() << "Некорректный размер: " << strValue << endl;
        QMessageBox *pqtclErrorMessage = new QMessageBox(QMessageBox::Critical,
                                                         "Ошибка",
                                                         "Некорректный размер!",
                                                         QMessageBox::Ok);
        pqtclErrorMessage->exec();
        delete pqtclErrorMessage;
        return -1;
    }
}
//------------------------------------------------------------------------------
void MainWindow::MetToXml()
{
    qtclMetFile.open(QIODevice::WriteOnly);
    QDomDocument doc("met");
    QDomElement metGeom = doc.createElement("geometry");
    doc.appendChild(metGeom);

    QDomElement metW = doc.createElement("metW");
//        QDomAttr metAtrW = doc.createAttribute("metW");
//        metAtrW.setValue("metW");
//        metW.setAttributeNode(metAtrW);
    QDomText domTextW = doc.createTextNode(QString("%1").arg(ui32MetW));
    metW.appendChild(domTextW);
    metGeom.appendChild(metW);

    QDomElement metH = doc.createElement("metH");
//        QDomAttr metAtrH = doc.createAttribute("metH");
//        metAtrH.setValue("metH");
//        metH.setAttributeNode(metAtrH);
    QDomText domTextH = doc.createTextNode(QString("%1").arg(ui32MetH));
    metH.appendChild(domTextH);
    metGeom.appendChild(metH);

    doc.appendChild(metGeom);
    QTextStream(&qtclMetFile) << doc.toString();
    qtclMetFile.close();
}
//------------------------------------------------------------------------------
void MainWindow::slotMetSet()
{
    //Обработчик кнопки: Изменить размер листа металла

    qint32 i32Res = TestValue(ui->lineEdit_met_w_value->text(), MET_MAX_W);
    if (i32Res != -1)
        ui32MetW = i32Res;
    else
    {
        ui32MetW = MET_W;
        ui->lineEdit_met_w_value->setText(QString("%1").arg(ui32MetW));
    }

    i32Res = TestValue(ui->lineEdit_met_h_value->text(), MET_MAX_H);
    if (i32Res != -1)
        ui32MetH = i32Res;
    else
    {
        ui32MetH = MET_H;
        ui->lineEdit_met_h_value->setText(QString("%1").arg(ui32MetH));
    }

    qDebug() << "Ширина листа: " << ui32MetW << "мм. Длина листа: " << ui32MetH << " м." << endl;

    //сохраняем в xml
    MetToXml();

    // запускаем размещение
    PlaceRects();
}
//------------------------------------------------------------------------------
void MainWindow::slotRectSet()
{
    //Обработчик кнопки: Добавить новую деталь

    qDebug() << "Добавить новую деталь" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
void MainWindow::slotRectsLoad()
{
    //Обработчик кнопки: Загрузить файл с размерами деталей

    qDebug() << "Загрузить файл с размерами деталей" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
void MainWindow::slotRectsGenerate()
{
    //Обработчик кнопки: Генерировать 10 деталей с произвольными размерами

    qDebug() << "Генерировать 10 деталей с произвольными размерами" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
void MainWindow::slotRectsClear()
{
    //Обработчик кнопки: Очистить список деталей

    qDebug() << "Очистить список деталей" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
void MainWindow::PlaceRects()
{
    //Размещаем детали

    qDebug() << "Размещаем детали" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------
