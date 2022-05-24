/***************************************************************************
                          widgetwobbeln.cpp  -  description
                             -------------------
    begin                : Son Aug 10 2003
    copyright            : (C) 2003 by Andreas Lindenau
    email                : DL4JAL@darc.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include <stdlib.h>
#include <QDateTime>
#include <QCursor>
#include <QFileDialog>
#include <QLocale>
#include <QtGlobal>
//#include <QPaintDeviceMetrics>
#include <QMessageBox>
#include <QInputDialog>
#include <QStylePainter>
#include <QTextStream>
#include <QMapIterator>
#include <QVector>
#include <QCheckBox>
#include <QAction>
#include <QPushButton>
#include <QMenu>
#include <QPrintDialog>

#include "widgetwobbeln.h"

//#define LDEBUG

/*
class winsinfo : public QWidget
{
  Q_OBJECT

public:
  winsinfo(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~winsinfo();

public slots:
private slots:
  void changefont();
  void changexpos();
  void changeypos();
  void clickinfo();
  
private:

  QSpinBox *spfontsize;
  QSpinBox *spxpos;
  QSpinBox *spypos;
  
  QLabel *labelfontsize; 
  QLabel *labelxpos; 
  QLabel *labelypos; 

  QPushButton *buttonok;
  QPushButton *buttoncancel;
  QPushButton *fontcolor;
  
  QCheckBox *checkboxinfo;
  
  signals:
  void  pixauffrischen();
};
*/

winsinfo::winsinfo(QWidget* parent, Qt::WindowFlags flags): QDialog(parent, flags)
{
  this->resize(270, 260);
  this->setMinimumSize(270, 260);
  
  checkboxinfo = new QCheckBox(tr("Info einblenden"), this);
  checkboxinfo->setGeometry(10,10,200,30);
  QObject::connect( checkboxinfo, SIGNAL(stateChanged (int)), this, SLOT(changesetboxinfo()));
  
  ginfo = new QGroupBox(tr("Info platzieren"), this);
  ginfo->setGeometry(10,50,250,150);
  ginfo->setEnabled(false);

  spfontsize = new QSpinBox(ginfo);
  spfontsize->setGeometry(150,20,60,30);
  spfontsize->setRange(5,30);
  spfontsize->setSingleStep(1);
  spfontsize->setValue(10);
  QObject::connect( spfontsize, SIGNAL(valueChanged(int)), this, SLOT(changefont(int)));
  
  labelfontsize = new QLabel(tr("Schriftgroesse"), ginfo);
  labelfontsize->setGeometry(10,20,80,30);
   
  QLabel *labelxpos; 
  QLabel *labelypos; 
  
  spxpos = new QSpinBox(ginfo);
  spxpos->setGeometry(150,60,60,30);
  spxpos->setRange(0,1000);
  spxpos->setSingleStep(10);
  QObject::connect( spxpos, SIGNAL(valueChanged(int)), this, SLOT(changexpos(int)));
  
  labelxpos = new QLabel(tr("X-Position"), ginfo);
  labelxpos->setGeometry(10,60,80,30);
  
  spypos = new QSpinBox(ginfo);
  spypos->setGeometry(150,100,60,30);
  spypos->setRange(0,1000);
  spypos->setSingleStep(10);
  QObject::connect( spypos, SIGNAL(valueChanged(int)), this, SLOT(changeypos(int)));
  
  labelypos = new QLabel(tr("Y-Position"), ginfo);
  labelypos->setGeometry(10,100,80,30);
  
  buttonok = new QPushButton(tr("Bild speichern"), this);
  buttonok->setGeometry(130,210,120,30);
  connect(buttonok, SIGNAL(clicked()), SLOT(accept()));
  
  buttoncancel = new QPushButton(tr("Abbruch"), this);
  buttoncancel->setGeometry(20,210,100,30);
  connect(buttoncancel, SIGNAL(clicked()), SLOT(reject()));
}

winsinfo::~winsinfo()
{
}

void winsinfo::setBildinfo(TBildinfo binfo)
{
  bbildinfo = binfo;
  spxpos->setValue(bbildinfo.posx);
  spypos->setValue(bbildinfo.posy);
  spfontsize->setValue(bbildinfo.fontsize);
}

void winsinfo::changefont(int a)
{
  bbildinfo.fontsize = a;
  emit pixauffrischen(bbildinfo);
}

void winsinfo::changexpos(int x)
{
  bbildinfo.posx = x;
  emit pixauffrischen(bbildinfo);
}

void winsinfo::changeypos(int y)
{
  bbildinfo.posy = y;
  emit pixauffrischen(bbildinfo);
}

void winsinfo::changesetboxinfo()
{
  if(checkboxinfo->checkState() == Qt::Checked){
    ginfo->setEnabled(true);
    bbildinfo.infoanzeigen = true;
  }else{
    ginfo->setEnabled(false);
    bbildinfo.infoanzeigen = false;
  }
  emit pixauffrischen(bbildinfo);
}

WidgetWobbeln::WidgetWobbeln(QWidget *parent) : QWidget(parent)
{
  this->resize(800,480);
  this->setMinimumSize(400,400);
  this->setWindowTitle(tr("Display","Wobbelfenster"));
//  this->setWindowIcon(QIcon(":/images/appicon.png"));
  
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::WidgetWobbeln()");
  #endif
  //  setBackgroundMode( NoBackground );
  pix = QPixmap(size());
  pix1 = QPixmap(size());
  ObererRand= 35;
  LinkerRand= 35;
  kursorix = 0;
  
  int i;
  for(i=0; i<=5; i++){
    mouseposx[kursorix] = 0;
    mouseposx1[kursorix] = 0;
    mouseposy[kursorix] = 0;
  }
  
  banfang = false;
  bende = false;
  blupeplus = false;
  blupeminus = false;
  mousefrequenz = 0.0;
 /*
  menmouse = new Q3PopupMenu(this);
  menmouse->insertItem(tr("Kursor # loeschen","Popup Menu in der Grafik"), this, SLOT(kursor_loeschen()));
  menmouse->insertItem(tr("Alle Kursor loeschen","Popup Menu in der Grafik"), this, SLOT(alle_kursor_loeschen()));
  menmouse->insertItem(tr("Kurve loeschen","Popup Menu in der Grafik"), this, SLOT(kurve_loeschen()));
  menmouse->insertSeparator();
  menmouse->insertItem(tr("Frequenzanfang","Popup Menu in der Grafik"), this, SLOT(wsetfrqanfang()));
  menmouse->insertItem(tr("Frequenzende","Popup Menu in der Grafik"), this, SLOT(wsetfrqende()));
  menmouse->insertSeparator();
  menmouse->insertItem(tr("Ermitteln der geometrischen Kabellaenge","Popup Menu in der Grafik"), this, SLOT(kabellaenge()));
  menmouse->insertItem(tr("Bestimmung der elektrischen Daten des Kabels","Popup Menu in der Grafik"), this, SLOT(kabeldaten()));
  menmouse->insertSeparator();
  menmouse->insertItem(tr("Kursor-Frequenz zum Berechnen","Popup Menu in der Grafik"), this, SLOT(setberechnung()));
*/  
  popupmenu = new QMenu();
  akursor_clr = new QAction(tr("Kursor # loeschen","PopupMenue"), popupmenu);
  connect(akursor_clr, SIGNAL(triggered()), this, SLOT(kursor_loeschen()));
  popupmenu->addAction(akursor_clr);
  akursor_clr_all = new QAction(tr("Alle Kursor loeschen","PopupMenue"), popupmenu);
  connect(akursor_clr_all, SIGNAL(triggered()), this, SLOT(alle_kursor_loeschen()));
  popupmenu->addAction(akursor_clr_all);
  akurve_clr = new QAction(tr("Kurve loeschen","PopupMenue"), popupmenu);
  connect(akurve_clr, SIGNAL(triggered()), this, SLOT(kurve_loeschen()));
  popupmenu->addAction(akurve_clr);
  popupmenu->addSeparator();
  afrequenzanfang = new QAction(tr("Frequenzanfang","PopupMenue"), popupmenu);
  connect(afrequenzanfang, SIGNAL(triggered()), this, SLOT(wsetfrqanfang()));
  popupmenu->addAction(afrequenzanfang);
  afrequenzende = new QAction(tr("Frequenzende","PopupMenue"), popupmenu);
  connect(afrequenzende, SIGNAL(triggered()), this, SLOT(wsetfrqende()));
  popupmenu->addAction(afrequenzende);
  popupmenu->addSeparator();
  akabellaenge = new QAction(tr("Ermitteln der geometrischen Kabellaenge","PopupMenue"), popupmenu);
  connect(akabellaenge, SIGNAL(triggered()), this, SLOT(kabellaenge()));
  popupmenu->addAction(akabellaenge);
  akabeldaten = new QAction(tr("Bestimmung der elektrischen Daten des Kabels","PopupMenue"), popupmenu);
  connect(akabeldaten, SIGNAL(triggered()), this, SLOT(kabeldaten()));
  popupmenu->addAction(akabeldaten);
  popupmenu->addSeparator();
  aberechnen = new QAction(tr("Kursor-Frequenz zum Berechnen","PopupMenue"), popupmenu);
  connect(aberechnen, SIGNAL(triggered()), this, SLOT(setberechnung()));
  popupmenu->addAction(aberechnen);
  akursor_clr->setEnabled(false);
  akursor_clr_all->setEnabled(false);
  afrequenzanfang->setEnabled(false);
  afrequenzende->setEnabled(false);
  akabellaenge->setEnabled(false);
  akabeldaten->setEnabled(false);
  aberechnen->setEnabled(false);
  
  setBackgroundRole(QPalette::Dark);
  setAutoFillBackground(true);
  setCursor(Qt::CrossCursor);
  kurve1.mess.daten_enable = false;
  kurve2.mess.daten_enable = false;
  kurve3.mess.daten_enable = false;
  kurve4.mess.daten_enable = false;
  wobbeldaten.bhintergrund = false;
  bresize = true;
  //obere Anzeige der LOG_Y-Achse
  ymax=10.0;//dB
  //obere Anzeige der LOG_Y-Achse
  ymin=-90.0;//dB
  //obere Anzeige der LOG_Y-Achse
  ydisplayteilung_log= (ymax - ymin) / 10.0;
  //fuer Stringliste Quarzdaten
  speichern = false; 
}

WidgetWobbeln::~WidgetWobbeln()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::~WidgetWobbeln()");
  #endif
  /*
  if(kurve1 != NULL) delete kurve1;
  if(kurve2 != NULL) delete kurve2;
  if(kurve3 != NULL) delete kurve3;
  if(kurve4 != NULL) delete kurve4;
  */
}

void WidgetWobbeln::setKurve(TWobbel akurve){
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::setKurve()");
  #endif
  switch(akurve.kurvennr){
    case 1:
      kurve1 = akurve;
      kurve1.absanfang = wobbeldaten.absanfang;
      kurve1.absende = wobbeldaten.absende;
    break;
    case 2:
      kurve2 = akurve;
      kurve2.absanfang = wobbeldaten.absanfang;
      kurve2.absende = wobbeldaten.absende;
    break;
    case 3:
      kurve3 = akurve;
      kurve3.absanfang = wobbeldaten.absanfang;
      kurve3.absende = wobbeldaten.absende;
    break;
    case 4:
      kurve4 = akurve;
      kurve4.absanfang = wobbeldaten.absanfang;
      kurve4.absende = wobbeldaten.absende;
    break;
    case 5:
      kurve5 = akurve;
      kurve5.absanfang = wobbeldaten.absanfang;
      kurve5.absende = wobbeldaten.absende;
    break;
    case 6:
      kurve6 = akurve;
      kurve6.absanfang = wobbeldaten.absanfang;
      kurve6.absende = wobbeldaten.absende;
    break;

  }
  refreshPixmap();
  repaint();
}

//Die Kursornummer kommt vom Hauptfenster
void WidgetWobbeln::setWKursorNr(int a){
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::setKursorNr()");
  #endif
  kursorix = 4 -a;
  refreshKurve();
}

void WidgetWobbeln::kabeldaten()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::kabeldaten()");
  #endif
  QString beschr(tr("<b>Ermittlung der Kabeldaten</b><br>"
                    "1. Das Kabelende offen lassen und einen Serienwiderstand<br>"
                    "   von 50 Ohm zwischen SWV-Bruecke und Seele des Kabels<br>"
                    "   einfuegen<br>"
                    "2. Der Kursor wird auf die unterste Resonanzfrequenz gesetzt"
                    "<BR><BR>"
                    "Gemessene Laenge des Kabels (m):","Inputdialog in der Grafik"));
  QString qs, qv, qe;
  double v, e;
  bool ok;
  double l = QInputDialog::getDouble(this, tr("Kabellaenge","Inputdialog in der Grafik"),
                                           beschr, 0.0, 0, 10000, 3, &ok); 
  if(ok){
    v = 0.0133 * l * (mousefrequenz / 1000000.0);
    e = 1 / (v * v);
    qv.sprintf("%1.3f",v);
    qe.sprintf("%1.3f",e);
    qs = tr("<B>Errechnung der Kabeldaten</B><BR>"
            "Verkuerzungsfaktor: ","InfoBox in der Grafik") +
            qv +
            "<BR>" +
            tr("Dielektrizitaetszahl: ","InfoBox in der Grafik") +
            qe;
    QMessageBox::warning( this, tr("Kabledaten","InfoBox in der Grafik"), qs);
  }
}


void WidgetWobbeln::kabellaenge()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::kabellaenge()");
  #endif
  QString beschr(tr("<b>Ermittlung der Kabellaenge</b><br>"
                    "Es gibt 2 Moeglichkeiten:<br>"
                    "1. Das Kabelende offen lassen und einen Serienwiderstand<br>"
                    "   von 50 Ohm zwischen SWV-Bruecke und Seele des Kabels<br>"
                    "   einfuegen<br>"
                    "2. Das Kabelende kurzschliessen und am Ausgang der SWV-<br>"
                    "   Bruecke 50 Ohm zwischen Seele und Masse des Kabels<br>"
                    "   schalten.<br><BR>"
                    "Nach Auswahl und entsprechender Beschaltung messen Sie das SWV<br> "
                    "von 0,1 bis 160 MHz und setzen dann den Cursor auf die unterste<br> "
                    "Resonanzfrequenz. Bei groesseren Kabellaengen ist es erforderlich<br> "
                    "die Endfrequenz zu verringern um genaue Messergebnisse zu erhalten"
                    "<br><br>"
                    "Verkuerzungsfaktor des Kabels (Vorgabe = PE-Kabel):","InputDialog in der Grafik"));
  QString qs, qer;
  double er;
  bool ok;
  double v = QInputDialog::getDouble(this, tr("Verkuerzungsfaktor","InputDialog in der Grafik"),
                                           beschr, 0.67, -10000, 10000, 3, &ok); 
  if(ok){
    er = v / (mousefrequenz / 1000000.0) * 75.0;
    qer.sprintf("%1.3f",er);
    qs = tr("<B>Errechnung der Kabellaenge</B><BR>"
            "Geometrische Laenge des Kabels: ","InfoBox in der Grafik") +
            qer +
            " m";
    QMessageBox::warning( this, tr("Geometrische Laenge","InfoBox in der Grafik"), qs);
  }
}

void WidgetWobbeln::kursor_loeschen()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::kursor_loeschen()");
  #endif
  resetmousecursor();
//  menmouse->hide();
}

void WidgetWobbeln::kurve_loeschen()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::kurve_loeschen()");
  #endif
  emit wkurve_loeschen();
}

void WidgetWobbeln::alle_kursor_loeschen()
{
  int k;
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::alle_kursor_loeschen()");
  #endif
  for(k=0;k<5;k++){
    kursorix = k;
    resetmousecursor();
  }  
//  menmouse->hide();
  setWKursorNr(4);
  emit resetKursor(4);
}

void WidgetWobbeln::wsetfrqanfang()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::wsetfrqanfang()");
  #endif
  banfang = true;
  refreshKurve();
//  menmouse->hide();
}

void WidgetWobbeln::wsetfrqende()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::wsetfrqende()");
  #endif
  bende = true;
  refreshKurve();
//  menmouse->hide();
}

void WidgetWobbeln::setberechnung()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::setberechnung()");
  #endif
  bberechnung = true;
  refreshKurve();
//  menmouse->hide();
}

void WidgetWobbeln::mouseMoveEvent(QMouseEvent * e)
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::mouseMoveEvent()");
  #endif
  if(wobbeldaten.mess.daten_enable){
    mouseposx[kursorix] = e->x();
    mouseposy[kursorix] = e->y();
    if(!wobbeldaten.mousesperre){
      refreshPixmap();
      repaint();
    }
  }
}

void WidgetWobbeln::mousePressEvent(QMouseEvent * e)
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::mousePressEvent()");
  #endif
  bnomove = false;
  if(wobbeldaten.mess.daten_enable){
    if(e->button() == Qt::LeftButton){
      mouseposx[kursorix] = e->x();
      mouseposy[kursorix] = e->y();
      bnomove = true;
    }
    if(e->button() == Qt::RightButton){
      if(mouseposx[kursorix] != 0){
        akursor_clr->setEnabled(true);
//        akursor_clr_all->setEnabled(true);
        afrequenzanfang->setEnabled(true);
        afrequenzende->setEnabled(true);
        akabellaenge->setEnabled(true);
        akabeldaten->setEnabled(true);
        aberechnen->setEnabled(true);
      }else{
        akursor_clr->setEnabled(false);
//        akursor_clr_all->setEnabled(false);
        afrequenzanfang->setEnabled(false);
        afrequenzende->setEnabled(false);
        akabellaenge->setEnabled(false);
        akabeldaten->setEnabled(false);
        aberechnen->setEnabled(false);
      }
      popupmenu->popup(QCursor::pos());
      akursor_clr_all->setEnabled(false);
      // Ist noch ein Kursor aktiv ?
      for(int i=0; i<5; i++){
        if(mouseposx[i] != 0){
          akursor_clr_all->setEnabled(true);
          break;
        }
      }
    }
    if(!wobbeldaten.mousesperre){
      refreshPixmap();
      repaint();
    }
  }
}

void WidgetWobbeln::setmousecursormitte()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::setmousecursormitte()");
  #endif
  QSize pixsize = this->rect().size();
  int i = pixsize.width();
  mouseposx[kursorix] = i/2;
  if(!wobbeldaten.mousesperre){
    refreshPixmap();
    repaint();
  }
}

