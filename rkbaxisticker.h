#ifndef RKBAXISTICKER_H
#define RKBAXISTICKER_H

#include "qcustomplot.h"

class QCP_LIB_DECL rkbAxisTicker : public QCPAxisTicker
{
public:
  rkbAxisTicker();
  // Setters
  void setFPS(double fps);
  void setVidLen(qint64 len);
  void setFrameNum(qint64 nframe);
  void setAxisType(qint64 type);

  // Getters
  QString frameToTime(double frame);
  double getFPS();
  qint64 getVidLen();
  qint64 getFrameNum();
  int getAxisType();

protected:
  // property members:
  double m_fps = 30.0;
  qint64 m_vidlen = 10000;
  qint64 m_nframe = 300;
  enum AxisType {TIME = 0, FIXED};
  int m_axis = TIME;

  // reimplemented virtual methods:
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) Q_DECL_OVERRIDE;
};


#endif // RKBAXISTICKER_H
