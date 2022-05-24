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
// * Edit dc5pi, 2016-03-24 /dev/ttypACM[0:3]
//
#include <qdialog.h>
#include <QColorDialog>
#include <QColor>
#include <QPainter>
#include <QFont>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>
#include <math.h>

#include "optiondlg.h"
#include "widgetwobbeln.h"
#ifdef Q_WS_WIN
#include "win_modem.h"
#else
#include "lin_modem.h"
#endif


//#####################################################################################
// Class fuer Option
//#####################################################################################
odlg::odlg(QWidget* parent, Qt::WindowFlags flags): QDialog(parent, flags)
{
  
  this->resize(560,520);
  this->setMinimumSize(0,0);
//  QFont font("Helvetica", pfontsize);

  ok = new QPushButton(tr("OK","Option Dialog"), this);
  ok->setGeometry(10,10,100,30);
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  cancel = new QPushButton(tr("Abbruch","Option Dialog"), this);
  cancel->setGeometry(10,50,100,30);
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
  
  tabwidget = new QTabWidget(this);
  tabwidget->setGeometry(120,0,440,510);
  wwobbel = new QWidget();
  tabwidget->addTab(wwobbel, tr("Grunddaten/Wobbeln","Option Dialog"));
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  kalibrierfrequ = new QGroupBox(wwobbel);
  kalibrierfrequ->setGeometry(10,10,270,90);
  kalibrierfrequ->setTitle(tr("Kalibrierfrequenzen (nur bei math. Korrektur)","Option Dialog"));
  labelanfang = new QLabel(kalibrierfrequ);
  labelanfang->setGeometry(10,25,120,30);
  labelanfang->setText(tr("Startfrequenz (Hz)","Option Dialog"));
  labelanfang->setAlignment(Qt::AlignRight);
  labelende = new QLabel(kalibrierfrequ);
  labelende->setGeometry(10,55,120,30);
  labelende->setText(tr("Endfrequenz (Hz)","Option Dialog"));
  labelende->setAlignment(Qt::AlignRight);
  eanfang = new QLineEdit(kalibrierfrequ);
  eanfang->setGeometry(140,20,110,25);
  eende = new QLineEdit(kalibrierfrequ);
  eende->setGeometry(140,50,110,25);
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ddstakt = new QGroupBox(wwobbel);
  ddstakt->setGeometry(10,105,270,85);
  ddstakt->setTitle(tr("DDS Taktfrequenz","Option Dialog"));
  labelddstakt = new QLabel(ddstakt);
  labelddstakt->setGeometry(10,25,100,30);
  labelddstakt->setText(tr("DDS Takt (Hz) ","Option Dialog"));
  labelddstakt->setAlignment(Qt::AlignRight);
  eddstakt = new QLineEdit(ddstakt);
  eddstakt->setGeometry(130,20,120,25);
  labelcpll = new QLabel(ddstakt);
  labelcpll->setGeometry(10,55,100,30);
  labelcpll->setText(tr("DDS Mode "));
  labelcpll->setAlignment(Qt::AlignRight);
  cpll = new QComboBox(ddstakt);
  cpll->setGeometry(130,50,120,25);
  cpll->addItem("");
  cpll->addItem(tr("ohne PLL"));
  cpll->addItem("PLL 2x");
  cpll->addItem("PLL 3x");
  cpll->addItem("PLL 4x");
  cpll->addItem("PLL 5x");
  cpll->addItem("PLL 6x");
  cpll->addItem("PLL 7x");
  cpll->addItem("PLL 8x");
  cpll->addItem("PLL 9x");
  cpll->addItem("PLL 10x");
  cpll->addItem("PLL 11x");
  cpll->addItem("PLL 12x");
  cpll->addItem("PLL 13x");
  cpll->addItem("PLL 14x");
  cpll->addItem("PLL 15x");
  cpll->addItem("PLL 16x");
  cpll->addItem("PLL 17x");
  cpll->addItem("PLL 18x");
  cpll->addItem("PLL 19x");
  cpll->addItem("PLL 20x");
  labelcpll->setEnabled(false);
  cpll->setEnabled(false);
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  gtty = new QGroupBox(wwobbel);
  gtty->setGeometry(10,195,270,60);
  gtty->setTitle(tr("Serielle Schnittstelle","Option Dialog"));
  labelctty = new QLabel(gtty);
  labelctty->setGeometry(10,20,120,30);
  labelctty->setText(tr("Schnittstelle:","Option Dialog"));
  ctty = new QComboBox(gtty);
  ctty->setGeometry(130,20,120,25);
  ctty->addItem("");
#ifdef Q_WS_WIN
  int i;
  QString qs;
  for(i=1;i<=256;i++){
    qs.sprintf("COM%i",i);
    ctty->addItem(qs);
  }
#else
  ctty->setEditable(true);
  ctty->addItem(schnittstelle1);
  ctty->addItem(schnittstelle2);
  ctty->addItem(schnittstelle3);
  ctty->addItem(schnittstelle4);
  ctty->addItem(schnittstelle5);
  ctty->addItem(schnittstelle6);
  ctty->addItem(schnittstelle7);
  ctty->addItem(schnittstelle8);
  ctty->addItem(schnittstelle9);
  ctty->addItem(schnittstelle10);
  ctty->addItem(schnittstelle11);
  ctty->addItem(schnittstelle12);
#endif
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  fname = new QGroupBox(wwobbel);
  fname->setGeometry(10,260,270,115);
  fname->setTitle(tr("Default Dateinamen","Option Dialog"));
  labelsonde1 = new QLabel(fname);
  labelsonde1->setGeometry(10,25,120,30);
  labelsonde1->setText(tr("Log-Sonde Kanal 1","Option Dialog"));
  labelsonde1->setAlignment(Qt::AlignRight);
  esonde1 = new QLineEdit(fname);
  esonde1->setGeometry(135,20,120,25);
  labelesonde1lin = new QLabel(fname);
  labelesonde1lin->setGeometry(10,55,120,30);
  labelesonde1lin->setText(tr("Lin-Sonde Kanal 1","Option Dialog"));
  labelesonde1lin->setAlignment(Qt::AlignRight);
  esonde1lin = new QLineEdit(fname);
  esonde1lin->setGeometry(135,50,120,25);
  labelsonde2 = new QLabel(fname);
  labelsonde2->setGeometry(10,85,120,30);
  labelsonde2->setText(tr("Log-Sonde Kanal 2","Option Dialog"));
  labelsonde2->setAlignment(Qt::AlignRight);
  esonde2 = new QLineEdit(fname);
  esonde2->setGeometry(135,80,120,25);
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  fgrenzen = new QGroupBox(wwobbel);
  fgrenzen->setGeometry(10,380,270,85);
  fgrenzen->setTitle(tr("Frequenzgrenzen","Option Dialog"));
  labelwobmax = new QLabel(fgrenzen);
  labelwobmax->setGeometry(10,25,120,30);
  labelwobmax->setText(tr("max. Wobbel (Hz)","Option Dialog"));
  labelwobmax->setAlignment(Qt::AlignRight);
  wobmax = new QLineEdit(fgrenzen);
  wobmax->setGeometry(135,20,120,25);
  cbfrqfaktor = new QComboBox(fgrenzen);
  cbfrqfaktor->addItem("1");
  cbfrqfaktor->addItem("2");
  cbfrqfaktor->addItem("3");
  cbfrqfaktor->addItem("4");
  cbfrqfaktor->addItem("5");
  cbfrqfaktor->addItem("6");
  cbfrqfaktor->addItem("7");
  cbfrqfaktor->addItem("8");
  cbfrqfaktor->addItem("9");
  cbfrqfaktor->addItem("10");
  cbfrqfaktor->addItem("16");
  cbfrqfaktor->addItem("32");
  cbfrqfaktor->addItem("64");
  labelfrqfaktor = new QLabel(tr("Frequenzvervielfacher","Label"),fgrenzen);
  cbfrqfaktor->setGeometry(215, 50, 40, 20);
  labelfrqfaktor->setGeometry(35, 50, 160, 20);
  labelfrqfaktor->setAlignment(Qt::AlignRight);
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  gkalibrier = new QGroupBox(wwobbel);
  gkalibrier->setGeometry(290,10,140,50);
  gkalibrier->setTitle(tr("Daempfungs-Glied","Option Dialog"));
  daempfungfa = new QCheckBox(gkalibrier);
  daempfungfa->setGeometry(10,20,90,25);
  daempfungfa->setText(tr("FA-Daempf ","Option Dialog"));
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  geinkanalig = new QGroupBox(wwobbel);
  geinkanalig->setGeometry(290,70,140,60);
  geinkanalig->setTitle(tr("Kanaele","Option Dialog"));
  boxeinkanalig = new QCheckBox(geinkanalig);
  boxeinkanalig->setGeometry(10,20,90,30);
  boxeinkanalig->setText(tr("Einkanalig","Option Dialog"));
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  gswriteration = new QGroupBox(wwobbel);
  gswriteration->setGeometry(290,135,140,60);
  gswriteration->setTitle(tr("SWV Iteration","Option Dialog"));
  boxiteration = new QCheckBox(gswriteration);
  boxiteration->setGeometry(10,20,90,30);
  boxiteration->setText(tr("math. Korr.","Option Dialog"));
//#############################################################################
//#############################################################################
//Grundaten fuer den Spekrumanalyser
//#############################################################################
//#############################################################################
  
  wspectrum = new QWidget();
  tabwidget->addTab(wspectrum, tr("SAV (1)","Option Dialog"));
  
  gallgemein = new QGroupBox(wspectrum);
  gallgemein->setGeometry(10,10,220,80);
  gallgemein->setTitle(tr("Allgemein","Option Dialog"));
  boxsastatus = new QCheckBox(gallgemein);
  boxsastatus->setGeometry(10,20,200,30);
  boxsastatus->setText(tr("HW-Rueckmeldung vom SA","Option Dialog"));
  QObject::connect( boxsastatus, SIGNAL( toggled (bool)), this, SLOT(checkboxsastatus_checked(bool)));
  
  boxschrittkorr = new QCheckBox(gallgemein);
  boxschrittkorr->setGeometry(10,50,200,30);
  boxschrittkorr->setText(tr("Automatische Schrittkorrektur","Option Dialog"));
  QObject::connect( boxschrittkorr, SIGNAL( toggled (bool)), this, SLOT(checkboxschrittkorr_checked(bool)));
  
  gsakw = new QGroupBox(wspectrum);
  gsakw->setGeometry(10,100,220,150);
  gsakw->setTitle(tr("Frequenzbereich 1/3","Option Dialog"));
  
  labelfrqa1 = new QLabel(gsakw);
  labelfrqa1->setGeometry(10,22,110,30);
  labelfrqa1->setText(tr("untere Freq.(Hz)","Option Dialog"));
  labelfrqa1->setAlignment(Qt::AlignRight);
  efrqa1 = new QLineEdit(gsakw);
  efrqa1->setGeometry(130,20,80,25);
  labelfrqb1 = new QLabel(gsakw);
  labelfrqb1->setGeometry(10,52,110,30);
  labelfrqb1->setText(tr("obere Freq.(Hz)","Option Dialog"));
  labelfrqb1->setAlignment(Qt::AlignRight);
  efrqb1 = new QLineEdit(gsakw);
  efrqb1->setGeometry(130,50,80,25);
  labelfrqzf1 = new QLabel(gsakw);
  labelfrqzf1->setGeometry(10,82,110,30);
  labelfrqzf1->setText(tr(" ZF + (Hz)","Option Dialog"));
  labelfrqzf1->setAlignment(Qt::AlignRight);
  efrqzf1 = new QLineEdit(gsakw);
  efrqzf1->setGeometry(130,80,80,25);
  labelfrqshift = new QLabel(gsakw);
  labelfrqshift->setGeometry(10,112,110,30);
  labelfrqshift->setText(tr("Shift - (Hz)","Option Dialog"));
  labelfrqshift->setAlignment(Qt::AlignRight);
  efrqshift = new QLineEdit(gsakw);
  efrqshift->setGeometry(130,110,80,25);
  
  gsaukw = new QGroupBox(wspectrum);
  gsaukw->setGeometry(10,260,220,120);
  gsaukw->setTitle(tr("Frequenzbereich 2","Option Dialog"));
  
  labelfrqa2 = new QLabel(gsaukw);
  labelfrqa2->setGeometry(10,22,110,30);
  labelfrqa2->setText(tr("untere Freq.(Hz)","Option Dialog"));
  labelfrqa2->setAlignment(Qt::AlignRight);
  efrqa2 = new QLineEdit(gsaukw);
  efrqa2->setGeometry(130,20,80,25);
  labelfrqb2 = new QLabel(gsaukw);
  labelfrqb2->setGeometry(10,52,110,30);
  labelfrqb2->setText(tr("obere Freq.(Hz)","Option Dialog"));
  labelfrqb2->setAlignment(Qt::AlignRight);
  efrqb2 = new QLineEdit(gsaukw);
  efrqb2->setGeometry(130,50,80,25);
  labelfrqzf2 = new QLabel(gsaukw);
  labelfrqzf2->setGeometry(10,82,110,30);
  labelfrqzf2->setText(tr(" ZF - (Hz)","Option Dialog"));
  labelfrqzf2->setAlignment(Qt::AlignRight);
  efrqzf2 = new QLineEdit(gsaukw);
  efrqzf2->setGeometry(130,80,80,25);
  
  gpegelkorr = new QGroupBox(wspectrum);
  gpegelkorr->setGeometry(240,10,180,210);
  gpegelkorr->setTitle(tr("Pegelkorr. fuer dBm-Anzeige","Option Dialog"));
  labelabsolut1 = new QLabel(gpegelkorr);
  labelabsolut1->setGeometry(10,22,80,30);
  labelabsolut1->setText(tr("Bereich1 (dB)","Option Dialog"));
  labelabsolut1->setAlignment(Qt::AlignRight);
  eabsolut1 = new QLineEdit(gpegelkorr);
  eabsolut1->setGeometry(100,20,70,25);
  labelabsolut2 = new QLabel(gpegelkorr);
  labelabsolut2->setGeometry(10,52,80,30);
  labelabsolut2->setText(tr("Bereich2 (dB)","Option Dialog"));
  labelabsolut2->setAlignment(Qt::AlignRight);
  eabsolut2 = new QLineEdit(gpegelkorr);
  eabsolut2->setGeometry(100,50,70,25);
  labelabsolut3 = new QLabel(gpegelkorr);
  labelabsolut3->setGeometry(10,82,80,30);
  labelabsolut3->setText(tr("Bereich3 (dB)","Option Dialog"));
  labelabsolut3->setAlignment(Qt::AlignRight);
  eabsolut3 = new QLineEdit(gpegelkorr);
  eabsolut3->setGeometry(100,80,70,25);
  labelb300 = new QLabel(gpegelkorr);
  labelb300->setGeometry(10,112,80,30);
  labelb300->setText(tr("B300Hz (dB)","Option Dialog"));
  labelb300->setAlignment(Qt::AlignRight);
  eb300 = new QLineEdit(gpegelkorr);
  eb300->setGeometry(100,110,70,25);
  labelb7k = new QLabel(gpegelkorr);
  labelb7k->setGeometry(10,142,80,30);
  labelb7k->setText(tr("B7kHz (dB)","Option Dialog"));
  labelb7k->setAlignment(Qt::AlignRight);
  eb7k = new QLineEdit(gpegelkorr);
  eb7k->setGeometry(100,140,70,25);
  labelb30k = new QLabel(gpegelkorr);
  labelb30k->setGeometry(10,172,80,30);
  labelb30k->setText(tr("B30kHz (dB)","Option Dialog"));
  labelb30k->setAlignment(Qt::AlignRight);
  eb30k = new QLineEdit(gpegelkorr);
  eb30k->setGeometry(100,170,70,25);

  groupsavinfo = new QGroupBox(tr("Infofenster-Kursoranzeige","Option Dialog"), wspectrum);
  groupsavinfo->setGeometry(240,230,180,90);
  //groupsavinfo->setAlignment(Qt::AlignCenter);
  savdbm = new QCheckBox(groupsavinfo);
  savdbm->setGeometry(10,20,80,30);
  savdbm->setText(tr("dBm","Option Dialog"));
  savdbuv = new QCheckBox(groupsavinfo);
  savdbuv->setGeometry(90,20,80,30);
  savdbuv->setText(tr("dBuV","Option Dialog"));
  savuv = new QCheckBox(groupsavinfo);
  savuv->setGeometry(10,50,80,30);
  savuv->setText(tr("Volt","Option Dialog"));
  savwatt = new QCheckBox(groupsavinfo);
  savwatt->setGeometry(90,50,80,30);
  savwatt->setText(tr("Watt","Option Dialog"));

  groupsavfehler = new QGroupBox(tr("Messgenauigkeit / Grenzen","Option Dialog"), wspectrum);
  groupsavfehler->setGeometry(240,330,180,110);
  labelsafehlermax = new QLabel(groupsavfehler);
  labelsafehlermax->setGeometry(10,22,100,30);
  labelsafehlermax->setText(tr("genau max(dBm)","Option Dialog"));
  labelsafehlermax->setAlignment(Qt::AlignRight);
  esafehlermax = new QLineEdit(groupsavfehler);
  esafehlermax->setGeometry(120,20,60,25);
  labelsafehlermin = new QLabel(groupsavfehler);
  labelsafehlermin->setGeometry(10,52,100,30);
  labelsafehlermin->setText(tr("genau min(dBm)","Option Dialog"));
  labelsafehlermin->setAlignment(Qt::AlignRight);
  esafehlermin = new QLineEdit(groupsavfehler);
  esafehlermin->setGeometry(120,50,60,25);
  labelsaminschritte = new QLabel(groupsavfehler);
  labelsaminschritte->setGeometry(10,82,100,30);
  labelsaminschritte->setText(tr("min Schrittanzahl","Option Dialog"));
  labelsaminschritte->setAlignment(Qt::AlignRight);
  esaminschritte = new QLineEdit(groupsavfehler);
  esaminschritte->setGeometry(120,80,60,25);

  buttonsacal = new QPushButton(tr("Kalibrierung","Option Dialog"), wspectrum);
  buttonsacal->setGeometry(240,450,100,30);
  connect(buttonsacal, SIGNAL(clicked()), SLOT(sacal()));
  
  buttonsacalreset = new QPushButton(tr("Kal= 0.0","Option Dialog"), wspectrum);
  buttonsacalreset->setGeometry(120,450,100,30);
  connect(buttonsacalreset, SIGNAL(clicked()), SLOT(sacalreset()));
  
  sastandart = new QPushButton(tr("Standardwerte","Option Dialog"), wspectrum);
  sastandart->setGeometry(10,450,100,30);
  connect(sastandart, SIGNAL(clicked()), SLOT(setStandart()));

  wspectrum2 = new QWidget();
  tabwidget->addTab(wspectrum2, tr("SAV (2)","Option Dialog"));

  groupsavgrenzen = new QGroupBox(tr("Frequenzen fuer automatische Schrittkorrektur","Option Dialog"), wspectrum2);
  groupsavgrenzen->setGeometry(10,10,280,140);
  labelgsabmin = new QLabel(groupsavgrenzen);
  labelgsabmin->setGeometry(120,22,100,30);
  labelgsabmin->setText(tr("min.(Hz)","Option Dialog"));
  labelgsabmax = new QLabel(groupsavgrenzen);
  labelgsabmax->setGeometry(190,22,100,30);
  labelgsabmax->setText(tr("max.(Hz)","Option Dialog"));
  labelgsab300 = new QLabel(groupsavgrenzen);
  labelgsab300->setGeometry(10,52,100,30);
  labelgsab300->setText(tr("Bandbreite 300Hz","Option Dialog"));
  labelgsab300->setAlignment(Qt::AlignRight);
  egsab300min = new QLineEdit(groupsavgrenzen);
  egsab300min->setGeometry(120,50,60,25);
  egsab300max = new QLineEdit(groupsavgrenzen);
  egsab300max->setGeometry(190,50,60,25);
  labelgsab7k = new QLabel(groupsavgrenzen);
  labelgsab7k->setGeometry(10,82,100,30);
  labelgsab7k->setText(tr("Bandbreite 7kHz","Option Dialog"));
  labelgsab7k->setAlignment(Qt::AlignRight);
  egsab7kmin = new QLineEdit(groupsavgrenzen);
  egsab7kmin->setGeometry(120,80,60,25);
  egsab7kmax = new QLineEdit(groupsavgrenzen);
  egsab7kmax->setGeometry(190,80,60,25);
  labelgsab30k = new QLabel(groupsavgrenzen);
  labelgsab30k->setGeometry(10,112,100,30);
  labelgsab30k->setText(tr("Bandbreite 30kHz","Option Dialog"));
  labelgsab30k->setAlignment(Qt::AlignRight);
  egsab30kmin = new QLineEdit(groupsavgrenzen);
  egsab30kmin->setGeometry(120,110,60,25);
  egsab30kmax = new QLineEdit(groupsavgrenzen);
  egsab30kmax->setGeometry(190,110,60,25);

  groupsawarnung = new QGroupBox(tr("Warnungen","Option Dialog"), wspectrum2);
  groupsawarnung->setGeometry(10,160,160,80);
  boxwarnsavbw = new QCheckBox(groupsawarnung);
  boxwarnsavbw->setGeometry(10,20,200,30);
  boxwarnsavbw->setText(tr("SAV Schrittsteuerung","Option Dialog"));
  boxwarnsavbwmax = new QCheckBox(groupsawarnung);
  boxwarnsavbwmax->setGeometry(10,50,200,30);
  boxwarnsavbwmax->setText(tr("SAV Schritte max.","Option Dialog"));

  wallgemein = new QWidget();
  tabwidget->addTab(wallgemein, tr("Allgemein","Option Dialog"));
  
  
  grouppen = new QGroupBox(tr("Pen/Color","Option Dialog"), wallgemein);
  grouppen->setGeometry(10,10,110,150);
  buttoncolorh = new QPushButton(tr("Hintergrund","Option Dialog"), grouppen);
  buttoncolorh->setGeometry(10,20,90,30);
  connect(buttoncolorh, SIGNAL(clicked()), SLOT(setColorh()));
  buttoncolor1 = new QPushButton(tr("Color Kanal1","Option Dialog"), grouppen);
  buttoncolor1->setGeometry(10,50,90,30);
  connect(buttoncolor1, SIGNAL(clicked()), SLOT(setColor1()));
  buttoncolor2 = new QPushButton(tr("Color Kanal2","Option Dialog"), grouppen);
  buttoncolor2->setGeometry(10,80,90,30);
  connect(buttoncolor2, SIGNAL(clicked()), SLOT(setColor2()));
  labelspwidth = new QLabel(tr("Breite","Option Dialog"), grouppen);
  labelspwidth->setGeometry(10,117,40,20);
  labelspwidth->setAlignment(Qt::AlignLeft);
  spwidth = new QSpinBox(grouppen);
  spwidth->setGeometry(50,110,50,30);
  spwidth->setRange(1, 3);

  groupspinpfsize = new QGroupBox(tr("Programm","Option Dialog"), wallgemein);
  groupspinpfsize->setGeometry(10,165,110,85);
  groupspinpfsize->setAlignment(Qt::AlignCenter);
  labelspinpfsize = new QLabel(tr("Schriftgroesse","Option Dialog"), groupspinpfsize);
  labelspinpfsize->setGeometry(20,20,90,20);
  labelspinpfsize->setAlignment(Qt::AlignLeft);
  spinpfsize = new QSpinBox(groupspinpfsize);
  spinpfsize->setGeometry(20,45,50,30);
  spinpfsize->setRange(7, 12);
  QObject::connect( spinpfsize, SIGNAL( valueChanged(int)), this, SLOT(psetfontsize(int)));

  groupwmprezision = new QGroupBox(tr("Wattmeter","Option Dialog"), wallgemein);
  groupwmprezision->setGeometry(10,255,110,55);
  groupwmprezision->setAlignment(Qt::AlignCenter);
  wmprezision = new QCheckBox(groupwmprezision);
  wmprezision->setGeometry(10,20,200,30);
  wmprezision->setText(tr("Prezision 2","Option Dialog"));


  groupfocus = new QGroupBox(tr("Focusumschaltung","Option Dialog"), wallgemein);
  groupfocus->setGeometry(130,10,160,120);
  groupfocus->setAlignment(Qt::AlignCenter);
  fwobbeln = new QCheckBox(groupfocus);
  fwobbeln->setGeometry(10,20,200,30);
  fwobbeln->setText(tr("Button Wobbeln","Option Dialog"));
  feinmal = new QCheckBox(groupfocus);
  feinmal->setGeometry(10,50,200,30);
  feinmal->setText(tr("Button Einmal","Option Dialog"));
  fstop = new QCheckBox(groupfocus);
  fstop->setGeometry(10,80,200,30);
  fstop->setText(tr("Button Stop","Option Dialog"));

  groupnwtruhe = new QGroupBox(tr("NWT-Inaktiv","Option Dialog allgemein"), wallgemein);
  groupnwtruhe->setGeometry(130,140,160,60);
  groupnwtruhe->setAlignment(Qt::AlignCenter);
  chset0hz = new QCheckBox(groupnwtruhe);
  chset0hz->setGeometry(10,20,200,30);
  chset0hz->setText(tr("SET 0 Hz","Option Dialog allgemein"));

  groupwdarstellung = new QGroupBox(tr("Wobbeldarstellung","Option Dialog allgemein"), wallgemein);
  groupwdarstellung->setGeometry(130,200,160,60);
  groupwdarstellung->setAlignment(Qt::AlignCenter);
  chsetfliessend = new QCheckBox(groupwdarstellung);
  chsetfliessend->setGeometry(10,20,200,30);
  chsetfliessend->setText(tr("fliessend","Option Dialog allgemein"));

  groupaudio = new QGroupBox(tr("Wobbeln NF-Audio","Option Dialog"), wallgemein);
  groupaudio->setGeometry(130,270,160,145);
  groupaudio->setAlignment(Qt::AlignCenter);
  labelspinwtime = new QLabel(tr("Wobbelzeit (Sek)","Option Dialog"), groupaudio);
  labelspinwtime->setGeometry(20,20,120,20);
  labelspinwtime->setAlignment(Qt::AlignLeft);
  spinwtime = new QSpinBox(groupaudio);
  spinwtime->setGeometry(20,45,50,30);
  spinwtime->setRange(5, 30);
  QObject::connect( spinwtime, SIGNAL( valueChanged(int)), this, SLOT(setspinwtime(int)));
  labelspinwsens = new QLabel(tr("Kurvenkontrolle","Option Dialog"), groupaudio);
  labelspinwsens->setGeometry(20,80,120,20);
  labelspinwsens->setAlignment(Qt::AlignLeft);
  spinwsens = new QSpinBox(groupaudio);
  spinwsens->setGeometry(20,105,50,30);
  spinwsens->setRange(1, 99);



//#############################################################################
//#############################################################################
//Grundaten fuer den Spekrumanalyser
//#############################################################################
//#############################################################################
  
};

