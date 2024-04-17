/**
 * @file mainwindow.cpp
 * @author Imiloin
 */

#include "mainwindow.h"

#include "Circuit.h"
#include "call_parser.h"

#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QtWidgets>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("Spicial"));
    setWindowIcon(QIcon(":/icons/spicial"));

    textEdit = new QTextEdit(this);
    setCentralWidget(textEdit);

    createActions();
    createMenus();
    createToolBars();

    resize(800, 600);
}

MainWindow::~MainWindow() {}

/**
 * @brief Define and connect the actions.
 */
void MainWindow::createActions() {
    /** @brief file new action */
    fileNewAction = new QAction(QIcon(":/icons/filenew"), tr("New"), this);
    fileNewAction->setShortcut(Qt::CTRL + Qt::Key_N);
    fileNewAction->setStatusTip(tr("New file"));
    connect(fileNewAction, SIGNAL(triggered()), this, SLOT(slotNewFile()));

    /** @brief file open action */
    fileOpenAction = new QAction(QIcon(":/icons/fileopen"), tr("Open"), this);
    fileOpenAction->setShortcut(Qt::CTRL + Qt::Key_O);
    fileOpenAction->setStatusTip(tr("Open file"));
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(slotOpenFile()));   

    /** @brief save file action */
    fileSaveAction = new QAction(QPixmap(":/icons/filesave"), tr("Save"), this);
    fileSaveAction->setShortcut(Qt::CTRL + Qt::Key_S);
    fileSaveAction->setStatusTip(tr("Save file"));
    connect(fileSaveAction, SIGNAL(triggered()), this, SLOT(slotSaveFile()));

    /** @brief copy action */
    copyAction = new QAction(QIcon(":/icons/editcopy"), tr("Copy"), this);
    copyAction->setShortcut(Qt::CTRL + Qt::Key_C);
    copyAction->setStatusTip(tr("Copy to clipboard"));
    connect(copyAction, SIGNAL(triggered()), textEdit, SLOT(copy()));
    
    /** @brief cut action */
    cutAction = new QAction(QIcon(":/icons/editcut"), tr("Cut"), this);
    cutAction->setShortcut(Qt::CTRL + Qt::Key_X);
    cutAction->setStatusTip(tr("Cut to clipboard"));
    connect(cutAction, SIGNAL(triggered()), textEdit, SLOT(cut()));

    /** @brief paste action */
    pasteAction = new QAction(QIcon(":/icons/editpaste"), tr("Paste"), this);
    pasteAction->setShortcut(Qt::CTRL + Qt::Key_V);
    pasteAction->setStatusTip(tr("Paste clipboard to selection"));
    connect(pasteAction, SIGNAL(triggered()), textEdit, SLOT(paste()));

    /** @brief simulate action */
    simulateAction =
        new QAction(QIcon(":/icons/simulate"), tr("simulate"), this);
    simulateAction->setShortcut(Qt::CTRL + Qt::Key_R);
    simulateAction->setToolTip(tr("Do netlist simulation"));
    connect(simulateAction, SIGNAL(triggered()), this, SLOT(slotSimulate()));

    /** @brief parse action */
    parseAction = new QAction(QIcon(":/icons/parse"), tr("Parse"), this);
    parseAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);
    parseAction->setStatusTip(tr("Parse"));
    connect(parseAction, SIGNAL(triggered()), this, SLOT(slotParse()));
    
    /** @brief debug action */
    debugAction = new QAction(QIcon(":/icons/debug"), tr("Debug"), this);
    debugAction->setShortcut(Qt::Key_F11);
    debugAction->setStatusTip(tr("Debug"));
    connect(debugAction, SIGNAL(triggered()), this, SLOT(slotDebug()));
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("File"));
    editMenu = menuBar()->addMenu(tr("Edit"));
    simulateMenu = menuBar()->addMenu(tr("Simulate"));
    debugMenu = menuBar()->addMenu(tr("Debug"));

    fileMenu->addAction(fileNewAction);
    fileMenu->addSeparator();  /// Add separator between 2 actions.
    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(fileSaveAction);

    editMenu->addAction(copyAction);
    editMenu->addAction(cutAction);
    editMenu->addAction(pasteAction);

    simulateMenu->addAction(simulateAction);

    debugMenu->addAction(parseAction);
    debugMenu->addAction(debugAction);
}

