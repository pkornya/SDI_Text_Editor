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
#include <QFontDialog>
#include <QColorDialog>
#include <QSettings>

#include "sdiwindow.h"
#include "infowidget.h"
#include "finddialog.h"

SdiWindow::SdiWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(QString("%1[*] - %2").arg("Untitled").arg("Notepad"));
    setWindowIcon(QIcon(":/images/images/logo.png"));

    findDialog = 0;

    docWidget = new QTextEdit(this);
    setCentralWidget(docWidget);

    connect(docWidget->document(), SIGNAL(modificationChanged(bool)),
            this, SLOT(setWindowModified(bool)));

    createDocks();
    createActions();
    createMenus();
    createToolbars();

    readSettings();

    statusBar()->showMessage("Ready");

    foreach (QWidget *win, QApplication::topLevelWidgets()) {
        if (SdiWindow *mainWin = qobject_cast<SdiWindow *>(win))
            mainWin->updateRecentFileActions();
    }
}

void SdiWindow::createActions()
{
    newAction = new QAction(tr("New"), this);
    newAction->setIcon(QIcon(":/images/images/new.png"));
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setStatusTip(tr("Create a new document"));
    connect(newAction, SIGNAL(triggered()),
            this, SLOT(fileNew()));

    openAction = new QAction(tr("Open"), this);
    openAction->setIcon(QIcon(":/images/images/open.png"));
    openAction->setShortcut(tr("Ctrl+M"));
    openAction->setStatusTip(tr("Open a document"));
    connect(openAction, SIGNAL(triggered()),
            this, SLOT(fileOpen()));

    saveAction = new QAction(tr("Save"), this);
    saveAction->setIcon(QIcon(":/images/images/save.png"));
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save a document"));
    connect(saveAction, SIGNAL(triggered()),
            this, SLOT(fileSave()));

    saveAsAction = new QAction(tr("SaveAs"), this);
    saveAsAction->setIcon(QIcon(":/images/images/saveAs.png"));
    saveAsAction->setShortcut(tr("Ctrl+T"));
    saveAsAction->setStatusTip(tr("Save a document as"));
    connect(saveAsAction, SIGNAL(triggered()),
            this, SLOT(fileSaveAs()));

    printAction = new QAction(tr("Print"), this);
    printAction->setIcon(QIcon(":/images/images/print.png"));
    printAction->setShortcut(tr("Ctrl+P"));
    printAction->setStatusTip(tr("Print information"));
    connect(printAction, SIGNAL(triggered(bool)),
            this, SLOT(print()));

    closeAction = new QAction(tr("Close"), this);
    closeAction->setIcon(QIcon(":/images/images/close.png"));
    closeAction->setShortcut(tr("Ctrl+W"));
    closeAction->setStatusTip(tr("Close this window"));
    connect(closeAction, SIGNAL(triggered()),
            this, SLOT(close()));

    exitAction = new QAction(tr("Exit"), this);
    exitAction->setIcon(QIcon(":/images/images/exit.png"));
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()),
            qApp, SLOT(closeAllWindows()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setIcon(QIcon(":/images/images/recentfile.png"));
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    undoAction = new QAction(tr("Undo"), this);
    undoAction->setIcon(QIcon(":/images/images/undo.png"));
    undoAction->setShortcut (tr("Ctrl+Z"));
    undoAction->setStatusTip (tr("Undo"));
    undoAction->setEnabled(false);
    connect(docWidget, SIGNAL(undoAvailable(bool)),
            undoAction, SLOT(setEnabled(bool)));
    connect(undoAction, SIGNAL(triggered()),
            docWidget, SLOT(undo()));

    redoAction = new QAction(tr("Redo"), this);
    redoAction->setIcon(QIcon(":/images/images/redo.png"));
    redoAction->setShortcut (tr("Ctrl+Y"));
    redoAction->setStatusTip (tr("Redo"));
    redoAction->setEnabled(false);
    connect(docWidget, SIGNAL(redoAvailable(bool)),
            redoAction, SLOT(setEnabled(bool)));
    connect(redoAction, SIGNAL(triggered()),
            docWidget, SLOT(redo()));

    cutAction = new QAction(tr("Cut"), this);
    cutAction->setIcon(QIcon(":/images/images/cut.png"));
    cutAction->setShortcut (tr("Ctrl+X"));
    cutAction->setStatusTip (tr("Cut"));
    cutAction->setEnabled(false);
    connect(docWidget, SIGNAL(copyAvailable(bool)),
            cutAction, SLOT(setEnabled(bool)));
    connect(cutAction, SIGNAL(triggered()),
            docWidget, SLOT(cut()));

    copyAction = new QAction(tr("Copy"), this);
    copyAction->setIcon(QIcon(":/images/images/copy.png"));
    copyAction->setShortcut (tr("Ctrl+C"));
    copyAction->setStatusTip (tr("Copy"));
    copyAction->setEnabled(false);
    connect(docWidget, SIGNAL(copyAvailable(bool)),
            copyAction, SLOT(setEnabled(bool)));
    connect(copyAction, SIGNAL(triggered()),
            docWidget, SLOT(copy()));

    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setIcon(QIcon(":/images/images/paste.png"));
    pasteAction->setShortcut (tr("Ctrl+V"));
    pasteAction->setStatusTip (tr("Paste"));
    connect(pasteAction, SIGNAL(triggered()),
            docWidget, SLOT(paste()));

    deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setIcon(QIcon(":/images/images/delete.png"));
    deleteAction->setShortcut (tr("Delete"));
    deleteAction->setStatusTip (tr("Delete"));
    connect(deleteAction, SIGNAL(triggered()),
            this, SLOT(deleteText()));

    selectAllAction = new QAction(tr("Select All"), this);
    selectAllAction->setIcon(QIcon(":/images/images/selectall.png"));
    selectAllAction->setShortcut (tr("Ctrl+A"));
    selectAllAction->setStatusTip (tr("Select All"));
    connect(selectAllAction, SIGNAL(triggered()),
            docWidget, SLOT(selectAll()));

    findAction = new QAction(tr("Find"), this);
    findAction->setIcon(QIcon(":/images/images/find.png"));
    findAction->setShortcut (tr("Ctrl+F"));
    findAction->setStatusTip (tr("Find Information"));
    connect(findAction, SIGNAL(triggered()),
            this, SLOT(find()));

    fontAction = new QAction(tr("Font"), this);
    fontAction->setIcon(QIcon(":/images/images/font.png"));
    fontAction->setStatusTip (tr("Choose font for text"));
    connect(fontAction, SIGNAL(triggered()),
            this, SLOT(changeFont()));

    colorAction = new QAction(tr("Color"), this);
    colorAction->setIcon(QIcon(":/images/images/color.png"));
    colorAction->setStatusTip (tr("Choose color for text"));
    connect(colorAction, SIGNAL(triggered()),
            this, SLOT(changeColor()));

    backgroundColorAction = new QAction(tr("Background Color"), this);
    backgroundColorAction->setIcon(QIcon(":/images/images/backgroundcolor.png"));
    backgroundColorAction->setStatusTip (tr("Choose background color for text"));
    connect(backgroundColorAction, SIGNAL(triggered()),
            this, SLOT(changeBackgroundColor()));

    aboutQtAction = new QAction( tr("About Qt"), this );
    aboutQtAction->setIcon(QIcon(":/images/images/about.png"));
    aboutQtAction->setStatusTip( tr("About the Qt toolkit") );
    connect(aboutQtAction, SIGNAL(triggered()),
             this, SLOT(about()));
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
    menu->addAction(deleteAction);
    menu->addSeparator();
    menu->addAction(selectAllAction);
    menu->addAction(findAction);

    menu = menuBar()->addMenu(tr("&Format"));
    menu->addAction(fontAction);
    menu->addAction(colorAction);
    menu->addAction(backgroundColorAction);

    //menu = menuBar()->addMenu(tr("&View"));
    //menu->addAction(dock->toggleViewAction());

    menu = menuBar()->addMenu(tr("&Help"));
    menu->addAction(aboutQtAction);
}

