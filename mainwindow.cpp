#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtXml>
#include <QTextStream>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QPainter>
#include <QFileDialog>

#define MET_W 286
#define MET_H 15

#define MET_MAX_W 700
#define MET_MAX_H 1000000

#define MULTIPLIER 0.5   //Множитель в генераторе случайных размеров деталей
//------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qsrand(QTime::currentTime().msec());
    qtstrApplicationPath = QApplication::applicationDirPath();

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
    ui32MaxHeight = 0;
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
    qlistRectsSourceVert.clear();
    qlistRectsDestination.clear();
    QWidget::repaint();
    // Загружаем список деталей
    RectsXmlLoad(qtstrApplicationPath + "/rects.xml");
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
    //Сохраняем параметры листа в XML файл
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
    QString srtFileName = QFileDialog::getOpenFileName(this, QString(""), qtstrApplicationPath,
                                            "Objects (*.xml *.xml)");
    RectsXmlLoad(srtFileName);
}
//------------------------------------------------------------------------------
void MainWindow::RectsXmlLoad(QString strFileName)
{   //Загружаем детали из XML файла
    QFile qtclRectsCustFile;
    qtclRectsCustFile.setFileName(strFileName);

    if (qtclRectsCustFile.open(QIODevice::ReadOnly))
    {
        QDomDocument docRects;
        if (docRects.setContent(&qtclRectsCustFile))
        {
            QDomElement domEl = docRects.documentElement();
            QDomNode domNode = domEl.firstChild();

            StructRect stctRect;
            while(!domNode.isNull())
            {
                QDomNode domRect = domNode.firstChild();
                QDomElement domGetEl = domRect.toElement();

                QString strTmp = "";
                if (!domGetEl.isNull())
                {
                    qint32 i32Res = TestValue(domGetEl.text(), ui32MetW);
                    if (i32Res != -1)
                        stctRect.ui32RectW = i32Res;
                }
                domRect = domRect.nextSibling();
                domGetEl = domRect.toElement();
                if (!domGetEl.isNull())
                {
                    qint32 i32Res = TestValue(domGetEl.text(), ui32MetH*1000);
                    if (i32Res != -1)
                        stctRect.ui32RectH = i32Res;
                }
                qlistRects.append(stctRect);
                if (ui32TableRowsCurr == ui32TableRowsTotal)
                    TableAddRow();
                ui->tableWidget->item(ui32TableRowsCurr, 0)->setText(QString("%1").arg(stctRect.ui32RectW));
                ui->tableWidget->item(ui32TableRowsCurr++, 1)->setText(QString("%1").arg(stctRect.ui32RectH));
                qDebug() << "Новая деталь - ширина: " << stctRect.ui32RectW << " мм. длина: " << stctRect.ui32RectH << " мм.";

                domNode = domNode.nextSibling();
            }
        }
        qtclRectsCustFile.close();
    }
    qDebug() << "Загрузили файл с размерами деталей" << endl; //Отладочное сообщение
    RectsToXML();

    if (qlistRects.size())
        PlaceRects();
}
//------------------------------------------------------------------------------
void MainWindow::slotRectsGenerate()
{
    //Обработчик кнопки: Генерировать 10 деталей с произвольными размерами
    qDebug() << "Генерируем 10 деталей с произвольными размерами";
    StructRect stctRect;
    for(int i=0; i<10; i++)
    {
        stctRect.ui32RectW = qrand() % int(ui32MetW * MULTIPLIER - 10) + 10;
        stctRect.ui32RectH = qrand() % int(ui32MetW * MULTIPLIER - 10) + 10;
        qlistRects.append(stctRect);
        qDebug() << "\tШирина детали: " << stctRect.ui32RectW << " мм. Длина детали: " << stctRect.ui32RectH << " мм.";
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
    qlistRectsSourceVert.clear();
    qlistRectsDestination.clear();
    ClearTable(); //Очищаем таблицу
    RectsToXML(); //Очищаем XML файл
    QWidget::repaint(); //Очищаем изображение
}
//------------------------------------------------------------------------------
void MainWindow::RectsToXML()
{
    //Сохраняем детали в файл
    qtclRectsFile.setFileName(qtstrApplicationPath + "/rects.xml");
    qtclRectsFile.open(QIODevice::WriteOnly);
    QDomDocument doc("rects");
    QDomElement rects = doc.createElement("rects");
    doc.appendChild(rects);

    for (qsizetype i = 0; i < qlistRects.size(); ++i)
    {
//        qDebug() << "\tШирина: " << qlistRects.at(i).ui32RectW << "Длина: " << qlistRects.at(i).ui32RectH;

        QDomElement rect = doc.createElement("rect");
        QDomAttr rectAtr = doc.createAttribute("number");
        rectAtr.setValue(QString("%1").arg(i));
        rect.setAttributeNode(rectAtr);
        rects.appendChild(rect);

        QDomElement rectW = doc.createElement("rectW");
        QDomText domTextW = doc.createTextNode(QString("%1").arg(qlistRects.at(i).ui32RectW));
        rectW.appendChild(domTextW);
        rect.appendChild(rectW);

        QDomElement rectH = doc.createElement("rectH");
        QDomText domTextH = doc.createTextNode(QString("%1").arg(qlistRects.at(i).ui32RectH));
        rectH.appendChild(domTextH);
        rect.appendChild(rectH);
    }

    doc.appendChild(rects);
    QTextStream(&qtclRectsFile) << doc.toString();
    qtclRectsFile.close();

    qDebug() << "Сохранили детали в файл" << endl; //Отладочное сообщение
}
//------------------------------------------------------------------------------
void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    //Рисуем чистый лист
    QBrush brush(Qt::white, Qt::SolidPattern);
    painter.fillRect(ui->widget_left_top->width() + 8,
                     3,
                     ui32MetW,
                     ui->widget_viz->height()-3,
                     brush);

    if (qlistRectsDestination.size())
    {
        for (qsizetype i = 0; i < qlistRectsDestination.size(); ++i)
        {
            QBrush brush(qlistRectsDestination.at(i).rgbColor, Qt::SolidPattern);
            painter.fillRect(ui->widget_left_top->width() + 8 + qlistRectsDestination.at(i).ui32X,
                             3 + qlistRectsDestination.at(i).ui32Y,
                             qlistRectsDestination.at(i).ui32W,
                             qlistRectsDestination.at(i).ui32H,
                             brush);
        }
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
    qlistRectsSourceVert.clear();
    qlistRectsDestination.clear();
    qDebug() << "Размещаем детали" << endl; //Отладочное сообщение

    //Формируем список источник
    StructRect stctRectsSourceVert;
    ui32MaxHeight = 0;
    for (qsizetype i = 0; i < qlistRects.size(); ++i)
    {
        //Поворачиваем детали вертикально
        if((qlistRects.at(i).ui32RectW >= qlistRects.at(i).ui32RectH)||(qlistRects.at(i).ui32RectH > ui32MetW))
        {
            stctRectsSourceVert.ui32RectW = qlistRects.at(i).ui32RectW;
            stctRectsSourceVert.ui32RectH = qlistRects.at(i).ui32RectH;
        }
        else
        {
            stctRectsSourceVert.ui32RectW = qlistRects.at(i).ui32RectH;
            stctRectsSourceVert.ui32RectH = qlistRects.at(i).ui32RectW;
        }
        qlistRectsSourceVert.append(stctRectsSourceVert);
        ui32MaxHeight += stctRectsSourceVert.ui32RectH;
    }
    qDebug() << "Максимальная высота: " << ui32MaxHeight << endl;

    //Формируем список назначения (с параметрами размещения)
    StructRectDest stctRectsDestination;
    quint32 ui32CurrH = 0;
//    quint32 ui32CurrW = 0;
    for (qsizetype i = 0; i < qlistRectsSourceVert.size(); ++i)
    {//Отладочный алгоритм (ставим вертикально все подряд)
        stctRectsDestination.ui32X = 0;
        stctRectsDestination.ui32Y = ui32CurrH;
        stctRectsDestination.ui32W = qlistRectsSourceVert.at(i).ui32RectW;
        stctRectsDestination.ui32H = qlistRectsSourceVert.at(i).ui32RectH;
        stctRectsDestination.rgbColor = ColorGenerator();
        ui32CurrH += stctRectsDestination.ui32H;
        qlistRectsDestination.append(stctRectsDestination);
    }
    QWidget::repaint();
}
//------------------------------------------------------------------------------
QRgb MainWindow::ColorGenerator()
{
    //Генератор цвета  //    QRgb rgb;
    int r, g, b;
    do
    {
        r = rand() % 255;
        g = rand() % 255;
        b = rand() % 255;
    }
    while(r+g+b == 765);

//    qDebug() << "Генерируем случайный цвет" << rgb << QColor::fromRgb(rgb) << endl; //Отладочное сообщение
    return qRgba(r, g, b, 255);
}
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------
