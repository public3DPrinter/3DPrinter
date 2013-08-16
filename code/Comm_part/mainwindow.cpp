#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnConnect_clicked() {
    pPrinterComm = new B9PrinterComm;
}

void MainWindow::on_btnSend_clicked() {
    pPrinterComm->SendCmd(ui->lineEdit->text());
}
