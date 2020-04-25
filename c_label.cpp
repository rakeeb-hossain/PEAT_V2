//
//  c_label.cpp
//  PEAT_V2
//
//  Created by Hossain, Rakeeb on 2018-01-02.
//  Copyright © 2018 Hossain, Rakeeb. All rights reserved.
//
#include "c_label.h"

C_Label::C_Label(QWidget * parent) : QLabel(parent)
{
    orig_x_pos = 254;
    x_old_pos = orig_x_pos;
    x_new_pos = orig_x_pos;
}

C_Label::~C_Label()
{
}

void C_Label::mouseMoveEvent ( QMouseEvent * event )
{
    x_new_pos = event->globalPos().x();
    if (leftClicked == true && orig_x_pos+x_new_pos-x_old_pos >= 254 && orig_x_pos+x_new_pos-x_old_pos <= 425)
        emit mouseMoved();
}

void C_Label::mousePressEvent ( QMouseEvent * event )
{
    if (event->button() & Qt::LeftButton) {
        leftClicked = true;
    }
    x_old_pos = event->globalPos().x();
    emit mousePressed();
}

void C_Label::mouseReleaseEvent ( QMouseEvent * event )
{
    if (event->button() & Qt::LeftButton) {
        orig_x_pos = this->pos().x();
        leftClicked = false;
    }
}
