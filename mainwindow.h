#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotMetSet();
    void slotRectSet();
    void slotRectsLoad();
    void slotRectsGenerate();
    void slotRectsClear();

private:
    Ui::MainWindow *ui;

    QString qtstrApplicationPath;

    quint32 ui32MetW;
    quint32 ui32MetH;

    QFile qtclMetFile;
    QFile qtclRectsFile;

    quint32 ui32TableRowsTotal;
    quint32 ui32TableRowsCurr;

    quint32 ui32MaxHeight;
    QLabel *pqtlbMaxHeight;
    quint32 ui32NFDHHeight;
    QLabel *pqtlbNFDHHeight;
    quint32 ui32FCNRHeight;
    QLabel *pqtlbFCNRHeight;

    qint32 TestValue(QString strValue, quint32 ui32Border);
    void ErrorMessageBox(QString strMessageText);
    void TableAddRow();
    void MetToXml();
    void PlaceRects();
    void RectsToXML();
    void RectsXmlLoad(QString strFileName);
    void ClearTable();
    QRgb ColorGenerator();

    struct StructRect   //Структура с габаритами деталей
    {
        quint32 ui32RectW;
        quint32 ui32RectH;
        QRgb rgbColor;
    };
    QList<StructRect> qlistRects;   //Список деталей

    QList<StructRect> qlistRectsSourceVert;   // Список источник
    struct StructRectDest   //Структура с параметрами размещения
    {
        quint32 ui32X;
        quint32 ui32Y;
        quint32 ui32W;
        quint32 ui32H;
        QRgb rgbColor;
    };
    QList<StructRectDest> qlistRectsDestination;   //Список с параметрами размещения

protected:
    void paintEvent(QPaintEvent *);
};

#endif // MAINWINDOW_H
