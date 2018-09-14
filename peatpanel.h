#ifndef PEATPANEL_H
#define PEATPANEL_H

#include <QMainWindow>

namespace Ui {
class PEATPanel;
}

class PEATPanel : public QMainWindow
{
    Q_OBJECT

public:
    explicit PEATPanel(QWidget *parent = 0);
    ~PEATPanel();

private:
    Ui::PEATPanel *ui;
};

#endif // PEATPANEL_H
