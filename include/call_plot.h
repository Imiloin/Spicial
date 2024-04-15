#ifndef SPICIAL_CALL_PLOT_H
#define SPICIAL_CALL_PLOT_H

#include <QColor>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "Circuit.h"
#include "qcustomplot.h"

// Define color cycle
extern QList<QColor> colorCycle;

void callPlot(ColumnData xdata, std::vector<ColumnData> ydata, const std::string& title);

#endif  // SPICIAL_CALL_PLOT_H
