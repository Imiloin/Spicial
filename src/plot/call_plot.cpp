#include "call_plot.h"
#include <QDebug>

// Initialize color cycle
QList<QColor> colorCycle = {
    QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"),
    QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"),
    QColor("#bcbd22"), QColor("#17becf")};

void callPlot(ColumnData xdata, std::vector<ColumnData> ydata) {
    qDebug() << "callPlot()";
    // add two new graphs and set their look:
    QCustomPlot* customPlot =
        new QCustomPlot;  // Declare and define the customPlot object

    // Convert xdata and ydata to QVector
    QVector<double> x = QVector<double>::fromStdVector(xdata.values);

    // Add graphs for each ydata
    for (size_t i = 0; i < ydata.size(); ++i) {
        QVector<double> y = QVector<double>::fromStdVector(ydata[i].values);

        customPlot->addGraph();
        customPlot->graph(i)->setData(x, y);
        customPlot->graph(i)->setName(
            QString::fromStdString(ydata[i].name));  // Add legend

        // Set color from color cycle
        QColor color = colorCycle.at(i % colorCycle.size());
        customPlot->graph(i)->setPen(QPen(color));
    }

    // Configure axes
    customPlot->xAxis->setLabel(
        QString::fromStdString(xdata.name));  // Add x-axis label
    customPlot->xAxis2->setVisible(true);
    customPlot->xAxis2->setTickLabels(false);
    customPlot->yAxis2->setVisible(true);
    customPlot->yAxis2->setTickLabels(false);

    // Set x-axis range
    double xMin = *std::min_element(x.begin(), x.end());
    double xMax = *std::max_element(x.begin(), x.end());
    customPlot->xAxis->setRange(xMin, xMax);

    // Decide whether to use logarithmic scale
    QVector<double> xCopy =
        x;  // Create a copy of x, because nth_element will rearrange elements
    std::nth_element(xCopy.begin(), xCopy.begin() + xCopy.size() / 2,
                     xCopy.end());
    double median = xCopy[xCopy.size() / 2];
    if (xMax / median > 1000) {
        customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        customPlot->xAxis->setTicker(logTicker);
    }

    // Rescale y-axis
    customPlot->yAxis->rescale();

    // Extend y-axis range by 10%
    double yRange = customPlot->yAxis->range().size();
    customPlot->yAxis->setRange(
        customPlot->yAxis->range().lower - 0.1 * yRange,
        customPlot->yAxis->range().upper + 0.1 * yRange);

    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and
    // select graphs by clicking
    customPlot->replot();
    customPlot->setMinimumSize(450, 300);
    customPlot->setAttribute(Qt::WA_DeleteOnClose);
    customPlot->legend->setVisible(true);  // Show legend
    customPlot->show();
}
