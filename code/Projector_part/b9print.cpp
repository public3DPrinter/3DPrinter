#include "b9print.h"
#include "ui_b9print.h"

B9Print::B9Print(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::B9Print)
{
    ui->setupUi(this);
}

B9Print::~B9Print()
{
    delete ui;
}