void odlg::setspinwtime(int s){
  ogrunddaten.audioztime = 1000 * s;
}


void odlg::psetfontsize(int s)
{
  QFont font("Helvetica", s);

  ok->setFont(font);
  cancel->setFont(font);
  tabwidget->setFont(font);
  kalibrierfrequ->setFont(font);
  labelanfang->setFont(font);
  labelende->setFont(font);
  ddstakt->setFont(font);
  labelddstakt->setFont(font);
  labelcpll->setFont(font);
  cpll->setFont(font);
  gtty->setFont(font);
  labelctty->setFont(font);
  ctty->setFont(font);
  fname->setFont(font);
  labelsonde1->setFont(font);
  labelsonde2->setFont(font);
  fgrenzen->setFont(font);
  labelwobmax->setFont(font);
  gkalibrier->setFont(font);
  daempfungfa->setFont(font);
  geinkanalig->setFont(font);
  boxeinkanalig->setFont(font);
  gswriteration->setFont(font);
  boxiteration->setFont(font);
  buttoncolorh->setFont(font);
  buttoncolor1->setFont(font);
  buttoncolor2->setFont(font);
  tabwidget->setFont(font);
  gallgemein->setFont(font);
  gsakw->setFont(font);
  labelfrqa1->setFont(font);
  labelfrqb1->setFont(font);
  efrqb1->setFont(font);
  labelfrqzf1->setFont(font);
  efrqzf1->setFont(font);
  gsaukw->setFont(font);
  labelfrqa2->setFont(font);
  labelfrqb2->setFont(font);
  labelfrqzf2->setFont(font);
  sastandart->setFont(font);
  spinpfsize->setFont(font);
  groupspinpfsize->setFont(font);
  labelspinpfsize->setFont(font);
}

