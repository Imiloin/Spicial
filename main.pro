TEMPLATE = app
TARGET = bin/spicial
INCLUDEPATH += $$PWD/include

QT += widgets printsupport

CONFIG += c++17  # enable C++17 support
CONFIG += debug
#CONFIG += release

CONFIG(debug, release|debug) {
    message("Compiling in debug mode.")
}

CONFIG(release, release|debug) {
    message("Compiling in release mode.")
}

macx{
    CONFIG -= sdk_no_version_check
    CONFIG -= app_bundle
    message("Compiling on macOS.")
}

MOC_DIR     = build
OBJECTS_DIR = build
RCC_DIR     = build
UI_DIR      = build

LIBS += -larmadillo

HEADERS += include/*.h \
           src/parser/scanner.hpp \
           src/parser/parser.hpp \

SOURCES += src/main.cpp \
           src/parser/*.cpp \
           src/mainwindow/*.cpp \
           src/netlist/*.cpp \
           src/circuit/*.cpp \
           src/simulation/*.cpp \
           src/plot/*.cpp


RESOURCES += src/mainwindow/mainwindow.qrc