void WidgetWobbeln::resetmousecursor()
{
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::resetmousecursor()");
  #endif
  mouseposx[kursorix] = 0;
  mouseposy[kursorix] = 0;
  if(!wobbeldaten.mousesperre){
    refreshPixmap();
    repaint();
  }
}

double WidgetWobbeln::getkalibrierwertk1(double afrequenz)
{
  double findex;
  int i;
  
  #ifdef LDEBUG
//    qDebug("WidgetWobbeln::getkalibrierwertk1()");
  #endif
  if(wobbeldaten.bkalibrierkorrk1 and wobbeldaten.ebetriebsart == ewobbeln)
  {
    //qDebug("WidgetWobbeln::getkalibrierwertk1()");
    //Stuetzfrequenzen beziehen sich auf die Grundfrequenz
    //deshalb wieder durch den Frqfaktor teilen
    findex = afrequenz / wobbeldaten.frqfaktor;
    findex = round(findex / eteiler);
    i = int(findex);
    if(i > (maxmesspunkte-1))i = maxmesspunkte-1;
    return wobbeldaten.kalibrierarray.arrayk1[i];
  }else{
    return 0.0;
  }
}

double WidgetWobbeln::getkalibrierwertk2(double afrequenz)
{
  double findex;
  int i;
  
  #ifdef LDEBUG
//    qDebug("WidgetWobbeln::getkalibrierwertk2()");
  #endif
  if(wobbeldaten.bkalibrierkorrk2 and wobbeldaten.ebetriebsart == ewobbeln)
  {
    //Stuetzfrequenzen beziehen sich auf die Grundfrequenz
    //deshalb wieder durch den Frqfaktor teilen
    findex = afrequenz / wobbeldaten.frqfaktor;
    findex = round(findex / eteiler);
    i = int(findex);
    if(i > (maxmesspunkte-1))i = maxmesspunkte-1;
    return wobbeldaten.kalibrierarray.arrayk2[i];
  }else
  {
    return 0.0;
  }
}

double WidgetWobbeln::getsavcalwert(double afrequenz)
{
  int a,b;
  double pegela, pegelb, pegeldiff;
  double frequa, frequb, frequdiff;
  
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::getsavcalwert()");
  #endif
  //aus der Frequenz den Index errechnen
  // Bereich 0 - 100 GHz
  a = int(savcalkonst * log10(afrequenz));
  b = a + 1;
  //arraygrenzen einhalten
  if(a<0)a=0;
  if(a > calmaxindex-1)a = calmaxindex-1;
  if(b > calmaxindex-1)b = calmaxindex-1;
  pegela = wobbeldaten.savcalarray.p[a];
  // Ist das Ende des Array erreicht?
  if(a != b){
    //Eine Iteration ist moeglich
    pegelb = wobbeldaten.savcalarray.p[b];
    pegeldiff = pegelb - pegela;
    frequa = pow(10.0,(double(a)/savcalkonst));
    frequb = pow(10.0,(double(b)/savcalkonst));
    frequdiff = frequb - frequa;
    //Iteration zwischen PegelA und PegelB
    pegela = pegela + (((afrequenz - frequa)/frequdiff) * pegeldiff);
    //qDebug("index %i Pegel %f",i, pegel);
  }
  return (pegela);
}

double WidgetWobbeln::getswrkalibrierwertk1(double afrequenz)
{
  double findex;
  int i;
  double w1;
  
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::getswrkalibrierwertek1()");
  #endif
  //Stuetzfrequenzen beziehen sich auf die Grundfrequenz
  //deshalb wieder durch den Frqfaktor teilen
  findex = afrequenz / wobbeldaten.frqfaktor;
  findex = round(findex / eteiler);
  i = int(findex);
  if(i > (maxmesspunkte-3))i = maxmesspunkte-3;
//  return wswrkalibrierarray.arrayk1[i];
  w1 = (wobbeldaten.swrkalibrierarray.arrayk1[i-2] +
        wobbeldaten.swrkalibrierarray.arrayk1[i-1] +  
        wobbeldaten.swrkalibrierarray.arrayk1[i] +  
        wobbeldaten.swrkalibrierarray.arrayk1[i+1] +  
	wobbeldaten.swrkalibrierarray.arrayk1[i+2]) / 5.0;
  return (w1);
}

double WidgetWobbeln::getswrkalibrierwertk2(double afrequenz)
{
  double findex;
  int i;
  
  #ifdef LDEBUG
    qDebug("WidgetWobbeln::getswrkalibrierwertek2()");
  #endif
  //Stuetzfrequenzen beziehen sich auf die Grundfrequenz
  //deshalb wieder durch den Frqfaktor teilen
  findex = afrequenz / wobbeldaten.frqfaktor;
  findex = round(findex / eteiler);
  i = int(findex);
  if(i > (maxmesspunkte-1))i = maxmesspunkte-1;
  return wobbeldaten.swrkalibrierarray.arrayk2[i];
}

void WidgetWobbeln::setParameterPixmap()
{
  QSize pixsize;
  bool drawenable = false;

  //qDebug("WidgetWobbeln::setParameterPixmap()");
  pixsize = this->rect().size();
  drawenable = (pixsize.width() > 0) and (pixsize.height() > 0);
  //qDebug("WidgetWobbeln::setParameterPixmap():w:%i h:%i",pixsize.width(), pixsize.height());
  //loeschen des Pixmap
  if(drawenable){
    pix = QPixmap(pixsize);
    QPainter p;
    p.begin(&pix);
    if(wobbeldaten.linear1 or wobbeldaten.linear2){
      ymax = 10.0;
      ymin = -100.0;
    }else{
      ymax = double(wobbeldaten.ydbmax);
      ymin = double(wobbeldaten.ydbmin);
    }
    //qDebug("ymax: %f", ymax);
    //qDebug("ymin: %f", ymin);
    //Hintergrundfarbe des Wobbelfensters
    if((wobbeldaten.ebetriebsart == espektrumanalyser) or
       (wobbeldaten.ebetriebsart == espekdisplayshift)){
      //Hintergrund SAV mit grauen Flaechen
      if(wobbeldaten.linear1 or wobbeldaten.linear2){
        //Hintergrund ohne graue Flaechen
        p.fillRect( this->rect(), wobbeldaten.colorhintergrund);
      }else{
        int x, x1, x2;
        int y;
        int w;
        int h1, h2, h3, h4, h5;
        double hd = 0.9;
        double h5db;
        int r,g,b,a;
        bool boben = true, bunten = true;
  
        //originalgroesse holen
        QRect rect = this->rect();
        //und zuerst einmal mit dem gueltigen Kintergrund fuellen
        p.fillRect( this->rect(), wobbeldaten.colorhintergrund);
        //auslesen des ganzen Rechteckes
        x = rect.x();
        y = rect.y();
        w = rect.width();
        h5 = rect.height();
        //erste Spalte nach Beschriftung links
        x1 = LinkerRand;
        //zweite Spalte vor Beschriftung rechts
        x2 = w - LinkerRand;
        //dB Y-Achse aus den Daten lesen
        //db Teilung auf der Y-Achse berechnen
        ydisplayteilung_log = (ymax - ymin) / 10.0;
        //Pixel pro 5dB Einheit errechnen
        h5db = (double(h5) - (double(ObererRand) * 2.0)) / (double(ydisplayteilung_log) * 2.0);
        //Zeilen festlegen
        //h1 Zeile nach der Beschriftung oben
        h1 = ObererRand;
        //h2 Zeile bei + 5dB
        h2 = h1 + int(round(((ymax - wobbeldaten.safehlermax) / 5.0) * h5db));
        //h3 Zeile bei -60 dB
        h3 = h1 + int(round(((ymax - wobbeldaten.safehlermin) / 5.0) * h5db));
        //h4 Zeile vor der Beschriftung unten
        h4 = h5 - ObererRand;
        //Test ob obere Abdunklung groesser 5 dB eingeblendet werden soll
        if(h2 <= h1)boben = false;
        //Test ob untere Abdunklung ab -60 dB eingeblendet werden soll
        if(h3 >= h4)bunten = false;
        //Abdunklung einstellen
        QColor chd = wobbeldaten.colorhintergrund;
        chd.getRgb(&r,&g,&b,&a);
        chd.setRgb(int(r*hd), int(g*hd), int(b*hd), int(a*hd));
        QRect rect1(x, y, x1, h5);//rechts
        QRect rect2(x2, y, w-x2, h5);//links
        QRect rect3(x1, y, w-x1-x1, h1);//von Oben bis Zeile h1
        QRect rect4(x1, y + h4, w-x1-x1, h5 - h4);//von Zeile h4 bis h5
        QRect rect5(x1, y + h2, w-x1-x1, h3 - h2);//von Zeile h2 (5dB) bis h3 (-60dB)
        QRect rect6(x1, y + h1, w-x1-x1, h2 - h1);//von Zeile h1 bis h2
        QRect rect7(x1, y + h3, w-x1-x1, h4 - h3);//von Zeile h3 bis h4
        p.fillRect( rect1, wobbeldaten.colorhintergrund);
        p.fillRect( rect2, wobbeldaten.colorhintergrund);
        p.fillRect( rect3, wobbeldaten.colorhintergrund);
        p.fillRect( rect4, wobbeldaten.colorhintergrund);
        p.fillRect( rect5, wobbeldaten.colorhintergrund);
        if(boben)p.fillRect( rect6, chd);
        if(bunten)p.fillRect( rect7, chd);
      }
    }else{
      //kein SAV Hintergrund ohne graue Flaechen
      p.fillRect( this->rect(), wobbeldaten.colorhintergrund);
    }
    p.end();
  }
}

void WidgetWobbeln::refreshKurve()
{
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::refreshKurve()");
  #endif
  QPainter p;
  //Beschriftetes Pixmap holen
  pix = pix1;
  p.begin(&pix);
  //und Kurve einzeichnen
  //wobbeldaten.bhintergrund = false;
  drawKurve(&p);
  p.end();
  //und anzeigen
  update(); //paintEvent erzeugen
}

void WidgetWobbeln::refreshPixmap()
{
  TWobbel awobbeldaten;
  
  //qDebug("WidgetWobbeln::refreshPixmap()");
  //Groesse anpassen und Pixmap loeschen
  setParameterPixmap();
  QPainter p;
  p.begin(&pix);
  //Beschriftung erzeugen
  drawBeschriftung(&p);
  //Kurven in den hintergrund einblenden wenn alles OK ist
  awobbeldaten = wobbeldaten;
  wobbeldaten = kurve1;
  drawKurve(&p);
  wobbeldaten = kurve2;
  drawKurve(&p);
  wobbeldaten = kurve3;
  drawKurve(&p);
  wobbeldaten = kurve4;
  drawKurve(&p);
  wobbeldaten = kurve5;
  drawKurve(&p);
  wobbeldaten = kurve6;
  drawKurve(&p);

  wobbeldaten = awobbeldaten;
  //Beschriftetes Pixmap zusaetzlich abspeichern
  //bevor die Kurve gezeichnet wird
  pix1 = pix;
  //Kurve zeichen
  drawKurve(&p);
  p.end();
  //und anzeigen
  update();
}

void WidgetWobbeln::paintEvent(QPaintEvent*)
{
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::paintEvent()");
  #endif
  QPainter painter(this);
  //Beschriftung und Kurve ins Window kopieren
  //zeichen die Groesse hat sich geaendert
  painter.drawPixmap(0, 0, pix);
}

void WidgetWobbeln::resizeEvent(QResizeEvent*)
{
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::resizeEvent()");
  #endif
  // bei Paintevent noch einmal zeichen
  bresize = true;
  //Kurve neu zeichen
  refreshPixmap();
}

