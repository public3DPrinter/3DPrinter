#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "crushbitmap.h"
#include "b9print.h"
#include "b9terminal.h"
#include "dlgprintprep.h"

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
    void closeEvent ( QCloseEvent * event );
    Ui::MainWindow *ui;
    CrushedPrintJob *m_pCPJ;
    B9Print *pMW4;
    B9Terminal *pTerminal;
    DlgPrintPrep* m_pPrintPrep;

private slots:
    void on_commandPrint_clicked();
    void doPrint();
    void handleW4Hide();
};

#endif // MAINWINDOW_H