void odlg::setdaten(TGrunddaten agrunddaten){
  QString s;

  ogrunddaten = agrunddaten;
  s.sprintf("%1.0f", ogrunddaten.kalibrieranfang);
  eanfang->setText(s);
  s.sprintf("%1.0f", ogrunddaten.kalibrierende);
  eende->setText(s);
  s.sprintf("%9.0f", ogrunddaten.ddstakt);
  eddstakt->setText(s);
  s.sprintf("%9.0f", ogrunddaten.maxwobbel);
  wobmax->setText(s);
  s.sprintf("%i", ogrunddaten.frqfaktor);
  cbfrqfaktor->setCurrentText(s);
  ctty->setCurrentText(ogrunddaten.str_tty);
  daempfungfa->setChecked(ogrunddaten.bdaempfungfa);
  fwobbeln->setChecked(ogrunddaten.grperwobbeln);
  feinmal->setChecked(ogrunddaten.grpereinmal);
  fstop->setChecked(ogrunddaten.grperstop);
  chset0hz->setChecked(ogrunddaten.bset0hz);
  chsetfliessend->setChecked(ogrunddaten.bfliessend);
  boxwarnsavbw->setChecked(ogrunddaten.bwarnsavbw);
  boxwarnsavbwmax->setChecked(ogrunddaten.bwarnsavbwmax);
  savdbm->setChecked(ogrunddaten.bsavdbm);
  savuv->setChecked(ogrunddaten.bsavuv);
  savwatt->setChecked(ogrunddaten.bsavwatt);
  savdbuv->setChecked(ogrunddaten.bsavdbuv);
  boxiteration->setChecked(ogrunddaten.bswriteration);
  boxeinkanalig->setChecked(ogrunddaten.einkanalig);
  wmprezision->setChecked(ogrunddaten.wmprezision == 2);

  esonde1->setText(ogrunddaten.strsonde1);
  esonde1lin->setText(ogrunddaten.strsonde1lin);
  esonde2->setText(ogrunddaten.strsonde2);
  cpll->setCurrentIndex(ogrunddaten.pll);
  if(ogrunddaten.pllmodeenable){
    labelcpll->setText(tr("DDS Mode:","Option Dialog"));
    labelcpll->setEnabled(true);
    cpll->setEnabled(true);
  }else{
    labelcpll->setText(tr("nicht aktiv!","Option Dialog"));
  }
  // Button Farbr setzen
  QColor color = ogrunddaten.penkanal1.color();
  QPalette palette = buttoncolor1->palette();
  palette.setColor(QPalette::ButtonText, color);
  buttoncolor1->setPalette(palette);
  
  color = ogrunddaten.penkanal2.color();
  palette = buttoncolor2->palette();
  palette.setColor(QPalette::ButtonText, color);
  buttoncolor2->setPalette(palette);
  spwidth->setValue(ogrunddaten.penwidth);
  spinwtime->setValue(ogrunddaten.audioztime / 1000);
  spinwsens->setValue(ogrunddaten.audiosens);

  //SA
  s.sprintf("%0.0f", ogrunddaten.frqa1);
  efrqa1->setText(s);
  s.sprintf("%0.0f", ogrunddaten.frqb1);
  efrqb1->setText(s);
  s.sprintf("%0.0f", ogrunddaten.frqa2);
  efrqa2->setText(s);
  s.sprintf("%0.0f", ogrunddaten.frqb2);
  efrqb2->setText(s);
  s.sprintf("%0.0f", ogrunddaten.frqzf1);
  efrqzf1->setText(s);
  s.sprintf("%0.0f", ogrunddaten.frqzf2);
  efrqzf2->setText(s);
  s.sprintf("%0.0f", ogrunddaten.frqshift);
  efrqshift->setText(s);
  s.sprintf("%0.2f", ogrunddaten.psavabs1);
  eabsolut1->setText(s);
  s.sprintf("%0.2f", ogrunddaten.psavabs2);
  eabsolut2->setText(s);
  s.sprintf("%0.2f", ogrunddaten.psavabs3);
  eabsolut3->setText(s);
  s.sprintf("%0.2f", ogrunddaten.psav300);
  eb300->setText(s);
  s.sprintf("%0.2f", ogrunddaten.psav7k);
  eb7k->setText(s);
  s.sprintf("%0.2f", ogrunddaten.psav30k);
  eb30k->setText(s);
  
  s.sprintf("%0.0f", ogrunddaten.bw300_max);
  egsab300max->setText(s);
  s.sprintf("%0.0f", ogrunddaten.bw300_min);
  egsab300min->setText(s);
  s.sprintf("%0.0f", ogrunddaten.bw7kHz_max);
  egsab7kmax->setText(s);
  s.sprintf("%0.0f", ogrunddaten.bw7kHz_min);
  egsab7kmin->setText(s);
  s.sprintf("%0.0f", ogrunddaten.bw30kHz_max);
  egsab30kmax->setText(s);
  s.sprintf("%0.0f", ogrunddaten.bw30kHz_min);
  egsab30kmin->setText(s);
  
  s.sprintf("%i", ogrunddaten.safehlermax);
  esafehlermax->setText(s);
  s.sprintf("%i", ogrunddaten.safehlermin);
  esafehlermin->setText(s);
  s.sprintf("%i", ogrunddaten.saminschritte);
  esaminschritte->setText(s);
  boxsastatus->setChecked(ogrunddaten.sastatus);
  boxschrittkorr->setChecked(ogrunddaten.bschrittkorr);
  spinpfsize->setValue(ogrunddaten.pfsize);
  if(ogrunddaten.fwfalsch){
    boxsastatus->setEnabled(false);
    boxschrittkorr->setEnabled(false);
    eabsolut1->setEnabled(false);
    labelabsolut1->setEnabled(false);
    eabsolut2->setEnabled(false);
    labelabsolut2->setEnabled(false);
    eabsolut3->setEnabled(false);
    labelabsolut3->setEnabled(false);
    eb300->setEnabled(false);
    labelb300->setEnabled(false);
    eb7k->setEnabled(false);
    labelb7k->setEnabled(false);
    eb30k->setEnabled(false);
    labelb30k->setEnabled(false);
  }else{
    if(!boxsastatus->isChecked()){
      eb300->setEnabled(false);
      labelb300->setEnabled(false);
      eb7k->setEnabled(false);
      labelb7k->setEnabled(false);
      eb30k->setEnabled(false);
      labelb30k->setEnabled(false);
    }
    if(!boxschrittkorr->isChecked()){
      esaminschritte->setEnabled(false);
      labelsaminschritte->setEnabled(false);
    }
  }
};

