#ifndef PROFILDLG_H
#define PROFILDLG_H

#include <qdialog.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qstring.h>

#include "konstdef.h"

class profildlg : public QDialog
{
    Q_OBJECT

public:
  profildlg(QWidget* parent=0, Qt::WindowFlags flags=Qt::WindowFlags());
  void setprofil(TProfil aprofil,int index);
  TProfil getprofil(int);
  void tip(bool atip);
  
private slots:
  QString linenormalisieren(const QString &);
  void allespruefen();
  void allesloeschen();

private:
//  tprofil profilarray[profilanz];
  
  QPushButton *ok;
  QPushButton *cancel;
  QPushButton *pruefen;
  QPushButton *loeschen;
  
  QLineEdit *eanfang[profilanz];
  QLineEdit *eende[profilanz];
  QLineEdit *eschritte[profilanz];
  QLineEdit *ebeschreibung[profilanz];
  QLabel *labelbeschreibung;
  QLabel *labelanfang;
  QLabel *labelende;
  QLabel *labelschritte;
  
};

#endif // PROFILDLG_H
