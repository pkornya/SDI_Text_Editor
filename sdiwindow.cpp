#include <QApplication>
#include <QTextEdit>
#include <QStatusBar>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QPrintDialog>
#include <QPrinter>

#include "sdiwindow.h"
#include "infowidget.h"

SdiWindow::SdiWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString("%1[*] - %2").arg("Untitled").arg("SDI"));

    docWidget = new QTextEdit(this);
    setCentralWidget(docWidget);

    connect(docWidget->document(), SIGNAL(modificationChanged(bool)),
            this, SLOT(setWindowModified(bool)));

    createDocks();
    createActions();
    createMenus();
    createToolbars();

    statusBar()->showMessage("Done");

    foreach (QWidget *win, QApplication::topLevelWidgets()) {
        if (SdiWindow *mainWin = qobject_cast<SdiWindow *>(win))
            mainWin->updateRecentFileActions();
    }
}

void SdiWindow::createActions()
{
    newAction = new QAction(tr("New"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setStatusTip(tr("Create a new document"));
    connect(newAction, SIGNAL(triggered()),
            this, SLOT(fileNew()));

    openAction = new QAction(tr("Open"), this);
    openAction->setStatusTip(tr("Open a document"));
    connect(openAction, SIGNAL(triggered()),
            this, SLOT(fileOpen()));

    saveAction = new QAction(tr("Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save a document"));
    connect(saveAction, SIGNAL(triggered()),
            this, SLOT(fileSave()));

    saveAsAction = new QAction(tr("SaveAs"), this);
    saveAsAction->setStatusTip(tr("Save a document as"));
    connect(saveAsAction, SIGNAL(triggered()),
            this, SLOT(fileSaveAs()));

    printAction = new QAction(tr("Print"), this);
    printAction->setStatusTip(tr("Print information"));
    connect(printAction, SIGNAL(triggered(bool)),
            this, SLOT(print()));

    closeAction = new QAction(tr("Close"), this);
    closeAction->setStatusTip(tr("Close this window"));
    connect(closeAction, SIGNAL(triggered()),
            this, SLOT(close()));

    exitAction = new QAction(tr("Exit"), this);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()),
            qApp, SLOT(closeAllWindows()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    undoAction = new QAction(tr("Undo"), this);
    undoAction->setShortcut (tr("Ctrl+Z"));
    undoAction->setStatusTip (tr("Undo"));
    undoAction->setEnabled(false);
    connect(docWidget, SIGNAL(undoAvailable(bool)),
            undoAction, SLOT(setEnabled(bool)));
    connect(undoAction, SIGNAL(triggered()),
            docWidget, SLOT(undo()));

    redoAction = new QAction(tr("Redo"), this);
    redoAction->setShortcut (tr("Ctrl+Y"));
    redoAction->setStatusTip (tr("Redo"));
    redoAction->setEnabled(false);
    connect(docWidget, SIGNAL(redoAvailable(bool)),
            redoAction, SLOT(setEnabled(bool)));
    connect(redoAction, SIGNAL(triggered()),
            docWidget, SLOT(redo()));

    cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut (tr("Ctrl+X"));
    cutAction->setStatusTip (tr("Cut"));
    cutAction->setEnabled(false);
    connect(docWidget, SIGNAL(copyAvailable(bool)),
            cutAction, SLOT(setEnabled(bool)));
    connect(cutAction, SIGNAL(triggered()),
            docWidget, SLOT(cut()));

    copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut (tr("Ctrl+C"));
    copyAction->setStatusTip (tr("Copy"));
    copyAction->setEnabled(false);
    connect(docWidget, SIGNAL(copyAvailable(bool)),
            copyAction, SLOT(setEnabled(bool)));
    connect(copyAction, SIGNAL(triggered()),
            docWidget, SLOT(copy()));

    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut (tr("Ctrl+V"));
    pasteAction->setStatusTip (tr("Paste"));
    pasteAction->setEnabled(false);
    connect(pasteAction, SIGNAL(triggered()),
            docWidget, SLOT(paste()));

    aboutQtAction = new QAction( tr("About Qt"), this );
    aboutQtAction->setStatusTip( tr("About the Qt toolkit") );
    connect(aboutQtAction, SIGNAL(triggered()),
             qApp, SLOT(aboutQt()));
}

void SdiWindow::createMenus()
{
    menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(newAction);
    menu->addAction(openAction);
    menu->addSeparator();
    menu->addAction(saveAction);
    menu->addAction(saveAsAction);

    separatorAction = menu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        menu->addAction(recentFileActions[i]);
    menu->addSeparator();

    menu->addAction(printAction);
    menu->addSeparator();

    menu->addAction(closeAction);
    menu->addSeparator();
    menu->addAction(exitAction);

    menu = menuBar()->addMenu(tr("&Edit"));
    menu->addAction(undoAction);
    menu->addAction(redoAction);
    menu->addSeparator();
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
    toolbar->addAction(openAction);
    toolbar->addSeparator();
    toolbar->addAction(saveAction);
    toolbar->addAction(saveAsAction);
    toolbar->addSeparator();
    toolbar->addAction(cutAction);
    toolbar->addAction(copyAction);
    toolbar->addAction(pasteAction);
    toolbar->addAction(aboutQtAction);
}

void SdiWindow::createDocks()
{
    dock = new QDockWidget(tr("Information"), this);
    InfoWidget *info = new InfoWidget(dock);
    dock->setWidget(info);
    dock->setVisible(false);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(docWidget->document(), SIGNAL(contentsChange(int,int,int)),
            info, SLOT(documentChanged(int,int,int)));
}

void SdiWindow::fileNew()
{
    if (docWidget->document()->isEmpty() && !docWidget->document()->isModified())
        return;
    else
        (new SdiWindow())->show();
}

void SdiWindow::fileOpen()
{
    if (isSafeToClose()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool SdiWindow::fileSave()
{
    if (currentFilename.isEmpty())
        return fileSaveAs();
    else
        return saveFile(currentFilename);
}

bool SdiWindow::fileSaveAs()
{
    QString fileName =
    QFileDialog::getSaveFileName(this, tr("Save As"), currentFilename);

    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void SdiWindow::print()
{
    QPrinter printer;
    printer.setPrinterName("HP LaserJet 1020");

    QPrintDialog dialog(&printer, this);

    if (dialog.exec() == QDialog::Rejected)
        return;
    docWidget->print(&printer);
}

void SdiWindow::openRecentFile()
{
    if (isSafeToClose()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if(action)
            loadFile(action->data().toString());
    }
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
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel)) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            return fileSave();
        default:
            return true;
        }
    }

    return true;
}

void SdiWindow::readSettings()
{

}

void SdiWindow::writeSettings()
{

}

bool SdiWindow::saveFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("SDI"), tr("Failed to save file."));
        return false;
    }

    QTextStream stream(&file);
    stream << docWidget->toPlainText();

    setCurrentFile(filename);

    return true;
}

void SdiWindow::loadFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("SDI"), tr("Failed to open file."));
        return;
    }

    QTextStream stream(&file);
    docWidget->setPlainText(stream.readAll());

    setCurrentFile(filename);
}

void SdiWindow::updateRecentFileActions()
{
    QMutableListIterator<QString> i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles.count()) {
            QString text = tr("%1")
                           .arg(strippedName(recentFiles[j]));
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);
            recentFileActions[j]->setVisible(true);

        } else {
            recentFileActions[j]->setVisible(false);
        }
        separatorAction->setVisible(!recentFiles.isEmpty());
    }
}

QString SdiWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void SdiWindow::setCurrentFile(const QString &fileName)
{
    currentFilename = fileName;
    docWidget->document()->setModified(false);

    QString shownName = "Untitled";
    if (!currentFilename.isEmpty()) {
        shownName = strippedName(currentFilename);
        recentFiles.removeAll(currentFilename);
        recentFiles.prepend(currentFilename);
        updateRecentFileActions();
    }
    setWindowTitle(tr("%1[*] - %2").arg(shownName)
                                   .arg(tr("SDI")));
}