void WidgetWobbeln::drawBeschriftung(QPainter *p)
{
  int i=0,j=0;
  int xmodulo=2;
  int xshift=10;
  int x=0;
  int y=0;
  int x1=0;
  int y1=0;

  int displayshift; 
  double px1; 
  double px2; 
  double py1; 
  double bx;
  double wxanfang; 
  double wxende; 
  double wxbereich; 
  double xRaster=1.0; 
  double me=1.0; 
  double yzeichenfaktor_log = 0.0;
  double yzeichenfaktor_lin = 0.0;
  QString xRasterStr, s, qstr;
  bool zeilenwechsel;
  bool boolrot = false;
  efrqbereich frqbereich;
  QPen penblacksolid = QPen( Qt::black, 0, Qt::SolidLine); 
  QPen pendarkgreensolid = QPen( Qt::darkGreen, 0, Qt::SolidLine);
  QPen penredsolid = QPen( Qt::red, 0, Qt::SolidLine); 
  QPen penblackdot = QPen( Qt::black, 0, Qt::DotLine); 
  QPen penlightgraydot = QPen( Qt::lightGray, 0, Qt::DotLine);
  QPen pencyansolid = QPen( Qt::cyan, 0, Qt::SolidLine);
  QPen penreddot = QPen( Qt::red, 0, Qt::DotLine);
  QPen penhline = QPen( Qt::lightGray, 0, Qt::DashLine);
  
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::drawBeschriftung()");
  #endif
  
  
  ydisplayteilung_log = ymax - ymin;
  displayshift = wobbeldaten.displayshift / -10;
  if(displayshift != 0)boolrot = true;
  yswrteilung = wobbeldaten.swraufloesung;//Y-Teilung bei SWV
  breite = this->rect().width() - LinkerRand * 2;
//  qDebug("breite %i", breite);
  hoehe = this->rect().height() - ObererRand * 2;
  double zw1 = double(breite) / (wobbeldaten.absende - wobbeldaten.absanfang);
  xSchritt = zw1 * wobbeldaten.schrittfrequenz;
  xAnfang = zw1 * (wobbeldaten.anfangsfrequenz - wobbeldaten.absanfang);
  ySchritt = double(hoehe) / yaufloesung;

  //###################################################################################
  // ANFANG Beschriftung des Y-Bereiches
  //###################################################################################
    p->setPen( penblacksolid );
  // Schrift setzen
  p->setFont( QFont("Sans", 9));
  px1 = LinkerRand;
  px2 = breite + LinkerRand;
  switch(wobbeldaten.ebetriebsart){
    case eantdiagramm:
      // Kreise mit db
      if(hoehe >= breite)durchmesser = breite;
      if(hoehe < breite)durchmesser = hoehe;
      p->setPen(penblacksolid);
      int idbmax;
      idbmax = wobbeldaten.antdiagramm.dbmax;
      int idbmin;
      idbmin = wobbeldaten.antdiagramm.dbmin;
      double db_teilung;
      db_teilung = double(idbmax - idbmin) / double(antdia_db_teilung);
      //qDebug()<<db_teilung;
      double w10db;
      w10db = double(durchmesser)/2.0/db_teilung;
      p->setPen(penblacksolid);
      p->drawEllipse ( LinkerRand, ObererRand, durchmesser, durchmesser);
      for(i=idbmax, j=0; i>=idbmin; i-=antdia_db_teilung, j++){
        p->setPen(penhline);
        p->drawEllipse ( LinkerRand + j*w10db, ObererRand+j*w10db, durchmesser-j*w10db*2, durchmesser-j*w10db*2);
        s.sprintf("%idB",i);
        p->setPen(penblacksolid);
        p->drawText( LinkerRand+durchmesser/2-15, ObererRand + j*w10db-3, s);
        //in der Mitte nur einmal Beschriften
        if(i != idbmin) p->drawText( LinkerRand+durchmesser/2-15, ObererRand + durchmesser - j*w10db + 13, s);
      }
      // Gradlinien
      p->setPen(penhline);
      for(i=0; i<=360; i+=10){
        x = LinkerRand + durchmesser/2 + int(round(sin(double(i)*M_PI/180.0)*durchmesser/2));
        y = ObererRand + durchmesser/2 - int(round(cos(double(i)*M_PI/180.0)*durchmesser/2));
        x1 = LinkerRand + durchmesser/2 + int(round(sin(double(i)*M_PI/180.0)*durchmesser/20));
        y1 = ObererRand + durchmesser/2 - int(round(cos(double(i)*M_PI/180.0)*durchmesser/20));
        p->drawLine(x, y, x1, y1);
        s.sprintf("%i",i);
        if(i<=90){
          if(i!=0)p->drawText( x+10, y-10, s);
          if(i==0)p->drawText( x+30, y-10, s);
        }
        if(i>90 and i<=180){
          if(i>90 and i<180)p->drawText( x+10, y+10, s);
          if(i==180){
            p->drawText( x+30, y+20, s);
            p->drawText( x-40, y+20, s);
          }
        }
        if(i>180 and i<=270){
          if(i>180 and i<270)p->drawText( x-30, y+10, s);
          if(i==270){
            p->drawText( x-30, y+10, s);
          }
        }
        if(i>270 and i<=360){
          if(i>270 and i<360)p->drawText( x-30, y-10, s);
          if(i==360){
            if(i>270 and i<360)p->drawText( x-30, y-10, s);
            if(i==360)p->drawText( x-40, y-10, s);
          }
        }
      }
      break;
    case espektrumanalyser:
    case espekdisplayshift:
    case ewobbeln:
      if(!wobbeldaten.linear1 && !wobbeldaten.linear2){ // logaritmische Beschriftung
        //py1 ist die Pixelanzahl pro 10dB
        py1 = double(hoehe) / (ydisplayteilung_log / 10.0);
        //Farbe einstellen abhaengig von SHIFT-Einstellung
        if((wobbeldaten.dbshift1 != 0.0) or (wobbeldaten.dbshift2 != 0.0))boolrot = true;
        if(boolrot){
          p->setPen( penredsolid );
        }else{
          p->setPen( penblacksolid );
        }
        //bei log. Messkopf Messeinheit anzeigen  linker Rand rechtbuendig
        if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
          s = "dBm";
        }else{
          s = "dB";
        }
        p->drawText(1, int(round(py1*0.5))+ ObererRand, s);
        p->drawText(int(round(px2))+5, int(round(py1*0.5))+ ObererRand, s);
        //Linen und Beschriftung beginnend bei ymax(obere dB-Zahl) bis ymin(untere dB-Zahl)
        for(i=int(ymax/10.0), j=0; i>=int(ymin/10.0); i--, j++)
        {
          //Farbe einstellen abhaengig von SHIFT-Einstellung
          if(boolrot){
            p->setPen(penredsolid);
          }else{
            p->setPen(penblacksolid);
          }
          //dB Zahlen generieren rechtsbuendig
          s.sprintf("%3i", (i-displayshift)*10);
          p->drawText(1, int(round(py1*j))+ ObererRand + 3,s);
          // nur bei log-Sonde beschriften
          //an den rechten Rand schreiben linksbuendig
          p->drawText(int(round(px2))+3, int(round(py1*j))+ ObererRand + 3,s);
          //Null-Linie als anderen Strich darstellen
          if((i-displayshift) == 0){
            p->setPen(penblackdot);
          }else{
            p->setPen(penlightgraydot);
          }
          //Abschliessend noch die Linie zeichnen
          //qDebug("%3i %i", i*10, int(round(py1*j))+ ObererRand);
          p->drawLine(int(round(px1)), int(round(py1*j))+ ObererRand,
                      int(round(px2)), int(round(py1*j))+ ObererRand);
        }
        p->setPen(penhline);
        yzeichenfaktor_log = hoehe / (ymin - ymax);
        if(wobbeldaten.hline != 0.0){
          py1 = (wobbeldaten.hline - ymax - wobbeldaten.displayshift) * yzeichenfaktor_log + ObererRand;
          if(py1 < (hoehe + ObererRand))p->drawLine(int(round(px1)), int(py1), int(round(px2)), int(py1));
          s.sprintf("%0.2fdB", wobbeldaten.hline);
          p->drawText(int(round(px1)), int(py1), s);
          p->drawText(int(round(px2))-60, int(py1)+10, s);
        }
      }else{ //Linear Beschriftung
        py1 = hoehe / ydisplayteilung_lin;
        for(i=0; i<(ydisplayteilung_lin + 1); i++)
        {
          //lineare Beschriftung
          p->setPen(pendarkgreensolid);
          p->drawText(1, int(round(py1*0.5)+ ObererRand + 3.0)," dB");
          bx = double((11 - i) * 0.1);
          //qDebug("%f",bx);
          bx = 20.0 * log10(bx);
          //qDebug("%f",bx);
          if(bx > -10.0){
            s.sprintf("%1.2f", bx);
          }else{
            s.sprintf("%1.1f", bx);
          }
          if(bx < -99.9){
            s = tr("UnEn","Beschriftung bei linearer Sonde = unendlich nur 4 Zeichen !!");
          }  
          //bx = bx / 20.0;
          //bx = pow(10.0 , bx);
          //qDebug("pow: %f",bx);
          //an den linken Rand schreiben
          p->drawText(1, int(round(py1*i))+ ObererRand + 3,s);
          //lineare Beschriftung
          s.sprintf("%1.1f", double((11 - i) * 0.1));
          //an den rechten Rand schreiben
          p->setPen(penblacksolid);
          p->drawText(int(round(px2))+3, int(round(py1*i))+ ObererRand + 3,s);
          p->setPen(penlightgraydot);
          //Nulllinie fest an der 1.0 Stelle
          if(i == 1){
            p->setPen(penblackdot);
          }else{
            p->setPen(penlightgraydot);
          }
          p->drawLine(int(round(px1)), int(round(py1*i))+ ObererRand,
                     int(round(px2)), int(round(py1*i))+ ObererRand);
        }
        p->setPen(penhline);
        yzeichenfaktor_lin = hoehe / (-10.0 - 1.0);
        if(wobbeldaten.hline != 0.0){
          bx = wobbeldaten.hline / 20.0;
          bx = pow(10.0, bx);
          //qDebug("pow: %f",bx);
          py1 = (bx - 1.0) * 10.0 * yzeichenfaktor_lin + ObererRand - yzeichenfaktor_lin;
          //qDebug("py1: %f",py1);
          //qDebug("hoehe: %i",hoehe);
          //qDebug("yzeichenfaktor_lin: %f",yzeichenfaktor_lin);
          if(py1 < (hoehe + ObererRand))p->drawLine(int(round(px1)), int(py1), int(round(px2)), int(py1));
          s.sprintf("%0.2fdB", wobbeldaten.hline);
          p->drawText(int(round(px1)), int(py1), s);
          p->drawText(int(round(px2))-60, int(py1)+10, s);
        }
      }
      break;
    case eswrneu:
    case eswrant:
      //2 kanalige SWV-Anzeige
      py1 = hoehe / double(yswrteilung*10);       //Einteilung Y
      p->setPen( penblacksolid);
      p->drawText(1, hoehe + ObererRand + 20,"SWV");
      //Messeinheit anzeigen  rechter Rand
      p->drawText(int(round(px2))+5, hoehe + ObererRand + 20,"SWV");
      if(yswrteilung > 6){
        xmodulo = 4;
      }else{
        xmodulo = 2;
      }
      if(yswrteilung > 8){
        xmodulo = 5;
      }
      for(i=0; i< (yswrteilung*10+1); i++)
      {
        //modulo 2
//        qDebug(((i+1) % xmodulo));
        if((i % xmodulo) == 0){
          //SWV Beschriftung aller 2 Teilungen Y-Achse
          s.sprintf("%1.1f", double(((yswrteilung*10 - i) * 0.1)) + 1.0);
          //an den linken Rand schreiben
          p->setPen(penblacksolid);
          p->drawText(1, int(round(py1*i))+ ObererRand + 3,s);
          //SWV Beschriftung
          s.sprintf("%1.1f", double(((yswrteilung*10 - i) * 0.1)) + 1.0);
          p->setPen(penblacksolid);
          p->drawText(int(round(px2))+3, int(round(py1*i))+ ObererRand + 3,s);
          p->setPen( penlightgraydot);
          p->drawLine(int(round(px1)), int(round(py1*i))+ ObererRand,
                     int(round(px2)), int(round(py1*i))+ ObererRand);
        }
      }
      break;
    case eimpedanz:
      //2 kanalige SWV-Anzeige
      py1 = hoehe / 200.0;       //Einteilung Y
      p->setPen(penblacksolid);
      p->drawText(1, hoehe + ObererRand + 20,"Z(Ohm)");
      //Messeinheit anzeigen  rechter Rand
      p->drawText(int(round(px2))-15, hoehe + ObererRand + 20,"Z(Ohm)");
      for(i=0; i <= 200; i++)
      {
        //modulo 2
//        qDebug(((i+1) % xmodulo));
        if((i % 10) == 0){
          //Impedanz Beschriftung aller 10 Ohm an der Y-Achse
          s.sprintf("%i",  200 - i);
          //an den linken Rand schreiben
          p->setPen(penblacksolid);
          p->drawText(1, int(round(py1*i))+ ObererRand + 3,s);
          p->drawText(int(round(px2))+3, int(round(py1*i))+ ObererRand + 3,s);
          if(i == 150){
            p->setPen(penblackdot);
          }else{
            p->setPen(penlightgraydot);
          }
          p->drawLine(int(round(px1)), int(round(py1*i))+ ObererRand,
                     int(round(px2)), int(round(py1*i))+ ObererRand);
        }
      }
      break;
  }
  
  //###################################################################################
  // ENDE Beschriftung des Y-Bereiches
  //###################################################################################
  //###################################################################################
  // Anfang Beschriftung des X-Bereiches
  //###################################################################################
  if(wobbeldaten.ebetriebsart != eantdiagramm){
  //Festlegung der X-Achseneinteilung
  wxanfang = wobbeldaten.anfangsfrequenz;
  wxende = wxanfang + (double(wobbeldaten.schritte) - 1.0) * wobbeldaten.schrittfrequenz;
  wxbereich = (wxende - wxanfang) / 5.0;
  if((wxbereich <=           2.5) & (wxbereich >           1.0))xRaster =           1.0;
  if((wxbereich <=           5.0) & (wxbereich >           2.5))xRaster =           2.5;
  if((wxbereich <=          10.0) & (wxbereich >           5.0))xRaster =           5.0;
  if((wxbereich <=          25.0) & (wxbereich >          10.0))xRaster =          10.0;
  if((wxbereich <=          50.0) & (wxbereich >          25.0))xRaster =          25.0;
  if((wxbereich <=         100.0) & (wxbereich >          50.0))xRaster =          50.0;
  if((wxbereich <=         250.0) & (wxbereich >         100.0))xRaster =         100.0;
  if((wxbereich <=         500.0) & (wxbereich >         250.0))xRaster =         250.0;
  if((wxbereich <=        1000.0) & (wxbereich >         500.0))xRaster =         500.0;
  if((wxbereich <=        2500.0) & (wxbereich >        1000.0))xRaster =        1000.0;
  if((wxbereich <=        5000.0) & (wxbereich >        2500.0))xRaster =        2500.0;
  if((wxbereich <=       10000.0) & (wxbereich >        5000.0))xRaster =        5000.0;
  if((wxbereich <=       25000.0) & (wxbereich >       10000.0))xRaster =       10000.0;
  if((wxbereich <=       50000.0) & (wxbereich >       25000.0))xRaster =       25000.0;
  if((wxbereich <=      100000.0) & (wxbereich >       50000.0))xRaster =       50000.0;
  if((wxbereich <=      250000.0) & (wxbereich >      100000.0))xRaster =      100000.0;
  if((wxbereich <=      500000.0) & (wxbereich >      250000.0))xRaster =      250000.0;
  if((wxbereich <=     1000000.0) & (wxbereich >      500000.0))xRaster =      500000.0;
  if((wxbereich <=     2500000.0) & (wxbereich >     1000000.0))xRaster =     1000000.0;
  if((wxbereich <=     5000000.0) & (wxbereich >     2500000.0))xRaster =     2500000.0;
  if((wxbereich <=    10000000.0) & (wxbereich >     5000000.0))xRaster =     5000000.0;
  if((wxbereich <=    25000000.0) & (wxbereich >    10000000.0))xRaster =    10000000.0;
  if((wxbereich <=    50000000.0) & (wxbereich >    25000000.0))xRaster =    25000000.0;
  if((wxbereich <=   100000000.0) & (wxbereich >    50000000.0))xRaster =    50000000.0;
  if((wxbereich <=   250000000.0) & (wxbereich >   100000000.0))xRaster =   100000000.0;
  if((wxbereich <=   500000000.0) & (wxbereich >   250000000.0))xRaster =   250000000.0;
  if((wxbereich <=  1000000000.0) & (wxbereich >   500000000.0))xRaster =   500000000.0;
  if((wxbereich <=  2500000000.0) & (wxbereich >  1000000000.0))xRaster =  1000000000.0;
  if((wxbereich <=  5000000000.0) & (wxbereich >  2500000000.0))xRaster =  2500000000.0;
  if((wxbereich <= 10000000000.0) & (wxbereich >  5000000000.0))xRaster =  5000000000.0;
  if((wxbereich <= 25000000000.0) & (wxbereich > 10000000000.0))xRaster = 10000000000.0;
  xRasterStr = "Hz";
  me = 1.0;
  frqbereich = hz;
  if(wxende >= 1000.0){
    xRasterStr = "kHz";
    me = 1000.0;
    frqbereich = khz;
  }  
  if(wxende >= 1000000.0){
    xRasterStr = "MHz";
    me = 1000000.0;
    frqbereich = mhz;
  } 
  if(wxende >= 1000000000.0){
    xRasterStr = "GHz";
    me = 1000000000.0;
    frqbereich = ghz;
  } 
  //ganz linke Linie ziehen
  p->setPen(penlightgraydot);
  p->drawLine(LinkerRand, ObererRand, LinkerRand, hoehe + ObererRand);
  //ganz rechts Linie ziehen
  p->drawLine(breite + LinkerRand, ObererRand, breite + LinkerRand, hoehe + ObererRand);
  if(boolrot){
    p->setPen(penredsolid);
  }else{
    p->setPen(penblacksolid);
  }
  p->drawText(2, 13, xRasterStr);
  i = 1;
  if(xRaster < wxanfang)i = int(round( wxanfang / xRaster)) + 1;
  //Beschriftung x-Achse + Linien
  //erste Beschriftung oben
  zeilenwechsel = true;
  while( (xRaster * i) < wxende){
    x = int(round((((xRaster * i) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    //je nach laenge des Strings den Text etwas nach links schieben
    QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));
    //Grundeinstellung
    switch(frqbereich){
      case hz:
        xshift = 10;
        s.sprintf("%3.0f", xRaster*i/me);
        break;
      case khz:
        xshift = 10;
        s.sprintf("%3.1f", xRaster*i/me);
        if((xRaster) < 1000.0){
          // Bereich kleiner 1MHz
          xshift = 15;//Zahl 30px weiter links
          s.sprintf("%3.2f", xRaster*i/me);
        }  
        if((xRaster) < 100.0){
          // Bereich kleiner 100kHz
          xshift = 20;//Zahl 30px weiter links
          s.sprintf("%3.3f", xRaster*i/me);
        }  
        if((xRaster) < 10.0){
          // Bereich kleiner 10kHz
          xshift = 25;//Zahl 30px weiter links
          s.sprintf("%3.4f", xRaster*i/me);
        }  
        break;
      case mhz:
        xshift = 10;
        s.sprintf("%3.1f", xRaster*i/me);
        if((xRaster) < 1000000.0){
          // Bereich kleiner 1MHz
          xshift = 15;//Zahl 30px weiter links
          s.sprintf("%3.2f", xRaster*i/me);
        }  
        if((xRaster) < 100000.0){
          // Bereich kleiner 100kHz
          xshift = 20;//Zahl 30px weiter links
          s.sprintf("%3.3f", xRaster*i/me);
        }  
        if((xRaster) < 10000.0){
          // Bereich kleiner 10kHz
          xshift = 25;//Zahl 30px weiter links
          s.sprintf("%3.4f", xRaster*i/me);
        }  
        if((xRaster) < 1000.0){
          // Bereich kleiner 1kHz
          xshift = 30;//Zahl 30px weiter links
          s.sprintf("%3.5f", xRaster*i/me);
        }  
        if((xRaster) < 100.0){
          // Bereich kleiner 100Hz
          xshift = 35;//Zahl 30px weiter links
          s.sprintf("%3.6f", xRaster*i/me);
        }
        break;
      case ghz:
        s.sprintf("%3.1f", xRaster*i/me);
        xshift = 10;//Zahl 30px weiter links
        if((xRaster) < 10000000000.0){
          // Bereich kleiner 1GHz
          xshift = 15;//Zahl 30px weiter links
          s.sprintf("%3.2f", xRaster*i/me);
        }  
        if((xRaster) < 1000000000.0){
          // Bereich kleiner 100MHz
          xshift = 20;//Zahl 30px weiter links
          s.sprintf("%3.3f", xRaster*i/me);
        }  
        if((xRaster) < 100000000.0){
          // Bereich kleiner 10MHz
          xshift = 25;//Zahl 30px weiter links
          s.sprintf("%3.4f", xRaster*i/me);
        }  
        if((xRaster) < 10000000.0){
          // Bereich kleiner 1MHz
          xshift = 30;//Zahl 30px weiter links
          s.sprintf("%3.5f", xRaster*i/me);
        }  
        if((xRaster) < 100000.0){
          // Bereich kleiner 100kHz
          xshift = 35;//Zahl 30px weiter links
          s.sprintf("%3.6f", xRaster*i/me);
        }  
        if((xRaster) < 10000.0){
          // Bereich kleiner 10kHz
          xshift = 40;//Zahl 30px weiter links
          s.sprintf("%3.7f", xRaster*i/me);
        }  
        if((xRaster) < 1000.0){
          // Bereich kleiner 1kHz
          xshift = 45;//Zahl 30px weiter links
          s.sprintf("%3.8f", xRaster*i/me);
        }  
        if((xRaster) < 100.0){
          // Bereich kleiner 100Hz
          xshift = 50;//Zahl 30px weiter links
          s.sprintf("%3.9f", xRaster*i/me);
        }
        break;
    }
    if(boolrot){
      p->setPen(penredsolid);
    }else{
      p->setPen(penblacksolid);
    }
    if(zeilenwechsel){
      p->drawText(x-xshift, 28, s);
      zeilenwechsel = false;
    }else{  
      p->drawText(x-xshift, 13, s);
      zeilenwechsel = true;
    }
    p->setPen(penlightgraydot);
    p->drawLine(x, ObererRand, x, hoehe + ObererRand);
    i++;
  }
  if(boolrot){
    p->setPen(penredsolid);
  }else{
    p->setPen(penblacksolid);
  }
  xRasterStr = "Hz";
  if(xRaster >= 1000.0){
    xRaster /= 1000.0;
    xRasterStr = "kHz";
  }
  if(xRaster >= 1000.0){
    xRaster /= 1000.0;
    xRasterStr = "MHz";
  }
  qstr = tr("X-Raster:", "im Display");
  s.sprintf(" %1.1f ", xRaster);
  s = qstr + s + xRasterStr;
  if(boolrot){
    p->setPen(penredsolid);
  }else{
    p->setPen(penblacksolid);
  }
  p->drawText(int(px2/2.0)-50, hoehe + ObererRand + 20,s);
  if(boolrot){
    p->setPen(penredsolid);
    s = tr("Shift aktiv", "im Display");
    p->drawText(LinkerRand + 20, hoehe + ObererRand + 20,s);
    p->drawText(breite - 80, hoehe + ObererRand + 20,s);
  }
  //###################################################################################
  // ENDE Beschriftung des X-Bereiches
  //###################################################################################
  //###################################################################################
  // Frequenzmarken 
  //###################################################################################
  p->setPen(pencyansolid);
  if(wfmarken.b160m){
    x = int(round((((wfmarken.f1160m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f2160m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st160m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b80m){
    x = int(round((((wfmarken.f180m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f280m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st80m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b40m){
    x = int(round((((wfmarken.f140m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f240m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st40m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b30m){
    x = int(round((((wfmarken.f130m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f230m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st30m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b20m){
    x = int(round((((wfmarken.f120m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f220m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st20m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b17m){
    x = int(round((((wfmarken.f117m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f217m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st17m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b15m){
    x = int(round((((wfmarken.f115m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f215m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st15m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b12m){
    x = int(round((((wfmarken.f112m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f212m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st12m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b10m){
    x = int(round((((wfmarken.f110m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f210m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st10m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b6m){
    x = int(round((((wfmarken.f16m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f26m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st6m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.b2m){
    x = int(round((((wfmarken.f12m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f22m) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.st2m);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.bu1){
    x = int(round((((wfmarken.f1u1) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f2u1) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.stu1);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.bu2){
    x = int(round((((wfmarken.f1u2) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f2u2) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.stu2);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  if(wfmarken.bu3){
    x = int(round((((wfmarken.f1u3) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    x1 = int(round((((wfmarken.f2u3) - wxanfang) / (wxende - wxanfang))* breite) - 1 + LinkerRand);
    if((x > LinkerRand)and(x < (breite+LinkerRand)))
      p->drawLine(x, ObererRand, x, hoehe + ObererRand);
      if((x1 - x) > 50){
        p->drawText(x-20+(x1-x)/2, (hoehe + ObererRand)/2 , wfmarken.stu3);
      }	
    if((x1 > LinkerRand)and(x1 < (breite+LinkerRand)))
      p->drawLine(x1, ObererRand, x1, hoehe + ObererRand);
  }  
  //###################################################################################
  // Ende Frequenzmarken 
  //###################################################################################
}else{

}
}


void WidgetWobbeln::drawKurve(QPainter *p)
{
//Variablen

  int i,
  k,
  j1,
  j2,
  j3,
  x, 
  x1=0, 
  y=0, 
  y1=0,
  y3=0,
  y4=0,
  mx1[5]={0,0,0,0,0},
  mx2[5]={0,0,0,0,0},
  my1[5]={0,0,0,0,0},
  my2[5]={0,0,0,0,0},
  xm1[5]={0,0,0,0,0},
  xm2[5]={0,0,0,0,0},
  kx [5]={0,0,0,0,0},
  kx1[5]={0,0,0,0,0},
  kx2[5]={0,0,0,0,0},
  kx3[5]={0,0,0,0,0},
  ky1[5]={0,0,0,0,0},
  ky2[5]={0,0,0,0,0},
  kw1[5]={0,0,0,0,0},
  kw2[5]={0,0,0,0,0},
  kw3[5]={0,0,0,0,0},
  db3x1=0,
  db3x2=0,
  maxk1,
  maxk2,
  mink1,
  mink2,
  maxk1f=0,
  maxk2f=0,
  mink1f=0,
  mink2f=0;
  double db3xa, 
  db3xb,
  db3ya, 
  db3yb,
  bandbreite3db=0.0,
  bandbreite6db=0.0,
  px1=0.0,
  py1,
  py2, 
  y1faktor,
  y2faktor,
  y1shift,
  y2shift,
  displayshift,
  minswr, //fuer SWV neu
  maxswr, //fuer SWV neu
  afrequenz=1000000.0,
  uricht=0.0,
  w1,
  w2,
  w3,
  swv=1.0,
  maxdbk1=-100.0,
  mindbk1=20.0,
  maxdbk2=-100.0,
  mindbk2=20.0;

  QString xRasterStr, s, qstr;

  double s_ant;
  double a;
  double r_tx;
  double r_ant;
  //Kabellaengendaten
  double a_100 = 1.0;
  double laenge = 10.0;
  //Kabellaengendaten
  double yzeichenfaktor_log = 0.0;
  double yzeichenshift_log = 0.0;
  double yzeichenfaktor_lin = 0.0;
  double yzeichenshift_lin = 0.0;
  double bx;
  bool bkurvezeichnen;
  bool bmouse = true;
  
  double db3xai = 0.0;
  double db3xbi = 0.0;
  double db3yai = 0.0;
  double db3ybi = 0.0;
  int db3x1i = 0;
  int db3x2i = 0;
  double xfrequenz;
  double grad1;
  double ateiler = 32.0;

  QPen penreddot = QPen( Qt::red, 0, Qt::DotLine);
  QPen penreddash = QPen( Qt::red, 0, Qt::DashLine);
  QPen penbluedash = QPen( Qt::blue, 0, Qt::DashLine);
  QPen pendarkgreendot = QPen( Qt::darkGreen, 0, Qt::DotLine);
  QPen pendarkgreendash = QPen( Qt::darkGreen, 0, Qt::DashLine);
  QPen pendarkbluesolid = QPen( Qt::darkBlue, 0, Qt::SolidLine);
  QPen pendarkredsolid = QPen( Qt::darkRed, 0, Qt::SolidLine);
  QPen penbluesolid = QPen( Qt::blue, 0, Qt::SolidLine);
  
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::drawKurve()");
  #endif
  a_100 = wobbeldaten.a_100;
  laenge = wobbeldaten.kabellaenge;
  double zw1 = double(breite) / (wobbeldaten.absende - wobbeldaten.absanfang);
  xSchritt = zw1 * wobbeldaten.schrittfrequenz;
  xAnfang = zw1 * (wobbeldaten.anfangsfrequenz - wobbeldaten.absanfang);
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
  //###################################################################################
  // ANFANG ermitteln der max- und min-Werte
  //###################################################################################
  y1faktor = wobbeldaten.faktor1;
  y1shift = wobbeldaten.shift1;
  y2faktor = wobbeldaten.faktor2;
  y2shift = wobbeldaten.shift2;
  displayshift = wobbeldaten.displayshift;
  afrequenz = wobbeldaten.anfangsfrequenz;
  maxk1 = 0;
  maxk2 = 0;
  minswr = 99.9;
  maxswr = 1.0;
  //Messdaten mit 10bit laenge
  mink1 = 1023;
  mink2 = 1023;
  maxdbk1 = -100.0;
  mindbk1 = 20.0;
  maxdbk2 = -100.0;
  mindbk2 = 20.0;
  //Neue Berechnung ab V3.05
  yzeichenfaktor_log = hoehe / (ymin - ymax);
  yzeichenshift_log = (ymax * yzeichenfaktor_log) * -1.0;
  yzeichenfaktor_lin = hoehe / (-100.0 - 10.0);
  yzeichenshift_lin = (10.0 * yzeichenfaktor_lin) * -1.0;
  //qDebug("y1faktor %f",y1faktor);
  //qDebug("y1shift  %f",y1shift);
  
  //###################################################################################
  // ANFANG Kurven darstellen
  //###################################################################################
  if(wobbeldaten.mess.daten_enable){
    //##########################################################
    // Alle Gemeinsamkeiten erledigen
    //##########################################################
    // alle Listen loeschen aber nur wenn es die Onlinekurve ist und kein
    // Hintergrund
    if(!wobbeldaten.bhintergrund)emit multieditloeschen();
    switch (wobbeldaten.ebetriebsart){
      case eantdiagramm:
        //Stiftfarbe Kanal1 setzen
        p->setPen( wobbeldaten.penkanal1 );
        //Daten aus Struct umspeichern
        int adbegin;
        adbegin = wobbeldaten.antdiagramm.adbegin;
        int adend;
        adend = wobbeldaten.antdiagramm.adend;
        int idbmin;
        idbmin = wobbeldaten.antdiagramm.dbmin;
        int idbmax;
        idbmax = wobbeldaten.antdiagramm.dbmax;
        int diabegin;//Gradzahl Diagramm Beginn
        diabegin = wobbeldaten.antdiagramm.diabegin;
        int diaend;//Gradzahl Diagramm Ende
        diaend = wobbeldaten.antdiagramm.diaend;
        //aus (gradende - gradbegin) / (adwertgradende - adwertgradbegin)
        //grad1 ist der gradwert pro adschritt
        grad1 = double(diaend - diabegin) / double(adend - adbegin);
        kurve.clear();
        double db_bereich;
        db_bereich = double(idbmax-idbmin);//dB Bereich errechnen
        double grad2bogen;
        grad2bogen = M_PI/180.0;//Umrechnung von Grad auf Bogenmass
        double calgradcorr;//Winkelkorrektur durch adbegin und diabegin
        calgradcorr = (diabegin-adbegin*360/1023)*grad2bogen;
        for(i=adbegin; i<=adend; i++){//von Spannung als AD-Wert fuer unterstes Gradmass
                                          //bis Spannung als AD-Wert fuer oberstes Gradmass
          if(wobbeldaten.mess.k1[i]!=0){  //alle ungueltigen Messwerte nichtberuecksichtigen
            //dBm Wert berechnen aus der Wattmeterkalibrierung
            py1 = wobbeldaten.mess.k1[i] * wobbeldaten.mfaktor1 + wobbeldaten.mshift1;
            //Ueberschreitung des maximalen dB Diagrammwertes
            if(int(py1) > idbmax){
              py1 = double(idbmax);
            }
            //Unterschreitung des minimalen dB Diagrammwertes
            if(int(py1) < idbmin){
              py1 = double(idbmin);
            }
            //Zeigerlaenge des dBm Wertes mit dem Grafikradius berechnen
            py1 = (py1 - double(idbmin))/db_bereich * double(durchmesser/2);
            //Zeiger in x und y Punkte berechnen
            //Zeigeranfang ist Kreismittelpunkt; Zeigerende ist x,y Punkt
            x = LinkerRand + durchmesser/2 +
                int(round(sin(calgradcorr+grad1*double(i)*grad2bogen)*py1));
            y = ObererRand + durchmesser/2 -
                int(round(cos(calgradcorr+grad1*double(i)*grad2bogen)*py1));
            //errechnete Werte in einem Polygon abspeichern
            kurve.append(QPoint(x,y));
          }
        }
        p->drawPolyline(kurve);
        break;
      case espektrumanalyser:
      case espekdisplayshift:
      case ewobbeln:
      //##########################################################
      // Betriebsart Wobbeln
      //##########################################################
      //##########################################################
      // Zuerst Kanal 1 
      //##########################################################
        if(wobbeldaten.bkanal1){
          // Pen setzen
          p->setPen( wobbeldaten.penkanal1 );
          kurve.clear();
          for(i=0; i < wobbeldaten.schritte; i++){
            bkurvezeichnen = true;
            x = int(round(i * xSchritt) + LinkerRand) + (int)xAnfang;
            if( x < LinkerRand)bkurvezeichnen = false;
            if( x > (LinkerRand + breite))bkurvezeichnen = false;
            //x1 = int(round((i+1) * xSchritt) + LinkerRand) + (int)xAnfang;
            //if( x1 < LinkerRand)bkurvezeichnen = false;
            //if( x1 > (LinkerRand + breite))bkurvezeichnen = false;
            //Neue Berechnung ab V3.05
            if(wobbeldaten.linear1){
              //dB ausrechnen LINEAR
              py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift; 
              //py2 = wobbeldaten.mess.k1[i+1] * y1faktor + y1shift;
              py1 = py1 * yzeichenfaktor_lin + yzeichenshift_lin;
              //py2 = py2 * yzeichenfaktor_lin + yzeichenshift_lin;
            }else{
              //dB ausrechnen LOG
              py1 = (wobbeldaten.mess.k1[i] * y1faktor + y1shift) - 
                    getkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * i);
              //py2 = (wobbeldaten.mess.k1[i+1] * y1faktor + y1shift) -
                //    getkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * (i+1));
              //Zuerst dB-Shift hinzu
              py1 = py1 + wobbeldaten.dbshift1;
              //py2 = py2 + wobbeldaten.dbshift1;
              //qDebug("py1+dbshift1 %f",py1);
              //Pegelkorrektur vom SAV
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * i);
                //py2 = korrsavpegel(py2) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * (i+1));
              }
              //Y-Position im Windows errechen
              py1 = py1 * yzeichenfaktor_log + yzeichenshift_log;
              //py2 = py2 * yzeichenfaktor_log + yzeichenshift_log;
              //qDebug("py1 %f",py1);
            }
            // Oberen Rand hinzu addieren
            y = int(round(py1) + ObererRand);
            //y1 = int(round(py2) + ObererRand);
            //qDebug("y %i",y);
            //auf extremwerte reagieren Linie soll Anzeige nicht nach oben oder unten verlassen
            if(y > (hoehe + ObererRand))y = hoehe + ObererRand;
            if(y < ObererRand)y = ObererRand;
            //if(y1 > (hoehe + ObererRand))y1 = hoehe + ObererRand;
            //if(y1 < ObererRand)y1 = ObererRand;
            //Linie zeichnen
            //if(bkurvezeichnen)p->drawLine(x, y, x1, y1);
            if(bkurvezeichnen)kurve.append(QPoint(x,y));
            //Max Min Punkt 1
            if(wobbeldaten.linear1){
              py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift;
            }else{
              py1 = (wobbeldaten.mess.k1[i] * y1faktor + y1shift) - 
                    getkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * i); 
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * i);
              }
            }
            if(py1 > maxdbk1){
              maxdbk1 = py1;
              maxk1f = i;//passende Frequenz dazu
            }
            //minimum Kanal1 ermitteln
            if(py1 < mindbk1){
              mindbk1 = py1;
              mink1f = i;//passende Frequenz dazu
            }
            //Max Min Punkt 2
            //if(wobbeldaten.linear1){
            //  py1 = wobbeldaten.mess.k1[i+1] * y1faktor + y1shift;
            //}else{
            //  py1 = (wobbeldaten.mess.k1[i+1] * y1faktor + y1shift) -
            //        getkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * i+1);
            //  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
            //    py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * (i+1));
            //  }
            //}
            //maximum Kanal1 ermitteln
            //if(py1 > maxdbk1){
            //  maxdbk1 = py1;
            //  maxk1f = i+1;//passende Frequenz dazu
            //}
            //minimum Kanal1 ermitteln
            //if(py1 < mindbk1){
            //  mindbk1 = py1;
            //  mink1f = i+1;//passende Frequenz dazu
            //}
            //Anfangswerte setzen
            for(k=0;k<5;k++){
              if(mouseposx1[k] != mouseposx[k]){
                if(i == 0){
                  xm1[k] = LinkerRand;
                  mx1[k] = x;
                  my1[k] = y;
                  kx[k] = i;
                  ky1[k] = wobbeldaten.mess.k1[i];
                  kxm1[k] = kx[k];
                }
                //neue Werte setzen
                xm2[k] = abs(x - mouseposx[k]);
                if(xm2[k] < xm1[k]){
                  xm1[k] = xm2[k];
                  mx1[k] = x;
                  my1[k] = y;
                  kx[k] = i;
                  ky1[k] = wobbeldaten.mess.k1[kx[k]];
                  kxm1[k] = kx[k];
                }
                /*
                xm2[k] = abs(x1 - mouseposx[k]);
                if(xm2[k] < xm1[k]){
                  xm1[k] = xm2[k];
                  mx1[k] = x1;
                  my1[k] = y1;
                  kx[k] = i + 1;
                  ky1[k] = wobbeldaten.mess.k1[kx[k]];
                  kxm1[k] = kx[k];
                }
                */
              }else{
                mx1[k] = int(round(kxm1[k] * xSchritt) + LinkerRand);
                py1 = (wobbeldaten.mess.k1[kxm1[k]] * y1faktor + y1shift) - 
                      getkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * kxm1[k]);
                py1 = py1 + wobbeldaten.dbshift1;
                if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                  py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * kxm1[k]);
                }
                py1 = py1 * yzeichenfaktor_log + yzeichenshift_log;
                my1[k] = int(round(py1) + ObererRand);
                if(my1[k] > (hoehe + ObererRand))my1[k] = hoehe + ObererRand;
                if(my1[k] < ObererRand)my1[k] = ObererRand;
              }
            }
          }
          p->drawPolyline(kurve);
        }
        //##########################################################
        // Dann Kanal 2 
        //##########################################################
        if(wobbeldaten.bkanal2){
          p->setPen( wobbeldaten.penkanal2);
          kurve.clear();
          //for(i=0; i < wobbeldaten.schritte - 1; i++){
          for(i=0; i < wobbeldaten.schritte; i++){
            //ab V4.11 wobbeldaten.maudio2
            double messk2 = wobbeldaten.mess.k2[i];
            if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
            //Neue Berechnung ab V3.05
            if(wobbeldaten.linear1){
              //dB ausrechnen LINEAR
              py1 = messk2 * y2faktor + y2shift;
              //py2 = wobbeldaten.mess.k2[i+1] * y2faktor + y2shift;
              py1 = py1 * yzeichenfaktor_lin + yzeichenshift_lin;
              //py2 = py2 * yzeichenfaktor_lin + yzeichenshift_lin;
            }else{
              //dB ausrechnen LOG
              py1 = messk2 * y2faktor + y2shift -
                    getkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * i);
              //py2 = wobbeldaten.mess.k2[i+1] * y2faktor + y2shift -
              //      getkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * (i+1));
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * i);
                //py2 = korrsavpegel(py2) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * (i+1));
              }
              //eventuell dbshift addieren
              py1 = py1 + wobbeldaten.dbshift2;
              //py2 = py2 + wobbeldaten.dbshift2;
              //Y-Position im Windows errechen
              py1 = py1 * yzeichenfaktor_log + yzeichenshift_log;
              //py2 = py2 * yzeichenfaktor_log + yzeichenshift_log;
            }
            y = int(round(py1) + ObererRand);
            //y1 = int(round(py2) + ObererRand);
            if(y > (hoehe + ObererRand))y = hoehe + ObererRand;
            if(y < ObererRand)y = ObererRand;
            //if(y1 > (hoehe + ObererRand))y1 = hoehe + ObererRand;
            //if(y1 < ObererRand)y1 = ObererRand;
            bkurvezeichnen = true;
            x = int(round(i * xSchritt) + LinkerRand) + (int)xAnfang;
            if( x < LinkerRand)bkurvezeichnen = false;
            if( x > (LinkerRand + breite))bkurvezeichnen = false;
            //x1 = int(round((i+1) * xSchritt) + LinkerRand) + (int)xAnfang;
            //if( x1 < LinkerRand)bkurvezeichnen = false;
            //if( x1 > (LinkerRand + breite))bkurvezeichnen = false;
            if(bkurvezeichnen)kurve.append(QPoint(x,y));
            if(wobbeldaten.linear2){
              py1 = messk2 * y2faktor + y2shift;
            } else {
              py1 = messk2 * y2faktor + y2shift -
                    getkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * i);
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * i);
              }
            }
            //maximum Kanal2 ermitteln
            if(py1 > maxdbk2){
              maxdbk2 = py1;
              maxk2f = i;//passende Frequenz dazu
            }
            //minimum Kanal2 ermitteln
            if(py1 < mindbk2){
              mindbk2 = py1;
              mink2f = i;//passende Frequenz dazu
            }
            /*
            if(wobbeldaten.linear2){
              py1 = wobbeldaten.mess.k2[i+1] * y2faktor + y2shift;
            } else {
              py1 = wobbeldaten.mess.k2[i+1] * y2faktor + y2shift - 
                    getkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * (i+1)); 
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * (i+1));
              }
            }
            //maximum Kanal2 ermitteln
            if(py1 > maxdbk2){
              maxdbk2 = py1;
              maxk2f = i+1;//passende Frequenz dazu
            }
            //minimum Kanal2 ermitteln
            if(py1 < mindbk2){
              mindbk2 = py1;
              mink2f = i+1;//passende Frequenz dazu
            }
            */
            for(k=0;k<5;k++){
              if(mouseposx1[k] != mouseposx[k]){
                if(i == 0){
                  xm1[k] = LinkerRand;
                  mx2[k] = x;
                  my2[k] = y;
                  kx[k] = i;
                  ky2[k] = messk2;
                  kxm2[k] = kx[k];
                }
                xm2[k] = abs(x - mouseposx[k]);
                if(xm2[k] < xm1[k]){
                  xm1[k] = xm2[k];
                  mx2[k] = x;
                  my2[k] = y;
                  kx[k] = i;
                  double messk21 = wobbeldaten.mess.k2[kx[k]];
                  if(wobbeldaten.maudio2)messk21 = messk21 / ateiler;
                  ky2[k] = messk21;
                  kxm2[k] = kx[k];
                }
                /*
                xm2[k] = abs(x1 - mouseposx[k]);
                if(xm2[k] < xm1[k]){
                  xm1[k] = xm2[k];
                  mx2[k] = x1;
                  my2[k] = y1;
                  kx[k] = i + 1;
                  ky2[k] = wobbeldaten.mess.k2[kx[k]];
                  kxm2[k] = kx[k];
                }
                */
              }else{
                mx2[k] = int(round(kxm2[k] * xSchritt) + LinkerRand);
                double messk21 = wobbeldaten.mess.k2[kxm2[k]];
                if(wobbeldaten.maudio2)messk21 = messk21 / ateiler;
                py1 = (messk21 * y2faktor + y2shift) -
                      getkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * kxm2[k]);
                py1 = py1 + wobbeldaten.dbshift2;
                if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                  py1 = korrsavpegel(py1) - getsavcalwert(afrequenz + wobbeldaten.schrittfrequenz * kxm2[k]);
                }
                py1 = py1 * yzeichenfaktor_log + yzeichenshift_log;
                my2[k] = int(round(py1) + ObererRand);
                if(my2[k] > (hoehe + ObererRand))my2[k] = hoehe + ObererRand;
                if(my2[k] < ObererRand)my2[k] = ObererRand;
              }
            }
          }
          p->drawPolyline(kurve);
        }
        if(!wobbeldaten.bhintergrund){
          //##########################################################
          // Kursor Berechnungen 
          //##########################################################
          //Aus aktiven Kursor die Frequenz ermitteln
          if((mouseposx[kursorix] != 0) && (mouseposy[kursorix] != 0)){ //Cursorindex aktiv
            if(wobbeldaten.bkanal1){
              px1 = (kxm1[kursorix] * wobbeldaten.schrittfrequenz ) + afrequenz;
            }  
            if(wobbeldaten.bkanal2){
              px1 = (kxm2[kursorix] * wobbeldaten.schrittfrequenz ) + afrequenz;
            }  
            if(banfang){ //Uebergabe aus dem Mausmenue
              //qDebug("banfang = false;");
              banfang = false;
              //neue Anfangsfrequenz an nwtlinux.cpp uebermittlen
              emit writeanfang(px1);
            }
            if(bende){ //Uebergabe aus dem Mausmenue
              bende = false;
              //neue Endfrequenz an nwtlinux.cpp uebermittlen
              emit writeende(px1);
            }
            if(bberechnung){ //Uebergabe aus dem Mausmenue
              bberechnung = false;
              //Berechnungsfrequenz an nwtlinux.cpp uebermittlen
              emit writeberechnung(px1);
            }
            mousefrequenz = px1;
          }
          for(k=0;k<5;k++){
            bmouse = true;
            if(k!=0)bmouse = !wobbeldaten.mousesperre;
            if((mouseposx[k] != 0) && (mouseposy[k] != 0) && bmouse ){ //Cursorkreuz aktiv
              qstr = tr("Kursor", "im Infofenster");
              s.sprintf(" %i:", k+1);
              s = qstr + s;
              emit multieditinsert(s);
              if(wobbeldaten.bkanal1){
                px1 = (kxm1[k] * wobbeldaten.schrittfrequenz ) + afrequenz;
              }  
              if(wobbeldaten.bkanal2){
                px1 = (kxm2[k] * wobbeldaten.schrittfrequenz ) + afrequenz;
              }
              s.sprintf("%2.6f MHz", px1 / 1000000.0);
              emit multieditinsert(s);
              //##########################################################
              // Kursortext Kanal 1 
              //##########################################################
              if(wobbeldaten.bkanal1){
                ky1[k] = wobbeldaten.mess.k1[kxm1[k]];
                if(wobbeldaten.linear1){
                  py1 = ky1[k] * y1faktor + y1shift;
                }else{
                  py1 = ky1[k] * y1faktor + y1shift - getkalibrierwertk1(px1); 
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(px1);
                  }
                }
                qstr = tr("Kanal1:", "im Infofenster");
                if(wobbeldaten.linear1){
                  py1 = (100.0 + py1) / 100.0;
                  bx = 20.0 * log10(py1);
                  s.sprintf(" %1.2f, %1.2fdB", py1, bx);
                  s = qstr + s;
                } else {
                  py1 = py1 + wobbeldaten.dbshift1 + displayshift;
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    s = kursorsavstr(py1);
                  }else{
                    s.sprintf(" %2.2fdB", py1);
                    s = qstr + s;
                  }
                }
                emit multieditinsert(s);
              }
              //##########################################################
              // Kursortext Kanal 2 
              //##########################################################
              if(wobbeldaten.bkanal2){
                double messk21 = wobbeldaten.mess.k2[kxm2[k]];
                if(wobbeldaten.maudio2)messk21 = messk21 / ateiler;
                ky2[k] = messk21;
                if(wobbeldaten.linear2){
                  py1 = ky2[k] * y2faktor + y2shift;
                }else{
                  py1 = ky2[k] * y2faktor + y2shift - getkalibrierwertk2(px1);
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(px1);
                  }
                }
                qstr = tr("Kanal2:", "im Infofenster");
                if(wobbeldaten.linear2){
                  py1 = (100.0 + py1) / 100.0;
                  bx = 20.0 * log10(py1);
                  s.sprintf(" %1.2f, %1.2fdB", py1, bx);
                } else {
                  py1 = py1 + wobbeldaten.dbshift2 + displayshift;
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    s = kursorsavstr(py1);
                  }else{
                    s.sprintf(" %2.2fdB", py1);
                    s = qstr + s;
                  }
                }
                emit multieditinsert(s);
              }
              s = "---------------------";
              emit multieditinsert(s);
            }
          }
          //##########################################################
          // max min Text Kanal 1
          //##########################################################
          if(wobbeldaten.bkanal1){
            s = tr("Kanal 1", "im Infofenster");
            emit multieditinsert(s);
            px1 = (maxk1f * wobbeldaten.schrittfrequenz ) + afrequenz;
            px1 = px1 / 1000000.0;
            qstr = tr("max:", "im Infofenster");
            if(wobbeldaten.linear1){
              py1 = (100.0 + maxdbk1) / 100.0;
              bx = 20.0 * log10(py1);
              s.sprintf("%1.2f, %1.2fdB %2.6fMHz", py1, bx, px1);
              squdaten.sprintf("%6.0f;%1.2fdB",px1 * 1000000.0,bx); 
            } else {
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                s.sprintf("%2.2fdBm %2.6fMHz", maxdbk1 + wobbeldaten.dbshift1 + displayshift, px1);
              }else{
                s.sprintf("%2.2fdB %2.6fMHz", maxdbk1 + wobbeldaten.dbshift1 + displayshift, px1);
              }
            }
            s = qstr + s;
            emit multieditinsert(s);
            px1 = (mink1f * wobbeldaten.schrittfrequenz ) + afrequenz;
            px1 = px1 / 1000000.0;
            qstr = tr("min:", "im Infofenster");
            if(wobbeldaten.linear1){
              py1 = (100.0 + mindbk1) / 100.0;
              bx = 20.0 * log10(py1);
              s.sprintf("%1.2f, %1.2fdB %2.6fMHz", py1, bx, px1);
            } else {
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                s.sprintf("%2.2fdBm %2.6fMHz", mindbk1 + wobbeldaten.dbshift1 + displayshift, px1);
              }else{
                s.sprintf("%2.2fdB %2.6fMHz", mindbk1 + wobbeldaten.dbshift1 + displayshift, px1);
              }
            }
            s = qstr + s;
            emit multieditinsert(s);
            //#####################################################
            // Bandbreite 3dB
            //#####################################################
            if(wobbeldaten.bandbreite3db){
              db3xa = 0.0;
              db3xb = 0.0;
              //ermitteln der 1. Frequenz in Richtung tiefer Frequenz
              for(i=maxk1f; i>=0; i--){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(xfrequenz);
                  }
                  // 3db Unterschritten
                  if((maxdbk1 - py1) > 3.0){
                    db3ya = py1;
                    db3xa = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1 = i;
                    break;// Schleife verlassen
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift; 
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  //maxdb
                  py2 = (100.0 + maxdbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((bx - py1) > 3.0){
                    db3ya = py1;
                    db3xa = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1 = i;
                    break;
                  }
                }
              }
              //ermitteln der 2. Frequenz
              for(i=maxk1f; i < wobbeldaten.schritte; i++){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(xfrequenz);
                  }
                  if((maxdbk1 - py1) > 3.0){
                    db3yb = py1;
                    db3xb = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2 = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift; 
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + maxdbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((bx - py1) > 3.0){
                    db3yb = py1;
                    db3xb = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2 = i;
                    break;
                  }
                }
              }
              //invers noch ermitteln
              //ermitteln der 1. Frequenz
              db3xai = 0.0;
              db3xbi = 0.0;
              for(i=mink1f; i>=0; i--){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((py1 - mindbk1) > 3.0){
                    db3yai = py1;
                    db3xai = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1i = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift; 
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 3.0){
                    db3yai = py1;
                    db3xai = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1i = i;
                    break;
                  }
                }
              }
              //ermitteln der 2. Frequenz
              for(i=mink1f; i < wobbeldaten.schritte; i++){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((py1 - mindbk1) > 3.0){
                    db3ybi = py1;
                    db3xbi = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2i = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift; 
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 3.0){
                    db3ybi = py1;
                    db3xbi = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2i = i;
                    break;
                  }
                }
              }
              // es hat geklappt, es ist eine Durchlasskurve voerhanden
              if((db3xa != 0.0) and (db3xb != 0.0)){
                bandbreite3db = db3xb - db3xa;
                s.sprintf(";%1.3f", bandbreite3db);
                squdaten = squdaten + s;
                qstr = tr("B3db:", "im Infofenster");
                s.sprintf(" %1.3f Hz", bandbreite3db);
                if(bandbreite3db > 1000.0){
                  s.sprintf(" %1.3f kHz", bandbreite3db/1000.0);
                }
                if(bandbreite3db > 1000000.0){
                  s.sprintf(" %1.3f MHz", bandbreite3db/1000000.0);
                }
                s = qstr + s;
                emit multieditinsert(s);
                s.sprintf("Q: %1.2f", ((db3xa + db3xb)/2.0)/bandbreite3db);
                emit multieditinsert(s);
                if(wobbeldaten.bandbreitegrafik){
                  p->setPen(penreddot);
                  x = int(round(db3x1 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  x = int(round(db3x2 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                }
                //untere Frequenz
                s.sprintf("f1: %1.6f MHz", db3xa/1000000.0);
                emit multieditinsert(s);
                //mitten Frequenz
                s.sprintf("fm: %1.6f MHz", (db3xa + ((db3xb - db3xa) / 2))/1000000.0);
                emit multieditinsert(s);
                //obere Frequenz
                s.sprintf("f2: %1.6f MHz", db3xb/1000000.0);
                emit multieditinsert(s);
              } else {
                //es hat nicht geklappt nichts gefunden
                s = tr("B3db: keine", "im Infofenster");
                emit multieditinsert(s);
              }
              //inverser 3dB Buckel
              if(wobbeldaten.binvers){
                if((db3xai != 0.0) and (db3xbi != 0.0)){
                  bandbreite3db = db3xbi - db3xai;
                  qstr = tr("B3db_inv:", "im Infofenster");
                  s.sprintf(" %1.3f Hz", bandbreite3db);
                  if(bandbreite3db > 1000.0){
                    s.sprintf(" %1.3f kHz", bandbreite3db/1000.0);
                  }
                  if(bandbreite3db > 1000000.0){
                    s.sprintf(" %1.3f MHz", bandbreite3db/1000000.0);
                  }
                  s = qstr + s;
                  emit multieditinsert(s);
                  qstr = tr("Q_inv:", "im Infofenster");

                  s.sprintf(" %1.2f", ((db3xai + db3xbi)/2.0)/bandbreite3db);
                  s = qstr + s;
                  emit multieditinsert(s);
                  if(wobbeldaten.bandbreitegrafik){
                    p->setPen(penreddot);
                    x = int(round(db3x1i * xSchritt) + LinkerRand);
                    p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                    x = int(round(db3x2i * xSchritt) + LinkerRand);
                    p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  }
                  s.sprintf("f1: %1.6f MHz", db3xai/1000000.0);
                  emit multieditinsert(s);
                  s.sprintf("f2: %1.6f MHz", db3xbi/1000000.0);
                  emit multieditinsert(s);
                } else {
                  s = tr("B3db_inv: keine", "im Infofenster");
                  emit multieditinsert(s);
                }
              }
            }
            // Bandbreite 6dB
            if(wobbeldaten.bandbreite6db){
              db3xa = 0.0;
              db3xb = 0.0;
              //ermitteln der 1. Frequenz
              for(i=maxk1f; i>=0; i--){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(xfrequenz);
                  }
                  if((maxdbk1 - py1) > 6.0){
                    db3ya = py1;
                    db3xa = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1 = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift; 
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + maxdbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((bx - py1) > 6.0){
                    db3ya = py1;
                    db3xa = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1 = i;
                    break;
                  }
                }
              }
              //ermitteln der 2. Frequenz
              for(i=maxk1f; i < wobbeldaten.schritte; i++){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(xfrequenz);
                  }
                  if((maxdbk1 - py1) > 6.0){
                    db3yb = py1;
                    db3xb = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2 = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift; 
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + maxdbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((bx - py1) > 6.0){
                    db3yb = py1;
                    db3xb = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2 = i;
                    break;
                  }
                }
              }
              //es hat geklappt Durchlasskurve vorhanden
              if((db3xa != 0.0) and (db3xb != 0.0)){
                bandbreite6db = db3xb - db3xa;
                qstr = tr("B6db:", "im Infofenster");
                s.sprintf(" %1.3f Hz", bandbreite6db);
                if(bandbreite3db > 1000.0){
                  s.sprintf(" %1.3f kHz", bandbreite6db/1000.0);
                }
                if(bandbreite3db > 1000000.0){
                  s.sprintf(" %1.3f MHz", bandbreite6db/1000000.0);
                }
                s = qstr + s;
                emit multieditinsert(s);
                if(wobbeldaten.bandbreitegrafik){
                  p->setPen(penreddash);
                  x = int(round(db3x1 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  x = int(round(db3x2 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                }
                s.sprintf("f1: %1.6f MHz", db3xa/1000000.0);
                emit multieditinsert(s);
                s.sprintf("f2: %1.6f MHz", db3xb/1000000.0);
                emit multieditinsert(s);
              } else {
                s = tr("B6db: keine", "im Infofenster");
                emit multieditinsert(s);
              }
            }
            //noch die 60dB Bandbreite ermitteln wenn vorhanden
            if(wobbeldaten.bandbreite6db){
              db3xa = 0.0;
              db3xb = 0.0;
              //ermitteln der 1. Frequenz
              for(i=maxk1f; i>=0; i--){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(xfrequenz);
                  }
                  if((maxdbk1 - py1) > 60.0){
                    db3ya = py1;
                    db3xa = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1 = i;
                    break;
                  }
                }
              }
              //ermitteln der 2. Frequenz
              for(i=maxk1f; i < wobbeldaten.schritte; i++){
                if(!wobbeldaten.linear1){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = wobbeldaten.mess.k1[i] * y1faktor + y1shift - getkalibrierwertk1(xfrequenz); 
                  if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                    py1 = korrsavpegel(py1) - getsavcalwert(xfrequenz);
                  }
                  if((maxdbk1 - py1) > 60.0){
                    db3yb = py1;
                    db3xb = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2 = i;
                    break;
                  }
                }
              }
             //es hat geklappt Durchlasskurve vorhanden
              if((db3xa != 0.0) and (db3xb != 0.0)){
                bandbreite3db = db3xb - db3xa;
                qstr = tr("B60db:", "im Infofenster");
                s.sprintf(" %1.3f Hz", bandbreite3db);
                if(bandbreite3db > 1000.0){
                  s.sprintf(" %1.3f kHz", bandbreite3db/1000.0);
                }
                if(bandbreite3db > 1000000.0){
                  s.sprintf(" %1.3f MHz", bandbreite3db/1000000.0);
                }
                s = qstr + s;
                emit multieditinsert(s);
                if(wobbeldaten.bandbreitegrafik){
                  p->setPen(penbluedash);
                  x = int(round(db3x1 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  x = int(round(db3x2 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                }
                s.sprintf("f1: %1.6f MHz", db3xa/1000000.0);
                emit multieditinsert(s);
                s.sprintf("f2: %1.6f MHz", db3xb/1000000.0);
                emit multieditinsert(s);
                s.sprintf("Shape-Faktor: %1.6f", bandbreite3db / bandbreite6db);
                emit multieditinsert(s);
              } else {
                s = "B60db: keine";
                emit multieditinsert(s);
              }
            }
            s = "---------------------";
            emit multieditinsert(s);
          }
          //##########################################################
          // max min Text Kanal 2
          //##########################################################
          if(wobbeldaten.bkanal2){
            s = tr("Kanal 2", "im Infofenster");
            emit multieditinsert(s);
            px1 = (maxk2f * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
            px1 = px1 / 1000000.0;
            qstr = tr("max:", "im Infofenster");
            if(wobbeldaten.linear2){
              py1 = (100.0 + maxdbk2) / 100.0;
              bx = 20.0 * log10(py1);
              s.sprintf("%1.2f, %1.2fdB %2.6fMHz", py1, bx, px1);
            } else {
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                s.sprintf("%2.2fdBm %2.6fMHz", maxdbk2 + wobbeldaten.dbshift2 + displayshift, px1);
              }else{
                s.sprintf("%2.2fdB %2.6fMHz", maxdbk2 + wobbeldaten.dbshift2 + displayshift, px1);
              }
            }
            s = qstr + s;
            emit multieditinsert(s);
            px1 = (mink2f * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
            px1 = px1 / 1000000.0;
            qstr = tr("min:", "im Infofenster");
            if(wobbeldaten.linear2){
              py1 = (100.0 + mindbk2) / 100.0;
              bx = 20.0 * log10(py1);
              s.sprintf("%1.2f, %1.2fdB %2.6fMHz", py1, bx, px1);
            } else {
              if((wobbeldaten.ebetriebsart == espektrumanalyser) or (wobbeldaten.ebetriebsart == espekdisplayshift)){
                s.sprintf("%2.2fdBm %2.6fMHz", mindbk2 + wobbeldaten.dbshift2 + displayshift, px1);
              }else{
                s.sprintf("%2.2fdB %2.6fMHz", mindbk2 + wobbeldaten.dbshift2 + displayshift, px1);
              }
            }
            s = qstr + s;
            emit multieditinsert(s);
            //3dB Bandbreite + Q fuer Kanal 2
            if(wobbeldaten.bandbreite3db){
              db3xa = 0.0;
              db3xb = 0.0;
              //ermitteln der unteren Frequenz
              for(i=maxk2f; i>=0; i--){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((maxdbk2 - py1) > 3.0){
                    db3ya = py1;
                    db3xa = xfrequenz;
                    db3x1 = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift;
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 3.0){
                    db3ya = py1;
                    db3xa = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1 = i;
                    break;
                  }
                }
              }
              //ermitteln der oberen Frequenz
              for(i=maxk2f; i < wobbeldaten.schritte; i++){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((maxdbk2 - py1) > 3.0){
                    db3yb = py1;
                    db3xb = xfrequenz;
                    db3x2 = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift;
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 3.0){
                    db3yb = py1;
                    db3xb = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2 = i;
                    break;
                  }
                }
              }
              //invers
              db3xai = 0.0;
              db3xbi = 0.0;
              //ermitteln der unteren Frequenz
              for(i=mink2f; i>=0; i--){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((py1 - mindbk2) > 3.0){
                    db3yai = py1;
                    db3xai = xfrequenz;
                    db3x1i = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift;
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 3.0){
                    db3yai = py1;
                    db3xai = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1i = i;
                    break;
                  }
                }
              }
              //ermitteln der oberen Frequenz
              for(i=mink2f; i < wobbeldaten.schritte; i++){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((py1 - mindbk2) > 3.0){
                    db3ybi = py1;
                    db3xbi = xfrequenz;
                    db3x2i = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift;
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 3.0){
                    db3ybi = py1;
                    db3xbi = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2i = i;
                    break;
                  }
                }
              }
              //Wurden 3dB Werte gefunden
              if((db3xa != 0.0) and (db3xb != 0.0)){
                bandbreite3db = db3xb - db3xa;
                qstr = tr("B3db:", "im Infofenster");
                s.sprintf(" %1.3f Hz", bandbreite3db);
                if(bandbreite3db > 1000.0)s.sprintf(" %1.3f kHz", bandbreite3db/1000.0);
                if(bandbreite3db > 1000000.0)s.sprintf(" %1.3f MHz", bandbreite3db/1000000.0);
                s = qstr + s;
                emit multieditinsert(s);
                s.sprintf("Q: %1.2f", ((db3xa + db3xb)/2.0)/bandbreite3db);
                emit multieditinsert(s);
                //Bandbreitengrafik aktiv
                if(wobbeldaten.bandbreitegrafik){
                  p->setPen(pendarkgreendot);
                  x = int(round(db3x1 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  x = int(round(db3x2 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                }
                s.sprintf("f1: %1.6f MHz", db3xa/1000000.0);
                emit multieditinsert(s);
                s.sprintf("f2: %1.6f MHz", db3xb/1000000.0);
                emit multieditinsert(s);
              } else {
                s = tr("B3db: keine", "im Infofenster");
                emit multieditinsert(s);
              }
              //inverse Resonanz ermitteln nur 3dB Kurve
              if(wobbeldaten.binvers){
                if((db3xai != 0.0) and (db3xbi != 0.0)){
                  bandbreite3db = db3xbi - db3xai;
                  qstr = tr("B3db_inv:", "im Infofenster");
                  s.sprintf(" %1.3f Hz", bandbreite3db);
                  if(bandbreite3db > 1000.0)s.sprintf(" %1.3f kHz", bandbreite3db/1000.0);
                  if(bandbreite3db > 1000000.0)s.sprintf(" %1.3f MHz", bandbreite3db/1000000.0);
                  s = qstr + s;
                  emit multieditinsert(s);
                  qstr = tr("Q_inv:", "im Infofenster");
                  s.sprintf(" %1.2f", ((db3xai + db3xbi)/2.0)/bandbreite3db);
                  s = qstr + s;
                  emit multieditinsert(s);
                  if(wobbeldaten.bandbreitegrafik){
                    p->setPen(penreddot);
                    x = int(round(db3x1i * xSchritt) + LinkerRand);
                    p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                    x = int(round(db3x2i * xSchritt) + LinkerRand);
                    p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  }
                  s.sprintf("f1: %1.6f MHz", db3xai/1000000.0);
                  emit multieditinsert(s);
                  s.sprintf("f2: %1.6f MHz", db3xbi/1000000.0);
                  emit multieditinsert(s);
                } else {
                  s = tr("B3db_inv: keine", "im Infofenster");
                  emit multieditinsert(s);
                }
              }
            }
            //Bandbreite 6dB ermitteln
            if(wobbeldaten.bandbreite6db){
              db3xa = 0.0;
              db3xb = 0.0;
              //ermitteln der 1. Frequenz in Richtung tiefer Frequenz
              for(i=maxk2f; i>=0; i--){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((maxdbk2 - py1) > 6.0){
                    db3ya = py1;
                    db3xa = xfrequenz;
                    db3x1 = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift;
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 6.0){
                    db3ya = py1;
                    db3xa = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x1 = i;
                    break;
                  }
                }
              }
              //ermitteln der 2. Frequenz in Richtung hoher Frequenz
              for(i=maxk2f; i < wobbeldaten.schritte; i++){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((maxdbk2 - py1) > 6.0){
                    db3yb = py1;
                    db3xb = xfrequenz;
                    db3x2 = i;
                    break;
                  }
                }else{
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift;
                  py1 = (100.0 + py1) / 100.0;
                  py1 = 20.0 * log10(py1);
                  py2 = (100.0 + mindbk1) / 100.0;
                  bx = 20.0 * log10(py2);
                  if((py1 - bx) > 6.0){
                    db3yb = py1;
                    db3xb = (i * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
                    db3x2 = i;
                    break;
                  }
                }
              }
              if((db3xa != 0.0) and (db3xb != 0.0)){
                bandbreite6db = db3xb - db3xa;
                qstr = tr("B6db:", "im Infofenster");
                s.sprintf(" %1.3f Hz", bandbreite6db);
                if(bandbreite3db > 1000.0)s.sprintf(" %1.3f kHz", bandbreite6db/1000.0);
                if(bandbreite3db > 1000000.0)s.sprintf(" %1.3f MHz", bandbreite6db/1000000.0);
                s = qstr + s;
                emit multieditinsert(s);
                if(wobbeldaten.bandbreitegrafik){
                  p->setPen(pendarkgreendash);
                  x = int(round(db3x1 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  x = int(round(db3x2 * xSchritt) + LinkerRand);
                  p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                }
                s.sprintf("f1: %1.6f MHz", db3xa/1000000.0);
                emit multieditinsert(s);
                s.sprintf("f2: %1.6f MHz", db3xb/1000000.0);
                emit multieditinsert(s);
              } else {
                s = tr("B6db: keine", "im Infofenster");
                emit multieditinsert(s);
              }
            }
            //Ermitteln der Bandbreite 60dB
            if(wobbeldaten.bandbreite6db){
              db3xa = 0.0;
              db3xb = 0.0;
              //ermitteln der 1. Frequenz
              for(i=maxk2f; i>=0; i--){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((maxdbk2 - py1) > 60.0){
                    db3ya = py1;
                    db3xa = xfrequenz;
                    db3x1 = i;
                    break;
                  }
                }
              }
              //ermitteln der 2. Frequenz
              for(i=maxk2f; i < wobbeldaten.schritte; i++){
                double messk2 = wobbeldaten.mess.k2[i];
                if(wobbeldaten.maudio2)messk2 = messk2 / ateiler;
                if(!wobbeldaten.linear2){
                  xfrequenz = afrequenz + wobbeldaten.schrittfrequenz * i;
                  py1 = messk2 * y2faktor + y2shift - getkalibrierwertk2(xfrequenz);
                  if((maxdbk2 - py1) > 60.0){
                    db3yb = py1;
                    db3xb = xfrequenz;
                    db3x2 = i;
                    break;
                  }
                }
              }
              //es hat geklappt Durchlasskurve vorhanden
              if((!wobbeldaten.linear2)){
                if((db3xa != 0.0) and (db3xb != 0.0)){
                  bandbreite3db = db3xb - db3xa;
                  qstr = tr("B60db:", "im Infofenster");
                  s.sprintf(" %1.3f Hz", bandbreite3db);
                  if(bandbreite3db > 1000.0)s.sprintf(" %1.3f kHz", bandbreite3db/1000.0);
                  if(bandbreite3db > 1000000.0)s.sprintf(" %1.3f MHz", bandbreite3db/1000000.0);
                  s = qstr + s;
                  emit multieditinsert(s);
                  if(wobbeldaten.bandbreitegrafik){
                    p->setPen(penbluedash);
                    x = int(round(db3x1 * xSchritt) + LinkerRand);
                    p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                    x = int(round(db3x2 * xSchritt) + LinkerRand);
                    p->drawLine(x, ObererRand, x, hoehe + ObererRand);
                  }
                  s.sprintf("f1: %1.6f MHz", db3xa/1000000.0);
                  emit multieditinsert(s);
                  s.sprintf("f2: %1.6f MHz", db3xb/1000000.0);
                  emit multieditinsert(s);
                  qstr = tr("Shape-Faktor:", "im Infofenster");
                  s.sprintf(" %1.6f", bandbreite3db / bandbreite6db);
                  s =qstr + s;
                  emit multieditinsert(s);
                } else {
                  s = tr("B60db: keine", "im Infofenster");
                  emit multieditinsert(s);
                }
              }
            }
            s = "---------------------";
            emit multieditinsert(s);
          }
        }
        break;
      case eswrneu:
      case eswrant:
      case eimpedanz:
      //##########################################################
      // Betriebsart SWVneu 
      //##########################################################
      afrequenz = wobbeldaten.anfangsfrequenz;
      kurve.clear();
      kurveb.clear();
      //for(i=0; i < wobbeldaten.schritte - 1; i++){
      for(i=0; i < wobbeldaten.schritte; i++){
        if(wobbeldaten.bswrkanal2){
          p->setPen( wobbeldaten.penkanal2 );
        }else{
          p->setPen( wobbeldaten.penkanal1 );
        }
        bkurvezeichnen = true;
        x = int(round(i * xSchritt) + LinkerRand) + (int)xAnfang;
        if( x < LinkerRand)bkurvezeichnen = false;
        if( x > (LinkerRand + breite))bkurvezeichnen = false;
        /*
        x1 = int(round((i+1) * xSchritt) + LinkerRand) + (int)xAnfang;
        if( x1 < LinkerRand)bkurvezeichnen = false;
        if( x1 > (LinkerRand + breite))bkurvezeichnen = false;
        */
        j1 = i - 1;
        j2 = i;
        j3 = i + 1;
        if(j1<0)j1=0;
        if(j3>(wobbeldaten.schritte-1))j3 = wobbeldaten.schritte-1;
        //Return loss berechnen
        if(wobbeldaten.bswrkanal2){
          w1 = double(wobbeldaten.mess.k2[j1]);
          w2 = double(wobbeldaten.mess.k2[j2]);
          w3 = double(wobbeldaten.mess.k2[j3]);
          py1 = w2 * y2faktor + y2shift;
          py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(j2));
        }else{
          w1 = double(wobbeldaten.mess.k1[j1]);
          w2 = double(wobbeldaten.mess.k1[j2]);
          w3 = double(wobbeldaten.mess.k1[j3]);
          py1 = w2 * y1faktor + y1shift;
          py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(j2));
        }
        if(py1 > -1.0)py1 = -1.0;
        uricht = pow(10.0 , (py1/20.0));
        swv = (1.0 + uricht)/(1.0 - uricht);
        if(wobbeldaten.bswriteration){
          if(wobbeldaten.bswrkanal2){
            py1 = w1 * y2faktor + y2shift;
            py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(j1));
          }else{
            py1 = w1 * y1faktor + y1shift;
            py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(j1));
          }
          if(py1 > -1.0)py1 = -1.0;
          uricht = pow(10.0 , (py1/20.0)); //Reflektionsfaktor ausrechnen
          swv = swv + (1.0 + uricht)/(1.0 - uricht); //swv ausrechnen
          if(wobbeldaten.bswrkanal2){
            py1 = w3 * y2faktor + y2shift;
            py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(j3));
          }else{
            py1 = w3 * y1faktor + y1shift;
            py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(j3));
          }
          if(py1 > -1.0)py1 = -1.0;
          uricht = pow(10.0 , (py1/20.0));
          swv = swv + (1.0 + uricht)/(1.0 - uricht);
          swv = swv / 3.0; // da Iteration durch 3 Teilen
        }
        if((wobbeldaten.ebetriebsart == eswrneu)or(wobbeldaten.ebetriebsart == eswrant)){
          y = int(round(hoehe - round((swv - 1.0) * (double(hoehe)/yswrteilung)) - 1 + ObererRand));
        } 
        if(wobbeldaten.ebetriebsart == eimpedanz){
          swv = (50.0 * swv) - 50.0;
          y = int(round(hoehe - round((swv - 1.0) * (double(hoehe)/200.0)) - 2 + ObererRand));
        } 
        if(wobbeldaten.ebetriebsart == eswrant){ 
          a = a_100 * laenge / 100.0;
          r_tx = (swv - 1.0) / (swv + 1.0);
          r_ant = r_tx * pow(10.0 , a/10.0);
          if(r_ant >= 1.0)r_ant = 0.99;
          s_ant = (1.0 + r_ant) / (1.0 - r_ant);
          y3 = int(round(hoehe - round((s_ant - 1.0) * (double(hoehe)/yswrteilung)) - 1 + ObererRand));
        }
        j1 = i ;
        j2 = i + 1;
        j3 = i + 2;
        if(j2>(wobbeldaten.schritte-1))j2 = wobbeldaten.schritte-1;
        if(j3>(wobbeldaten.schritte-1))j3 = wobbeldaten.schritte-1;
        if(wobbeldaten.bswrkanal2){
          w1 = double(wobbeldaten.mess.k2[j1]);
          w2 = double(wobbeldaten.mess.k2[j2]);
          w3 = double(wobbeldaten.mess.k2[j3]);
          py1 = w2 * y2faktor + y2shift;
          py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(j2));
        }else{
          w1 = double(wobbeldaten.mess.k1[j1]);
          w2 = double(wobbeldaten.mess.k1[j2]);
          w3 = double(wobbeldaten.mess.k1[j3]);
          py1 = w2 * y1faktor + y1shift;
          py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(j2));
        }
        if(py1 > -1.0)py1 = -1.0;
        uricht = pow(10.0 , (py1/20.0));
        swv = (1.0 + uricht)/(1.0 - uricht);
        if(wobbeldaten.bswriteration){
          if(wobbeldaten.bswrkanal2){
            py1 = w1 * y2faktor + y2shift;
            py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(j1));
          }else{
            py1 = w1 * y1faktor + y1shift;
            py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(j1));
          }
          if(py1 > -1.0)py1 = -1.0;
          uricht = pow(10.0 , (py1/20.0));
          swv = swv + (1.0 + uricht)/(1.0 - uricht);
          if(wobbeldaten.bswrkanal2){
            py1 = w3 * y2faktor + y2shift;
            py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(j3));
          }else{
            py1 = w3 * y1faktor + y1shift;
            py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(j3));
          }
          if(py1 > -1.0)py1 = -1.0;
          uricht = pow(10.0 , (py1/20.0));
          swv = swv + (1.0 + uricht)/(1.0 - uricht);
          swv = swv / 3.0;
         }
	//ermitteln des minimalen SWV in ganzen Bereich
         if(minswr > swv){
           minswr = swv;
           mink1 = j2;
         }  
	//ermitteln des maximalen SWV in ganzen Bereich
         if(maxswr < swv){
           maxswr = swv;
           maxk1 = j2;
        }  
        if((wobbeldaten.ebetriebsart == eswrneu)or(wobbeldaten.ebetriebsart == eswrant)){
          y1 = int(round(hoehe - round((swv - 1.0) * (double(hoehe)/yswrteilung)) - 1 + ObererRand));
        }
        if(wobbeldaten.ebetriebsart == eimpedanz){
          swv = (50.0 * swv) - 50.0;
          y1 = int(round(hoehe - round((swv - 1.0) * (double(hoehe)/200.0)) - 2 + ObererRand));
        }
        if(wobbeldaten.ebetriebsart == eswrant){ 
          a = a_100 * laenge / 100.0;
          r_tx = (swv - 1.0) / (swv + 1.0);
          r_ant = r_tx * pow(10.0 , a/10.0);
          if(r_ant >= 1.0)r_ant = 0.99;
          s_ant = (1.0 + r_ant) / (1.0 - r_ant);
          y4 = int(round(hoehe - round((s_ant - 1.0) * (double(hoehe)/yswrteilung)) - 1 + ObererRand));
        }
        //auf extremwerte reagieren Linie soll Anzeige nicht nach oben verlassen
        if(y > (hoehe + ObererRand))y = hoehe + ObererRand;
        if(y < ObererRand)y = ObererRand;
        if(wobbeldaten.ebetriebsart == eswrant){ 
          if(y3 > (hoehe + ObererRand))y3 = hoehe + ObererRand;
          if(y3 < ObererRand)y3 = ObererRand;
        }  
        //Linie zeichnen
        if(bkurvezeichnen){
          kurve.append(QPoint(x, y)); // neuen Messpunkt anfuegen
          if(wobbeldaten.ebetriebsart == eswrant){
            kurveb.append(QPoint(x, y3)); // neuen Ant-Messpunkt anfuegen
          }
        }
        // keine Hintergrundkurven aus den WKM Marker-Berechnung
        if(!wobbeldaten.bhintergrund){ 
          //Anfangswerte setzen
          for(k=0;k<5;k++){
            if(mouseposx1[k] != mouseposx[k]){
              if(i == 0){
                xm1[k] = LinkerRand;
                mx1[k] = x; //pos im Wobbelfenster
                my1[k] = y; //pos im Wobbelfenster
                kx[k] = i; //nr des Schrittes
                kx1[k] = i - 1;
                kx2[k] = i ;
                if(kx1[k]<0)kx1[k]=0;
                kx3[k] = i + 1;
                if(kx3[k]>(wobbeldaten.schritte-1))kx3[k] = wobbeldaten.schritte-1;
                if(wobbeldaten.bswrkanal2){
                  kw1[k] = wobbeldaten.mess.k2[kx1[k]];
                  kw2[k] = wobbeldaten.mess.k2[kx2[k]];
                  kw3[k] = wobbeldaten.mess.k2[kx3[k]];
                }else{
                  kw1[k] = wobbeldaten.mess.k1[kx1[k]];
                  kw2[k] = wobbeldaten.mess.k1[kx2[k]];
                  kw3[k] = wobbeldaten.mess.k1[kx3[k]];
                }
                kxm1[k] = kx[k];
              }
              //neue Werte setzen
              xm2[k] = abs(x - mouseposx[k]);
              if(xm2[k] < xm1[k]){
                xm1[k] = xm2[k];
                mx1[k] = x; //pos im Wobbelfenster
                my1[k] = y; //pos im Wobbelfenster
                kx[k] = i;  //nr des Schrittes
                kx1[k] = kx[k] - 1;  //nr des Schrittes
                kx2[k] = kx[k];      //nr des Schrittes
                kx3[k] = kx[k] + 1;  //nr des Schrittes
                if(kx1[k]<0)kx1[k]=0;
                if(kx3[k]>(wobbeldaten.schritte-1))kx3[k] = wobbeldaten.schritte-1;
                if(wobbeldaten.bswrkanal2){
                  kw1[k] = wobbeldaten.mess.k2[kx1[k]];
                  kw2[k] = wobbeldaten.mess.k2[kx2[k]];
                  kw3[k] = wobbeldaten.mess.k2[kx3[k]];
                }else{
                  kw1[k] = wobbeldaten.mess.k1[kx1[k]];
                  kw2[k] = wobbeldaten.mess.k1[kx2[k]];
                  kw3[k] = wobbeldaten.mess.k1[kx3[k]];
                }
                if(wobbeldaten.ebetriebsart == eswrant){ 
                  my2[k] = y3;
                  mx2[k] = mx1[k];
                }
                kxm1[k] = kx[k];
              }
              xm2[k] = abs(x1 - mouseposx[k]);
              if(xm2[k] < xm1[k]){
                xm1[k] = xm2[k];
                mx1[k] = x1; //pos im Wobbelfenster
                my1[k] = y1; //pos im Wobbelfenster
                kx[k] = i + 1;  //nr des Schrittes
                kx1[k] = kx[k] - 1;  //nr des Schrittes
                kx2[k] = kx[k];      //nr des Schrittes
                kx3[k] = kx[k] + 1;  //nr des Schrittes
                if(kx1[k]<0)kx1[k]=0;
                if(kx3[k]>(wobbeldaten.schritte-1))kx3[k] = wobbeldaten.schritte-1;
                if(wobbeldaten.bswrkanal2){
                  kw1[k] = wobbeldaten.mess.k2[kx1[k]];
                  kw2[k] = wobbeldaten.mess.k2[kx2[k]];
                  kw3[k] = wobbeldaten.mess.k2[kx3[k]];
                }else{
                  kw1[k] = wobbeldaten.mess.k1[kx1[k]];
                  kw2[k] = wobbeldaten.mess.k1[kx2[k]];
                  kw3[k] = wobbeldaten.mess.k1[kx3[k]];
                }
                if(wobbeldaten.ebetriebsart == eswrant){ 
                  my2[k] = y4;
                  mx2[k] = mx1[k];
                }
                kxm1[k] = kx[k];
              }
            }else{
              kx1[k] = kxm1[k] - 1;  //nr des Schrittes
              kx2[k] = kxm1[k];      //nr des Schrittes
              kx3[k] = kxm1[k] + 1;  //nr des Schrittes
              if(kx1[k]<0)kx1[k]=0;
              if(kx3[k]>(wobbeldaten.schritte-1))kx3[k] = wobbeldaten.schritte-1;
              if(wobbeldaten.bswrkanal2){
                kw1[k] = wobbeldaten.mess.k2[kx1[k]];
                kw2[k] = wobbeldaten.mess.k2[kx2[k]];
                kw3[k] = wobbeldaten.mess.k2[kx3[k]];
              }else{
                kw1[k] = wobbeldaten.mess.k1[kx1[k]];
                kw2[k] = wobbeldaten.mess.k1[kx2[k]];
                kw3[k] = wobbeldaten.mess.k1[kx3[k]];
              }
              mx1[k] = int(round(kxm1[k] * xSchritt) + LinkerRand);
              if(wobbeldaten.ebetriebsart == eswrant){ 
                mx2[k] = mx1[k];
              }
              if(wobbeldaten.bswrkanal2){
                w1 = double(wobbeldaten.mess.k2[kx1[k]]);
                w2 = double(wobbeldaten.mess.k2[kx2[k]]);
                w3 = double(wobbeldaten.mess.k2[kx3[k]]);
                py1 = w2 * y2faktor + y2shift;
                py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(kx2[k]));
              }else{
                w1 = double(wobbeldaten.mess.k1[kx1[k]]);
                w2 = double(wobbeldaten.mess.k1[kx2[k]]);
                w3 = double(wobbeldaten.mess.k1[kx3[k]]);
                py1 = w2 * y1faktor + y1shift;
                py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(kx2[k]));
              }
              if(py1 > -1.0)py1 = -1.0;
              uricht = pow(10.0 , (py1/20.0));
              swv = (1.0 + uricht)/(1.0 - uricht);
              if(wobbeldaten.bswriteration){
                if(wobbeldaten.bswrkanal2){
                  py1 = w1 * y2faktor + y2shift;
                  py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(kx1[k]));
                }else{
                  py1 = w1 * y1faktor + y1shift;
                  py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(kx1[k]));
                }
                if(py1 > -1.0)py1 = -1.0;
                uricht = pow(10.0 , (py1/20.0));
                swv = swv + (1.0 + uricht)/(1.0 - uricht);
                if(wobbeldaten.bswrkanal2){
                  py1 = w3 * y2faktor + y2shift;
                  py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(kx3[k]));
                }else{
                  py1 = w3 * y1faktor + y1shift;
                  py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(kx3[k]));
                }
                if(py1 > -1.0)py1 = -1.0;
                uricht = pow(10.0 , (py1/20.0));
                swv = swv + (1.0 + uricht)/(1.0 - uricht);
                swv = swv / 3.0;
              }
              if((wobbeldaten.ebetriebsart == eswrneu)or(wobbeldaten.ebetriebsart == eswrant)){
                my1[k] = int(round(hoehe - round((swv - 1.0) * (double(hoehe)/yswrteilung)) - 1 + ObererRand));
              }
              if(wobbeldaten.ebetriebsart == eimpedanz){
                swv = (50.0 * swv) - 50.0;
    //            swv = 10.0;
                my1[k] = int(round(hoehe - round((swv - 1.0) * (double(hoehe)/200.0)) - 2 + ObererRand));
              } 
              if(wobbeldaten.ebetriebsart == eswrant){ 
                a = a_100 * laenge / 100.0;
                r_tx = (swv - 1.0) / (swv + 1.0);
                r_ant = r_tx * pow(10.0 , a/10.0);
                if(r_ant >= 1.0)r_ant = 0.99;
                s_ant = (1.0 + r_ant) / (1.0 - r_ant);
                my2[k] = int(round(hoehe - round((s_ant - 1.0) * (double(hoehe)/yswrteilung)) - 1 + ObererRand));
                if(my2[k] > (hoehe + ObererRand))my2[k] = hoehe + ObererRand;
                if(my2[k] < ObererRand)my2[k] = ObererRand;
              }
              if(my1[k] > (hoehe + ObererRand))my1[k] = hoehe + ObererRand;
              if(my1[k] < ObererRand)my1[k] = ObererRand;
            }
          }
        }
        #ifdef LDEBUG
          qDebug("WidgetWobbeln::drawKurve():kx[kursorix] %i mx1[kursorix] %i mouseposx[kursorix] %i", kx[kursorix], mx1[kursorix], mouseposx[kursorix]);
        #endif
      }
      p->drawPolyline(kurve); // Kurve zeichnen
      // Ant Kurve zeichnen in blau
      if(wobbeldaten.ebetriebsart == eswrant){
        p->setPen(penbluesolid);
        p->drawPolyline(kurveb);
      }
      if(!wobbeldaten.bhintergrund){
        //Aus aktiven Kursor die Frequenz ermitteln
        if((mouseposx[kursorix] != 0) && (mouseposy[kursorix] != 0)){ //Cursorindex aktiv
          mousefrequenz = (kxm1[kursorix] * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
        }
        for(k=0;k<5;k++){
          bmouse = true;
          if(k!=0)bmouse = !wobbeldaten.mousesperre;
          if((mouseposx[k] != 0) && (mouseposy[k] != 0) && bmouse ){ //Cursorkreuz aktiv
            qstr = tr("Kursor", "im Infofenster");
            s.sprintf(" %i:",k+1);
            s = qstr + s;
            if(bnomove)emit multieditinsert(s);
            px1 = (kxm1[k] * wobbeldaten.schrittfrequenz ) + wobbeldaten.anfangsfrequenz;
            if(banfang){ //Uebergabe aus dem Mausmenue
              //qDebug("banfang = false;");
              banfang = false;
              //neue Anfangsfrequenz an nwtlinux.cpp uebermittlen
              emit writeanfang(px1);
            }
            if(bende){ //Uebergabe aus dem Mausmenue
              bende = false;
              //neue Endfrequenz an nwtlinux.cpp uebermittlen
              emit writeende(px1);
            }
            if(bberechnung){ //Uebergabe aus dem Mausmenue
              bberechnung = false;
              //neue Endfrequenz an nwtlinux.cpp uebermittlen
              emit writeberechnung(px1);
            }
            s.sprintf("%2.6f MHz", px1 / 1000000.0);
            emit multieditinsert(s);
            if(wobbeldaten.bswrkanal2){
              py1 = kw2[k] * y2faktor + y2shift;
              py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(kx2[k]));
            }else{
              py1 = kw2[k] * y1faktor + y1shift;
              py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(kx2[k]));
            }
            if(py1 > -1.0)py1 = -1.0;
            uricht = pow(10.0 , (py1/20.0));
            swv = (1.0 + uricht)/(1.0 - uricht);
            if(wobbeldaten.bswriteration){
              if(wobbeldaten.bswrkanal2){
                py1 = kw1[k] * y2faktor + y2shift;
                py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(kx1[k]));
              }else{
                py1 = kw1[k] * y1faktor + y1shift;
                py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(kx1[k]));
              }
              if(py1 > -1.0)py1 = -1.0;
              uricht = pow(10.0 , (py1/20.0));
              swv = swv + (1.0 + uricht)/(1.0 - uricht);
              if(wobbeldaten.bswrkanal2){
                py1 = kw3[k] * y2faktor + y2shift;
                py1 = py1 - getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(kx3[k]));
              }else{
                py1 = kw3[k] * y1faktor + y1shift;
                py1 = py1 - getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(kx3[k]));
              }
              if(py1 > -1.0)py1 = -1.0;
              uricht = pow(10.0 , (py1/20.0));
              swv = swv + (1.0 + uricht)/(1.0 - uricht);
              swv = swv / 3.0;
            }
            if((wobbeldaten.ebetriebsart == eswrneu)or(wobbeldaten.ebetriebsart == eswrant)){
              qstr = tr("SWV:", "im Infofenster");
              s.sprintf(" %4.4f", swv);
            }
            if(wobbeldaten.ebetriebsart == eimpedanz){
              if(wobbeldaten.bswrkanal2){
                py1=getswrkalibrierwertk2(afrequenz + wobbeldaten.schrittfrequenz * double(kx1[k]));
              }else{
                py1=getswrkalibrierwertk1(afrequenz + wobbeldaten.schrittfrequenz * double(kx1[k]));
              }
//              s.sprintf("Eich: %4.4f dB", py1);
//              emit multieditinsert(s);
              swv = (50.0 * swv) - 50.0;
              qstr = tr("Z:", "im Infofenster");
              s.sprintf(" %4.4f Ohm", swv);
            }
            s = qstr + s;
            emit multieditinsert(s);
            // SWV Messung ueber Kabel
            if(wobbeldaten.ebetriebsart == eswrant){ 
              a = a_100 * laenge / 100.0;
              r_tx = (swv - 1.0) / (swv + 1.0);
              r_ant = r_tx * pow(10.0 , a/10.0);
              if(r_ant >= 1.0)r_ant = 0.99;
              s_ant = (1.0 + r_ant) / (1.0 - r_ant);
              qstr = tr("SWV_ant:", "im Infofenster");
              s.sprintf(" %4.4f", s_ant);
              s = qstr + s;
              emit multieditinsert(s);
            }
            s = "---------------------";
            emit multieditinsert(s);
          }
        }
        if((wobbeldaten.ebetriebsart == eswrneu) or (wobbeldaten.ebetriebsart == eswrant)){
          // SWV minimum anzeigen
          px1 = (mink1 * wobbeldaten.schrittfrequenz ) + afrequenz;
          px1 = px1 / 1000000.0;
          qstr = tr("SWV min:", "im Infofenster");
          s.sprintf(" %2.2f  %2.6f MHz", minswr, px1);
          s = qstr + s;
          emit multieditinsert(s);
          // SWV maximum anzeigen
          px1 = (maxk1 * wobbeldaten.schrittfrequenz ) + afrequenz;
          px1 = px1 / 1000000.0;
          qstr = tr("SWV max:", "im Infofenster");
          s.sprintf(" %2.2f  %2.6f MHz", maxswr, px1);
          s = qstr + s;
          emit multieditinsert(s);
          s = "---------------------";
          emit multieditinsert(s);
        }
        if(wobbeldaten.ebetriebsart == eimpedanz){
          // Impedanz minimum anzeigen
          px1 = (mink1 * wobbeldaten.schrittfrequenz ) + afrequenz;
          px1 = px1 / 1000000.0;
          swv = (50.0 * minswr) - 50.0;
          qstr = tr("Z min:", "im Infofenster");
          s.sprintf(" %2.2f  %2.6f MHz", swv, px1);
          s = qstr + s;
          emit multieditinsert(s);
          // SWV maximum anzeigen
          px1 = (maxk1 * wobbeldaten.schrittfrequenz ) + afrequenz;
          px1 = px1 / 1000000.0;
          swv = (50.0 * maxswr) - 50.0;
          qstr = tr("Z max:", "im Infofenster");
          s.sprintf(" %2.2f  %2.6f MHz", swv, px1);
          s = qstr + s;
          emit multieditinsert(s);
          s = "---------------------";
          emit multieditinsert(s);
        }
      }
      break;
    }  
    //#####################################################################
    // Switch zuende
    //#####################################################################
    if(!wobbeldaten.bhintergrund){ 
      if((mouseposx[kursorix] != 0) && (mouseposy[kursorix] != 0)){ //Cursorindex aktiv
      //Kursorfrequenz an nwt7linux.cpp uebermitteln
        emit setKursorFrequenz(mousefrequenz);
        //qDebug("KursorFrequenz %f",mousefrequenz);
      }else{
        //0.0 an nwt7linux.cpp uebermitteln
        emit setKursorFrequenz(0.0);
      }
      for(k=0;k<5;k++){
        //Mauskreuz zeigen aber nur wenn pos aktiv und keine Frequenzermittlung
        if((mouseposx[k] != 0) && (mouseposy[k] != 0) && !banfang && !bende){
          double dbz=8.0; //Mauszeigergroesse
          int bz = (int)((double)breite/430.0 * dbz); //Kursorgroesse automatisch vergroessern
          p->setPen(pendarkredsolid);
          // Kursor als Dreieck
          p->drawLine(mx1[k], my1[k], mx1[k]-bz/2, my1[k]-bz);
          p->drawLine(mx1[k], my1[k], mx1[k]+bz/2, my1[k]-bz);
          p->drawLine(mx1[k]+bz/2, my1[k]-bz, mx1[k]-bz/2, my1[k]-bz);
          p->drawLine(mx2[k], my2[k], mx2[k]-bz/2, my2[k]-bz);
          p->drawLine(mx2[k], my2[k], mx2[k]+bz/2, my2[k]-bz);
          p->drawLine(mx2[k]+bz/2, my2[k]-bz, mx2[k]-bz/2, my2[k]-bz);
          s.sprintf("%i", k+1);
          p->drawText(mx1[k]-2, my1[k]-bz-1, s);
          p->drawText(mx2[k]-2, my2[k]-bz-1, s);
        }
      }
    //*************************************************************************************
    //*************************************************************************************
    //*************************************************************************************
    //*************************************************************************************
    //  bitBlt(this, 0,0, pix);
      for(k=0;k<5;k++){
        mouseposx1[k] = mouseposx[k];
      }
    }
    // p->end();
  }
}

QString WidgetWobbeln::kursorsavstr(double pegel)
{
  double uvolt, power;
  QString s1, s2;
  
  if(wobbeldaten.bsavdbm){
    s2.sprintf("%2.2fdBm", pegel);
    s1 = s1 + s2 + "; ";
  }
  if(wobbeldaten.bsavdbuv){
    s2.sprintf("%2.2fdBuV", pegel + 106.9897);
    s1 = s1 + s2 + "; ";
  }
  if(wobbeldaten.bsavuv){
    //aus dBm Volt errechnen
    uvolt = sqrt((pow(10.0 , pegel/10.0) * 0.001) * 50.0);
    //Messbereche automatisch umschalten
    if (uvolt > 0.999){
      s2.sprintf("%3.1f V", uvolt);
    }
    if (uvolt < 1.0){
      uvolt = uvolt * 1000.0;
      s2.sprintf("%3.1f mV", uvolt);
    }
    if (uvolt < 1.0){
      uvolt = uvolt * 1000.0;
      s2.sprintf("%3.1f uV", uvolt);
    }
    s1 = s1 + s2 + "; ";
  }
  if(wobbeldaten.bsavwatt){
    uvolt = sqrt((pow(10.0 , pegel/10.0) * 0.001) * 50.0);
    power = uvolt * uvolt / 50.0;
    if (power > 0.999){
      s2.sprintf("%3.1f W", power);
      if (power > 999.999){
        s2.sprintf("%3.2f kW", power/1000.0);
      }
    }
    if (power < 1.0){
      power = power * 1000.0;
      s2.sprintf("%3.1f mW", power);
    }
    if (power < 1.0){
      power = power * 1000.0;
      s2.sprintf("%3.1f uW", power);
    }
    if (power < 1.0){
      power = power * 1000.0;
      s2.sprintf("%3.1f nW", power);
    }
    if (power < 1.0){
      power = power * 1000.0;
      s2.sprintf("%3.1f pW", power);
    }
    s1 = s1 + s2 + "; ";
  }
  s2 = s1;
  return s2;
}

double WidgetWobbeln::korrsavpegel(double p)
{
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::korrsavpegel(double p)");
  #endif
  if(wobbeldaten.ebbsav == e300b1)p = p + wobbeldaten.psav300;
  if(wobbeldaten.ebbsav == e7kb1)p = p + wobbeldaten.psav7k;
  if(wobbeldaten.ebbsav == e30kb1)p = p + wobbeldaten.psav30k;
  if(wobbeldaten.ebbsav == e300b2)p = p + wobbeldaten.psav300;
  if(wobbeldaten.ebbsav == e7kb2)p = p + wobbeldaten.psav7k;
  if(wobbeldaten.ebbsav == e30kb2)p = p + wobbeldaten.psav30k;
  p = p + wobbeldaten.psavabs;
  return p;
}

void WidgetWobbeln::setWobbeldaten(TWobbel wob)
{
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::setWobbeldaten()");
  #endif
  //Uebernahme der Wobbeldaten
  wobbeldaten = wob;
  //Befehl zum Neuzeichnen ausloesen
  repaint();
}

void WidgetWobbeln::wobbeldatenempfang(const TWobbel& wob)
{
  //qDebug("WidgetWobbeln::wobbeldatenempfang()");
  //Uebernahme der Wobbeldaten
  wobbeldaten = wob;
  kurve1.absanfang = wobbeldaten.absanfang;
  kurve1.absende = wobbeldaten.absende;
  kurve2.absanfang = wobbeldaten.absanfang;
  kurve2.absende = wobbeldaten.absende;
  kurve3.absanfang = wobbeldaten.absanfang;
  kurve3.absende = wobbeldaten.absende;
  kurve4.absanfang = wobbeldaten.absanfang;
  kurve4.absende = wobbeldaten.absende;
  refreshPixmap();
  //Befehl zum Neuzeichnen ausloesen
  //repaint();
}

void WidgetWobbeln::wobbeldatenkurve(const TMess &amess)
{
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::wobbeldatenkurve()");
  #endif
  wobbeldaten.mess = amess;
  refreshKurve();
  repaint();
}

void WidgetWobbeln::empfangfrqmarken(const TFrqmarken &afmarken)
{
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::empfangfrqmarken()");
  #endif
  //Frequenzmarken uebergeben
  wfmarken = afmarken;
  refreshPixmap();
  repaint();
}

TWobbel WidgetWobbeln::getWobbeldaten(){
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::getWobbeldaten()");
  #endif
  return wobbeldaten;
}

void WidgetWobbeln::printdiagramm(const QFont &pfont, const QStringList &astrlist)
{
  QPrinter *print = new QPrinter;
  int xp = 40;
  int yp = 0;
  int ypmerk;
  int zeilenabstand = 20;
  unsigned int i;
  double mx, my, mxx;
  QString qs, qs1, qa, qe, qw, qm, qz;
  QFont afont;
  
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::printdiagramm()");
  #endif
  QDateTime dt = QDateTime::currentDateTime();
  qs1 = dt.toString("dd MMMM yyyy, hh:mm");
  afont = pfont;
  refreshPixmap();
  repaint();
  QPrintDialog dialog(print, this);
  if(dialog.exec()) {
    refreshPixmap();
    repaint();
    QPainter p(print);
    const QPaintDevice& metrics(*p.device());
    //Druckplattbreite ermitteln
    mx = metrics.width();
    mxx = mx;
    my = metrics.height();
    //linken Druckrand abziehen
    mx = mx - xp;
    //breite des Anzeigefensters der Grafik
    double px = this->rect().width();
    
    mx = mx/px * 0.8;
    p.setFont(pfont);
    qs = "NWT 4 Linux & Windows  " + qs1;
    yp += zeilenabstand;
    yp += zeilenabstand;
    p.drawText(xp, yp, qs);
    yp += zeilenabstand;
    p.drawLine(xp, yp, metrics.width() - xp, yp);
    yp += zeilenabstand;
    
    qa.sprintf("%3.6f", double(wobbeldaten.anfangsfrequenz)/1000000.0);
    qe.sprintf("%3.6f", double(((wobbeldaten.schritte - 1) * wobbeldaten.schrittfrequenz)+
                                       wobbeldaten.anfangsfrequenz)/1000000.0);
    qw.sprintf("%4.3f", double(wobbeldaten.schrittfrequenz)/1000.0);
    qm.sprintf("%4i", wobbeldaten.schritte);
    qz.sprintf("%4i", wobbeldaten.ztime * 10);
    
    qs = tr("Startfrequenz: ","Grafik Druckerausgabe") + 
         qa +
         tr(" MHz","Grafik Druckerausgabe") +
         "; " +
         tr("Endfrequenz: ","Grafik Druckerausgabe") +
         qe +
         tr(" MHz","Grafik Druckerausgabe") +
         "; " +
         tr("Schrittweite: ","Grafik Druckerausgabe") +
         qw +
         tr(" kHz","Grafik Druckerausgabe");
    
    p.drawText(xp, yp, qs);
    yp += zeilenabstand;
    
    qs = tr("Messpunkte: ","Grafik Druckerausgabe") + 
         qm +
         "; " +
         tr("Zwischenzeit: ","Grafik Druckerausgabe") +
         qz +
         tr(" uSek","Grafik Druckerausgabe");
    
    p.drawText(xp, yp, qs);
    yp += zeilenabstand;
    //Linie ueber der Grafik
    p.drawLine(xp, yp, metrics.width() - xp, yp);
    yp += zeilenabstand;
    //Auf richtige Groesse bringen
//    p.scale(mx, mx * 0.9);
    p.drawPixmap(int(xp), int(yp), pix);
    yp += int(this->rect().height() * mx);
    //Groesse wieder restaurieren
//    p.scale(1/mx, 1/(mx * 0.9));
    //Linie unter dem Bild
    p.drawLine(xp, yp, metrics.width() - xp, yp);
    ypmerk = yp;
    if(astrlist.count() > 0){
      yp += zeilenabstand;
//      qDebug("yp=%i ",yp);
      qs = astrlist.first();
      p.drawText(xp, yp, qs);
      for(i=1; i < uint(astrlist.count()); i++){
        yp += zeilenabstand;
//        qDebug("yp=%i xp=%i", yp, xp);
        if((yp + zeilenabstand) > (int)my){
          yp = ypmerk;
          xp = xp + int(mxx / 3.0);
          yp += zeilenabstand;
        }
        qs = astrlist.at(i);
        p.drawText(xp,yp, qs);
      }
      yp += zeilenabstand;
      //Abschlusslinie
  //    p.drawLine(xp, yp, print->width()-xp, yp);
    }
  }  
}


void WidgetWobbeln::printdiagrammpdf(const QFont &pfont, const QStringList &astrlist)
{
//  QPrinter *print = new QPrinter(QPrinter::PrinterResolution);
  QPrinter *print = new QPrinter();
  int xp = 40;
  int yp = 0;
  int zeilenabstand = 20;
  int ypmerk;
  unsigned int i;
  QString qs, qs1, qa, qe, qw, qm, qz;
  QFont afont;
  double mx;
  int my;
  
  #ifdef LDEBUG
  qDebug("WidgetWobbeln::printdiagrammpdf()");
  #endif
  print->setOutputFormat(QPrinter::PdfFormat);
  print->setPageSize(QPrinter::A4);
  print->setOrientation(QPrinter::Portrait);
  QString s = QFileDialog::getSaveFileName(this, tr("PDF Speichern","Grafik Filedialog"),
                                                    "out.pdf","PDF (*.pdf *.PDF)");
  //Datei ueberpruefen ob Sufix vorhanden
  if((s.indexOf(".")== -1)) s += ".pdf";

  print->setOutputFileName(s);
  QDateTime dt = QDateTime::currentDateTime();
  qs1 = dt.toString("dd MMMM yyyy, hh:mm");
  afont = pfont;
  refreshPixmap();
  repaint();
  QPainter p(print);
//  QPaintDeviceMetrics metrics(p);
  //Druckblattbreite ermitteln
  mx = print->width();
  //Druckblatthoehe ermitteln
  my = print->height();
//  qDebug("breite=%f hoehe=%i", mx, my);
//  qDebug("breite=%i hoehe=%i",print->widthMM(), print->heightMM());
  //linken Druckrand abziehen
  mx = mx - xp;
  //breite des Anzeigefensters der Grafik
  double px = this->rect().width();
  
  mx = mx/px * 0.8;
  p.setFont(pfont);
  qs = "NWT 4 Linux & Windows  " + qs1;
  yp += zeilenabstand;
  yp += zeilenabstand;
  p.drawText(xp, yp, qs);
  yp += zeilenabstand;
  p.drawLine(xp, yp, print->width()-xp, yp);
  yp += zeilenabstand;
  
  qa.sprintf("%3.6f", double(wobbeldaten.anfangsfrequenz)/1000000.0);
  qe.sprintf("%3.6f", double(((wobbeldaten.schritte - 1) * wobbeldaten.schrittfrequenz)+
                                      wobbeldaten.anfangsfrequenz)/1000000.0);
  qw.sprintf("%4.3f", double(wobbeldaten.schrittfrequenz)/1000.0);
  qm.sprintf("%4i", wobbeldaten.schritte);
  qz.sprintf("%4i", wobbeldaten.ztime * 10);
  
  qs = tr("Startfrequenz: ","Grafik PDF Druckerausgabe") + qa +
       tr(" MHz","Grafik PDF Druckerausgabe") + "; " +
       tr("Endfrequenz: ","Grafik PDF Druckerausgabe") + qe +
       tr(" MHz","Grafik PDF Druckerausgabe") + "; " +
       tr("Schrittweite: ","Grafik PDF Druckerausgabe") + qw +
       tr(" kHz","Grafik PDF Druckerausgabe");
  
  p.drawText(xp, yp, qs);
  yp += zeilenabstand;
  
  qs = tr("Messpunkte: ","Grafik PDF Druckerausgabe") + qm + "; " +
        tr("Zwischenzeit: ","Grafik PDF Druckerausgabe") + qz +
        tr(" uSek","Grafik PDF Druckerausgabe");
  
  p.drawText(xp, yp, qs);
  yp += zeilenabstand;
  //Linie ueber der Grafik
  p.drawLine(xp, yp, print->width()-xp, yp);
  yp += zeilenabstand;
  //Auf richtige Groesse bringen
//  p.scale(mx, mx * 0.9);
  p.drawPixmap(int(xp), int(yp), pix);
//  yp += int(this->rect().height() * mx)-50;
  yp += int(this->rect().height());
  //Groesse wieder restaurieren
//  p.scale(1/mx, 1/(mx * 0.9));
  //Linie unter dem Bild
  p.drawLine(xp, yp, print->width()-xp, yp);
  yp += zeilenabstand;
  ypmerk = yp;
  if(astrlist.count() > 0){
    yp += zeilenabstand;
//    qDebug("yp=%i ",yp);
    qs = astrlist.first();
    p.drawText(xp, yp, qs);
    for(i=1; i < uint(astrlist.count()); i++){
      yp += zeilenabstand;
//      qDebug("yp=%i xp=%i", yp, xp);
      if((yp + zeilenabstand) > (int)my){
        yp = ypmerk;
        xp = xp + (print->width() / 3);
        yp += zeilenabstand;
      }
      qs = astrlist.at(i);
      p.drawText(xp,yp, qs);
    }
    yp += zeilenabstand;
    //Abschlusslinie
//    p.drawLine(xp, yp, print->width()-xp, yp);
  }
}

void WidgetWobbeln::neubildmitinfo(const TBildinfo &abildinfo)
{
  int xp = 40;
  int yp = 60;
  int zeilenabstand = 20;
  QString qs;
  
  refreshPixmap();
  
  wbildinfo = abildinfo;
  xp = wbildinfo.posx;
  yp = wbildinfo.posy;
  zeilenabstand = int(1.2 * double(wbildinfo.fontsize));
  
  if(wbildinfo.infoanzeigen){
    QPainter p;
    //Beschriftetes Pixmap holen
    p.begin(&pix);
    QFont ifont;
    ifont.setPixelSize (wbildinfo.fontsize);
    p.setFont(ifont);
    //und Kurve einzeichnen
    for(uint i=0; i < uint(infolist.count()); i++){
      qs = infolist.at(i);
      //qDebug(qs);
      p.drawText(xp,yp, qs);
      yp += zeilenabstand;
    }
    p.end();
    //und anzeigen
    update(); //paintEvent erzeugen
  }
}

void WidgetWobbeln::bildspeichern(const TBildinfo &bildinfo, const QStringList &astrlist)
{
  //unsigned int i;
  QString qs, qs1;
  QFont afont;
  QString s;


  wbildinfo = bildinfo;
  infolist = astrlist;
  
  winsinfo *insinfo = new winsinfo(this);
  insinfo->setBildinfo(bildinfo);
  QObject::connect( insinfo, SIGNAL(pixauffrischen(const TBildinfo &)), this, SLOT(neubildmitinfo(const TBildinfo &)));

  bool ok = insinfo->exec();

  if(ok){
    QSettings settings("AFU", "NWT");
    QDir imgpath;
    imgpath.setPath(settings.value("imgpath", QDir::homePath()).toString());
    s = QFileDialog::getSaveFileName(this, tr("Bild Speichern","Grafik Filedialog"),
        imgpath.path(),"PNG (*.png *.PNG);;BMP (*.bmp *.BMP);;JPG (*.jpg *.JPG)", &qs1);

    if(!s.isEmpty()){
      qDebug("save");
      //Datei ueberpruefen ob Sufix vorhanden
      if(!s.contains(".")){
        if(qs1.contains("PNG"))s += ".png";
        if(qs1.contains("BMP"))s += ".bmp";
        if(qs1.contains("JPG"))s += ".jpg";
      }
      QImage img(size(), QImage::Format_RGB32);
      render(&img);
      QImageWriter iw(s);
      iw.write(img);
      imgpath.setPath(s);
      settings.setValue("imgpath",imgpath.absolutePath());
    }
  }
  refreshPixmap();
  //qDebug(imgpath.absolutePath());
}

void WidgetWobbeln::closeEvent(QCloseEvent *event)
{
  event->ignore();
}

void WidgetWobbeln::keyPressEvent(QKeyEvent *event){
  int k;
  
  switch(event->key()){
    case Qt::Key_Up: if(wobbeldaten.mess.daten_enable){ mouseposx[kursorix] = (this->rect().width()) / 2;} emit lupeplus(); break;
    case Qt::Key_Down: if(wobbeldaten.mess.daten_enable){ mouseposx[kursorix] = (this->rect().width()) / 2;} emit lupeminus(); break;
    case Qt::Key_Left: 
      if(wobbeldaten.mess.daten_enable){ 
        mouseposx[kursorix] = mouseposx[kursorix] - 5;
        if(!wobbeldaten.mousesperre){
          refreshPixmap();
          repaint();
        }
      }
    break;
    case Qt::Key_Right:
      if(wobbeldaten.mess.daten_enable){
        mouseposx[kursorix] = mouseposx[kursorix] + 5;
        if(!wobbeldaten.mousesperre){
          refreshPixmap();
          repaint();
        }
      }
    break;
    case Qt::Key_Space: if(wobbeldaten.mess.daten_enable){ mouseposx[kursorix] = (this->rect().width()) / 2;} emit lupegleich(); break;
    case Qt::Key_Return:
      if(wobbeldaten.mess.daten_enable){
        bnomove = false;
        if(wobbeldaten.mess.daten_enable){
          mouseposx[kursorix] = (this->rect().width()) / 2;
          mouseposy[kursorix] = (this->rect().height()) / 2;
          bnomove = true;
          if(!wobbeldaten.mousesperre){
            refreshPixmap();
            repaint();
          }
        }
      }
    break;
    case Qt::Key_1: setWKursorNr(4); emit resetKursor(4); break;
    case Qt::Key_2: setWKursorNr(3); emit resetKursor(3); break;
    case Qt::Key_3: setWKursorNr(2); emit resetKursor(2); break;
    case Qt::Key_4: setWKursorNr(1); emit resetKursor(1); break;
    case Qt::Key_5: setWKursorNr(0); emit resetKursor(0); break;
    case Qt::Key_0:
      for(k=0;k<5;k++){
        kursorix = k;
        resetmousecursor();
      }  
//      menmouse->hide();
      setWKursorNr(4);
      emit resetKursor(4);
    break;
    case Qt::Key_W: emit wobbelndauer(); break;
    case Qt::Key_E: emit wobbelneinmal(); break;
    case Qt::Key_S: emit wobbelnstop(); break;
    case Qt::Key_F: CreateQuarzDatei(); break;
    case Qt::Key_D: WriteToQuarzDatei(squdaten); break;
    case Qt::Key_C: CloseQuarzDatei(); break;
    default:QWidget::keyPressEvent(event);
  }
}

void WidgetWobbeln::CreateQuarzDatei()
{
  qusl.clear();
  qunr = 1;
  QString qs = "Nr;Frequ;Loss(dB);B3dB(Hz)";
  qusl.append(qs);
  QString s = QFileDialog::getSaveFileName(this, "Crystal-File",
                    "crystallist.csv","CSV (*.csv *.CSV)");
  //Datei ueberpruefen ob Sufix vorhanden
  if(!s.isNull()){
    if((s.indexOf(".")== -1)) s += ".csv";
    fname = s;
    speichern = true;
  }
}

void WidgetWobbeln::WriteToQuarzDatei(const QString &qs)
{
  QString s, as;
  
  if(speichern){
    s.sprintf("%i;",qunr);
    as = s + qs;
    qunr++;
    qusl.append(as);
    //qDebug(as);
  }
}

void WidgetWobbeln::CloseQuarzDatei()
{
  QString qs;
  QStringList::Iterator it;
  QFile *f1;
  
  if(speichern){
    f1 = new QFile(fname);
    if(f1->open(QIODevice::WriteOnly)){
      QTextStream ts(f1);
      for(it = qusl.begin() ; it != qusl.end(); it++){
        qs = (*it).toLatin1();
        ts << qs << endl;
      }
      f1->close();
      delete(f1);
    }
  }
}





