#ifndef FMARKEDLG_H
#define FMARKEDLG_H

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

class fmdlg : public QDialog
{
    Q_OBJECT

public:
  fmdlg(QWidget* parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

  void setdaten(TFrqmarken afmarken);
  TFrqmarken getdaten();

private:
  TFrqmarken fmarken;
  
  QPushButton *ok;
  QPushButton *cancel;
  QLineEdit *l160m1;
  QLineEdit *l80m1;
  QLineEdit *l40m1;
  QLineEdit *l30m1;
  QLineEdit *l20m1;
  QLineEdit *l17m1;
  QLineEdit *l15m1;
  QLineEdit *l12m1;
  QLineEdit *l10m1;
  QLineEdit *l6m1;
  QLineEdit *l2m1;
  QLineEdit *lu11;
  QLineEdit *lu21;
  QLineEdit *lu31;
  QLineEdit *l160m2;
  QLineEdit *l80m2;
  QLineEdit *l40m2;
  QLineEdit *l30m2;
  QLineEdit *l20m2;
  QLineEdit *l17m2;
  QLineEdit *l15m2;
  QLineEdit *l12m2;
  QLineEdit *l10m2;
  QLineEdit *l6m2;
  QLineEdit *l2m2;
  QLineEdit *lu12;
  QLineEdit *lu22;
  QLineEdit *lu32;
  QCheckBox *check160m;
  QCheckBox *check80m;
  QCheckBox *check40m;
  QCheckBox *check30m;
  QCheckBox *check20m;
  QCheckBox *check17m;
  QCheckBox *check15m;
  QCheckBox *check12m;
  QCheckBox *check10m;
  QCheckBox *check6m;
  QCheckBox *check2m;
  QCheckBox *checku1;
  QCheckBox *checku2;
  QCheckBox *checku3;
  QLineEdit *s160m;
  QLineEdit *s80m;
  QLineEdit *s40m;
  QLineEdit *s30m;
  QLineEdit *s20m;
  QLineEdit *s17m;
  QLineEdit *s15m;
  QLineEdit *s12m;
  QLineEdit *s10m;
  QLineEdit *s6m;
  QLineEdit *s2m;
  QLineEdit *su1;
  QLineEdit *su2;
  QLineEdit *su3;
};

#endif // FMARKEDLG_H
