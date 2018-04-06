#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QLabel>

class InfoWidget : public QLabel
{
    Q_OBJECT

public:
    InfoWidget(QWidget *parent = 0);

public slots:
    void documentChanged(int position, int charsRemoved, int charsAdded);
};

#endif // INFOWIDGET_H
