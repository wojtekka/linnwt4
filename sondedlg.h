//
// C++ Interface: sondedlg
//
// Description: 
//
//
// Author: Andreas Lindenau <DL4JAL@darc.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SONDEDLG_H
#define SONDEDLG_H

#include <qdialog.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>
#include <QScrollArea>

#include "konstdef.h"

class sdlg : public QDialog
{
    Q_OBJECT

public:
  sdlg(QWidget* parent=0, const char* name=0, bool modal=false);
  ~sdlg();
  void setdaten(const TSonde &asonde);
  TSonde getdaten();
  void tip(bool);

private slots:

private:
  TSonde osonde;
  
  QScrollArea *scrollBar;
  QPushButton *ok;
  QPushButton *cancel;
  //QGroupBox *gkopf;
  //QLabel *label;
  QLabel *labelstr;
  QLabel *labeloff;
  //QLineEdit *sname;
  QGroupBox *goffset;
  QLineEdit *stroffset[30];
  QLineEdit *offset[30];
};

#endif // SONDEDLG_H
