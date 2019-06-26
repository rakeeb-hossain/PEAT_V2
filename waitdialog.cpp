#include "waitdialog.h"
#include "ui_waitdialog.h"
#include "mainframe.h"

waitDialog::waitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::waitDialog)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedSize(QSize(393,264));

    setWindowFlags( Qt::CustomizeWindowHint );

    this->setWindowTitle("Analyzing frames...");

}

waitDialog::~waitDialog()
{
    delete ui;
}

void waitDialog::setProgressBarValue(int value)
{
    ui->progressBar->setValue(value);
}

void waitDialog::setProgressBarRange(int min, int max)
{
    ui->progressBar->setRange(min, max);
}

void waitDialog::setImg(QPixmap img)
{
    ui->label_2->setPixmap(img);
    QCoreApplication::processEvents();
}
