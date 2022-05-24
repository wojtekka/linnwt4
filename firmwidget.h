#ifndef FIRMWIDGET_H
#define FIRMWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QString>

#include "konstdef.h"

class firmwidget : public QDialog
{
    Q_OBJECT

public:
  firmwidget(QWidget* parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
  void setText(QString s);

private:
  QLabel *label1;
};

#endif // FIRMWIDGET_H