TGrunddaten odlg::getdaten(){
  QString s;
  bool ok;

  s = eanfang->text();
  ogrunddaten.kalibrieranfang = s.toDouble(&ok);
  if(!ok)ogrunddaten.kalibrieranfang = 100000.0;
  
  s = eende->text();
  ogrunddaten.kalibrierende = s.toDouble(&ok);
  if(!ok)ogrunddaten.kalibrierende = 150000000.0;
  
  s = eddstakt->text();
  ogrunddaten.ddstakt = s.toDouble(&ok);
  if(!ok)ogrunddaten.ddstakt = 400000000.0;
  
  s = wobmax->text();
  ogrunddaten.maxwobbel = s.toDouble(&ok);
  if(!ok)ogrunddaten.maxwobbel = 200000000.0;
  
  s = cbfrqfaktor->currentText();
  ogrunddaten.frqfaktor = s.toInt(&ok);
  if(!ok)ogrunddaten.frqfaktor = 1;
  
  ogrunddaten.str_tty = ctty->currentText();
  ogrunddaten.bdaempfungfa = daempfungfa->isChecked();
  ogrunddaten.einkanalig = boxeinkanalig->isChecked();
  ogrunddaten.bswriteration = boxiteration->isChecked();
  ogrunddaten.grperwobbeln = fwobbeln->isChecked();
  ogrunddaten.grpereinmal = feinmal->isChecked();
  ogrunddaten.grperstop = fstop->isChecked();
  ogrunddaten.bset0hz = chset0hz->isChecked();
  ogrunddaten.bwarnsavbw = boxwarnsavbw->isChecked();
  ogrunddaten.bwarnsavbwmax = boxwarnsavbwmax->isChecked();
  ogrunddaten.bsavdbm = savdbm->isChecked();
  ogrunddaten.bsavuv = savuv->isChecked();
  ogrunddaten.bsavwatt = savwatt->isChecked();
  ogrunddaten.bsavdbuv = savdbuv->isChecked();

  if(wmprezision->isChecked()){
    ogrunddaten.wmprezision = 2;
  }else{
    ogrunddaten.wmprezision = 1;
  }
  ogrunddaten.strsonde1 = esonde1->text();
  ogrunddaten.strsonde1lin = esonde1lin->text();
  ogrunddaten.strsonde2 = esonde2->text();
  ogrunddaten.pll = cpll->currentIndex();
  ogrunddaten.pfsize = spinpfsize->value();
  ogrunddaten.audioztime = spinwtime->value() * 1000;
  ogrunddaten.audiosens = spinwsens->value();
  ogrunddaten.penwidth = spwidth->value();

  //***************************************************************
  //SA Einstellungen speichern
  //***************************************************************
  s = efrqa1->text();
  ogrunddaten.frqa1 = s.toDouble(&ok);
  if(!ok)ogrunddaten.frqa1 = 1000000.0;
  
  s = efrqb1->text();
  ogrunddaten.frqb1 = s.toDouble(&ok);
  if(!ok)ogrunddaten.frqb1 = 72000000.0;
  
  s = efrqa2->text();
  ogrunddaten.frqa2 = s.toDouble(&ok);
  if(!ok)ogrunddaten.frqa2 = 135000000.0;
  
  s = efrqb2->text();
  ogrunddaten.frqb2 = s.toDouble(&ok);
  if(!ok)ogrunddaten.frqb2 = 149000000.0;
  
  s = efrqzf1->text();
  ogrunddaten.frqzf1 = s.toDouble(&ok);
  if(!ok)ogrunddaten.frqzf1 = 85300000.0;
  
  s = efrqzf2->text();
  ogrunddaten.frqzf2 = s.toDouble(&ok);
  if(!ok)ogrunddaten.frqzf2 = -85300000.0;
  
  s = efrqshift->text();
  ogrunddaten.frqshift = s.toDouble(&ok);
  if(!ok)ogrunddaten.frqshift = -410000000.0;
  
  s = eabsolut1->text();
  ogrunddaten.psavabs1 = s.toDouble(&ok);
  if(!ok)ogrunddaten.psavabs1 = 0.0;
  
  s = eabsolut2->text();
  ogrunddaten.psavabs2 = s.toDouble(&ok);
  if(!ok)ogrunddaten.psavabs2 = 0.0;
  
  s = eabsolut3->text();
  ogrunddaten.psavabs3 = s.toDouble(&ok);
  if(!ok)ogrunddaten.psavabs3 = 0.0;
  
  s = eb300->text();
  ogrunddaten.psav300 = s.toDouble(&ok);
  if(!ok)ogrunddaten.psav300 = 0.0;
  
  s = eb7k->text();
  ogrunddaten.psav7k = s.toDouble(&ok);
  if(!ok)ogrunddaten.psav7k = 0.0;
  
  s = eb30k->text();
  ogrunddaten.psav30k = s.toDouble(&ok);
  if(!ok)ogrunddaten.psav30k = 0.0;
  
  ogrunddaten.sastatus = boxsastatus->isChecked();
  ogrunddaten.bschrittkorr = boxschrittkorr->isChecked();
  s = esafehlermax->text();
  ogrunddaten.safehlermax = s.toInt(&ok);
  if(!ok)ogrunddaten.safehlermax = -5;
  
  s = esafehlermin->text();
  ogrunddaten.safehlermin = s.toInt(&ok);
  if(!ok)ogrunddaten.safehlermin = -60;
  
  s = esaminschritte->text();
  ogrunddaten.saminschritte = s.toInt(&ok);
  if(!ok)ogrunddaten.saminschritte = 401;
  
  s = egsab300max->text();
  ogrunddaten.bw300_max = s.toDouble(&ok);
  if(!ok)ogrunddaten.bw300_max = 100.0;
  
  s = egsab300min->text();
  ogrunddaten.bw300_min = s.toDouble(&ok);
  if(!ok)ogrunddaten.bw300_min = 50.0;
  
  s = egsab7kmax->text();
  ogrunddaten.bw7kHz_max = s.toDouble(&ok);
  if(!ok)ogrunddaten.bw7kHz_max = 2500.0;
  
  s = egsab7kmin->text();
  ogrunddaten.bw7kHz_min = s.toDouble(&ok);
  if(!ok)ogrunddaten.bw7kHz_min = 1250.0;
  
  s = egsab30kmax->text();
  ogrunddaten.bw30kHz_max = s.toDouble(&ok);
  if(!ok)ogrunddaten.bw30kHz_max = 10000.0;
  
  s = egsab30kmin->text();
  ogrunddaten.bw30kHz_min = s.toDouble(&ok);
  if(!ok)ogrunddaten.bw30kHz_min = 5000.0;
  
  if(ogrunddaten.bw300_min >= ogrunddaten.bw300_max)ogrunddaten.bw300_min = ogrunddaten.bw300_max - 1.0;
  if(ogrunddaten.bw7kHz_min >= ogrunddaten.bw7kHz_max)ogrunddaten.bw7kHz_min = ogrunddaten.bw7kHz_max - 1.0;
  if(ogrunddaten.bw30kHz_min >= ogrunddaten.bw30kHz_max)ogrunddaten.bw30kHz_min = ogrunddaten.bw30kHz_max - 1.0;
//***************************************************************
  return ogrunddaten;
};

