//
// C++ Implementation: sondedlg
//
// Description: 
//
//
// Author: Andreas Lindenau <DL4JAL@darc.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
//
// C++ Implementation: optiondlg
//
// Description: 
//
//
// Author: Andreas Lindenau <DL4JAL@darc.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qdialog.h>

#include "sondedlg.h"
//#include "tooltipsonde.h"


//#####################################################################################
// Class fuer Option
//#####################################################################################
sdlg::sdlg(QWidget* parent, const char* name, bool modal): QDialog(parent, name, modal)
{
  this->resize(450,1090);
  this->setMinimumSize(0,0);
  ok = new QPushButton(tr("Speichern","Sonde Dialog"), this);
  ok->setGeometry(10,10,100,30);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  cancel = new QPushButton(tr("Abbruch","Sonde Dialog"), this);
  cancel->setGeometry(120,10,100,30);
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
//  gkopf = new QGroupBox(this);
//  gkopf->setGeometry(150,10,270,60);
//  gkopf->setTitle(tr("Messkopf","Sonde Dialog"));
//  label = new QLabel(gkopf);
//  label->setGeometry(10,25,60,30);
//  label->setText(tr("Datei","Sonde Dialog"));
//  label->setAlignment(Qt::AlignRight);
//  sname = new QLineEdit(gkopf);
//  sname->setReadOnly(TRUE);
//  sname->setGeometry(80,20,170,30);
  goffset = new QGroupBox(this);
  goffset->setGeometry(10,60,430,1020);
  goffset->setTitle(tr("Messabweichung Wattmeter","Sonde Dialog"));
  goffset->setAlignment(Qt::AlignCenter);
  int h = 30;
  int h1 = 31;
  labelstr = new QLabel(goffset);
  labelstr->setGeometry(10,h,250,30);
  labelstr->setAlignment(Qt::AlignCenter);
  labelstr->setText(tr("Text in der ComboBox","Sonde Dialog"));
  labeloff = new QLabel(goffset);
  labeloff->setGeometry(270,h,150,30);
  labeloff->setAlignment(Qt::AlignCenter);
  labeloff->setText(tr("Abweichung in dB","Sonde Dialog"));
  h += h1;
  for(int i=0;i<30;i++){
    stroffset[i] = new QLineEdit(goffset);
    stroffset[i]->setGeometry(10,h,300,30);
    stroffset[i]->setAlignment(Qt::AlignCenter);
    offset[i] = new QLineEdit(goffset);
    offset[i]->setGeometry(320,h,100,30);
    offset[i]->setAlignment(Qt::AlignCenter);
    h += h1;
  }
  scrollBar = new QScrollArea;
  scrollBar->setWidget(this);
  scrollBar->resize(500,500);
  scrollBar->show();
};

sdlg::~sdlg()
{
  scrollBar->close(true);
}


void sdlg::setdaten(const TSonde &asonde){
  QString s;
  osonde = asonde;
  scrollBar->setWindowTitle(tr("%1 - [%2]","nicht uebersetzen").arg(tr("HFM9/NWT","nicht uebersetzen")).arg(osonde.dname));
  //sname->setText(osonde.sname);
  for(int i=0;i<30;i++){
    stroffset[i]->setText(osonde.stroffset[i]);
    s.sprintf("%2.2f", osonde.offset[i]);
    offset[i]->setText(s);
  }
};

TSonde sdlg::getdaten(){
  QString s;
  
  //osonde.sname = sname->text();
  for(int i=0;i<30;i++){
    s = offset[i]->text();
    osonde.offset[i] = s.toDouble();
    osonde.stroffset[i] = stroffset[i]->text();
  }
  return osonde;
};

void sdlg::tip(bool btip)
{
  

QString tip_sondenname = tr(
  "<b>Name der Sonde im Wobbelfenster</B><BR>"
  "Dieser Name soll die Sonde bezeichnen. Das ist vor allen fuer <br> "
  "ansteckbare externe Sonden gedacht, damit ich weiss mit welcher <br>"
  "Sonde gerade gemessen wird."
  );
QString tip_soffsetstr = tr(
  "<b>String fuer die Erklaerung der Abweichung</B><BR>"
  "Die Messsonden habe je nach Frequenz unterschiedliche Abweichung. <br> "
  "Dieser String beschreibt die Frequenz und die Abweichung<br> "
  );
QString tip_soffset = tr(
  "<b>Wert fuer die Abweichung</B><BR>"
  "Hier wird der Wert der Abweichung eingegeben. <br> "
  "Hat die Sonde zum Beispiel bei 500MHz eine negative Abweichung von -10db <br> "
  "dann muss hier ein positiver Wert als Ausgleich eingegeben werden. <br> "
  "In diesem Fall waeren das <b>10.0</b> als Ausgleich <br> "
  );
  
  if(btip){
    //sname->setToolTip(tip_sondenname);
    for(int i=0;i<30;i++){
      stroffset[i]->setToolTip(tip_soffsetstr);
      offset[i]->setToolTip(tip_soffset);
    }
  }else{
    for(int i=0;i<30;i++){
      stroffset[i]->setToolTip("");
      offset[i]->setToolTip("");
    }
  }
}
