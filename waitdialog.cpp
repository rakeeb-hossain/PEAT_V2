#include "waitdialog.h"
#include "ui_waitdialog.h"
#include "mainframe.h"

waitDialog::waitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::waitDialog)
{
    ui->setupUi(this);

    setWindowFlags( Qt::CustomizeWindowHint );

    this->setWindowTitle("Analyzing frames...");

}

waitDialog::~waitDialog()
{
    delete ui;
}
