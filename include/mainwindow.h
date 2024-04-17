/**
 * @file mainwindow.h
 * @author Imiloin
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QAction;
class QMenu;
class QToolBar;
class QTextEdit;
class QWidget;
class QTextStream;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /** @brief constructor */
    MainWindow(QWidget *parent = 0);
    /** @brief deconstructor */
    ~MainWindow();
    /** @brief All of the menus should be defined in this function*/
    void createMenus();
    /** @brief All of the actions should be defined in this function*/
    void createActions();
    /** @brief All of the toolbars should be defined in this function*/
    void createToolBars();

public slots:
    void slotNewFile();
    void slotOpenFile();
    void slotSaveFile();
    void slotSimulate();
    void slotParse();
    void slotDebug();
    

private:
    QMenu    *fileMenu;
    QMenu    *editMenu;
    QMenu    *simulateMenu;
    QMenu    *debugMenu;

    QToolBar *fileTool;
    QToolBar *editTool;
    QToolBar *simulateTool;
    QToolBar *debugTool;

    QAction *fileOpenAction;
    QAction *fileNewAction;
    QAction *fileSaveAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;

    QAction *simulateAction;

    QAction *parseAction;
    QAction *debugAction;

    QTextEdit *textEdit;

    QString fileName = "./";
};

#endif // MAINWINDOW_H
