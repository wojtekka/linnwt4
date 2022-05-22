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
#include <QString>

#include "profildlg.h"
//#include "modem.h"


//#####################################################################################
// Class fuer Option
//#####################################################################################
profildlg::profildlg(QWidget* parent, const char* name, bool modal): QDialog(parent, name, modal)
{
  int a = 32;

  this->resize(360,10+a*22);
  this->setMinimumSize(0,0);
  this->setWindowTitle(tr("Profile einstellen","Profil Dialog"));
  ok = new QPushButton("OK", this);
  ok->setGeometry(10,10+a*21,80,30);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  cancel = new QPushButton(tr("Abbruch","Profil Dialog"), this);
  cancel->setGeometry(95,10+a*21,80,30);
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
  pruefen = new QPushButton(tr("Normal","Profil Dialog"), this);
  pruefen->setGeometry(180,10+a*21,80,30);
  connect(pruefen, SIGNAL(clicked()), SLOT(allespruefen()));
  loeschen = new QPushButton(tr("Loeschen","Profil Dialog"), this);
  loeschen->setGeometry(265,10+a*21,80,30);
  connect(loeschen, SIGNAL(clicked()), SLOT(allesloeschen()));
  labelbeschreibung = new QLabel(tr("Name","Profil Label"), this);
  labelbeschreibung->setGeometry(10,10,100,30);
  labelanfang = new QLabel(tr("Anfang","Profil Label"), this);
  labelanfang->setGeometry(150,10,100,30);
  labelende = new QLabel(tr("Ende","Profil Label"), this);
  labelende->setGeometry(230,10,100,30);
  labelschritte = new QLabel(tr("Schritte","Profil Label"), this);
  labelschritte->setGeometry(310,10,100,30);
  int i;
  QString qs;
  for(i=0;i<profilanz;i++){
    ebeschreibung[i] = new QLineEdit(this);
    ebeschreibung[i]->setText("");
    ebeschreibung[i]->setGeometry(10,40+i*32,140,30);
    eanfang[i] = new QLineEdit(this);
    eanfang[i]->setGeometry(150,40+i*32,80,30);
    eanfang[i]->setText("");
    eende[i] = new QLineEdit(this);
    eende[i]->setGeometry(230,40+i*32,80,30);
    eende[i]->setText("");
  }
  for(i=0;i<profilanz;i++){
    eschritte[i] = new QLineEdit(this);
    eschritte[i]->setGeometry(310,40+i*32,40,30);
    eschritte[i]->setText("");
  }
};

void profildlg::setprofil(TProfil aprofil, int index)
{
  QString qs;
  
  if(index < profilanz){
    eanfang[index]->setText(aprofil.anfang);
    eende[index]->setText(aprofil.ende);
    eschritte[index]->setText(aprofil.schritte);
    ebeschreibung[index]->setText(aprofil.beschr);
  }
}

TProfil profildlg::getprofil(int a)
{ 
  TProfil cprofil;
  QString qs;
  
  if(a < profilanz){
    eanfang[a]->setText(linenormalisieren(eanfang[a]->text()));
    eende[a]->setText(linenormalisieren(eende[a]->text()));
    cprofil.anfang = eanfang[a]->text();
    cprofil.ende = eende[a]->text();
    qs = ebeschreibung[a]->text();
    qs.upper();
    if(qs.isEmpty()){
      cprofil.beschr = "NN";
    }else{
      cprofil.beschr = ebeschreibung[a]->text();
    }
    qs = eschritte[a]->text();
    if(qs.isEmpty()){
      cprofil.schritte = "1001";
    }else{
      cprofil.schritte = eschritte[a]->text();
    }
  }else{
    cprofil.beschr = "NN";
  }
  return cprofil;
}

//Eingabeumwandlung mit "m" und "k"
QString profildlg::linenormalisieren(const QString &line)
{
  bool ok;
  int pos, l;
  double faktor=1.0;
  double ergebnis = 0.0;
  
  QString aline(line);
  aline.lower();	//alles auf Kleinschreibung
  l = aline.length();
  pos = aline.find('g',0,FALSE);
  if(pos != -1){
    if((l-1) == pos){
      aline.remove(pos,1);
    }else{
      aline.replace(pos,1,'.');
    }  
    faktor = 1000000000.0;
  }
  pos = aline.find('m',0,FALSE);
  if(pos != -1){
    if((l-1) == pos){
      aline.remove(pos,1);
    }else{
      aline.replace(pos,1,'.');
    }  
    faktor = 1000000.0;
  }
  pos = aline.find('k',0,FALSE);
  if(pos != -1){
    if((l-1) == pos){
      aline.remove(pos,1);
    }else{
      aline.replace(pos,1,'.');
    }  
    faktor = 1000.0;
  }
  ergebnis = aline.toDouble(&ok);
  ergebnis *= faktor;
  aline.sprintf("%1.0f",ergebnis);
  return aline;
}

void profildlg::allespruefen()
{
  int i;
  QString qs;
  
  for(i=0;i<profilanz;i++){
    qs = ebeschreibung[i]->text();
    if(!qs.isEmpty()){
      eanfang[i]->setText(linenormalisieren(eanfang[i]->text()));
      eende[i]->setText(linenormalisieren(eende[i]->text()));
    }
  }  
}

void profildlg::allesloeschen()
{
  int i;
  QString qs;
  
  for(i=0;i<profilanz;i++){
    eanfang[i]->setText("");
    eende[i]->setText("");
    ebeschreibung[i]->setText("");
    eschritte[i]->setText("");
  }  
}

void profildlg::tip(bool atip)
{
  int i;
  QString tip_schritte = profildlg::tr(
  "<b>Schritte</b><br>"
  "Wird nichts eingetragen werden automatisch<br>"
  "1001 Schritte verwendet <br>"
  ,"tooltip text");
  QString tip_edit = profildlg::tr(
  "<b>Eingabe:</b><br>"
  "3m5 oder 3,5m oder 3.5m = 3 500 000 Hz<br>"
  "<b>Eingabe:</b><br>"
  "350k5 oder 350,5k oder 350.5k = 350 500 Hz"
  ,"tooltip text");
  QString tip_name = profildlg::tr(
  "<b>Beschreibung des Profiles</b><br>"
  "Diese Feld dient der Beschreibung <br>"
  "des Profils. Loeschen des Profiles <br>"
  "mit \"NN\" oder einfach dieses <br>"
  "Feld loeschen. <br>"
  "Dieser String erscheint in der <br>"
  "ComboBox \"Profil\" des Wobbelarbeitsblattes"
  ,"tooltip text");
  if(atip){
    for(i=0;i<profilanz;i++){
      ebeschreibung[i]->setToolTip(tip_name);
      eanfang[i]->setToolTip(tip_edit);
      eende[i]->setToolTip(tip_edit);
      eschritte[i]->setToolTip(tip_schritte);
    }
  }else{
    for(i=0;i<profilanz;i++){
      ebeschreibung[i]->setToolTip("");
      eanfang[i]->setToolTip("");
      eende[i]->setToolTip("");
      eschritte[i]->setToolTip("");
    }
  }
}
