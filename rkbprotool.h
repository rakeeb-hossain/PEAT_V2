#ifndef RKBPROTOOL_H
#define RKBPROTOOL_H

#include <QDialog>
#include <QThread>

namespace Ui {
class rkbProTool;
}

class rkbProTool : public QDialog
{
    Q_OBJECT
    QThread workerThread;

public:
    explicit rkbProTool(std::vector<std::vector<int > > vid_info, QString vid_file, QWidget *parent = 0);
    ~rkbProTool();
    int decision = 0;
    QString newFile;
    QString report;
    std::vector<std::vector<int > > vidData;
    QString vidFile;

private slots:
    void on_radioButton_2_clicked();

    void on_radioButton_3_clicked();

    void on_radioButton_4_clicked();

    void on_radioButton_clicked();

    void on_buttonBox_accepted();

    void on_pushButton_clicked();

    void on_radioButton_5_clicked();

    //void on_radioButton_6_clicked();

    void on_radioButton_7_clicked();

private:
    Ui::rkbProTool *ui;
    QTimer *timer;
    QMovie *mv;
};

#endif // RKBPROTOOL_H
