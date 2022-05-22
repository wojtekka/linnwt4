//
// C++ Interface: wkmanager
//
// Description: 
//
//
// Author: Andreas Lindenau <DL4JAL@darc.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WKMANAGER_H
#define WKMANAGER_H

#include <qpushbutton.h>
#include <QWidget>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <q3listbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qstring.h>
#include <QPen>
#include <QDir>

#include "widgetwobbeln.h"
#include "konstdef.h"

class owkmanager : public QWidget
{
    Q_OBJECT

public:
  owkmanager(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~owkmanager();
  void tip(bool);
  void setHomePath(QDir &);
  void ladenconfig();

public slots:
//  void tip(bool);
  void sopen1();
  void sopen2();
  void sopen3();
  void sopen4();
  void sopen5();
  void sopen6();

  void ssave1();
  void ssave2();
  void ssave3();
  void ssave4();
  void ssave5();
  void ssave6();

  void sholen1();
  void sholen2();
  void sholen3();
  void sholen4();
  void sholen5();
  void sholen6();

  void scolor11();
  void scolor12();
  void scolor21();
  void scolor22();  
  void scolor31();
  void scolor32();
  void scolor41();
  void scolor42();
  void scolor51();
  void scolor52();
  void scolor61();
  void scolor62();

  void saktiv1();
  void saktiv2();
  void saktiv3();
  void saktiv4();
  void saktiv5();
  void saktiv6();

  void saktivk11();
  void saktivk12();
  void saktivk21();
  void saktivk22();
  void saktivk31();
  void saktivk32();
  void saktivk41();
  void saktivk42();
  void saktivk51();
  void saktivk52();
  void saktivk61();
  void saktivk62();


  void suebergabe(const TWobbel &, const QString &);
  void beenden();
  void psetfontsize(int);
//  void setkdir(QString);

private slots:
  void save(int);
  
private:
  QDir homedir;
  int kurvennr;
  QLabel *labelk1; 
  QLabel *labelk2; 
  QLabel *labelk3; 
  QLabel *labelk4;
  QLabel *labelk5;
  QLabel *labelk6;

  QLabel *labelopen;
  QLabel *labelsave;
  QLabel *labeluebernahme;
  QLabel *labelcolor;
  QLabel *labelkanal;
  QLabel *labeleinblendung;
  QLabel *labelkanal1;
  QLabel *labelkanal2;
  QLabel *labelkanal3;
  QLabel *labelkanal4;
  QLabel *labelkanal5;
  QLabel *labelkanal6;

  QPushButton *open1;
  QPushButton *open2;
  QPushButton *open3;
  QPushButton *open4;
  QPushButton *open5;
  QPushButton *open6;

  QPushButton *save1;
  QPushButton *save2;
  QPushButton *save3;
  QPushButton *save4;
  QPushButton *save5;
  QPushButton *save6;

  QPushButton *uebernahme1;
  QPushButton *uebernahme2;
  QPushButton *uebernahme3;
  QPushButton *uebernahme4;
  QPushButton *uebernahme5;
  QPushButton *uebernahme6;

  QPushButton *color11;
  QPushButton *color12;
  QPushButton *color21;
  QPushButton *color22;
  QPushButton *color31;
  QPushButton *color32;
  QPushButton *color41;
  QPushButton *color42;
  QPushButton *color51;
  QPushButton *color52;
  QPushButton *color61;
  QPushButton *color62;

  QCheckBox *aktiv1;
  QCheckBox *aktiv2;
  QCheckBox *aktiv3;
  QCheckBox *aktiv4;
  QCheckBox *aktiv5;
  QCheckBox *aktiv6;

  QCheckBox *checkboxk11;
  QCheckBox *checkboxk12;
  QCheckBox *checkboxk21;
  QCheckBox *checkboxk22;
  QCheckBox *checkboxk31;
  QCheckBox *checkboxk32;
  QCheckBox *checkboxk41;
  QCheckBox *checkboxk42;
  QCheckBox *checkboxk51;
  QCheckBox *checkboxk52;
  QCheckBox *checkboxk61;
  QCheckBox *checkboxk62;

  TWobbel wobbeldaten;
  TWobbel wobbeldaten1;
  TWobbel wobbeldaten2;
  TWobbel wobbeldaten3;
  TWobbel wobbeldaten4;
  TWobbel wobbeldaten5;
  TWobbel wobbeldaten6;
  
  signals:
  void  loadkurve(int);
  void  savekurve(const TWobbel &);
  void  wkmsendwobbel(const TWobbel &);
  void  wkmgetwobbel(int);
//  void  setkurvendir(QString);
};

#endif // WKMANAGER_H
