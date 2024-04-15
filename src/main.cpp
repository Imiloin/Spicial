#include <iostream>
#include <unordered_set>
#include "Circuit.h"

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char** args) {
    std::cout
        << "################################################################\n";
    std::cout << "spicial -- a circuit simulator\n";
    std::cout << "Author: Imiloin\n";
    std::cout << "Course MST4307 2024, SJTU\n";
    std::cout
        << "################################################################\n";

    QApplication app(argc, args);

    MainWindow mainwindow;
    mainwindow.resize(800, 600);
    mainwindow.show();

    return app.exec();
}
