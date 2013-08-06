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
    pMW4 = new B9Print(0);
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
}
