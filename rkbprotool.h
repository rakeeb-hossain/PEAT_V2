#ifndef RKBPROTOOL_H
#define RKBPROTOOL_H

#include <QDialog>

namespace Ui {
class rkbProTool;
}

class rkbProTool : public QDialog
{
    Q_OBJECT

public:
    explicit rkbProTool(QWidget *parent = 0);
    ~rkbProTool();
    int decision;
    QString folder;
    QString report;

private slots:
    void on_radioButton_2_clicked();

    void on_radioButton_3_clicked();

    void on_radioButton_4_clicked();

    void on_radioButton_clicked();

    void on_buttonBox_accepted();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::rkbProTool *ui;
};

#endif // RKBPROTOOL_H