void odlg::tip(bool btip)
{
QString tip_einkanalig = tr(
  "<b>Aktivieren beim Betrieb mit nur einen Kanal</B><BR>"
  "Der NWT01 hat in der Grundausstattung einen Messkanal.<br> "
  "Wird ein zweiter Messkanal aufgebaut so dient PIN7 vom PIC als<br> "
  "Messeingang. Durch deaktivieren wird der zweite Kanal aktiviert.<br>"
  ,"tooltip option text");
QString tip_mathkorr = tr(
  "<b>Mathematische Glaettung der SWV-Kurve</B><BR>"
  "Bei den geringsten dB-Aenderungen aendert sich das SWV in grossen<br> "
  "Schritten. Aber nur bei grossem SWV. Um das Zickzack in der<br> "
  "Darstellung etwas zu glaetten wird aus 3 Messwerten der Mittelwert<br> "
  "gebildet. Das gleiche math. Verfahren wird beim Erzeugen der<br> "
  "SWV-Kalibrierdatei angewendet.<br>"
  ,"tooltip option text");
QString tip_defdatei = tr(
  "<b>Defaultdateiname der Sondendatei</B><BR>"
  "Dieser Dateiname wird als default-Einstellung fuer die Sondendatei<br> "
  "benutzt. Der Dateiname wird ohne \"EXT\" angegeben. Das \"EXT\" wird<br> "
  "vom der SW automatisch ergaenzt.<br> "
  ,"tooltip option text");
QString tip_fmaxwobbel = tr(
  "<b>Maximale Frequenz beim Wobbel</B><BR>"
  "Diese Frequenz kann maximal zum Wobbeln benutzt werden.<br>"
  "<em>NWT01-FA</em> \tetwa 200MHz<br> "
  "<em>HFM9</em> \tetwa 200MHz<br> "
  "<em>NWT7</em> mit DDS-Takt 180MHz \tetwa 70MHz<br> "
  "<em>NWT500</em> mit DDS-Takt 1200MHz \tetwa 500MHz<br> "
  ,"tooltip option text");
QString tip_serielle = tr(
  "<b>Serielle Schnittstelle</B><BR>"
  "Einstellung der seriellen Schnittstelle vom PC.<br> "
  "Funktioniert die Schnittstelle nicht, ist das Programm<br> "
  "noch einmal neu zu starten. Dieses Verhalten ist je nach<br> "
  "Betriebssystem unterschiedlich. <br> "
  "Unter Linux ist die Eingabe editierbar und unter Windows sind <br> "
  "COM1 bis COM256 auswaehlbar.<br> "
  ,"tooltip option text");
QString tip_pll = tr(
  "<b>Einstellung der PLL des DDS AD9951</B><BR>"
  "Dieser Punkt ist im normalfall deaktiviert. Beim NWT01<br> "
  "ist keine Einstellung notwendig. Dort ist die PLL im PIC<br> "
  "voreingestellt. Wird diese Einstellmoeglichkeit aktiviert<br> "
  "ist eine andere FW im PIC zu installieren.<br> "
  ,"tooltip option text");
QString tip_ddstakt = tr(
  "<b>Einstellung der Taktfrequenz des DDS</B><BR>"
  "Hier kann die genaue Taktfrequenz eingestellt werden "
  "Diese Zahl wird nach dem OK im PIC gespeichert und ist "
  "nach jedem Power ON wieder verfuegbar.<br>"
  "<em>Kalibrierung:</em><br>"
  "1. am VFO1 10MHz einstellen<br>"
  "2. anschliessend wirkliche Frequenz feststellen<br>"
  "3. F(Takt neu) = F(Takt alt) * (F(Messung) / 10 000 000)<br>"
  "4. F(Takt neu) eintragen und OK druecken. FW macht einen Warmstart.<br>"
  "5. Zum Wobbelfenster wechseln und wieder zum VFO wechseln<br>"
  "   dadurch wird VFO-Einstellung neu zum NWT gesendet<br><br> "
  "Jetzt muesste die die VFO-Frequenz genau 10MHz betragen"
  ,"tooltip option text");
QString tip_kalibrierend = tr(
  "<b>Einstellung der Endfrequenz bei Kalibrierkorrektor</B><BR>"
  "Hier wird die oberste Grenzfrequenz bei der Kalibrierung festgelegt<br> "
  "Das ist erforderlich um die Anzahl der Frequenzstuetzpunkte<br> "
  "fuer die mathematische Kalibrierkorrektur festzulegen.<br> "
  "Die zweite Funktion dieser Endfrequenz ist bei der Frequenzlupe<br>. "
  "Wird immer auf \"minus\" gedrueckt muss es eine obere Grenze geben.<br> "
  "Dazu wird diese Einstellung benutzt.<br>"
  ,"tooltip option text");
QString tip_kalibrieranf = tr(
  "<b>Einstellung der Anfangsfrequenz bei Kalibrierkorrektor</B><BR>"
  "Hier wird die unterste Grenzfrequenz bei der Kalibrierung festgelegt<br> "
  "Das ist erforderlich um die Anzahl der Frequenzstuetzpunkte<br> "
  "fuer die mathematische Kalibrierkorrektur festzulegen.<br> "
  "Die zweite Funktion dieser Anfangsfrequenz ist bei der Frequenzlupe.<br> "
  "Wird immer auf \"minus\" gedrueckt muss es eine untere Grenze geben.<br> "
  "Dazu wird diese Einstellung benutzt.<br>"
  ,"tooltip option text");
QString tip_color = tr(
  "<b>Einstellung der Farbe</B><BR>"
  "Die Farbwerte werden in der \"*.hfc\" Datei<br>"
  "abgespeichert.<br>"
  ,"tooltip option text");
QString tip_daempfung = tr(
  "<b>Aktivierung des FA-Daempfungsgliedes</B><BR>"
  "In der Grund-SW wird ein Daempfungsglied <br>"
  "10,20,20 dB unterstuetzt. Nach Aktivierung <br>"
  " wird das Daempfungsglied vom FA unterstuetzt. <br>"
  "Voraussetzung ist die FW ab 1.10 <br>"
  "<em>Bitte beim alten Daempfungsglied die <br>"
  "neue Beschaltung beachten.</em>"
  ,"tooltip option text");
QString tip_hwrueck = tr(
  "<b>Rueckmeldung Spektrumanalyser Software</B><BR>"
  "Wird ein Spektrumanalyservorsatz an den NWT-FA <br>"
  "angeschlossen ist es ratsam diesen Punkt zu aktivieren. <br>"
  "Dadurch werden alle Schalterstellungen vom SAV <br>"
  "zur SW zurueckgemeldet. Voraussetzung ist eine  <br>"
  "entsprechende FW im NWT-FA. Das funktioniert nur <br>"
  "mit dem NWT vom Funkamateur"
  ,"tooltip option SAV text");
QString tip_autoschritt = tr(
  "<b>Automatische Schrittkorrektur</B><BR>"
  "Ist die HW-Rueckmeldung aktiv kann mit diesem Punkt <br>"
  "die Schrittweite automatisch angepasst werden. Betraegt <br>"
  "z.B.: die Schrittweite im NWT 10kHz und am SAV ist 7kHz <br>"
  "eingestellt wuerde das zu Fehlmessungen fuehren. Ist der <br>"
  "Punkt aktiv, wird die Schrittzahl so weit erhoeht, das die <br>"
  "Schrittweite kleiner 3,5kHz ist. Ist das nicht moeglich <br>"
  "kommt ein Warnhinweis"
  ,"tooltip option SAV text");
QString tip_sabereich1 = tr(
  "<b>SA Bereich1/3 Einstellungen</B><BR>"
  "Hier werden minimale, maximale <br>"
  "Frequenz und die ZF fuer den  <br>"
  "Spektrumanalyservorsatz vom FA <br>"
  "eingegeben. Die maximale Frequenz  <br>"
  "ergibt sich aus der ersten ZF des SA. <br>"
  "\"Shift\" ist fuer \"Spek.FRQ-shift\" <br>"
  "Verschiebung der angezeigten Frequenz <br>"
  "im Display."
  ,"tooltip option SAV text");
QString tip_sabereich2 = tr(
  "<b>SA Bereich2 Einstellungen</B><BR>"
  "Hier werden minimale, maximale Frequenz und die ZF <br>"
  "fuer den Spektrumanalyservorsatz vom FA eingegeben <br>"
  "Die minimale Frequenz ergibt sich aus der ersten ZF des SA. <br>"
  "Die VFO-Frequenz ist hier unterhalb der ersten ZF"
  ,"tooltip option SAV text");
QString tip_sastandart = tr(
  "<b>Spektrumanalyser Standartwerte</B><BR>"
  "Wurde falsche Frequenzwerte eingegeben. Kann mann alles <br>"
  "wieder auf Standartwerte einstellen. "
  ,"tooltip option SAV text");
QString tip_sapegelabs = tr(
  "<b>Spektrumanalyser Kalibrierung</B><BR>"
  "Den Pegel Bereich 1/2/3 verschieben <br>"
  ,"tooltip option SAV text");
QString tip_sapegelbw = tr(
  "<b>Spektrumanalyser Kalibrierung</B><BR>"
  "Die verschiedenen Pegel der Bandweiten angleichen <br>"
  ,"tooltip option SAV text");
QString tip_sacalnull = tr(
  "<b>Spektrumanalyser Kalibrierung</B><BR>"
  "Das Kalibrierungaarray auf 0,0 setzen. <br>"
  ,"tooltip option SAV text");
QString tip_sacal = tr(
  "<b>Spektrumanalyser Kalibrierung</B><BR>"
  "Das Kalibrierungaarray mit Hilfe der <br>"
  "Kurvendateien optimieren. Der Daempfungsverlauf <br>"
  "des Tiefpass Bereich1 und des Bandpasses Bereich2 <br>"
  "wird mit in die Kalibrieung genommen. Siehe Beschreibung. <br>"
  ,"tooltip option SAV text");
QString tip_sabwunten = tr(
  "<b>Spektrumanalyser automatischer Schrittkorrektur</B><BR>"
  "Hier wird die Frequenzgrenze eingetragen, bei der <br>" 
  "die Schrittanzahl wieder automatisch verringert wird. <br>"
  "Dieser wert muss kleiner sein als der \"max Wert\"."
  ,"tooltip option SAV text");
QString tip_sabwoben = tr(
  "<b>Spektrumanalyser automatischer Schrittkorrektur</B><BR>"
  "Hier wird die Frequenzgrenze eingetragen, bei <br>"
  "die Schrittweite durch Erhoehung der Schrittanzahl <br>"
  "korrigiert wird. Der Wert muss so gross gewaehlt <br>"
  "werden, dass mindestens 2 Messwerte innerhalb <br>"
  "der 3 dB Durchlasskurve entstehen. Bei 300Hz <br>"
  "Bandweite muesste also 100 Hz eingetragen werden."
  ,"tooltip option SAV text");
QString tip_saungenau = tr(
  "<b>Spektrumanalyser Kalibrierung</B><BR>"
  "Die graue Einfaerbung der ungenauen <br>"
  "Messbereiche des SAV. <br>"
  ,"tooltip option SAV text");
QString tip_sawarnung = tr(
  "<b>Spektrumanalyser Warnungen</B><BR>"
  "Ist die automatische Schrittkorrektur <br>"
  "deaktiviert und die Schrittweite zu <br>"
  "gross erfolgt ein Hinweis. Dieser <br>"
  "Warnung kann hier unterdrueckt werden."
  ,"tooltip option SAV text");
QString tip_sawarnungmax = tr(
  "<b>Spektrumanalyser Warnungen</B><BR>"
  "Ist die maximale Schrittanzahl erreicht <br>"
  "und die Schrittweite immer noch zu gross, <br>"
  "kommt dieser Hinweis. Ist dieser <br>"
  "Hinweis stoerend, kann er hier <br>"
  "unterdrueckt werden."
  ,"tooltip option SAV text");
QString tip_saminschritt = tr(
  "<b>Spektrumanalyser Kalibrierung</B><BR>"
  "Die unterste Grenze der Schritte bei <br>"
  "\"automatischer Schrittkorrektur\" <br>"
  ,"tooltip option SAV text");
QString tip_schriftgroesse = tr(
  "<b>Einstellung der Schriftgroesse</B><BR>"
  "Die Schriftgroesse kann angepasst werden. <br>"
  "Es wirkt auf fast alle Texte im Programm. "
  ,"tooltip option allgemein text");
QString tip_focusumschaltung = tr(
  "<b>Automatische Focusumschaltung</B><BR>"
  "Durch die Herausloesung der grafischen Darstellung <br>"
  "ist die Bedienung des Programmes etwas schwieriger. <br>"
  "Eine Vereinfachung ist die automatische Focusuebergabe <br>"
  "nach dem Betaetigen eines Buttons. Welcher Button <br>"
  "das bewirken soll, wird hier festgelegt"
  ,"tooltip option allgemein text");
QString tip_nfaudio = tr(
  "<b>Wobbbel NF-Audio</B><BR>"
  "Die <em>Wobbelzeit</em> 5 - 30 Sekunden <br>"
  "ist ein Wobbeldurchlauf grob in Sekunden.<br>"
  "Dabei ist die Kontrolle bei starken Pegelschwankungen <br>"
  "<em>Wobbelkontrolle</em> nicht mit in diese Zeit eingerechnet. <br>"
  "<br>"
  "Die <em>Wobbelkontrolle</em> 1 - 99 <br>"
  "ist die Sensibilitaet mit der auf Pegelschwankungen <br>"
  "geachtet wird. Sobald eine Pegelschwankung erkannt wird, <br>"
  "werden die naechsten 5 Messpunkte ganz langsam gewobbelt. <br>"
  "Die ersten 50 Messpunkte werden immer ganz langsam gewobbelt, <br>"
  "da fast immer mit niedrigen Frequenzen kleiner 1000 Hz begonnen wird. <br>"
  "1 = sehr empfindlich <br>"
  "99 = am unempfindlichsten <br>"
  ,"tooltip option allgemein text");
QString tip_prezision = tr(
  "<b>Wattmeter</B><BR>"
  "2 Nachkommastellen bei der dBm-Anzeige <br>"
  ,"tooltip option allgemein text");

  
  if(btip){
    boxeinkanalig->setToolTip(tip_einkanalig);
    boxiteration->setToolTip(tip_mathkorr);
    esonde1->setToolTip(tip_defdatei);
    esonde2->setToolTip(tip_defdatei);
    wobmax->setToolTip(tip_fmaxwobbel);
    ctty->setToolTip(tip_serielle);
    cpll->setToolTip(tip_pll);
    eddstakt->setToolTip(tip_ddstakt);
    eende->setToolTip(tip_kalibrierend);
    eanfang->setToolTip(tip_kalibrieranf);
    buttoncolor1->setToolTip(tip_color);
    buttoncolor2->setToolTip(tip_color);
    daempfungfa->setToolTip(tip_daempfung);
    buttoncolorh->setToolTip(tip_color);
    boxsastatus->setToolTip(tip_hwrueck);
    boxschrittkorr->setToolTip(tip_autoschritt);
    gsakw->setToolTip(tip_sabereich1);
    gsaukw->setToolTip(tip_sabereich2);
    sastandart->setToolTip(tip_sastandart);
    eabsolut1->setToolTip(tip_sapegelabs);
    labelabsolut1->setToolTip(tip_sapegelabs);
    eabsolut2->setToolTip(tip_sapegelabs);
    labelabsolut2->setToolTip(tip_sapegelabs);
    eabsolut3->setToolTip(tip_sapegelabs);
    labelabsolut3->setToolTip(tip_sapegelabs);
    eb300->setToolTip(tip_sapegelbw);
    labelb300->setToolTip(tip_sapegelbw);
    eb7k->setToolTip(tip_sapegelbw);
    labelb7k->setToolTip(tip_sapegelbw);
    eb30k->setToolTip(tip_sapegelbw);
    labelb30k->setToolTip(tip_sapegelbw);
    buttonsacal->setToolTip(tip_sacal);
    buttonsacalreset->setToolTip(tip_sacalnull);
    esafehlermax->setToolTip(tip_saungenau);
    labelsafehlermax->setToolTip(tip_saungenau);
    esafehlermin->setToolTip(tip_saungenau);
    labelsafehlermin->setToolTip(tip_saungenau);
    esaminschritte->setToolTip(tip_saminschritt);
    labelsaminschritte->setToolTip(tip_saminschritt);
    groupspinpfsize->setToolTip(tip_schriftgroesse);
    groupwmprezision->setToolTip(tip_prezision);
    groupaudio->setToolTip(tip_nfaudio);
    groupfocus->setToolTip(tip_focusumschaltung);
    egsab300max->setToolTip(tip_sabwoben);
    egsab300min->setToolTip(tip_sabwunten);
    egsab7kmax->setToolTip(tip_sabwoben);
    egsab7kmin->setToolTip(tip_sabwunten);
    egsab30kmax->setToolTip(tip_sabwoben);
    egsab30kmin->setToolTip(tip_sabwunten);
    boxwarnsavbw->setToolTip(tip_sawarnung);
    boxwarnsavbwmax->setToolTip(tip_sawarnungmax);
  }else{
    boxeinkanalig->setToolTip("");
    boxiteration->setToolTip("");
    esonde1->setToolTip("");
    esonde2->setToolTip("");
    wobmax->setToolTip("");
    ctty->setToolTip("");
    cpll->setToolTip("");
    eddstakt->setToolTip("");
    eende->setToolTip("");
    eanfang->setToolTip("");
    buttoncolor1->setToolTip("");
    buttoncolor2->setToolTip("");
    daempfungfa->setToolTip("");
    buttoncolorh->setToolTip("");
    boxsastatus->setToolTip("");
    boxschrittkorr->setToolTip("");
    gsakw->setToolTip("");
    gsaukw->setToolTip("");
    sastandart->setToolTip("");
    eabsolut1->setToolTip("");
    labelabsolut1->setToolTip("");
    eabsolut2->setToolTip("");
    labelabsolut2->setToolTip("");
    eabsolut3->setToolTip("");
    labelabsolut3->setToolTip("");
    eb300->setToolTip("");
    labelb300->setToolTip("");
    eb7k->setToolTip("");
    labelb7k->setToolTip("");
    eb30k->setToolTip("");
    labelb30k->setToolTip("");
    buttonsacal->setToolTip("");
    buttonsacalreset->setToolTip("");
    esafehlermax->setToolTip("");
    labelsafehlermax->setToolTip("");
    esafehlermin->setToolTip("");
    labelsafehlermin->setToolTip("");
    esaminschritte->setToolTip("");
    labelsaminschritte->setToolTip("");
    groupspinpfsize->setToolTip("");
    groupwmprezision->setToolTip("");
    groupaudio->setToolTip("");
    groupfocus->setToolTip("");
    egsab300max->setToolTip("");
    egsab300min->setToolTip("");
    egsab7kmax->setToolTip("");
    egsab7kmin->setToolTip("");
    egsab30kmax->setToolTip("");
    egsab30kmin->setToolTip("");
    boxwarnsavbw->setToolTip("");
    boxwarnsavbwmax->setToolTip("");
  }
}

