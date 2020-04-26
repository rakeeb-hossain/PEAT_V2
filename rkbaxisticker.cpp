#include "rkbaxisticker.h"

rkbAxisTicker::rkbAxisTicker() {}

void rkbAxisTicker::setFPS(double fps) {
    m_fps = fps;
}

void rkbAxisTicker::setVidLen(qint64 len) {
    m_vidlen = len;
}

void rkbAxisTicker::setAxisType(qint64 type) {
    m_axis = type;
}

void rkbAxisTicker::setFrameNum(qint64 nframe) {
    m_nframe = nframe;
}

double rkbAxisTicker::getFPS() {
    return m_fps;
}

qint64 rkbAxisTicker::getVidLen() {
    return m_vidlen;
}

qint64 rkbAxisTicker::getFrameNum() {
    return m_nframe;
}

int rkbAxisTicker::getAxisType() {
    return m_axis;
}

QString rkbAxisTicker::frameToTime(double frame) {
    int dur = m_vidlen*(double)frame/(double)m_nframe;
    QString hours, minutes, seconds, milliseconds;
    auto hrs = qFloor(dur / 3600000.0);
    if (hrs < 10)
    {
        hours = "0" + QString::number(hrs);
    }
    else {
        hours = QString::number(hrs);
    }
    auto mins = qFloor((dur - hrs*3600000.0) / 60000.0);
    if (mins < 10)
    {
        minutes = "0" + QString::number(mins);
    }
    else {
        minutes = QString::number(mins);
    }
    auto secs = qFloor((dur - hrs*3600000.0 - mins*60000.0) / 1000.0);
    if (secs < 10)
    {
        seconds = "0" + QString::number(secs);
    }
    else {
        seconds = QString::number(secs);
    }
    auto ms = (dur - hrs*3600000 - mins*60000 - secs*1000);
    if (ms < 10)
    {
        milliseconds = "0" + QString::number(ms);
    }
    else if (100 < ms && ms >= 10)
    {
        milliseconds = "" + QString::number(ms);
    }
    else {
        milliseconds = QString::number(ms);
    }
    QString time = hours + ":" + minutes +":" + seconds + ":" + milliseconds;
    return time;
}

QString rkbAxisTicker::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) {
    if (tick < 0 || tick > m_nframe) return "";
    if (m_axis == TIME) return frameToTime(tick);
    return locale.toString(tick, formatChar.toLatin1(), precision);
}
