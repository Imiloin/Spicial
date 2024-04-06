#ifndef SPICIAL_CALL_PLOT_H
#define SPICIAL_CALL_PLOT_H

#include "Circuit.h"
#include "qcustomplot.h"
#include <QColor>
#include <QList>

// Define color cycle
extern QList<QColor> colorCycle;

void callPlot(ColumnData xdata, std::vector<ColumnData> ydata);

#endif  // SPICIAL_CALL_PLOT_H
