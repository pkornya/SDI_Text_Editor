#include "sdiwindow.h"
#include "infowidget.h"
#include <QApplication>
#include <QTextEdit>
#include <QStatusBar>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDockWidget>

SdiWindow::SdiWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString("%1[*] - %2").arg("unnamed").arg("SDI"));

    docWidget = new QTextEdit(this);
    setCentralWidget(docWidget);

    connect(docWidget->document(), SIGNAL(modificationChanged(bool)),
            this, SLOT(setWindowModified(bool)));

    createDocks();
    createActions();
    createMenus();
    createToolbars();

    statusBar()->showMessage( "Done" );
}

void SdiWindow::createActions()
{
    newAction = new QAction(tr("&New"), this);
    newAction->setShortcut(tr("Ctrl+N") );
    newAction->setStatusTip(tr("Create a new document"));
    connect(newAction, SIGNAL(triggered()),
            this, SLOT(fileNew()));

    cutAction = new QAction(tr("&Cut"), this);
    cutAction->setShortcut (tr("Ctrl+X"));
    cutAction->setStatusTip (tr("Cut"));
    cutAction->setEnabled(false);
    connect(docWidget, SIGNAL(copyAvailable(bool)),
            cutAction, SLOT(setEnabled(bool)));
    connect(cutAction, SIGNAL(triggered()),
            docWidget, SLOT(cut()));

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setShortcut (tr("Ctrl+C"));
    copyAction->setStatusTip (tr("Copy"));
    //copyAction->setEnabled(false);
    connect(copyAction, SIGNAL(triggered()),
            docWidget, SLOT(copy()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setShortcut (tr("Ctrl+V"));
    pasteAction->setStatusTip (tr("Paste"));
    //cutAction->setEnabled(false);
    connect(pasteAction, SIGNAL(triggered()),
            docWidget, SLOT(paste()));

    aboutQtAction = new QAction( tr("About &Qt"), this );
    aboutQtAction->setStatusTip( tr("About the Qt toolkit") );
    connect(aboutQtAction, SIGNAL(triggered()),
             qApp, SLOT(aboutQt()));

    closeAction = new QAction(tr("&Close"), this);
    closeAction->setStatusTip(tr("Close this window"));
    connect(closeAction, SIGNAL(triggered()),
            this, SLOT(close()));

    exitAction = new QAction(tr("&Exit"), this);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()),
            qApp, SLOT(closeAllWindows()));
}

void SdiWindow::createMenus()
{
    menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(newAction);
    menu->addAction(closeAction);
    menu->addSeparator();
    menu->addAction(exitAction);

    menu = menuBar()->addMenu(tr("&Edit"));
    menu->addAction(cutAction);
    menu->addAction(copyAction);
    menu->addAction(pasteAction);

    menu = menuBar()->addMenu(tr("&View"));
    menu->addAction(dock->toggleViewAction());

    menu = menuBar()->addMenu(tr("&Help"));
    menu->addAction(aboutQtAction);
}

void SdiWindow::createToolbars()
{
    toolbar = addToolBar(tr("File"));
    toolbar->addAction(newAction);
    toolbar->addSeparator();
    toolbar->addAction(cutAction);
    toolbar->addAction(aboutQtAction);
}

void SdiWindow::createDocks()
{
    dock = new QDockWidget(tr("Information"), this);
    InfoWidget *info = new InfoWidget(dock);
    dock->setWidget(info);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(docWidget->document(), SIGNAL(contentsChange(int,int,int)),
            info, SLOT(documentChanged(int,int,int)));
}

void SdiWindow::fileNew()
{
    (new SdiWindow())->show();
}

void SdiWindow::closeEvent(QCloseEvent *event)
{
    if (isSafeToClose())
        event->accept();
    else
        event->ignore();
}

bool SdiWindow::isSafeToClose()
{
    if (isWindowModified()) {
        switch( QMessageBox::warning( this, tr("SDI"),
            tr("The document has unsaved changes.\n"
                "Do you want to save it before it is closed?"),
                QMessageBox::Discard | QMessageBox::Cancel)) {
        case QMessageBox::Cancel:
            return false;
        default:
            return true;
        }
    }

    return true;
}