void odlg::setStandart(){
//  qDebug("setStandart()");
  efrqa1->setText("1000000");
  efrqb1->setText("72000000");
  efrqa2->setText("135000000");
  efrqb2->setText("149000000");
  efrqzf1->setText("85300000");
  efrqzf2->setText("-85300000");
  efrqshift->setText("0");
  eabsolut1->setText("0,00");
  eabsolut2->setText("0,00");
  eabsolut3->setText("0,00");
  eb300->setText("0,00");
  eb7k->setText("0,00");
  eb30k->setText("0,00");
  esafehlermax->setText("5");
  esafehlermin->setText("-60");
  esaminschritte->setText("401");
  egsab300max->setText("100");
  egsab300min->setText("50");
  egsab7kmax->setText("2500");
  egsab7kmin->setText("1250");
  egsab30kmax->setText("10000");
  egsab30kmin->setText("5000");
  savdbm->setChecked(true);
  savuv->setChecked(false);
  savwatt->setChecked(false);
  savdbuv->setChecked(false);
}

void odlg::setColor1(){
  bool ok;
  QColor color = ogrunddaten.penkanal1.color();
  QRgb rgb;
  rgb = color.rgb();
//  qDebug("setColor1");
  color = QColorDialog::getRgba(rgb, &ok, this);
  if(ok){
    ogrunddaten.penkanal1.setColor(color);
    QPalette palette = buttoncolor1->palette();
    palette.setColor(QPalette::Active, QPalette::Button, color);
    buttoncolor1->setPalette(palette);
  }
}

