#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include <QDialog>

namespace Ui {
class waitDialog;
}

class waitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit waitDialog(QWidget *parent = 0);
    ~waitDialog();

private:
    Ui::waitDialog *ui;
};

#endif // WAITDIALOG_H
