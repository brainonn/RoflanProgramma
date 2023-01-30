#ifndef HOVERCUSTOMPLOT_H
#define HOVERCUSTOMPLOT_H

#include "qcustomplot.h"
class hoverCustomPlot : public QCustomPlot
{
public:
    hoverCustomPlot();
protected:
    leaveEvent();
};

#endif // HOVERCUSTOMPLOT_H
