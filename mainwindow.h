#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>

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

    quint32 ui32MetW;
    quint32 ui32MetH;

    QFile qtclMetFile;
    QFile qtclRectsFile;

    qint32 TestValue(QString strValue, quint32 ui32Border);
    void MetToXml();
    void PlaceRects();
};

#endif // MAINWINDOW_H
