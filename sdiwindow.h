#ifndef SDIWINDOW_H
#define SDIWINDOW_H

#include <QMainWindow>

class QTextEdit;

class SdiWindow : public QMainWindow
{
    Q_OBJECT

public:
    SdiWindow(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createActions();
    void createMenus();
    void createToolbars();
    void createDocks();
    bool isSafeToClose();

private slots:
    void fileNew();

private:
    QTextEdit   *docWidget;
    QAction     *newAction;
    QAction     *cutAction;
    QAction     *copyAction;
    QAction     *pasteAction;
    QAction     *aboutQtAction;
    QAction     *closeAction;
    QAction     *exitAction;
    QMenu       *menu;
    QToolBar    *toolbar;
    QDockWidget *dock;
};

#endif // SDIWINDOW_H
