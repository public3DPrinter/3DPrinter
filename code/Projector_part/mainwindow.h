#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "crushbitmap.h"
#include "b9print.h"
#include "b9terminal.h"

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
    B9Terminal *pTerminal;

private slots:
    void on_commandPrint_clicked();
    void doPrint();
};

#endif // MAINWINDOW_H
