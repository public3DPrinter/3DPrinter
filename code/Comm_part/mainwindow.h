#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "b9printercomm.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    B9PrinterComm *pPrinterComm;
    Ui::MainWindow *ui;

private slots:
    void on_btnConnect_clicked();
    void on_btnSend_clicked();
};

#endif // MAINWINDOW_H
