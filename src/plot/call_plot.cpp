#include "call_plot.h"
#include <QDebug>

// Initialize color cycle
QList<QColor> colorCycle = {
    QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"),
    QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"),
    QColor("#bcbd22"), QColor("#17becf")};

void callPlot(ColumnData xdata,
              std::vector<ColumnData> ydata,
              const std::string& title) {
    // qDebug() << "callPlot()";
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
    if (xMax / median > 100) {
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

    // Set legend position to top left
    // customPlot->axisRect()->insetLayout()->setInsetAlignment(
    //     0, Qt::AlignTop | Qt::AlignLeft);

    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and
    // select graphs by clicking
    customPlot->replot();
    customPlot->setMinimumSize(600, 400);
    customPlot->setAttribute(Qt::WA_DeleteOnClose);
    // customPlot->legend->setVisible(true);  // Show legend

    // 设置图例行优先排列
    customPlot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
    // 设置4个图例自动换行
    customPlot->legend->setWrap(4);
    // 设置图例可见
    customPlot->legend->setVisible(true);
    // 设置图例位置，这里选择显示在QCPAxisRect上方
    customPlot->plotLayout()->insertRow(0);
    customPlot->plotLayout()->addElement(0, 0, customPlot->legend);
    // 设置显示比例
    customPlot->plotLayout()->setRowStretchFactor(0, 0.001);
    // 设置边框隐藏
    customPlot->legend->setBorderPen(Qt::NoPen);

    customPlot->show();

    // 创建一个按钮
    QPushButton* saveButton =
        new QPushButton(QCoreApplication::translate("Context", "Save as PDF"));
    saveButton->setSizePolicy(
        QSizePolicy::Fixed, QSizePolicy::Fixed);  // 设置按钮的大小策略为 Fixed
    saveButton->setFixedHeight(30);  // 设置按钮的固定高度为 30

    QObject::connect(saveButton, &QPushButton::clicked, [title,customPlot]() {
        QString defaultFileName = QString::fromStdString(title) + ".pdf";
        // 创建一个文件对话框，让用户选择文件名和目录
        QString fileName = QFileDialog::getSaveFileName(
            customPlot, QCoreApplication::translate("Context", "Save File"),
            "./" + defaultFileName, QCoreApplication::translate("Context", "PDF Files (*.pdf)"));

        // 如果用户没有取消对话框
        if (!fileName.isEmpty()) {
            // 检查文件名是否包含 .pdf 后缀
            QFileInfo fileInfo(fileName);
            if (fileInfo.suffix().toLower() != "pdf") {
                // 如果不包含，添加 .pdf 后缀
                fileName.append(".pdf");
            }

            // 保存图像为 PDF 文件
            bool success = customPlot->savePdf(fileName);

            // 创建一个消息框，确认用户的选择
            QMessageBox msgBox;
            if (success) {
                msgBox.setText("The file has been saved successfully.");
            } else {
                msgBox.setText("Failed to save the file.");
            }
            msgBox.exec();
        }
    });

    // 创建一个水平布局并添加 saveButton
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch(1);  // 添加一个弹性空间，使按钮对齐到左侧

    // 创建一个布局并添加 customPlot 和 saveButton
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(customPlot);
    layout->addWidget(saveButton);

    // 创建一个窗口，设置其布局，并显示
    QWidget* window = new QWidget;
    window->setLayout(layout);
    window->setWindowTitle(QString::fromStdString(title));  // 设置窗口标题
    window->show();
}