void odlg::setColor2(){
  bool ok;
  QColor color = ogrunddaten.penkanal2.color();
  QRgb rgb;
  rgb = color.rgb();
//  qDebug("setColor2");
  color = QColorDialog::getRgba(rgb, &ok, this);
  if(ok){
    ogrunddaten.penkanal2.setColor(color);
    QPalette palette = buttoncolor2->palette();
    palette.setColor(QPalette::Active, QPalette::Button, color);
    buttoncolor2->setPalette(palette);
  }
}

void odlg::setColorh(){
  bool ok;
  QColor chd;
  QColor color = ogrunddaten.colorhintergrund;
  QRgb rgb;
  
  rgb = color.rgb();
//  qDebug("setColor2");
  color = QColorDialog::getRgba(rgb, &ok, this);
  if(ok){
    ogrunddaten.colorhintergrund = color;
  }
}

void odlg::checkboxsastatus_checked(bool ok)
{
  if(ok){
    eb300->setEnabled(true);
    labelb300->setEnabled(true);
    eb7k->setEnabled(true);
    labelb7k->setEnabled(true);
    eb30k->setEnabled(true);
    labelb30k->setEnabled(true);
    if(boxschrittkorr->isChecked()){
      esaminschritte->setEnabled(true);
      labelsaminschritte->setEnabled(true);
    }
  }else{
    eb300->setEnabled(false);
    labelb300->setEnabled(false);
    eb7k->setEnabled(false);
    labelb7k->setEnabled(false);
    eb30k->setEnabled(false);
    labelb30k->setEnabled(false);
    esaminschritte->setEnabled(false);
    labelsaminschritte->setEnabled(false);
  }
}

void odlg::checkboxschrittkorr_checked(bool ok)
{
  if(ok){
    esaminschritte->setEnabled(true);
    labelsaminschritte->setEnabled(true);
  }else{
    esaminschritte->setEnabled(false);
    labelsaminschritte->setEnabled(false);
  }
}

void odlg::setsavcal(const TSavcalarray &asavcalarray){
  osavcalarray = asavcalarray;
}

TSavcalarray odlg::getsavcal(){
  return osavcalarray;
}

