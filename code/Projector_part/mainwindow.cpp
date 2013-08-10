#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //create terminal
    pTerminal = new B9Terminal(QApplication::desktop());
    pTerminal->setEnabled(true);

    pMW4 = new B9Print(pTerminal, 0);
    m_pCPJ = new CrushedPrintJob;
}

MainWindow::~MainWindow()
{
    delete m_pCPJ;
    delete ui;
    delete pMW4;
}

void MainWindow::on_commandPrint_clicked()
{
//    qDebug() << "clicked";
//    QDesktopWidget *desktop = QApplication::desktop();
//    qDebug() << desktop->screenCount()
//             << "---"
//             << QApplication::desktop()->screenCount()
//             << desktop->isVirtualDesktop();
    // Open the .b9j file
    m_pCPJ->clearAll();
    QFileDialog dialog(0);
    QSettings settings;
    QString openFile = dialog.getOpenFileName(this,"Select a B9Creator Job File to print", settings.value("WorkingDir").toString(), tr("B9Creator Job Files (*.b9j)"));
    if(openFile.isEmpty()) return;
    settings.setValue("WorkingDir", QFileInfo(openFile).absolutePath());
    QFile file(openFile);
    if(!m_pCPJ->loadCPJ(&file)) {
        QMessageBox msgBox;
        msgBox.setText("Error Loading File.  Unknown Version?");
        msgBox.exec();
        return;
    }
    m_pCPJ->showSupports(true);
    doPrint();

}

void MainWindow::doPrint()
{
    // print using variables set by wizard...
    this->hide(); // Comment this out if not hiding mainwindow while showing this window
    pMW4->show();
    //pTerminal->setIsPrinting(true);
    //pMW4->print3D(m_pCPJ, 0, 0, m_pCPJ->getTotalLayers(), m_pPrintPrep->m_iToverMS, m_pPrintPrep->m_iTattachMS, m_pPrintPrep->m_iNumAttach, m_pPrintPrep->m_iLastLayer, m_pPrintPrep->m_bDryRun, m_pPrintPrep->m_bDryRun, m_pPrintPrep->m_bMirrored);
    pMW4->print3D(m_pCPJ,0,0,42,6,50,2,431,true,false,false);
}

