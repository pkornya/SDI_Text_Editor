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

    bool saveFile(const QString &filename);
    void loadFile(const QString &filename);

private slots:
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();

private:
    QTextEdit   *docWidget;
    QString     currentFilename;
    QDockWidget *dock;

    QMenu       *menu;
    QToolBar    *toolbar;

    QAction     *newAction;
    QAction     *openAction;
    QAction     *saveAction;
    QAction     *saveAsAction;
    QAction     *closeAction;
    QAction     *exitAction;

    QAction     *undoAction;
    QAction     *redoAction;
    QAction     *cutAction;
    QAction     *copyAction;
    QAction     *pasteAction;


    QAction     *aboutQtAction;
};

#endif // SDIWINDOW_H