void odlg::sacal(){
  int i, j, k, calix, calixalt;
  int ikversion = 0; //neue Versionserkennung
  //bool v305 = false;
  //bool v404 = false;
  //bool v405 = false;
  QString zeile;
  QString qs;
  double faktor1, faktor2;
  double shift1, shift2;
  bool ok;
  TWobbel w;
  QFile f;
  double fanfang;
  double fende;
  double frequenz;
  double pegel_0;
  double pegel, pegelalt;
  double pegelmin = 20.0;
  double pegelmax = -100.0;
  QString beschr;
  
  QDir kurvendir = ogrunddaten.kurvendir;
  QStringList filters;
  filters << "Image files (*.png *.xpm *.jpg)"
          << "Text files (*.txt)"
          << "Any files (*)";

  QString s = QFileDialog::getOpenFileName(this, tr("Kalibrierkurve laden","FileDialog"),
                                                 kurvendir.absolutePath(),
                                                 tr("NWT Kurvendatei (*.hfd)","FileDialog")
                                                 );
  //String ist nicht leer
  if (!s.isNull())
  {
    kurvendir.setPath(s);
    qs = kurvendir.absolutePath();
    //qDebug(qs);
    //Filename setzen
    f.setFileName(s);
    if(f.open(QIODevice::ReadOnly )){
      QTextStream ts(&f);
      //erste Zeile lesen
      ts >> zeile;
      ikversion = 100;
      if(zeile == "#V3.05")ikversion = 305;
      if(zeile == "#V4.04")ikversion = 404;
      if(zeile == "#V4.05")ikversion = 405;
      if(zeile == "#V4.10")ikversion = 410;
      if(ikversion < 305){ //erste eingelesene Zeile ist keine Versionsbezeichnung
          w.anfangsfrequenz = zeile.toDouble(&ok);
      }else{
        //Es ist V4.10: diese Zeile entscheidet ob NF oder HF Wobblung
        if(ikversion == 410){
          ts >> i; // einlesen
          w.maudio2 = false;
          if(i == 1)w.maudio2 = true;
        }
        // Version >= 305 Anfangsfrequenz einlesen
        ts >> w.anfangsfrequenz;
      }
      //qDebug("a1 %li", wobdaten.anfangsfrequenz);
      ts >> w.schrittfrequenz;
      ts >> w.schritte;
      if(w.schritte > maxmesspunkte)w.schritte = maxmesspunkte;
      ts >> faktor1;
      ts >> shift1;
      ts >> faktor2;
      ts >> shift2;
      ts >> i;
      switch(i){
        //log
        case 0:
        case 1:
        case 3:
        case 4:
        case 5:
          if(ikversion < 305){
            w.faktor1 = faktor1 / 10.23;
            w.shift1 = ((((511.5 - shift1) / faktor1) * w.faktor1) + 40.0) * -1.0;
            w.faktor2 = faktor2 / 10.23;
            w.shift2 = ((((511.5 - shift2) / faktor2) * w.faktor2) + 40.0) * -1.0;
          }else{
            w.faktor1 = faktor1;
            w.shift1 = shift1;
            w.faktor2 = faktor2;
            w.shift2 = shift2;
          }
        break;
        //lin
        case 2:
          if(ikversion < 305){
            w.faktor1 = faktor1 / 9.3;
            w.shift1 = ((((465.0 - shift1) / faktor1) * w.faktor1) + 50.0) * -1.0;
            w.faktor2 = faktor2 / 9.3;
            w.shift2 = ((((465.0 - shift2) / faktor2) * w.faktor2) + 50.0) * -1.0;
          }else{
            w.faktor1 = faktor1;
            w.shift1 = shift1;
            w.faktor2 = faktor2;
            w.shift2 = shift2;
          }
        break;
      }
      switch(i){
        case 0:
          w.ebetriebsart = ewobbeln;
          w.linear1 = false;
          w.linear2 = false;
        break;
        case 1:  
        case 2:  
        case 3:  
        case 4:  
        case 5:  
        //Falsche Kalibrierdaten
          QMessageBox::warning( this, tr("SAV Kalibrierdatei","Kurvendatei laden"), 
                                      tr("<B>Das ist die falsche Betriebsart! </B><BR>"
                                         "Der Kurvedatei muss mit der log. Messonde erzeugt werden.  <BR>"
                                         "In der Betriebsart \"Wobbeln\". <br>","SAV Option Kalibrieren"));
        return;
        break;
      }
      for(i=0; i<w.schritte; i++){
        ts >> w.mess.k1[i];
      }
      for(i=0; i<w.schritte; i++){
        ts >> w.mess.k2[i];
      }
      f.close();
      beschr = tr("<B>Einlesen Kalibrierdatei</B><BR>"
                  "Frequenzgrenzen festlegen! <br>"
                  "<BR><BR>"
                  "Eingabe der Anfangsfrequenz: ","InputDialog");
      if(w.anfangsfrequenz <= 2000000.0){
        frequenz = double (ogrunddaten.frqa1);
      }else{
        frequenz = double (ogrunddaten.frqa2);
      }
      //Ueberpruefung der Anfangsfrequenz in der Kurvendatei
      if(frequenz < w.anfangsfrequenz)frequenz = w.anfangsfrequenz;
      frequenz = QInputDialog::getDouble(this, tr("Kalibrieren SAV","InputDialog"),
                                        beschr, frequenz, 0.0, 100000000000.0, 0, &ok); 
      if(ok){
        fanfang = frequenz;
      }else{
        return;
      }
      beschr = tr("<B>Einlesen Kalibrierdatei</B><BR>"
                  "Frequenzgrenzen festlegen! <br>"
                  "<BR><BR>"
                  "Eingabe der Endfrequenz: ","InputDialog");
      if(w.anfangsfrequenz <= 2000000.0){
        frequenz = double (ogrunddaten.frqb1);
      }else{
        frequenz = double (ogrunddaten.frqb2);
      }
      if(frequenz > (w.anfangsfrequenz + (w.schrittfrequenz * w.schritte)))
        frequenz = w.anfangsfrequenz+ w.schrittfrequenz * w.schritte;
      frequenz = QInputDialog::getDouble(this, tr("Kalibrieren SAV","InputDialog"),
                                        beschr, frequenz, 0.0, 100000000000.0, 0, &ok); 
      if(ok){
        fende = frequenz;
      }else{
        return;
      }
      j = 0;
      pegel_0 = 0.0;
      faktor1 = w.faktor1;
      shift1 = w.shift1;
      for(i=0; i<w.schritte; i++){
        frequenz = double (w.anfangsfrequenz + (w.schrittfrequenz * i));
        if((frequenz >= fanfang) and (frequenz <= fende)){
          pegel = w.mess.k1[i] * faktor1 + shift1 - getkalibrierwert(frequenz);
          j++;
          pegel_0 = pegel_0 + pegel;
          if(pegel < pegelmin)pegelmin = pegel;
          if(pegel > pegelmax)pegelmax = pegel;
        }
      }
      pegel_0 = pegel_0 / double(j);
      pegelmin = pegelmin - pegel_0;
      pegelmax = pegelmax - pegel_0;
      QString s1,s2,s3,s4;
      s1.sprintf(" %fdB", pegel_0);
      s2.sprintf(" %i", j);
      s3.sprintf(" %fdB", pegelmin);
      s4.sprintf(" %fdB", pegelmax);
      QMessageBox::information( this, tr("SAV Kalibrierdatei","Kalibrier-Kurvendatei laden"), 
                                      tr("<B>Die Daten wurden eingelesen und ergaenzt</B><BR><BR>","Kalibrier-Kurvendatei laden") +
                                      tr("Durchschnittlicher Pegel","Kalibrier-Kurvendatei laden") + s1 + " <BR>"+
                                      tr("Anzahl der benutzten Messpunkte","Kalibrier-Kurvendatei laden") + s2 + " <BR>"+
                                      tr("Pegelabweichung vom Durchschnitt","Kalibrier-Kurvendatei laden") + s3 + " <BR>"+
                                      tr("Pegelabweichung vom Durchschnitt","Kalibrier-Kurvendatei laden") + s4 + " <BR><BR>"+
                                      tr("Die Daten werden in die Datei \"sav.cal\" abgespeichert!","Kalibrier-Kurvendatei laden"));
      //qDebug("Pegeldurchschnitt %fdB ; Messpunkte %i; Pegelabweichung min %f; Pegelabweichung max %f", pegel_0, j, pegelmin, pegelmax);
      calixalt = 0;
      j = 1;
      pegelalt = 0.0;
      double pdelta;
      pegel = 0.0;
      for(i=0; i<w.schritte; i++){
        frequenz = double (w.anfangsfrequenz + (w.schrittfrequenz * i));
        if((frequenz >= fanfang) and (frequenz <= fende)){
          //qDebug("Frequenz %f", frequenz);
          calix = int(savcalkonst * log10(frequenz));
          //Grenzen einhalten
          if(calix < 0)calix = 0;
          if(calix > calmaxindex-1)calix = calmaxindex - 1;
          pegel = pegel + (w.mess.k1[i] * faktor1 + shift1 - pegel_0) - getkalibrierwert(frequenz);
          if(calix == calixalt){
            j++; 
          }else{
            pegel = pegel / j;
            pdelta = (pegel - pegelalt) / (calix - calixalt);
            //erster Durchlauf
            if(calixalt == 0){
              calixalt = calix;
              pegelalt = pegel;
            }
            //qDebug("calixalt %i ; calix %i; pegelalt %f; pegel %f; pegeldelta %f", calixalt, calix, pegelalt, pegel, pdelta);
            for(k=calixalt; k<=calix; k++){
              //qDebug("calindex %i; pegel %f; k-calixalt %i", k, pegelalt + pdelta * (k-calixalt), (k-calixalt));
              osavcalarray.p[k] = pegelalt + pdelta * (k-calixalt);
            }
            pegelalt = pegel;
            calixalt = calix;
            j = 1;
            pegel = 0.0;
          }
        }
      }
    }
  }
}

double odlg::getkalibrierwert(double afrequenz)
{
  double j;
  int i;
  
  //qDebug("WidgetWobbeln::getkalibrierwertk1()");
  j = round(afrequenz / eteiler);
  i = int(j);
  if(i > (maxmesspunkte - 1))i = maxmesspunkte - 1; // maximaler index 
  return okalibrierarray.arrayk1[i];
}


void odlg::setkalibrierarray(const TKalibrierarray &akalibrierarray){
  okalibrierarray = akalibrierarray;
}

void odlg::sacalreset(){
  int i;
  
  for(i=0; i < calmaxindex; i++){
    osavcalarray.p[i] = 0.0;
  }
}