void MainWindow::createToolBars() {
    /// You can use multiple toolbars. Actions are separated in the interface.
    fileTool = addToolBar(tr("File"));
    editTool = addToolBar(tr("Edit"));
    simulateTool = addToolBar(tr("Simulate"));
    debugTool = addToolBar(tr("Debug"));

    fileTool->addAction(fileNewAction);
    fileTool->addAction(fileOpenAction);
    fileTool->addAction(fileSaveAction);

    editTool->addAction(copyAction);
    editTool->addAction(cutAction);
    editTool->addAction(pasteAction);

    simulateTool->addAction(simulateAction);

    debugTool->addAction(parseAction);
    debugTool->addAction(debugAction);
}

/**
 * @brief New action will cover and create a new textedit.
 */
void MainWindow::slotNewFile() {
    textEdit->clear();           /// Clear the text
    textEdit->setHidden(false);  /// Display the text.
    fileName = "";               /// Reset fileName
}

/**
 * @brief Open action will open the saved files
 */
void MainWindow::slotOpenFile() {
    QString tempFileName = QFileDialog::getOpenFileName(this, tr("Open File"), tr(""),
                                            "SPICE Netlist (*.sp)");

    // qDebug() << tempFileName;

    /// If the dialog is directly closed, the tempFileName will be null.
    if (tempFileName == "" || tempFileName == "./") {
        return;
    } else {
        QFile file(tempFileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open file!"));
            return;
        } else {
            if (!file.isReadable()) {
                QMessageBox::warning(this, tr("Error"),
                                     tr("The file is unreadable"));
            } else {
                QTextStream textStream(&file);  // Use QTextStream to load text.
                textEdit->setPlainText(textStream.readAll());
                file.close();
                fileName = tempFileName; // Update fileName only after successful file read
            }
        }
    }
}

/**
 * @brief Save action will save the text into a new file when it has not been
 * saved. Otherwise it will be saved in the current file.
 */
void MainWindow::slotSaveFile() {
    statusBar()->showMessage(tr("Saving file..."));

    if (fileName == "")  /// File has not been saved.
    {
        /// Text is empty.
        if (textEdit->toPlainText() == "") {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("Content cannot be empty!"),
                                 QMessageBox::Ok);
        } else {
            QFileDialog fileDialog;
            fileName = fileDialog.getSaveFileName(this, tr("Open File"), "./",
                                                  "Text File(*.txt)");
            if (fileName == "") {
                return;
            }
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::warning(this, tr("Error"),
                                     tr("Failed to open file!"),
                                     QMessageBox::Ok);
                return;
            } else {
                /// Create a text stream and pass text in.
                QTextStream textStream(&file);
                QString str = textEdit->toPlainText();
                textStream << str;
            }
            file.close();
        }
    } else {  /// File has been saved.
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("Failed to open file!"));
            return;
        } else {
            QTextStream textStream(&file);
            QString str = textEdit->toPlainText();
            textStream << str;
            file.close();
        }
    }
}

void MainWindow::slotSimulate() {
    qDebug() << "slotSimulate()" << fileName;

    std::cout << "-------------------------------Simulation--------------------"
                 "-----------"
              << std::endl;

    std::unique_ptr<Netlist> netlist(  // use unique_ptr to avoid memory leak.
        callNetlistParser(fileName.toStdString().c_str()));
    if (!netlist) {
        qDebug() << "Call parser but return nullptr\n";
        return;
    }

    Circuit circuit(*netlist);

    circuit.runSimulations();

    circuit.outputResults();

    std::cout << "-------------------------------Simulation--------------------"
                 "-----------"
              << std::endl;
}

void MainWindow::slotParse() {
    qDebug() << "slotParse()" << fileName;

    std::cout << "-------------------------------Parsing--------------------"
                 "-----------"
              << std::endl;

    std::unique_ptr<Netlist> netlist(  // use unique_ptr to avoid memory leak.
        callNetlistParser(fileName.toStdString().c_str()));
    if (!netlist) {
        qDebug() << "Call parser but return nullptr\n";
        return;
    }

    std::cout << "-------------------------------Parsing--------------------"
                 "-----------"
              << std::endl;
}

void MainWindow::slotDebug() {
    qDebug() << "slotDebug()";

    std::cout << "********************************DEBUG************************"
                 "*********"
              << std::endl;

    std::unique_ptr<Netlist> netlist(
        callNetlistParser(fileName.toStdString().c_str()));
    if (!netlist) {
        qDebug() << "Call parser but return nullptr\n";
        return;
    }

    Circuit circuit(*netlist);

    circuit.printNodes();

    circuit.printBranches();

    circuit.printModels();

    circuit.printComponentSize();

    circuit.printMNATemplate();

    circuit.runSimulations();

    circuit.outputResults();

    std::cout << "********************************DEBUG************************"
                 "*********"
              << std::endl;
}
