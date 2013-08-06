#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "crushbitmap.h"
#include "b9print.h"

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
    Ui::MainWindow *ui;
    CrushedPrintJob *m_pCPJ;
    B9Print *pMW4;

private slots:
    void on_commandPrint_clicked();
    void doPrint();
};

#endif // MAINWINDOW_H
