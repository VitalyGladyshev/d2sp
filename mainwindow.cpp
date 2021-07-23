#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtXml>
#include <QTextStream>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QPainter>

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

    qsrand(QTime::currentTime().msec());
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

    ui32TableRowsTotal = 20;
    ui32TableRowsCurr = 0;
    QStringList horizontalHeader;
    horizontalHeader.append("Ширина");
    horizontalHeader.append("Длина");
    ui->tableWidget->setHorizontalHeaderLabels(horizontalHeader);

    for (quint32 i = 0; i < ui32TableRowsTotal; i++)
        for (int j = 0; j < 2; j++)
        {
            QTableWidgetItem *ptWi = new QTableWidgetItem("");
            ui->tableWidget->setItem(i, j, ptWi);
        }

    qlistRects.clear();
    bImgClean = true;
    QWidget::repaint();
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
        ErrorMessageBox("Некорректный размер!");
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

    qDebug() << "Ширина листа: " << ui32MetW << " мм. Длина листа: " << ui32MetH << " м." << endl;

    //сохраняем в xml
    MetToXml();

    // запускаем размещение
    PlaceRects();
}
//------------------------------------------------------------------------------
void MainWindow::ErrorMessageBox(QString strMessageText)
{
    //Сообщение об ошибке
    QMessageBox *pqtclErrorMessage = new QMessageBox(QMessageBox::Critical,
                                                     "Ошибка",
                                                     strMessageText,
                                                     QMessageBox::Ok);
    pqtclErrorMessage->exec();
    delete pqtclErrorMessage;
}
//------------------------------------------------------------------------------
void MainWindow::slotRectSet()
{
    //Обработчик кнопки: Добавить новую деталь

    qint32 i32Res = TestValue(ui->lineEdit_rect_w_value->text(), ui32MetW);
    if (i32Res != -1)
    {
        StructRect stctRect;
        stctRect.ui32RectW = i32Res;

        i32Res = TestValue(ui->lineEdit_rect_h_value->text(), ui32MetW*1000);
        if (i32Res != -1)
        {
            stctRect.ui32RectH = i32Res;
            qlistRects.append(stctRect);
            qDebug() << "Ширина детали: " << ui32MetW << " мм. Длина детали: " << ui32MetH << " мм." << endl;
            if (ui32TableRowsCurr == ui32TableRowsTotal)
                TableAddRow();
            ui->tableWidget->item(ui32TableRowsCurr, 0)->setText(QString("%1").arg(stctRect.ui32RectW));
            ui->tableWidget->item(ui32TableRowsCurr++, 1)->setText(QString("%1").arg(stctRect.ui32RectH));
            RectsToXML();
            PlaceRects();
        }
        else
            ErrorMessageBox("Длина детали больше длины листа!");
    }
    else
        ErrorMessageBox("Ширина детали больше ширины листа!");
}
//------------------------------------------------------------------------------
void MainWindow::TableAddRow()
{
    ui->tableWidget->insertRow(ui32TableRowsTotal);
    for (int j = 0; j < 2; j++)
    {
        QTableWidgetItem *ptWi = new QTableWidgetItem("");
        ui->tableWidget->setItem(ui32TableRowsTotal, j, ptWi);
    }
    ui32TableRowsTotal += 1;
}
//------------------------------------------------------------------------------
void MainWindow::slotRectsLoad()
{
    //Обработчик кнопки: Загрузить файл с размерами деталей

    if (false)
        PlaceRects();
    qDebug() << "Загрузить файл с размерами деталей" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
void MainWindow::slotRectsGenerate()
{
    //Обработчик кнопки: Генерировать 10 деталей с произвольными размерами
    qDebug() << "Генерируем 10 деталей с произвольными размерами";
    StructRect stctRect;
    for(int i=0; i<10; i++)
    {
        stctRect.ui32RectW = qrand() % (ui32MetW - 10) + 10;
        stctRect.ui32RectH = qrand() % int(ui32MetW * 1.2 - 10) + 10;
        qlistRects.append(stctRect);
        qDebug() << "Ширина детали: " << ui32MetW << " мм. Длина детали: " << ui32MetH << " мм." << endl;
        if (ui32TableRowsCurr == ui32TableRowsTotal)
            TableAddRow();
        ui->tableWidget->item(ui32TableRowsCurr, 0)->setText(QString("%1").arg(stctRect.ui32RectW));
        ui->tableWidget->item(ui32TableRowsCurr++, 1)->setText(QString("%1").arg(stctRect.ui32RectH));
    }
    RectsToXML();
    PlaceRects();
}
//------------------------------------------------------------------------------
void MainWindow::slotRectsClear()
{
    //Обработчик кнопки: Очистить список деталей
    qDebug() << "Очистить список деталей" << endl; //Отладочное сообщение

    qlistRects.clear();
    ClearTable(); //Очищаем таблицу
    RectsToXML(); //Очищаем XML файл
    bImgClean = true;
    QWidget::repaint(); //Очищаем изображение
}
//------------------------------------------------------------------------------
void MainWindow::RectsToXML()
{
    //Сохраняем детали в файл

    qDebug() << "Сохраняем детали в файл" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
void MainWindow::paintEvent(QPaintEvent *)
{
    if (bImgClean)
    { //Рисуем лист металла в масштабе
        QPainter painter(this);
//        QBrush brush(ColorGenerator(), Qt::SolidPattern);
        QBrush brush(Qt::white, Qt::SolidPattern);
        painter.fillRect(ui->widget_left_top->width() + 8,
                         3,
                         ui32MetW,
                         ui->widget_viz->height()-3,
                         brush);
    }
}
//------------------------------------------------------------------------------
void MainWindow::ClearTable()
{
    //Очищаем таблицу
    for(quint32 i = 0; i < ui32TableRowsTotal; i++)
        for(int j = 0; j < 2; j++)
        {
            ui->tableWidget->item(i, 0)->setText("");
            ui->tableWidget->item(i, 1)->setText("");
        }
    ui32TableRowsCurr = 0;
}
//------------------------------------------------------------------------------
void MainWindow::PlaceRects()
{
    //Размещаем детали
    bImgClean = false;
    qDebug() << "Размещаем детали" << endl; //Отладочное сообщение


}
//------------------------------------------------------------------------------
QRgb MainWindow::ColorGenerator()
{
    //Генератор цвета
    QRgb rgb;

    int r = rand() % 255;
    int g = rand() % 255;
    int b = rand() % 255;

    rgb = qRgba(r, g, b, 255);

    qDebug() << "Генерируем случайный цвет" << rgb << QColor::fromRgb(rgb) << endl; //Отладочное сообщение
    return rgb;
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------
