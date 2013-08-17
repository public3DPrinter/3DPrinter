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
    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    //create terminal
    pTerminal = new B9Terminal(QApplication::desktop());
    pTerminal->setEnabled(true);

    pMW4 = new B9Print(pTerminal, 0);
    m_pCPJ = new CrushedPrintJob;

    connect(pMW4, SIGNAL(eventHiding()),this, SLOT(handleW4Hide()));
}

MainWindow::~MainWindow()
{
    delete m_pCPJ;
    delete ui;
    delete pMW4;
    qDebug() << "Program End";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    pMW4->hide();
    pTerminal->hide();
    event->accept();
}

void MainWindow::handleW4Hide()
{
    this->show();  // Comment this out if not hiding mainwindow while showing this window
    pTerminal->setIsPrinting(false);
    //CROSS_OS_DisableSleeps(false);// return system screensavers back to normal.

}

void MainWindow::on_commandPrint_clicked()
{
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
    int iXYPixelMicrons = m_pCPJ->getXYPixelmm()*1000;
    if( pTerminal->isConnected()&& iXYPixelMicrons != (int)pTerminal->getXYPixelSize()){
            QMessageBox msgBox;
            msgBox.setText("WARNING");
            msgBox.setInformativeText("The XY pixel size of the selected job file ("+QString::number(iXYPixelMicrons)+" µm) does not agree with the Printer's calibrated XY pixel size ("+QString::number(pTerminal->getXYPixelSize())+" µm)!\n\n"
                                      "Printing will likely result in an object with incorrect scale and/or apsect ratio.\n\n"
                                      "Do you wish to continue?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int ret = msgBox.exec();
            if(ret==QMessageBox::No)return;
    }

    m_pPrintPrep = new DlgPrintPrep(m_pCPJ, pTerminal, this);
    connect (m_pPrintPrep, SIGNAL(accepted()),this,SLOT(doPrint()));
    m_pPrintPrep->exec();

    //m_pCPJ->showSupports(true);
    //doPrint();
}

void MainWindow::doPrint()
{
    // print using variables set by wizard...
    this->hide(); // Comment this out if not hiding mainwindow while showing this window
    pMW4->show();
    pTerminal->setIsPrinting(true);
    pMW4->print3D(m_pCPJ, 0, 0, m_pCPJ->getTotalLayers(), m_pPrintPrep->m_iToverMS, m_pPrintPrep->m_iTattachMS, m_pPrintPrep->m_iNumAttach, m_pPrintPrep->m_iLastLayer, true, m_pPrintPrep->m_bDryRun, m_pPrintPrep->m_bMirrored);
    //pMW4->print3D(m_pCPJ,0,0,42,6,50,2,431,true,false,false);
}
