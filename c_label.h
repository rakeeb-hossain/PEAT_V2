#ifndef C_LABEL_H
#define C_LABEL_H

#endif // C_LABEL_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <iostream>
#include <QApplication>
#include <QtWidgets>
#include <QtGui>

class C_Label : public QLabel
{
    Q_OBJECT

public:
    C_Label(QWidget *parent = 0);
    ~C_Label();
    int orig_x_pos;
    int orig_y_pos;
    int x_old_pos;
    int y_old_pos;
    int x_new_pos;
    int y_new_pos;
    int leftClicked = false;

signals:
    void mousePressed();
    void mouseMoved();

protected slots:

    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
};