void SdiWindow::createToolbars()
{
    toolbar = addToolBar(tr("File"));
    toolbar->addAction(newAction);
    toolbar->addAction(openAction);
    toolbar->addAction(saveAction);
    toolbar->addAction(saveAsAction);
    toolbar->addAction(printAction);
    toolbar->addSeparator();

    toolbar->addAction(undoAction);
    toolbar->addAction(redoAction);
    toolbar->addAction(cutAction);
    toolbar->addAction(copyAction);
    toolbar->addAction(pasteAction);
    toolbar->addAction(selectAllAction);
    toolbar->addAction(findAction);
    toolbar->addSeparator();

    toolbar->addAction(fontAction);
    toolbar->addAction(colorAction);
    toolbar->addAction(backgroundColorAction);
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

        QString filter = tr("Notepad files (*.nt)");
        QString fileName = QFileDialog::getOpenFileName(this,
                                        tr("Open Notepad"), ".",
                                        filter);

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

void SdiWindow::deleteText()
{
     QTextCursor cursor = docWidget->textCursor();
     cursor.removeSelectedText();
}

void SdiWindow::openRecentFile()
{
    if (isSafeToClose()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if(action)
            loadFile(action->data().toString());
    }
}

void SdiWindow::changeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if (ok) {
        docWidget->setFont(font);
    } else
        return;
}

void SdiWindow::changeColor()
{
    QColor color = QColorDialog::getColor(Qt::gray, this, "Choose color");
    if (color.isValid()) {
        docWidget->setTextColor(color);
    }
}

void SdiWindow::changeBackgroundColor()
{
    QColor color = QColorDialog::getColor(Qt::gray, this, "Choose background color");
    if (color.isValid()) {
        docWidget->setTextBackgroundColor(color);
    }
}

void SdiWindow::findNext(const QString &str, Qt::CaseSensitivity cs)
{
    bool isFound;
    if (cs == Qt::CaseSensitive) {
        isFound = docWidget->find(str, QTextDocument::FindCaseSensitively);
        if (!isFound)
            QMessageBox::warning(this, "Unsuccessful search",
                                 "Could not find anything by your request");
    }

    if (cs == Qt::CaseInsensitive) {
        isFound = docWidget->find(str);
        if (!isFound)
            QMessageBox::warning(this, "Unsuccessful search",
                                 "Could not find anything by your request");
    }

}

void SdiWindow::findPrevious(const QString &str, Qt::CaseSensitivity cs)
{
    bool isFound;
    if (cs == Qt::CaseSensitive) {
        isFound = docWidget->find(str, QTextDocument::FindBackward |
                             QTextDocument::FindCaseSensitively);
        if (!isFound)
            QMessageBox::warning(this, "Unsuccessful search",
                                 "Could not find anything by your request");
    }

    if (cs == Qt::CaseInsensitive) {
        isFound = docWidget->find(str, QTextDocument::FindBackward);
        if (!isFound)
            QMessageBox::warning(this, "Unsuccessful search",
                                 "Could not find anything by your request");
    }
}

void SdiWindow::find()
{
    if (!findDialog) {
        findDialog = new FindDialog(this);
        connect(findDialog, SIGNAL(findNext(const QString&,
                                            Qt::CaseSensitivity)),
                this, SLOT(findNext(const QString&,
                                           Qt::CaseSensitivity)));
        connect(findDialog, SIGNAL(findPrevious(const QString&,
                                            Qt::CaseSensitivity)),
                this, SLOT(findPrevious(const QString&,
                                           Qt::CaseSensitivity)));
    }

    findDialog->show();
    findDialog->activateWindow();
}

void SdiWindow::about()
{
    QMessageBox::about(this, tr("About Notepad"),
            tr("<h2>Notepad 3.3 </h2>"
            "<p>Copyright &copy; 2018 Software Inc."
            "<p>Notepad is a small apllication which "
               "gives users an opportunity to work with text files"
               "and edit them. You can also print files."));
}

void SdiWindow::closeEvent(QCloseEvent *event)
{
    if (isSafeToClose()) {
        writeSettings();
        event->accept();
    }
    else {
        event->ignore();
    }
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
    QSettings settings("Software Inc.", "Notepad");

    QRect rect = settings.value("geometry",
                                QRect(200, 200, 400, 400)).toRect();
    move(rect.topLeft());
    resize(rect.size());

    recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFileActions();

    QFont font;
    font.setPointSize(20);
    font.setFamily("Times New Roman");
    docWidget->setFont(font);
}

void SdiWindow::writeSettings()
{
    QSettings settings("Software Inc.", "Notepad");

    settings.setValue("geometry", geometry());
    settings.setValue("recentFiles", recentFiles);
}

bool SdiWindow::saveFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Notepad"), tr("Failed to save file."));
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
        QMessageBox::warning(this, tr("Notepad"), tr("Failed to open file."));
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
                                   .arg(tr("Notepad")));
}
