/***************************************************************************
                          nwt7linux.cpp  -  description
                             -------------------
    begin                : Don Aug  7 07:29:43 CEST 2003
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

#include <QPushButton>
#include <QMenuBar>
#include <QTimer>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QFileDialog>
#include <QPainter>
#include <QPixmap>
#include <QCheckBox>
#include <QProgressBar>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QPalette>
#include <QColor>
#include <QSettings>
#include <QSize>
#include <iostream>
#include <QScrollArea>


#include <cmath>
#include <math.h>
#include <stdlib.h>

#include "nwt7linux.h"
#include "widgetwobbeln.h"
#include "configfile.h"
#include "optiondlg.h"
#include "fmarkedlg.h"
#include "profildlg.h"
#include "firmwidget.h"
//#include "tooltip.h"
#include "sondedlg.h"
//#define LDEBUG

Nwt7linux::Nwt7linux(QWidget *parent) : QTabWidget(parent)
{

  ///////////////////////////////////////////////////////////////////////////////
  //Schnittstelle zum NWT7 erzeugen erzeugen fuer alle Widgets
  //  picmodem = new QextSerialPort("/dev/ttyS0");
  //  delete picmodem;
  picmodem = new Modem();
  //  picmodem->opentty(wgrunddaten.str_tty);
  QObject::connect( picmodem, SIGNAL(setTtyText(QString)), this, SLOT(setAText(QString)));
  bttyOpen = false;
  ///////////////////////////////////////////////////////////////////////////////
  // Wobbel Widget
  ///////////////////////////////////////////////////////////////////////////////
  // Setzen der Grunddaten fuer alle Faelle
  wobdaten.schritte = 1000;
  wobdaten.anfangsfrequenz = 1000000.0;
  wobdaten.schrittfrequenz = 29000.0;
  wobdaten.displayshift = 0; //keine Displaykorrektur
  wobdaten.swraufloesung = 4; //Aufloesung nur fuer swr
  wobdaten.penkanal1 = QPen( Qt::red, 0, Qt::SolidLine);
  wobdaten.penkanal2 = QPen( Qt::green, 0, Qt::SolidLine);
  wgrunddaten.stime = 50; 
  wgrunddaten.vtime = 50;
  wgrunddaten.idletime = 200;
  messtime = 20;
  wgrunddaten.pllmodeenable = false;
  wobdaten.mousesperre = true; //Kursoraktivitaeten sperren
  wobdaten.colorhintergrund = Qt::white;
  KursorFrequenz = 0.0;
  wobbelungueltig();
  ///////////////////////////////////////////////////////////////////////////////
  grafik = new WidgetWobbeln();
  grafik->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
  grafik->setWobbeldaten(wobdaten);
  //Fenster schon in die richtige Position bringen
  readSettings();
  grafik->hide();

  nwt7wobbeln = new QWidget();
  addTab(nwt7wobbeln, tr("Wobbeln","TabWindow"));
  //SIGNAL Grafikfenster nach Wobbelfester uebergeben
  QObject::connect( grafik, SIGNAL(wkurve_loeschen()), this, SLOT(nkurve_loeschen()));
  //SIGNAL werden Wobbeldaten an das Grafikfenster uebergeben
  QObject::connect( this, SIGNAL(sendefrqmarken(const TFrqmarken &)), grafik, SLOT(empfangfrqmarken(const TFrqmarken &)));
  
  QObject::connect( this, SIGNAL(sendewobbeldaten(const TWobbel&)), grafik, SLOT(wobbeldatenempfang(const TWobbel&)));
  QObject::connect( this, SIGNAL(sendewobbelkurve(const TMess &)), grafik, SLOT(wobbeldatenkurve(const TMess &)));
  //Signal zum Ausdrucken der Grafik im Widget Grafik
  QObject::connect( this, SIGNAL(drucken(const QFont &, const QStringList &)), grafik, SLOT(printdiagramm(const QFont &, const QStringList &)));
  //Signal zum Ausdrucken der Grafik im Widget Grafik als PDF
  QObject::connect( this, SIGNAL(druckenpdf(const QFont &, const QStringList &)), grafik, SLOT(printdiagrammpdf(const QFont &, const QStringList &)));
  //Signal zum Speichern der Grafik im Widget Grafik als PNG
  QObject::connect( this, SIGNAL(bildspeichern(const TBildinfo &, const QStringList &)), grafik, SLOT(bildspeichern(const TBildinfo &, const QStringList &)));
  
  groupwobbel = new QGroupBox(tr("Wobbeleinstellung","GroupBox"),nwt7wobbeln);
  editanfang = new QLineEdit(groupwobbel);
  //Alle Editierfenster abfragen und abgleichen
  QObject::connect( editanfang, SIGNAL(returnPressed ()), this, SLOT(wobnormalisieren()));
  editende = new QLineEdit(groupwobbel);
  QObject::connect( editende, SIGNAL(returnPressed ()), this, SLOT(wobnormalisieren()));
  editschritte = new QLineEdit(groupwobbel);
  QObject::connect( editschritte, SIGNAL(returnPressed ()), this, SLOT(wobnormalisieren()));
  
  labelzwischenzeit = new QLabel(tr("Zwischenzeit (uS)","Label"), groupwobbel);
  boxzwischenzeit = new QComboBox(groupwobbel);
  boxzwischenzeit->addItem("0");
  boxzwischenzeit->addItem("100");
  boxzwischenzeit->addItem("500");
  boxzwischenzeit->addItem("1000");
  boxzwischenzeit->addItem("2000");
  boxzwischenzeit->addItem("3000");
  boxzwischenzeit->addItem("4000");
  boxzwischenzeit->addItem("5000");
  boxzwischenzeit->addItem("6000");
  boxzwischenzeit->addItem("7000");
  boxzwischenzeit->addItem("8000");
  boxzwischenzeit->addItem("9000");
  boxzwischenzeit->addItem("9990");
  boxzwischenzeit->setCurrentIndex(0);
  QObject::connect( boxzwischenzeit, SIGNAL(currentIndexChanged(int)), this, SLOT(wobnormalisieren()));
  boxzwischenzeit->setEnabled(false);
  labelzwischenzeit->setEnabled(false);
 
  editdisplay = new QLineEdit(groupwobbel);
  QObject::connect( editdisplay, SIGNAL(returnPressed ()), this, SLOT(wobnormalisieren()));
  labeldisplayshift = new QLabel(tr("Displ.Shift","Label"), groupwobbel);
  
  boxprofil = new QComboBox(groupwobbel);
  boxprofil->setDuplicatesEnabled (false);
  QObject::connect( boxprofil, SIGNAL( activated(int)), this, SLOT(setProfil(int)));
  
  labelprofil = new QLabel(tr("Profil","label"), groupwobbel);
  int i;
  for(i=0;i<100;i++)aprofil[i].name = "NN";

  labelfrqfaktor= new QLabel(tr("Frequenzvervielfachung x 1","label"), groupwobbel); 
  
  editschrittweite = new QLineEdit(groupwobbel);
  editschrittweite->setEnabled(false);
  QObject::connect( editschrittweite, SIGNAL(returnPressed ()), this, SLOT(wobnormalisieren()));

  labelanfang = new QLabel(tr("Anfang (Hz)","Label"), groupwobbel);
  labelende = new QLabel(tr("Ende (Hz)","groupwobbel"), groupwobbel);
  labelschrittweite = new QLabel(tr("Weite (Hz)","Label"), groupwobbel);
  labelschritte = new QLabel(tr("Messpunkte","Label"), groupwobbel);
//  checkboxtime = new QCheckBox(groupwobbel);
//  checkboxtime->setText(tr("Scanzeit groesser (Sek)","CheckBox"));
//  checkboxtime->setChecked(false);
//  QObject::connect( checkboxtime, SIGNAL( stateChanged (int)), this, SLOT(checkboxtime_checked(int)));


  //  labelscantime = new QLabel(tr("Scanzeit"), groupwobbel);
  
  groupkanal = new QGroupBox(tr("Kanal","GroupBox"),nwt7wobbeln);
  checkboxk1 = new QCheckBox(groupkanal);
  checkboxk1->setText(tr("Kanal 1","CheckBox"));
  checkboxk1->setChecked(true);
  QObject::connect( checkboxk1, SIGNAL( toggled (bool)), this, SLOT(checkboxk1_checked(bool)));
  
  checkboxk2 = new QCheckBox(groupkanal);
  checkboxk2->setText(tr("Kanal 2","CheckBox"));
  checkboxk2->setChecked(true);
  QObject::connect( checkboxk2, SIGNAL( toggled (bool)), this, SLOT(checkboxk2_checked(bool)));

  groupbandbreite = new QGroupBox(tr("Bandbreite","GroupBox"),nwt7wobbeln);
  checkbox3db = new QCheckBox(groupbandbreite);
  checkbox3db->setText("3dB/Q");
  checkbox3db->setChecked(false);
  QObject::connect( checkbox3db, SIGNAL( toggled (bool)), this, SLOT(checkbox3db_checked(bool)));

  checkbox6db = new QCheckBox(groupbandbreite);
  checkbox6db->setText("6dB/60dB/Shape");
  checkbox6db->setChecked(false);
  QObject::connect( checkbox6db, SIGNAL( toggled (bool)), this, SLOT(checkbox6db_checked(bool)));

  checkboxgrafik = new QCheckBox(groupbandbreite);
  checkboxgrafik->setText(tr("Grafik-Linien","CheckBox"));
  checkboxgrafik->setChecked(false);
  QObject::connect( checkboxgrafik, SIGNAL( toggled (bool)), this, SLOT(checkboxgrafik_checked(bool)));

  checkboxinvers = new QCheckBox(groupbandbreite);
  checkboxinvers->setText(tr("Invers","CheckBox"));
  checkboxinvers->setChecked(false);
  QObject::connect( checkboxinvers, SIGNAL( toggled (bool)), this, SLOT(checkboxinvers_checked(bool)));

  groupbetriebsart = new QGroupBox(tr("Betriebsart","GroupBox"),nwt7wobbeln);
  betriebsart = new QComboBox(groupbetriebsart);
  betriebsart->addItem(tr("Wobbeln","Betriebsart"));
  betriebsart->addItem(tr("SWV","Betriebsart"));
  betriebsart->addItem(tr("SWV_ant","Betriebsart"));
  betriebsart->addItem(tr("Z-Impedanz","Betriebsart"));
  betriebsart->addItem(tr("Spektrumanalyser","Betriebsart"));
  betriebsart->addItem(tr("Spek.FRQ-shift","Betriebsart"));
  QObject::connect( betriebsart, SIGNAL( currentIndexChanged(int)), this, SLOT(setBetriebsart(int)));
  labelbetriebsart = new QLabel("",groupbetriebsart);
  
  kalibrier1 = new QCheckBox(groupbetriebsart);
  kalibrier1->setText(tr("math. Korrektur Kanal 1","Betriebsart"));
  QObject::connect( kalibrier1, SIGNAL( toggled (bool)), this, SLOT(kalibrier1_checked(bool)));
  kalibrier2 = new QCheckBox(groupbetriebsart);
  kalibrier2->setText(tr("math. Korrektur Kanal 2","Betriebsart"));
  QObject::connect( kalibrier2, SIGNAL( toggled (bool)), this, SLOT(kalibrier2_checked(bool)));

  aufloesung = new QComboBox(groupbetriebsart);
  aufloesung->addItem(tr("default","SWR"));
  aufloesung->addItem(tr("max SWV 2.0","SWR"));
  aufloesung->addItem(tr("max SWV 3.0","SWR"));
  aufloesung->addItem(tr("max SWV 4.0","SWR"));
  aufloesung->addItem(tr("max SWV 5.0","SWR"));
  aufloesung->addItem(tr("max SWV 6.0","SWR"));
  aufloesung->addItem(tr("max SWV 7.0","SWR"));
  aufloesung->addItem(tr("max SWV 8.0","SWR"));
  aufloesung->addItem(tr("max SWV 9.0","SWR"));
  aufloesung->addItem(tr("max SWV 10.0","SWR"));
  QObject::connect( aufloesung, SIGNAL( currentIndexChanged(int)), this, SLOT(setAufloesung(int)));
  labelaufloesung = new QLabel(tr("Aufloesung","Label"),groupbetriebsart);
  aufloesung->hide();
  labelaufloesung->hide();
  edita_100 = new QLineEdit(groupbetriebsart);
  QObject::connect( edita_100, SIGNAL(textChanged ( const QString & )), this, SLOT(swrantaendern()));
  edita_100->hide();
  labela_100 = new QLabel(tr("Daempfung (100m)","Label"),groupbetriebsart);
  labela_100->hide();
  editkabellaenge = new QLineEdit(groupbetriebsart);
  QObject::connect( editkabellaenge, SIGNAL(textChanged ( const QString & )), this, SLOT(swrantaendern()));
  editkabellaenge->hide();
  labelkabellaenge = new QLabel(tr("Kabellaenge (m)","Label"),groupbetriebsart);
  labelkabellaenge->hide();
  groupsa = new QGroupBox(tr("Status","GroupBox"),groupbetriebsart);
  groupsa->hide();
  labelsafrqbereich = new QLabel(tr("Scanbereich","Label"),groupsa);
  labelsafrqbereich->hide();
  labelsabandbreite = new QLabel(tr("Bandwidth:","Label"),groupsa);
  labelsabandbreite->hide();
  sabereich1 = new QRadioButton(tr("Scanbereich 1","RadioButton"),groupsa);
  QObject::connect( sabereich1, SIGNAL(clicked()), this, SLOT(setSaBereich1() ));
  sabereich1->setChecked(true);
  sabereich1->hide();
  sabereich2 = new QRadioButton(tr("Scanbereich 2","RadioButton"),groupsa);
  QObject::connect( sabereich2, SIGNAL(clicked()), this, SLOT(setSaBereich2() ));
  sabereich2->setChecked(false);
  sabereich2->hide();

  groupshift = new QGroupBox(tr("Display Y-Achse    Skala / Shift","GroupBox"),nwt7wobbeln);
  labelboxydbmax = new QLabel(tr("Ymax(dB)","Label"),groupshift);
  labelboxydbmin = new QLabel(tr("Ymin(dB)","Label"),groupshift);
  boxydbmax = new QComboBox(groupshift);
  boxydbmax->addItem("30");
  boxydbmax->addItem("20");
  boxydbmax->addItem("10");
  boxydbmax->addItem("0");
  boxydbmax->addItem("-10");
  boxydbmax->addItem("-20");
  boxydbmax->addItem("-30");
  boxydbmax->addItem("-40");
  boxydbmax->addItem("-50");
  boxydbmax->addItem("-60");
  boxydbmax->addItem("-70");
  boxydbmin = new QComboBox(groupshift);
  boxydbmin->addItem("-10");
  boxydbmin->addItem("-20");
  boxydbmin->addItem("-30");
  boxydbmin->addItem("-40");
  boxydbmin->addItem("-50");
  boxydbmin->addItem("-60");
  boxydbmin->addItem("-70");
  boxydbmin->addItem("-80");
  boxydbmin->addItem("-90");
  boxydbmin->addItem("-100");
  boxydbmin->addItem("-110");
  boxydbmin->addItem("-120");
  boxydbmax->setCurrentIndex(2);
  boxydbmin->setCurrentIndex(7);
  QObject::connect( boxydbmax, SIGNAL(currentIndexChanged (const QString)), this, SLOT(setDisplayYmax(const QString)));
  QObject::connect( boxydbmin, SIGNAL(currentIndexChanged (const QString)), this, SLOT(setDisplayYmin(const QString)));
  labelboxdbshift1 = new QLabel(tr("K1(dB)","Label"),groupshift);
  labelboxdbshift2 = new QLabel(tr("K2(dB)","Label"),groupshift);
  boxdbshift1 = new QComboBox(groupshift);
  boxdbshift1->setEditable(true);
  boxdbshift1->addItem("20");
  boxdbshift1->addItem("19");
  boxdbshift1->addItem("18");
  boxdbshift1->addItem("17");
  boxdbshift1->addItem("16");
  boxdbshift1->addItem("15");
  boxdbshift1->addItem("14");
  boxdbshift1->addItem("13");
  boxdbshift1->addItem("12");
  boxdbshift1->addItem("11");
  boxdbshift1->addItem("10");
  boxdbshift1->addItem("9");
  boxdbshift1->addItem("8");
  boxdbshift1->addItem("7");
  boxdbshift1->addItem("6");
  boxdbshift1->addItem("5");
  boxdbshift1->addItem("4");
  boxdbshift1->addItem("3");
  boxdbshift1->addItem("2");
  boxdbshift1->addItem("1");
  boxdbshift1->addItem("0");
  boxdbshift1->addItem("-1");
  boxdbshift1->addItem("-2");
  boxdbshift1->addItem("-3");
  boxdbshift1->addItem("-4");
  boxdbshift1->addItem("-5");
  boxdbshift1->addItem("-6");
  boxdbshift1->addItem("-7");
  boxdbshift1->addItem("-8");
  boxdbshift1->addItem("-9");
  boxdbshift1->addItem("-10");
  boxdbshift1->setCurrentIndex(20);
  boxdbshift2 = new QComboBox(groupshift);
  boxdbshift2->setEditable(true);
  boxdbshift2->addItem("20");
  boxdbshift2->addItem("19");
  boxdbshift2->addItem("18");
  boxdbshift2->addItem("17");
  boxdbshift2->addItem("16");
  boxdbshift2->addItem("15");
  boxdbshift2->addItem("14");
  boxdbshift2->addItem("13");
  boxdbshift2->addItem("12");
  boxdbshift2->addItem("11");
  boxdbshift2->addItem("10");
  boxdbshift2->addItem("9");
  boxdbshift2->addItem("8");
  boxdbshift2->addItem("7");
  boxdbshift2->addItem("6");
  boxdbshift2->addItem("5");
  boxdbshift2->addItem("4");
  boxdbshift2->addItem("3");
  boxdbshift2->addItem("2");
  boxdbshift2->addItem("1");
  boxdbshift2->addItem("0");
  boxdbshift2->addItem("-1");
  boxdbshift2->addItem("-2");
  boxdbshift2->addItem("-3");
  boxdbshift2->addItem("-4");
  boxdbshift2->addItem("-5");
  boxdbshift2->addItem("-6");
  boxdbshift2->addItem("-7");
  boxdbshift2->addItem("-8");
  boxdbshift2->addItem("-9");
  boxdbshift2->addItem("-10");
  boxdbshift2->setCurrentIndex(20);
  QObject::connect( boxdbshift1, SIGNAL( currentIndexChanged(int)), this, SLOT(setShift()));
  QObject::connect( boxdbshift2, SIGNAL( currentIndexChanged(int)), this, SLOT(setShift()));
  
  //  groupkursor = new QGroupBox(tr("Kursor-Nummer","GroupBox"),nwt7wobbeln);
  labelkursornr = new QLabel(tr("Kursor #","Label"),groupshift);
  boxkursornr = new QComboBox(groupshift);
  boxkursornr->addItem("5");
  boxkursornr->addItem("4");
  boxkursornr->addItem("3");
  boxkursornr->addItem("2");
  boxkursornr->addItem("1");
  boxkursornr->setCurrentIndex(4);
  QObject::connect( boxkursornr, SIGNAL( currentIndexChanged(int)), grafik, SLOT(setWKursorNr(int)));
  QObject::connect( grafik, SIGNAL( resetKursor(int)), this, SLOT(resetKursor(int)));
  
  groupdaempfung = new QGroupBox(tr("Daempfung","GroupBox"),nwt7wobbeln);
  daempfung1 = new QComboBox(groupdaempfung);
  QObject::connect( daempfung1, SIGNAL( currentIndexChanged(int)), this, SLOT(setDaempfungWob(int)));
  labeldaempfung = new QLabel(tr("0db/-50dB","Label"),groupdaempfung);
  
  wobdaten.bandbreite3db=false;
  wobdaten.bandbreite6db=false;

  if(checkboxk1->isChecked())wobdaten.bkanal1=true;
  if(checkboxk2->isChecked())wobdaten.bkanal2=true;
  if(checkbox3db->isChecked())wobdaten.bandbreite3db=true;
  if(checkbox6db->isChecked())wobdaten.bandbreite6db=true;

  buttonwobbeln = new QPushButton(tr("Wobbeln","Button"), nwt7wobbeln);
  buttonwobbeln->setEnabled(true);
  QObject::connect( buttonwobbeln, SIGNAL( clicked()), this, SLOT(clickwobbeln()));
  QObject::connect( grafik, SIGNAL( wobbelndauer()), this, SLOT(clickwobbeln()));
  
  buttoneinmal = new QPushButton(tr("Einmal","Button"), nwt7wobbeln);
  buttoneinmal->setEnabled(true);
  QObject::connect( buttoneinmal, SIGNAL( clicked()), this, SLOT(clickwobbelneinmal()));
  QObject::connect( grafik, SIGNAL( wobbelneinmal()), this, SLOT(clickwobbelneinmal()));
  
  buttonstop = new QPushButton(tr("Stop","Button"), nwt7wobbeln);
  buttonstop->setEnabled(false);
  QObject::connect( buttonstop, SIGNAL( clicked()), this, SLOT(clickwobbelnstop()));
  QObject::connect( grafik, SIGNAL( wobbelnstop()), this, SLOT(clickwobbelnstop()));
  
  groupzoom = new QGroupBox(tr("Frequenzzoom","groupbox"),nwt7wobbeln);
  labellupe = new QLabel(tr("2-fach +/-","Label"),groupzoom);
  labellupe->setEnabled(false);
  buttonlupeplus = new QPushButton("+", groupzoom);
  buttonlupeplus->setEnabled(false);
  QObject::connect( buttonlupeplus, SIGNAL( clicked()), this, SLOT(clicklupeplus()));
  QObject::connect( grafik, SIGNAL( lupeplus()), this, SLOT(clicklupeplus()));
  buttonlupeminus = new QPushButton("-", groupzoom);
  buttonlupeminus->setEnabled(false);
  QObject::connect( buttonlupeminus, SIGNAL( clicked()), this, SLOT(clicklupeminus()));
  buttonlupemitte = new QPushButton("^", groupzoom);
  buttonlupemitte->setEnabled(false);
  QObject::connect( buttonlupemitte, SIGNAL( clicked()), this, SLOT(clicklupegleich()));
  QObject::connect( this, SIGNAL( cursormitte()), grafik, SLOT(setmousecursormitte()));
  
  QObject::connect( grafik, SIGNAL( lupeminus()), this, SLOT(clicklupeminus()));
  QObject::connect( grafik, SIGNAL( lupegleich()), this, SLOT(clicklupegleich()));
  
  //groupbar = new QGroupBox("NWT",nwt7wobbeln);
  progressbar = new QProgressBar(groupshift);
  progressbar->setTextVisible(false);
  labelprogressbar = new QLabel(tr("Datenrueckfluss","Label"),groupshift);
  labelnwt = new QLabel("Offline",groupshift);

  mledit = new QTextEdit(nwt7wobbeln);
  //mledit->setReadOnly(true);
  //Signal vom Widget Grafik Loeschen des Multieditfenster
  QObject::connect( grafik, SIGNAL( multieditloeschen()), this, SLOT(mleditloeschen()));
  //Signal vom Widget Grafik Einfuegen von Text ins Multieditfenster
  QObject::connect( grafik, SIGNAL( multieditinsert(const QString &)), this, SLOT(mleditinsert(const QString &)));
  //mousezeiger bestimmt neue Anfangsfrequenz
  QObject::connect( grafik, SIGNAL( writeanfang(double)), this, SLOT(setanfang(double)));
  //mousezeiger bestimmt neue Endfrequenz
  QObject::connect( grafik, SIGNAL( writeende(double)), this, SLOT(setende(double)));
  QObject::connect( grafik, SIGNAL( writeberechnung(double)), this, SLOT(readberechnung(double)));
  QObject::connect( grafik, SIGNAL( setKursorFrequenz(double)), this, SLOT(setKursorFrequenz(double)));
  
  wobbeltimer = new QTimer(this);
  wobbeltimer->setSingleShot(true);
  //Signal vom Wobbeltimer (Auslesen der RS232)
  QObject::connect(wobbeltimer, SIGNAL(timeout()), this, SLOT(readtty() ));
  
  idletimer = new QTimer(this);
  //Kontaktaufnahme zur HW
  QObject::connect( idletimer, SIGNAL(timeout()), this, SLOT(idletimerbehandlung() ));
  
  vtimer = new QTimer(this);
  //(Auslesen der RS232) FW-Version
  QObject::connect(vtimer, SIGNAL(timeout()), this, SLOT(vtimerbehandlung() ));
  
  stimer = new QTimer(this);
  //(Auslesen der RS232) Status
  QObject::connect(stimer, SIGNAL(timeout()), this, SLOT(stimerbehandlung() ));
  
  wkmanager = new owkmanager();
  addTab(wkmanager, tr("Wobbelkurvenmanager","TabWindow"));
  //Verbindung vom Wobbelmanager zum Grafikfenster fuer die Wobbeldaten
  QObject::connect(wkmanager, SIGNAL(wkmsendwobbel(const TWobbel &)), grafik, SLOT(setKurve(const TWobbel &)));
  //verbindung zum holen der Wobbeldaten aus dem Hauptfenster zum Wobbelmanager
  QObject::connect(wkmanager, SIGNAL(loadkurve(int)), this, SLOT(wkmloadkurve(int)));
  QObject::connect(wkmanager, SIGNAL(savekurve(const TWobbel&)), this, SLOT(wkmsavekurve(const TWobbel&)));
  QObject::connect(wkmanager, SIGNAL(wkmgetwobbel(int)), this, SLOT(getwkm(int)));
//  connect(wkmanager, SIGNAL(setkurvendir(QString)), this, SLOT(getkdir(QString)));
//  connect(wkmanager, SIGNAL(warneichkorr()), this, SLOT(warneichkorr()));
  
  ///////////////////////////////////////////////////////////////////////////////
  // VFO Widget
  ///////////////////////////////////////////////////////////////////////////////
  QPalette dp;
  dp.setColor(QPalette::Background, Qt::yellow);
  QPalette dpd;
  dpd.setColor(QPalette::Background, Qt::darkYellow);

  lcdauswahl = 1;
  nwt7vfo = new QWidget();
  addTab(nwt7vfo, tr("VFO", "TabWindow"));
  
  QFont font("Helvetica", 16, QFont::Bold);
  
  vsp1hz = new QSpinBox(nwt7vfo);
  vsp1hz->setRange(-1, 10);
  vsp1hz->setFont(font);  
  QObject::connect( vsp1hz, SIGNAL( valueChanged(int)), this, SLOT(change1hz(int)));
  vsp10hz = new QSpinBox(nwt7vfo);
  vsp10hz->setFont(font);  
  vsp10hz->setRange(-1, 10);
  QObject::connect( vsp10hz, SIGNAL( valueChanged(int)), this, SLOT(change10hz(int)));
  vsp100hz = new QSpinBox(nwt7vfo);
  vsp100hz->setRange(-1, 10);
  vsp100hz->setFont(font);  
  QObject::connect( vsp100hz, SIGNAL( valueChanged(int)), this, SLOT(change100hz(int)));
  vsp1khz = new QSpinBox(nwt7vfo);
  vsp1khz->setRange(-1, 10);
  vsp1khz->setFont(font);  
  QObject::connect( vsp1khz, SIGNAL( valueChanged(int)), this, SLOT(change1khz(int)));
  vsp10khz = new QSpinBox(nwt7vfo);
  vsp10khz->setRange(-1, 10);
  vsp10khz->setFont(font);  
  QObject::connect( vsp10khz, SIGNAL( valueChanged(int)), this, SLOT(change10khz(int)));
  vsp100khz = new QSpinBox(nwt7vfo);
  vsp100khz->setRange(-1, 10);
  vsp100khz->setFont(font);  
  QObject::connect( vsp100khz, SIGNAL( valueChanged(int)), this, SLOT(change100khz(int)));
  vsp1mhz = new QSpinBox(nwt7vfo);
  vsp1mhz->setRange(-1, 10);
  vsp1mhz->setFont(font);  
  QObject::connect( vsp1mhz, SIGNAL( valueChanged(int)), this, SLOT(change1mhz(int)));
  vsp10mhz = new QSpinBox(nwt7vfo);
  vsp10mhz->setRange(-1, 10);
  vsp10mhz->setFont(font);  
  QObject::connect( vsp10mhz, SIGNAL( valueChanged(int)), this, SLOT(change10mhz(int)));
  vsp100mhz = new QSpinBox(nwt7vfo);
  vsp100mhz->setRange(-1, 10);
  vsp100mhz->setFont(font);  
  QObject::connect( vsp100mhz, SIGNAL( valueChanged(int)), this, SLOT(change100mhz(int)));
  vsp1ghz = new QSpinBox(nwt7vfo);
  vsp1ghz->setRange(-1, 10);
  vsp1ghz->setFont(font);  
  QObject::connect( vsp1ghz, SIGNAL( valueChanged(int)), this, SLOT(change1ghz(int)));
  
  editvfo = new QLineEdit(nwt7vfo);
  QObject::connect( editvfo, SIGNAL(returnPressed ()), this, SLOT(vfoedit()));
  labelvfo = new QLabel(tr("in Hz"),nwt7vfo);

  lmhz = new QLabel("MHz",nwt7vfo);
  lmhz->setFont(font);  
  lkhz = new QLabel("kHz",nwt7vfo);
  lkhz->setFont(font);  
  lhz = new QLabel("Hz",nwt7vfo);
  lhz->setFont(font);  
  
  labelfrqfaktorv= new QLabel(tr("Frequenzvervielfachung x 1","label"), nwt7vfo); 

  //LCD Anzeige 10 stellen mit punkt
  LCD1 = new QLCDNumber(10, nwt7vfo);
  //Hintergrung gelb
  LCD1->setPalette(dp);
  LCD1->setAutoFillBackground(true);
  LCD1->setBackgroundRole( QPalette::Background );
  
  //volle schwarze Zahlen
  LCD1->setSegmentStyle(QLCDNumber::Filled);
  //der Dezimalpunkt soll ganz wenig platz beanspruchen
  LCD1->setSmallDecimalPoint(true);
  rb1 = new QRadioButton(nwt7vfo);
  rb1->setChecked(true);
  QObject::connect( rb1, SIGNAL(clicked()), this, SLOT(lcd1clicked() ));
  //LCD Anzeige 10 stellen mit punkt
  LCD2 = new QLCDNumber(10, nwt7vfo);
  //Hintergrung gelb
  LCD2->setPalette(dpd);
  LCD2->setAutoFillBackground(true);
  LCD2->setBackgroundRole( QPalette::Background );
  //volle schwarze Zahlen
  LCD2->setSegmentStyle(QLCDNumber::Filled);
  //der Dezimalpunkt soll ganz wenig platz beanspruchen
  LCD2->setSmallDecimalPoint(true);
  rb2 = new QRadioButton(nwt7vfo);
  QObject::connect( rb2, SIGNAL(clicked()), this, SLOT(lcd2clicked() ));

  //LCD Anzeige 10 stellen mit punkt
  LCD3 = new QLCDNumber(10, nwt7vfo);
  //Hintergrung gelb
  LCD3->setPalette(dpd);
  LCD3->setAutoFillBackground(true);
  LCD3->setBackgroundRole( QPalette::Background );
  //volle schwarze Zahlen
  LCD3->setSegmentStyle(QLCDNumber::Filled);
  //der Dezimalpunkt soll ganz wenig platz beanspruchen
  LCD3->setSmallDecimalPoint(true);
  rb3 = new QRadioButton(nwt7vfo);
  QObject::connect( rb3, SIGNAL(clicked()), this, SLOT(lcd3clicked() ));

  //LCD Anzeige 10 stellen mit punkt
  LCD4 = new QLCDNumber(10, nwt7vfo);
  //Hintergrung gelb
  LCD4->setPalette(dpd);
  LCD4->setAutoFillBackground(true);
  LCD4->setBackgroundRole( QPalette::Background );
  //volle schwarze Zahlen
  LCD4->setSegmentStyle(QLCDNumber::Filled);
  //der Dezimalpunkt soll ganz wenig platz beanspruchen
  LCD4->setSmallDecimalPoint(true);
  rb4 = new QRadioButton(nwt7vfo);
  QObject::connect( rb4, SIGNAL(clicked()), this, SLOT(lcd4clicked() ));

  //LCD Anzeige 10 stellen mit punkt
  LCD5 = new QLCDNumber(10, nwt7vfo);
  //Hintergrung gelb
  LCD5->setPalette(dpd);
  LCD5->setAutoFillBackground(true);
  LCD5->setBackgroundRole( QPalette::Background );
  //volle schwarze Zahlen
  LCD5->setSegmentStyle(QLCDNumber::Filled);
  //der Dezimalpunkt soll ganz wenig platz beanspruchen
  LCD5->setSmallDecimalPoint(true);
  rb5 = new QRadioButton(nwt7vfo);
  QObject::connect( rb5, SIGNAL(clicked()), this, SLOT(lcd5clicked() ));

  editzf = new QLineEdit(nwt7vfo);
  QObject::connect( editzf, SIGNAL(returnPressed()), this, SLOT(setVfo() ));
  labelzf = new QLabel(nwt7vfo);
  labelzf->setText(tr("ZF"));
  labelhz = new QLabel(nwt7vfo);
  labelhz->setText("Hz");
  checkboxzf = new QCheckBox(nwt7vfo);
  checkboxzf->setText(tr("ZF fuer das Wobbeln aktivieren","im VFO"));
  checkboxzf->setChecked(false);
  checkboxiqvfo = new QCheckBox(nwt7vfo);
  checkboxiqvfo->setText(tr("VFO-Frequenz x 4 fuer I/Q Mischer","im VFO"));
  checkboxiqvfo->setChecked(false);
  QObject::connect( checkboxiqvfo, SIGNAL(clicked()), this, SLOT(setIQVFO() ));
  
  daempfung2 = new QComboBox(nwt7vfo);
  QObject::connect( daempfung2, SIGNAL( currentIndexChanged(int)), this, SLOT(setDaempfungVfo(int)));
  labeldaempfung1 = new QLabel(tr("Daempfungsglied","im VFO"),nwt7vfo);

  vfotimer = new QTimer(this);
  vfotimer->setSingleShot(true);
  //nach ablauf des vfotimers Daten an NWT7 senden (LCD daten)
  QObject::connect( vfotimer, SIGNAL(timeout()), this, SLOT(senddaten() ));

  //erste Diplayeinstellung
  LCDaendern();
  ///////////////////////////////////////////////////////////////////////////////
  // Mess Widget
  nwt7messen = new QWidget();
  addTab(nwt7messen, tr("Wattmeter","im Wattmeter"));

  progressbarmessk1 = new QProgressBar(nwt7messen);
  progressbarmessk1->setFormat("");
  mlabelk1 = new QLabel(nwt7messen);
  mlabelk1->setText("0");
  progressbarmessk2 = new QProgressBar(nwt7messen);
  progressbarmessk2->setFormat("");
  mlabelk2 = new QLabel(nwt7messen);
  mlabelk2->setText("0");

  ldaempfungk1 = new QComboBox(nwt7messen);
  ldaempfungk1->setEditable(true);
  labelldaempfungk1 = new QLabel(tr("Daempfung (dB)","im Wattmeter"),nwt7messen);

  ldaempfungk2 = new QComboBox(nwt7messen);
  ldaempfungk2->setEditable(true);
  labelldaempfungk2 = new QLabel(tr("Daempfung (dB)","im Wattmeter"),nwt7messen);

  messtimer = new QTimer(this);
  messlabel1 = new QLabel(nwt7messen);
  messlabel2 = new QLabel(nwt7messen);
  messlabel3 = new QLabel(nwt7messen);
  messlabel4 = new QLabel(nwt7messen);
  messlabel5 = new QLabel(nwt7messen);
  messlabel6 = new QLabel(nwt7messen);
  messedit = new QTextEdit(nwt7messen);
  //Schrifttyp einstellen
  messedit->setFontFamily("courier");
  buttonmess = new QPushButton(tr("Uebernahme","im Wattmeter"), nwt7messen);
  QObject::connect( buttonmess, SIGNAL( clicked()), this, SLOT(tabelleschreiben()));
  buttonmesssave = new QPushButton(tr("Speichern","im Wattmeter"), nwt7messen);
  QObject::connect( buttonmesssave, SIGNAL( clicked()), this, SLOT(tabellespeichern()));
  //nach ablauf des messtimers Daten vom nwt7 holen
  QObject::connect( messtimer, SIGNAL(timeout()), this, SLOT(getmessdaten() ));
  messcounter = 0;
  messungk1 = 0.0;
  messungk2 = 0.0;
  messungk1alt = 0.0;
  messungk2alt = 0.0;
  boxwattoffset1 = new QComboBox(nwt7messen);
  boxwattoffset2 = new QComboBox(nwt7messen);
  QString snum;
  for(i=0;i<30;i++)
  {
    wattoffsetk1[i].str_offsetwatt = "NN";
    wattoffsetk1[i].offsetwatt = 0.0;
    snum.sprintf("%2i| ",i+1);
    boxwattoffset1->addItem(snum + wattoffsetk1[i].str_offsetwatt);
    wattoffsetk2[i].str_offsetwatt = "NN";
    wattoffsetk2[i].offsetwatt = 0.0;
    boxwattoffset2->addItem(snum + wattoffsetk2[i].str_offsetwatt);
  }
  woffset1 = 0.0;
  woffset2 = 0.0;
  
  buttonvfo = new QPushButton(tr("VFO on/off","im Wattmeter"), nwt7messen);
  buttonwobbeln->setEnabled(true);
  QObject::connect( buttonvfo, SIGNAL( clicked()), this, SLOT(vfoum()));

  labelfrqfaktorm= new QLabel(tr("Frequenzvervielfachung x 1","label"), nwt7messen); 

  sp1hz = new QSpinBox(nwt7messen);
  sp1hz->setFont(font);  
  sp1hz->setRange(-1, 10);
  sp1hz->hide();
  QObject::connect( sp1hz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp10hz = new QSpinBox(nwt7messen);
  sp10hz->setFont(font);  
  sp10hz->setRange(-1, 10);
  sp10hz->hide();
  QObject::connect( sp10hz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp100hz = new QSpinBox(nwt7messen);
  sp100hz->setFont(font);  
  sp100hz->setRange(-1, 10);
  sp100hz->hide();
  QObject::connect( sp100hz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp1khz = new QSpinBox(nwt7messen);
  sp1khz->setFont(font);  
  sp1khz->setRange(-1, 10);
  sp1khz->hide();
  QObject::connect( sp1khz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp10khz = new QSpinBox(nwt7messen);
  sp10khz->setFont(font);  
  sp10khz->setRange(-1, 10);
  sp10khz->hide();
  QObject::connect( sp10khz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp100khz = new QSpinBox(nwt7messen);
  sp100khz->setFont(font);  
  sp100khz->setRange(-1, 10);
  sp100khz->hide();
  QObject::connect( sp100khz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp1mhz = new QSpinBox(nwt7messen);
  sp1mhz->setFont(font);  
  sp1mhz->setRange(-1, 10);
  sp1mhz->hide();
  QObject::connect( sp1mhz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp10mhz = new QSpinBox(nwt7messen);
  sp10mhz->setFont(font);  
  sp10mhz->setRange(-1, 10);
  sp10mhz->hide();
  QObject::connect( sp10mhz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp100mhz = new QSpinBox(nwt7messen);
  sp100mhz->setFont(font);  
  sp100mhz->setRange(-1, 10);
  sp100mhz->hide();
  QObject::connect( sp100mhz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  sp1ghz = new QSpinBox(nwt7messen);
  sp1ghz->setFont(font);  
  sp1ghz->setRange(-1, 9);
  sp1ghz->hide();
  QObject::connect( sp1ghz, SIGNAL( valueChanged(int)), this, SLOT(setmessvfo()));
  labelsphz = new QLabel("Hz", nwt7messen);
  labelsphz->setFont(font);  
  labelspkhz = new QLabel("kHz", nwt7messen);
  labelspkhz->setFont(font);  
  labelspmhz = new QLabel("MHz", nwt7messen);
  labelspmhz->setFont(font);  
//  labelspghz = new QLabel("GHz", nwt7messen);
//  labelspghz->setFont(font);  
  labelsphz->hide();
  labelspkhz->hide();
  labelspmhz->hide();
//  labelspghz->hide();
  daempfung3 = new QComboBox(nwt7messen);
  QObject::connect( daempfung3, SIGNAL( currentIndexChanged(int)), this, SLOT(setDaempfungWatt(int)));
  labeldaempfung3 = new QLabel(tr("Daempfung","im Wattmeter"),nwt7messen);
  daempfung3->hide();
  labeldaempfung3->hide();

  messvfotimer = new QTimer(this);
  messvfotimer->setSingleShot(true);
  //Signal vom Wobbeltimer (Auslesen der RS232)
  QObject::connect(messvfotimer, SIGNAL(timeout()), this, SLOT(messvfotimerende() ));

  checkboxmesshang1 = new QCheckBox(nwt7messen);
  checkboxmesshang1->setText(tr("Hang","CheckBox Wattmeter"));
  checkboxmesshang1->setChecked(true);
  
  checkboxmesshang2 = new QCheckBox(nwt7messen);
  checkboxmesshang2->setText(tr("Hang","CheckBox Wattmeter"));
  checkboxmesshang2->setChecked(true);

  // Mess Widget
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  // Berechnungen Widget
  berechnung = new QWidget();
  addTab(berechnung, tr("Berechnungen","in Berechnungen"));
  groupschwingkreisc = new QGroupBox(tr("Schwingkreisberechnung Kapazitaet","in Berechnungen"),berechnung);
  editf2 = new QLineEdit(groupschwingkreisc);
  QObject::connect( editf2, SIGNAL(textChanged(QString)), this, SLOT(schwingkreisfc()));
  editl2 = new QLineEdit(groupschwingkreisc);
  QObject::connect( editl2, SIGNAL(textChanged(QString)), this, SLOT(schwingkreisfc()));
  ergebnisc = new QLabel("", groupschwingkreisc);
  labell2 = new QLabel(tr("uH Induktivitaet","in Berechnungen"), groupschwingkreisc);
  labelf2 = new QLabel(tr("MHz Frequenz","in Berechnungen"), groupschwingkreisc);
  labelc2 = new QLabel(tr("pF Kapaziaet","in Berechnungen"), groupschwingkreisc);
  ergebnisxc = new QLabel("", groupschwingkreisc);
  labelxc = new QLabel("Ohm XC", groupschwingkreisc);
  
  groupschwingkreisl = new QGroupBox(tr("Schwingkreisber. Ind. / AL-Wert","in Berechnungen"),berechnung);
  editf1 = new QLineEdit(groupschwingkreisl);
  QObject::connect( editf1, SIGNAL(textChanged(QString)), this, SLOT(schwingkreisfl()));
  editc1 = new QLineEdit(groupschwingkreisl);
  QObject::connect( editc1, SIGNAL(textChanged(QString)), this, SLOT(schwingkreisfl()));
  editn = new QLineEdit(groupschwingkreisl);
  QObject::connect( editn, SIGNAL(textChanged(QString)), this, SLOT(schwingkreisfl()));
  ergebnisl = new QLabel("", groupschwingkreisl);
  ergebnisal = new QLabel("", groupschwingkreisl);
  labelf1 = new QLabel(tr("MHz Frequenz","in Berechnungen"), groupschwingkreisl);
  labell1 = new QLabel(tr("uH Induktivitaet","in Berechnungen"), groupschwingkreisl);
  labelc1 = new QLabel(tr("pF Kapaziaet","in Berechnungen"), groupschwingkreisl);
  labeln = new QLabel(tr("N Windungen","in Berechnungen"), groupschwingkreisl);
  labelal = new QLabel(tr("nH/N2 AL-Wert","in Berechnungen"), groupschwingkreisl);
  ergebnisxl = new QLabel("", groupschwingkreisl);
  labelxl = new QLabel("Ohm XL", groupschwingkreisl);
  
  groupwindungen = new QGroupBox(tr("Windungen berechnen aus AL-Wert","in Berechnungen"),berechnung);
  edital3 = new QLineEdit(groupwindungen);
  QObject::connect( edital3, SIGNAL(textChanged(QString)), this, SLOT(alwindungen()));
  labeledital3 = new QLabel(tr("nH/N2 AL-Wert","in Berechnungen"), groupwindungen);
  editl3 = new QLineEdit(groupwindungen);
  QObject::connect( editl3, SIGNAL(textChanged(QString)), this, SLOT(alwindungen()));
  labeleditl3 = new QLabel(tr("uH Induktivitaet","in Berechnungen"), groupwindungen);
  ergebnisw = new QLabel("", groupwindungen);
  labelergebnisw = new QLabel(tr("N Windungen","in Berechnungen"), groupwindungen);
  // Berechnungen Widget
  ///////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////
  // Impedanz Widget
  wimpedanz = new QWidget();
  addTab(wimpedanz, tr("Impedanzanpassung","in Impedanz"));
  
  gimp = new QGroupBox(tr("Anpassung mit:","in Impedanz"),wimpedanz);
  rbr = new QRadioButton(tr("R","RadioButton"),gimp);
  QObject::connect( rbr, SIGNAL(clicked()), this, SLOT(setimp() ));
  rbr->setChecked(true);
  rblc = new QRadioButton(tr("L/C","RadioButton"),gimp);
  QObject::connect( rblc, SIGNAL(clicked()), this, SLOT(setimp() ));

  
  gzr = new QGroupBox(tr("Anpassung mit R","in Impedanz"),wimpedanz);
  editz1 = new QLineEdit(gzr);
  QObject::connect( editz1, SIGNAL(textChanged(QString)), this, SLOT(zrausrechnen()));
  labeleditz1 = new QLabel(tr("(Ohm) Z1","in Impedanz"), gzr);
  editz2 = new QLineEdit(gzr);
  QObject::connect( editz2, SIGNAL(textChanged(QString)), this, SLOT(zrausrechnen()));
  labeleditz2 = new QLabel(tr("(Ohm) Z2","in Impedanz"), gzr);
  editz3 = new QLineEdit(gzr);
  QObject::connect( editz3, SIGNAL(textChanged(QString)), this, SLOT(zrausrechnen()));
  labeleditz3 = new QLabel(tr("(Ohm) Z3","in Impedanz"), gzr);
  editz4 = new QLineEdit(gzr);
  QObject::connect( editz4, SIGNAL(textChanged(QString)), this, SLOT(zrausrechnen()));
  labeleditz4 = new QLabel(tr("(Ohm) Z4","in Impedanz"), gzr);
  lr1 = new QLabel(tr("xxx","in Impedanz"), gzr);
  lr1->setAlignment(Qt::AlignRight);
  lr2 = new QLabel(tr("xxx","in Impedanz"), gzr);
  lr2->setAlignment(Qt::AlignRight);
  lr3 = new QLabel(tr("xxx","in Impedanz"), gzr);
  lr3->setAlignment(Qt::AlignRight);
  lr4 = new QLabel(tr("xxx","in Impedanz"), gzr);
  lr4->setAlignment(Qt::AlignRight);
  ldaempfung = new QLabel(tr("xxx","in Impedanz"), gzr);
  ldaempfung->setAlignment(Qt::AlignRight);
  lbeschrr1 = new QLabel(tr("(Ohm) R1","in Impedanz"), gzr);
  lbeschrr2 = new QLabel(tr("(Ohm) R2","in Impedanz"), gzr);
  lbeschrr3 = new QLabel(tr("(Ohm) R3","in Impedanz"), gzr);
  lbeschrr4 = new QLabel(tr("(Ohm) R4","in Impedanz"), gzr);
  lbeschrdaempfung = new QLabel(tr("(dB) Einfuegedaempfung","in Impedanz"), gzr);
  bild1 = new QLabel(wimpedanz);
  bild1->setPixmap(QPixmap::fromImage(QImage(":/images/zanp1.png")));
  
  gzlc = new QGroupBox(tr("Anpassung mit LC","in Impedanz"),wimpedanz);
  gzlc->hide();
  editzlc1 = new QLineEdit(gzlc);
  QObject::connect( editzlc1, SIGNAL(textChanged(QString)), this, SLOT(zlcausrechnen()));
  labeleditzlc1 = new QLabel(tr("(Ohm) Z1","in Impedanz"), gzlc);
  editzlc2 = new QLineEdit(gzlc);
  QObject::connect( editzlc2, SIGNAL(textChanged(QString)), this, SLOT(zlcausrechnen()));
  labeleditzlc2 = new QLabel(tr("(Ohm) Z2","in Impedanz"), gzlc);
  editzlc3 = new QLineEdit(gzlc);
  QObject::connect( editzlc3, SIGNAL(textChanged(QString)), this, SLOT(zlcausrechnen()));
  labeleditzlc3 = new QLabel(tr("(MHz) Frequenz","in Impedanz"), gzlc);
  ll = new QLabel(tr("xxx","in Impedanz"), gzlc);
  ll->setAlignment(Qt::AlignRight);
  lc = new QLabel(tr("xxx","in Impedanz"), gzlc);
  lc->setAlignment(Qt::AlignRight);
  lbeschrl = new QLabel(tr("(uH) L","in Impedanz"), gzlc);
  lbeschrc = new QLabel(tr("(pF) C","in Impedanz"), gzlc);
  bild2 = new QLabel(wimpedanz);
  bild2->setPixmap(QPixmap::fromImage(QImage(":/images/zanp2.png")));
  bild2->hide();
  //  scaleFactor = 1.0;
  // Impedanz Widget Ende
  ///////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////
  // Widget Diagramm
  wdiagramm = new QWidget();

  //addTab(wdiagramm, tr("Antennendiagramm"));
  wdiagramm->hide();//vorerst nicht sichtbar

  gdiagramm = new QGroupBox(tr("Antennendiagramm"), wdiagramm);
  bstart = new QPushButton(tr("Start"),gdiagramm);
  //QObject::connect( bstart, SIGNAL( clicked()), this, SLOT(diagrammsimulieren()));
  QObject::connect( bstart, SIGNAL( clicked()), this, SLOT(diagrammstart()));
  bstop = new QPushButton(tr("Stop"),gdiagramm);
  bstop->setEnabled(false);
  QObject::connect( bstop, SIGNAL( clicked()), this, SLOT(diagrammstop()));
  bspeichern = new QPushButton(tr("CSV Speichern"),gdiagramm);
  QObject::connect( bspeichern, SIGNAL( clicked()), this, SLOT(diagrammspeichern()));

  bsim = new QPushButton(tr("Simmulation"),gdiagramm);
  QObject::connect( bsim, SIGNAL( clicked()), this, SLOT(diagrammsimulieren()));

  lanzeige = new QLabel("xxx", gdiagramm);
  tdiagramm = new QTimer(this);
  tdiagramm->setSingleShot(true);
  QObject::connect( tdiagramm, SIGNAL(timeout()), this, SLOT(tdiagrammbehandlung() ));
  tdiagramm->stop();
  gdiagrammdim = new QGroupBox(tr("Dimension"), wdiagramm);
  boxdbmax = new QComboBox(gdiagrammdim);
  boxdbmax->addItem("10 dB");
  boxdbmax->addItem("0 dB");
  boxdbmax->addItem("-10 dB");
  boxdbmax->addItem("-20 dB");
  boxdbmax->addItem("-30 dB");
  boxdbmax->addItem("-40 dB");
  boxdbmax->addItem("-50 dB");
  boxdbmax->addItem("-70 dB");
  boxdbmax->addItem("-60 dB");
  boxdbmax->addItem("-80 dB");
  boxdbmin = new QComboBox(gdiagrammdim);
  boxdbmin->addItem("10 dB");
  boxdbmin->addItem("0 dB");
  boxdbmin->addItem("-10 dB");
  boxdbmin->addItem("-20 dB");
  boxdbmin->addItem("-30 dB");
  boxdbmin->addItem("-40 dB");
  boxdbmin->addItem("-50 dB");
  boxdbmin->addItem("-70 dB");
  boxdbmin->addItem("-60 dB");
  boxdbmin->addItem("-80 dB");
  ldbmax = new QLabel(tr("max"), gdiagrammdim);
  ldbmin = new QLabel(tr("min"), gdiagrammdim);
  boxdbmax->setCurrentIndex(1);
  boxdbmin->setCurrentIndex(4);

  spinadbegin = new QDoubleSpinBox(gdiagrammdim);
  spinadbegin->setRange(0.0,2.0);
  spinadbegin->setSingleStep(0.01);
  spinadbegin->setValue(0.0);
  spinadend = new QDoubleSpinBox(gdiagrammdim);
  spinadend->setRange(3.0,5.0);
  spinadend->setSingleStep(0.01);
  spinadend->setValue(5.0);
  spingradbegin = new QSpinBox(gdiagrammdim);
  spingradbegin->setRange(0,180);
  spingradbegin->setValue(0);
  spingradend = new QSpinBox(gdiagrammdim);
  spingradend->setRange(180,360);
  spingradend->setValue(360);
  lbegin = new QLabel(tr("Beginn(V)"), gdiagrammdim);
  lbegin1 = new QLabel(tr("Grad"), gdiagrammdim);
  lend = new QLabel(tr("Ende  (V)"), gdiagrammdim);
  lend1 = new QLabel(tr("Grad"), gdiagrammdim);

  QObject::connect( boxdbmax, SIGNAL( currentIndexChanged(int)), this, SLOT(diagrammdim(int)));
  QObject::connect( boxdbmin, SIGNAL( currentIndexChanged(int)), this, SLOT(diagrammdim(int)));
  QObject::connect(spinadbegin, SIGNAL( valueChanged(double)), this, SLOT(diagrammdimd(double)));
  QObject::connect(spinadend, SIGNAL( valueChanged(double)), this, SLOT(diagrammdimd(double)));
  QObject::connect(spingradbegin, SIGNAL( valueChanged(int)), this, SLOT(diagrammdim(int)));
  QObject::connect(spingradend, SIGNAL( valueChanged(int)), this, SLOT(diagrammdim(int)));

  // Diagramm Widget Ende
  ///////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////
  // Grafischen elemente anordnen
  grafiksetzen();

  //eichkorrektur auf 0.0 setzen
  for(i=0;i<maxmesspunkte;i++)
  {
    wobdaten.kalibrierarray.arrayk1[i] = 0.0;
    wobdaten.kalibrierarray.arrayk2[i] = 0.0;
    wobdaten.swrkalibrierarray.arrayk1[i] = -10.0;
    wobdaten.swrkalibrierarray.arrayk2[i] = -10.0;
  }
  //Korrekturwerte vom SAV TP und BP auf 0,0dB setzen
  for(i=0;i< calmaxindex;i++)
  {
    wobdaten.savcalarray.p[i] = 0.0;
  }
  //fuer die Lupenfunktion
  bdauerwobbeln = false;
  //fuer das Wattmeter zur Spitzenanzeige
  counterspitzek1 = 0;
  counterspitzek2 = 0;
  emit sendewobbeldaten(wobdaten);
  //keine FW-Version
  fwversion = 0;
  //keine Zwischenzeit da FW noch nicht bekannt
  boxzwischenzeit->setCurrentIndex(0);
  //diese Box erst freischalten wenn richtige FW erkannt wird
  boxzwischenzeit->setEnabled(false);
  labelzwischenzeit->setEnabled(false);
  vabfrage = true;
  //FW ist noch unbekannt
  wgrunddaten.fwfalsch = true;
  //Tabs ausblenden wenn noch keine Verbindung zum NWT besteht
  setTabEnabled(nwt7vfo, false);
  setTabEnabled(nwt7messen, false);
  setTabEnabled(berechnung, true);
  sabereich = bkein;
  sabereichalt = bkein;
  sabw = bwkein;
  //zur besseren Optik das Grafikfenster spaet sichtbar machen
  grafik->show();
  // Tabumschalten steuern
  QObject::connect( this, SIGNAL(currentChanged ( QWidget * )), this, SLOT(tabumschalten( QWidget* )));
  //Idletimer ist fuer die Kontaktaufnahme mit dem NWT
  idletimer->start(wgrunddaten.idletime);
  wgrunddaten.bfliessend = true;
  setCurrentIndex(0);
  messtimer->stop();
  bmessen = false;
  filenamesonde1 = "";
  filenamesonde2 = "";
  wobdaten.hline = 0.0;
}

Nwt7linux::~Nwt7linux()
{
  wkmanager->beenden();
}

void Nwt7linux::editsonde1(){
  int r;
  TSonde sonde;
  QString snum;
  int pos;
  
  QString qs = filenamesonde1;
  if((qs.indexOf(".")== -1)) qs = qs + ".hfm";
  
  sonde.dname = qs;
//  sonde.sname = checkboxk1->text();
  sonde.sname = qs;
  for(int i=0;i<30;i++){
    snum = wattoffsetk1[i].str_offsetwatt;
    pos = snum.indexOf("|");
    if(pos > -1){
      sonde.stroffset[i] = snum.right(pos+2);
    }else{
      sonde.stroffset[i] = snum;
    }
    sonde.offset[i] = wattoffsetk1[i].offsetwatt;
  }
  sdlg *dlg = new sdlg(this);

  dlg->setdaten(sonde);
  dlg->tip(btip);
  r = dlg->exec();
  if(r == QDialog::Accepted){
    sonde = dlg->getdaten();
    boxwattoffset1->clear(); // Box loeschen
    for(int i=0;i<30;i++){
      snum.sprintf("%2i| ",i+1);
      boxwattoffset1->addItem(snum + sonde.stroffset[i]);
      wattoffsetk1[i].str_offsetwatt = sonde.stroffset[i];
      wattoffsetk1[i].offsetwatt = sonde.offset[i];
    }
    wobdaten.beschreibung1 = sonde.sname;
    checkboxk1->setText(sonde.sname);
    qDebug()<< "Dialog OK";
    configfile nwt7sondenfile;
    bersterstart = true;
    messsondespeichern1(sonde.dname);
    bersterstart = false;
  }
  delete dlg;
}

void Nwt7linux::editsonde2(){
  int r;
  TSonde sonde;
  QString snum;
  int pos;

  QString qs = filenamesonde2;
  if((qs.indexOf(".")== -1)) qs = qs + ".hfm";
  
  sonde.dname = qs;
  sonde.sname = qs;
  for(int i=0;i<30;i++){
    snum = wattoffsetk2[i].str_offsetwatt;
    pos = snum.indexOf("|");
    if(pos > -1){
      sonde.stroffset[i] = snum.right(pos+2);
    }else{
      sonde.stroffset[i] = snum;
    }
    sonde.offset[i] = wattoffsetk2[i].offsetwatt;
  }
  
  sdlg *dlg = new sdlg(this);
  dlg->setdaten(sonde);
  dlg->tip(btip);
  r = dlg->exec();
  if(r == QDialog::Accepted){
    sonde = dlg->getdaten();
    boxwattoffset2->clear();
    for(int i=0;i<30;i++){
      snum.sprintf("%2i| ",i+1);
      boxwattoffset2->addItem(snum + sonde.stroffset[i]);
      wattoffsetk2[i].str_offsetwatt = sonde.stroffset[i];
      wattoffsetk2[i].offsetwatt = sonde.offset[i];
    }
    wobdaten.beschreibung2 = sonde.sname;
    checkboxk2->setText(sonde.sname);
    configfile nwt7sondenfile;
    bersterstart = true;
    messsondespeichern2(sonde.dname);
    bersterstart = false;
  }
  delete dlg;
}

void Nwt7linux::setDaempfungItem(){
  daempfung1->clear();
  daempfung2->clear();
  daempfung3->clear();
  if(wgrunddaten.bdaempfungfa){
    labeldaempfung->setText("0dB/66dB");
    labeldaempfung1->setText(tr("Daempfung 0dB/66dB","Tab VFO"));
    labeldaempfung3->setText(tr("Daempfung 0dB/66dB","Tab Wattmeter"));
    daempfung1->addItem("0dB");
    daempfung1->addItem("4dB");
    daempfung1->addItem("6dB");
    daempfung1->addItem("8dB");
    daempfung1->addItem("10dB");
    daempfung1->addItem("12dB");
    daempfung1->addItem("14dB");
    daempfung1->addItem("16dB");
    daempfung1->addItem("18dB");
    daempfung1->addItem("20dB");
    daempfung1->addItem("22dB");
    daempfung1->addItem("24dB");
    daempfung1->addItem("26dB");
    daempfung1->addItem("28dB");
    daempfung1->addItem("30dB");
    daempfung1->addItem("32dB");
    daempfung1->addItem("34dB");
    daempfung1->addItem("36dB");
    daempfung1->addItem("38dB");
    daempfung1->addItem("40dB");
    daempfung1->addItem("42dB");
    daempfung1->addItem("44dB");
    daempfung1->addItem("46dB");
    daempfung1->addItem("48dB");
    daempfung1->addItem("50dB");
    daempfung1->addItem("52dB");
    daempfung1->addItem("54dB");
    daempfung1->addItem("56dB");
    daempfung1->addItem("58dB");
    daempfung1->addItem("60dB");
    daempfung1->addItem("62dB");
    daempfung1->addItem("64dB");
    daempfung1->addItem("66dB");
    daempfung2->addItem("0dB");
    daempfung2->addItem("4dB");
    daempfung2->addItem("6dB");
    daempfung2->addItem("8dB");
    daempfung2->addItem("10dB");
    daempfung2->addItem("12dB");
    daempfung2->addItem("14dB");
    daempfung2->addItem("16dB");
    daempfung2->addItem("18dB");
    daempfung2->addItem("20dB");
    daempfung2->addItem("22dB");
    daempfung2->addItem("24dB");
    daempfung2->addItem("26dB");
    daempfung2->addItem("28dB");
    daempfung2->addItem("30dB");
    daempfung2->addItem("32dB");
    daempfung2->addItem("34dB");
    daempfung2->addItem("36dB");
    daempfung2->addItem("38dB");
    daempfung2->addItem("40dB");
    daempfung2->addItem("42dB");
    daempfung2->addItem("44dB");
    daempfung2->addItem("46dB");
    daempfung2->addItem("48dB");
    daempfung2->addItem("50dB");
    daempfung2->addItem("52dB");
    daempfung2->addItem("54dB");
    daempfung2->addItem("56dB");
    daempfung2->addItem("58dB");
    daempfung2->addItem("60dB");
    daempfung2->addItem("62dB");
    daempfung2->addItem("64dB");
    daempfung2->addItem("66dB");
    daempfung3->addItem("0dB");
    daempfung3->addItem("4dB");
    daempfung3->addItem("6dB");
    daempfung3->addItem("8dB");
    daempfung3->addItem("10dB");
    daempfung3->addItem("12dB");
    daempfung3->addItem("14dB");
    daempfung3->addItem("16dB");
    daempfung3->addItem("18dB");
    daempfung3->addItem("20dB");
    daempfung3->addItem("22dB");
    daempfung3->addItem("24dB");
    daempfung3->addItem("26dB");
    daempfung3->addItem("28dB");
    daempfung3->addItem("30dB");
    daempfung3->addItem("32dB");
    daempfung3->addItem("34dB");
    daempfung3->addItem("36dB");
    daempfung3->addItem("38dB");
    daempfung3->addItem("40dB");
    daempfung3->addItem("42dB");
    daempfung3->addItem("44dB");
    daempfung3->addItem("46dB");
    daempfung3->addItem("48dB");
    daempfung3->addItem("50dB");
    daempfung3->addItem("52dB");
    daempfung3->addItem("54dB");
    daempfung3->addItem("56dB");
    daempfung3->addItem("58dB");
    daempfung3->addItem("60dB");
    daempfung3->addItem("62dB");
    daempfung3->addItem("64dB");
    daempfung3->addItem("66dB");
  }else{
    labeldaempfung->setText("0dB/50dB");
    labeldaempfung1->setText(tr("Daempfung 0dB/50dB","Tab VFO"));
    labeldaempfung3->setText(tr("Daempfung 0dB/50dB","Tab Wattmeter"));
    daempfung1->addItem("0dB");
    daempfung1->addItem("10dB");
    daempfung1->addItem("20dB");
    daempfung1->addItem("30dB");
    daempfung1->addItem("40dB");
    daempfung1->addItem("50dB");
    daempfung2->addItem("0dB");
    daempfung2->addItem("10dB");
    daempfung2->addItem("20dB");
    daempfung2->addItem("30dB");
    daempfung2->addItem("40dB");
    daempfung2->addItem("50dB");
    daempfung3->addItem("0dB");
    daempfung3->addItem("10dB");
    daempfung3->addItem("20dB");
    daempfung3->addItem("30dB");
    daempfung3->addItem("40dB");
    daempfung3->addItem("50dB");
  }
  bmessen = false;
  messtimer->stop();
}

void Nwt7linux::resetKursor(int a){
  boxkursornr->setCurrentIndex(a);
}

void Nwt7linux::wobbelungueltig(){
  int i;
  
  for(i=0; i<maxmesspunkte; i++){
    wobdaten.mess.k1[i]=0;
    wobdaten.mess.k2[i]=0;
  }
  wobdaten.mess.daten_enable = false;
}

void Nwt7linux::setIQVFO(){
  setVfo();
}

void Nwt7linux::setShift(){
  QString sdbshift1, sdbshift2;
  bool ok;

  sdbshift1 = boxdbshift1->currentText();
  sdbshift2 = boxdbshift2->currentText();
  wobdaten.dbshift1 = sdbshift1.toDouble(&ok);
  wobdaten.dbshift2 = sdbshift2.toDouble(&ok);
  fsendewobbeldaten();
  #ifdef LDEBUG
    qDebug("setShift k1 %f k2 %f", wobdaten.dbshift1, wobdaten.dbshift2);
  #endif
}

void Nwt7linux::clicklupeplus(){
  double fa, fe, fm, fbereich;
  
  if(buttonlupeplus->isEnabled()){
    fa = wobdaten.anfangsfrequenz;
    fbereich = double(wobdaten.schritte) * wobdaten.schrittfrequenz;
    fm = KursorFrequenz;
    fa = fm - fbereich / 4.0;
    fe = fm + fbereich / 4.0;
    if(wobdaten.frqfaktor > 1){
      fa = fa / wobdaten.frqfaktor;
      fe = fe / wobdaten.frqfaktor;
    }
    if(fa < wgrunddaten.kalibrieranfang)fa = wgrunddaten.kalibrieranfang;
    if(fe > wgrunddaten.kalibrierende)fe = wgrunddaten.kalibrierende;
    if(wobdaten.frqfaktor > 1){
      fa = fa * wobdaten.frqfaktor;
      fe = fe * wobdaten.frqfaktor;
    }
    setanfang(fa);
    setende(fe);
    labellupe->setEnabled(false);
    buttonlupeplus->setEnabled(false);
    buttonlupeminus->setEnabled(false);
    buttonlupemitte->setEnabled(false);
    if(!bdauerwobbeln)clickwobbelneinmal();
  }
}

void Nwt7linux::clicklupegleich(){
  double fa, fe, fm, fbereich;
  
  fa = wobdaten.anfangsfrequenz;
  fbereich = double(wobdaten.schritte) * wobdaten.schrittfrequenz / 2.0;
  //  qDebug("%f",KursorFrequenz);
  fm = KursorFrequenz;
  if( fm > 0.0){
    fa = fm - fbereich;
    fe = fm + fbereich;
    if((wobdaten.ebetriebsart == espektrumanalyser) or
       (wobdaten.ebetriebsart == espekdisplayshift)){
      if(sabereich1->isChecked()){
        if(fa < wgrunddaten.kalibrieranfang)fa = wgrunddaten.frqa1;
        if(fe > wgrunddaten.kalibrierende)fe = wgrunddaten.frqb1;
      }
      if(sabereich2->isChecked()){
        if(fa < wgrunddaten.kalibrieranfang)fa = wgrunddaten.frqa2;
        if(fe > wgrunddaten.kalibrierende)fe = wgrunddaten.frqb2;
      }
    }else{
      if(wobdaten.frqfaktor > 1){
        fa = fa / wobdaten.frqfaktor;
        fe = fe / wobdaten.frqfaktor;
      }
      if(fa < wgrunddaten.kalibrieranfang)fa = wgrunddaten.kalibrieranfang;
      if(fe > wgrunddaten.kalibrierende)fe = wgrunddaten.kalibrierende;
      if(wobdaten.frqfaktor > 1){
        fa = fa * wobdaten.frqfaktor;
        fe = fe * wobdaten.frqfaktor;
      }
    }
    setanfang(fa);
    setende(fe);
  }
  labellupe->setEnabled(false);
  buttonlupeplus->setEnabled(false);
  buttonlupeminus->setEnabled(false);
  buttonlupemitte->setEnabled(false);
  if(!bdauerwobbeln)clickwobbelneinmal();
  emit cursormitte();
}

void Nwt7linux::clicklupeminus(){
  double fa, fe, fm, fbereich;
  
  if(buttonlupeminus->isEnabled()){
    fa = wobdaten.anfangsfrequenz;
    fbereich = double(wobdaten.schritte) * wobdaten.schrittfrequenz;
    fm = KursorFrequenz;
    fa = fm - fbereich;
    fe = fm + fbereich;
    if(wobdaten.frqfaktor > 1){
      fa = fa / wobdaten.frqfaktor;
      fe = fe / wobdaten.frqfaktor;
    }
    if(fa < wgrunddaten.kalibrieranfang)fa = wgrunddaten.kalibrieranfang;
    if(fe > wgrunddaten.kalibrierende)fe = wgrunddaten.kalibrierende;
    if(wobdaten.frqfaktor > 1){
      fa = fa * wobdaten.frqfaktor;
      fe = fe * wobdaten.frqfaktor;
    }
    setanfang(fa);
    setende(fe);
    labellupe->setEnabled(false);
    buttonlupeplus->setEnabled(false);
    buttonlupeminus->setEnabled(false);
    buttonlupemitte->setEnabled(false);
    if(!bdauerwobbeln)clickwobbelneinmal();
  }
}

void Nwt7linux::setKursorFrequenz(double afrq){
  KursorFrequenz = afrq;
  if(bdauerwobbeln){
    labellupe->setEnabled(false);
    buttonlupeplus->setEnabled(false);
    buttonlupeminus->setEnabled(false);
    buttonlupemitte->setEnabled(false);
  }else{
    if(KursorFrequenz == 0.0){
      labellupe->setEnabled(false);
      buttonlupeplus->setEnabled(false);
      buttonlupeminus->setEnabled(false);
      buttonlupemitte->setEnabled(false);
    }else{
      labellupe->setEnabled(true);
      buttonlupeplus->setEnabled(true);
      buttonlupeminus->setEnabled(true);
      buttonlupemitte->setEnabled(true);
    }
  }
}

void Nwt7linux::menuanfangsetzen(){
  emit setmenu(emEichenK1, true);
  if(wobdaten.einkanalig){
    emit setmenu(emEichenK2, false);
    emit setmenu(emSondeLadenK2, false);
    emit setmenu(emSondeSpeichernK2, false);
    emit setmenu(emWattEichenK2, false);
    emit setmenu(emWattEditSonde2, false);
    checkboxk2->setChecked(false);
    checkboxk2->setEnabled(false);
    progressbarmessk2->setValue(0);
    progressbarmessk2->setEnabled(false);
    ldaempfungk2->setEnabled(false);
    boxwattoffset2->setEnabled(false);
    messlabel2->setText("");
    messlabel2->setEnabled(false);
    messlabel4->setText("");
    messlabel4->setEnabled(false);
    messlabel6->setText("");
    messlabel6->setEnabled(false);
    mlabelk2->setText("");
    mlabelk2->setEnabled(false);
    labelldaempfungk2->setText("");
    checkboxmesshang2->setEnabled(false);
  }else{
    emit setmenu(emEichenK2, true);
    emit setmenu(emSondeLadenK2, true);
    emit setmenu(emSondeSpeichernK2, true);
    emit setmenu(emWattEichenK2, true);
    emit setmenu(emWattEditSonde2, true);
    checkboxk2->setEnabled(true);
    progressbarmessk2->setEnabled(true);
    ldaempfungk2->setEnabled(true);
    mlabelk2->setEnabled(true);
    boxwattoffset2->setEnabled(true);
    labelldaempfungk2->setText(tr("Daempfung (dB)","im Wattmeter"));
    checkboxmesshang2->setEnabled(true);
  }
}


void Nwt7linux::configurationladen(const QString &filename){
  configfile nwt7configfile;
  QString qs;
  int fontsize;
  int i,j,p;
  
  //qDebug("Nwt7linux::configurationladen(const QString &filename)");
  sw305 = false;
  nwt7configfile.open(filename);
  #ifdef Q_WS_WIN
  wgrunddaten.str_tty = nwt7configfile.readString("serielle_schnittstelle", "COM1");
  #else
  wgrunddaten.str_tty = nwt7configfile.readString("serielle_schnittstelle", "/dev/ttyS0");
  #endif
  if(!bttyOpen)bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
  editanfang->setText(nwt7configfile.readString("editanfang", "1000000"));
  editende->setText(nwt7configfile.readString("editende", "150000000"));
  editschritte->setText(nwt7configfile.readString("editschritte", "401"));
  editdisplay->setText(nwt7configfile.readString("editdisplay", "0"));
  wgrunddaten.grperwobbeln = nwt7configfile.readBool("grafikfocuswobbeln", false);
  wgrunddaten.grpereinmal = nwt7configfile.readBool("grafikfocuseinmal", false);
  wgrunddaten.grperstop = nwt7configfile.readBool("grafikfocusstop", false);
  wgrunddaten.bwarnsavbw = nwt7configfile.readBool("bwarnsavbw", true);
  wgrunddaten.bwarnsavbwmax = nwt7configfile.readBool("bwarnsavbwmax", true);
  wgrunddaten.bset0hz = nwt7configfile.readBool("bset0hz", true);
  wgrunddaten.bfliessend = nwt7configfile.readBool("bfliessend", true);
  //ab FW 1.20 neue Parameter fuer das NF Wobbeln
  wgrunddaten.audioztime = nwt7configfile.readInteger("audioztime", 10000);
  wgrunddaten.audiosens = nwt7configfile.readInteger("audiosens", 2);
  wgrunddaten.wmprezision = nwt7configfile.readInteger("wattmeter_prezision", 1);

  //////////////////////////////////////////////////////////
  bool bant = nwt7configfile.readBool("AntDiagramm", false);
  if(bant){
    wdiagramm->show();
    addTab(wdiagramm, tr("Antennendiagramm"));
  }
  //////////////////////////////////////////////////////////

  /////////////////////////////////////////////////
  //Paramatemeter fuer das Antennendiagram
  /////////////////////////////////////////////////
  wobdaten.antdiagramm.adbegin = nwt7configfile.readInteger("antdia_adbegin", 0);
  if(wobdaten.antdiagramm.adbegin < 0)wobdaten.antdiagramm.adbegin = 0;
  if(wobdaten.antdiagramm.adbegin > 1023)wobdaten.antdiagramm.adbegin = 0;
  wobdaten.antdiagramm.adend = nwt7configfile.readInteger("antdia_adend", 1023);
  if(wobdaten.antdiagramm.adend < 0)wobdaten.antdiagramm.adend = 1023;
  if(wobdaten.antdiagramm.adend > 1023)wobdaten.antdiagramm.adend = 1023;
  wobdaten.antdiagramm.dbmin = nwt7configfile.readInteger("antdia_dbmin", -40);
  wobdaten.antdiagramm.dbmax = nwt7configfile.readInteger("antdia_dbmax", 10);
  wobdaten.antdiagramm.diabegin = nwt7configfile.readInteger("antdia_diabegin", 10);
  wobdaten.antdiagramm.diaend = nwt7configfile.readInteger("antdia_diaend", 350);
  /////////////////////////////////////////////////
  //Paramatemeter fuer das Einblenden der Infotexte
  /////////////////////////////////////////////////
  nbildinfo.posx = nwt7configfile.readInteger("pngposx", 40);
  nbildinfo.posy = nwt7configfile.readInteger("pngposy", 120);
  nbildinfo.fontsize = nwt7configfile.readInteger("pngfontsize", 11);
  //////////////////////////////////////
  //Paramatemeter die nur gelesen werden
  //////////////////////////////////////
  wgrunddaten.stime = nwt7configfile.readInteger("stime", 50); 
  wgrunddaten.vtime = nwt7configfile.readInteger("vtime", 50);
  wgrunddaten.idletime = nwt7configfile.readInteger("idletime", 200);
  messtime = nwt7configfile.readInteger("messtime", 20);
  messtimeneu = nwt7configfile.readInteger("messtimeneu", 200);
  //begrenzen auf 10mSek
  if(messtime < 10)messtime=10;
  wgrunddaten.pllmodeenable = nwt7configfile.readBool("pllmodeenable", false);
  //qDebug("stime:%i",wgrunddaten.stime);
  //qDebug("vtime:%i",wgrunddaten.vtime);
  //qDebug("idletime:%i",wgrunddaten.idletime);
  //////////////////////////////////////
  //////////////////////////////////////
  //////////////////////////////////////
  //Profil ab 2.07
  //*******************************************************************
  aprofil[0].name = "default";
  aprofil[0].anfang = editanfang->text();
  aprofil[0].ende = editende->text();
  aprofil[0].schritte = editschritte->text();
  boxprofil->addItem(aprofil[0].name);
  p = nwt7configfile.readInteger("profile", 0);
  j=1;
  for(i=1; i < p; i++){
    qs.sprintf("profilname%02i",i);
    qs = nwt7configfile.readString(qs, "def");
    if(qs != "NN"){
      qs.sprintf("profilname%02i",i);
      aprofil[j].name = nwt7configfile.readString(qs, "0");
      qs.sprintf("profilanfang%02i",i);
      aprofil[j].anfang = nwt7configfile.readString(qs, "0");
      qs.sprintf("profilende%02i",i);
      aprofil[j].ende = nwt7configfile.readString(qs, "0");
      qs.sprintf("profilschritte%02i",i);
      aprofil[j].schritte = nwt7configfile.readString(qs, "1");
      boxprofil->addItem(aprofil[j].name);
      j++;
    } 
  }
  wgrunddaten.pfsize = nwt7configfile.readInteger("programfontsize", 9);
  emit setFontSize(wgrunddaten.pfsize);
  //*******************************************************************
  //gemerkte Wobbelfrequenzeinstellung laden
  //*******************************************************************
  wgrunddaten.wanfang = nwt7configfile.readDouble("mwanfang", 100000.0);
  wgrunddaten.wende = nwt7configfile.readDouble("mwende", 150000000.0);
  wgrunddaten.wschritte = nwt7configfile.readInteger("mwschritte", 1001);
  //*******************************************************************
  //gemerkte SA-Einstelungen laden
  //*******************************************************************
  
  wgrunddaten.sa1anfang = nwt7configfile.readDouble("msa1anfang", 1000000.0);
  wgrunddaten.sa1ende = nwt7configfile.readDouble("msa1ende", 72000000.0);
  wgrunddaten.sa1schritte = nwt7configfile.readInteger("msa1schritte", 1001);
  wgrunddaten.sa2anfang = nwt7configfile.readDouble("msa2anfang", 135000000.0);
  wgrunddaten.sa2ende = nwt7configfile.readDouble("msa2ende", 149000000.0);
  wgrunddaten.sa2schritte = nwt7configfile.readInteger("msa2schritte", 1001);
  wgrunddaten.sa3anfang = nwt7configfile.readDouble("msa3anfang", 410000000.0);
  wgrunddaten.sa3ende = nwt7configfile.readDouble("msa3ende", 430000000.0);
  wgrunddaten.sa3schritte = nwt7configfile.readInteger("msa3schritte", 1001);
  wgrunddaten.frqa1 = nwt7configfile.readDouble("sa1anfang", 1000000.0);
  wgrunddaten.frqb1 = nwt7configfile.readDouble("sa1ende", 72000000.0);
  wgrunddaten.frqa2 = nwt7configfile.readDouble("sa2anfang", 135000000.0);
  wgrunddaten.frqb2 = nwt7configfile.readDouble("sa2ende", 149000000.0);
  wgrunddaten.frqzf1 = nwt7configfile.readDouble("sazf1", 85300000.0);
  wgrunddaten.frqzf2 = nwt7configfile.readDouble("sazf2", -85300000.0);
  wgrunddaten.frqshift = nwt7configfile.readDouble("safrqshift", -410000000.0);
  wgrunddaten.sastatus = nwt7configfile.readBool("sastatus", false);
  wgrunddaten.bschrittkorr = nwt7configfile.readBool("saschrittkorr", true);
  wgrunddaten.psavabs1 = nwt7configfile.readDouble("pegelsavabs1", 0.0);
  wgrunddaten.psavabs2 = nwt7configfile.readDouble("pegelsavabs2", 0.0);
  wgrunddaten.psavabs3 = nwt7configfile.readDouble("pegelsavabs3", 0.0);
  wobdaten.psav300 = nwt7configfile.readDouble("pegelsav300hz", 0.0);
  wobdaten.psav7k = nwt7configfile.readDouble("pegelsav7khz", 0.0);
  wobdaten.psav30k = nwt7configfile.readDouble("pegelsav30khz", 0.0);
  wobdaten.bsavdbm = nwt7configfile.readBool("savdbmanz", true);
  wobdaten.bsavuv = nwt7configfile.readBool("savuvanz", false);
  wobdaten.bsavwatt = nwt7configfile.readBool("savwattanz", false);
  wobdaten.bsavdbuv = nwt7configfile.readBool("savdbuvanz", false);
  wobdaten.safehlermax = nwt7configfile.readInteger("safehlermax", -5);
  wobdaten.safehlermin = nwt7configfile.readInteger("safehlermin", -60);
  wgrunddaten.saminschritte = nwt7configfile.readInteger("saminschritte", 401);
  wgrunddaten.bw300_max = nwt7configfile.readDouble("bw300_max", 100.0);
  wgrunddaten.bw300_min = nwt7configfile.readDouble("bw300_min", 50.0);
  wgrunddaten.bw7kHz_max = nwt7configfile.readDouble("bw7kHz_max", 2500.0);
  wgrunddaten.bw7kHz_min = nwt7configfile.readDouble("bw7kHz_min", 1250.0);
  wgrunddaten.bw30kHz_max = nwt7configfile.readDouble("bw30kHz_max", 10000.0);
  wgrunddaten.bw30kHz_min = nwt7configfile.readDouble("bw30kHz_min", 5000.0);
  
  //*******************************************************************
  //ab V3.04.03
  //*******************************************************************
  wobdaten.ydbmax = nwt7configfile.readInteger("ydbmax", 10);
  wobdaten.ydbmin = nwt7configfile.readInteger("ydbmin", -90);
  boxydbmax->setCurrentIndex((30-wobdaten.ydbmax)/10);
  boxydbmin->setCurrentIndex((-10-wobdaten.ydbmin)/10);
  //*******************************************************************
  //ab V3.04.05
  //*******************************************************************
  qs = nwt7configfile.readString("kurvendir", "");
  wgrunddaten.kurvendir = qs;
  if(!qs.isEmpty()){
    kurvendir.setPath(qs);
  }else{
    kurvendir = homedir;
  } 
  //*******************************************************************
  qs = nwt7configfile.readString("messlabelfont", "");
  wgrunddaten.kalibrieranfang = nwt7configfile.readDouble("eichanfangsfrequenz", 100000.0);
  wgrunddaten.kalibrierende = nwt7configfile.readDouble("eichendfrequenz", 150000000.0);
  wgrunddaten.kalibrierstep = nwt7configfile.readLong("eichschritte", maxmesspunkte);
  
  //VFO im Wattmeter ein/aus
  bmessvfo = nwt7configfile.readInteger("messvfoaktiv", true);
  vfoeinaus();
  //VFO im Wattmeter Frequenz einstellen
  double fr;
  fr = nwt7configfile.readDouble("messvfo", 10000000.0);
  int a;
  a = int(fr/1000000000.0);
  sp1ghz->setValue(a);
  fr = fr - (double(a * 1000000000.0));
  a = int(fr/100000000.0);
  sp100mhz->setValue(a);
  fr = fr - (double(a * 100000000.0));
  a = int(fr/10000000.0);
  sp10mhz->setValue(a);
  fr = fr - (double(a * 10000000.0));
  a = int(fr/1000000.0);
  sp1mhz->setValue(a);
  fr = fr - (double(a * 1000000.0));
  a = int(fr/100000.0);
  sp100khz->setValue(a);
  fr = fr - (double(a * 100000.0));
  a = int(fr/10000.0);
  sp10khz->setValue(a);
  fr = fr - (double(a * 10000.0));
  a = int(fr/1000.0);
  sp1khz->setValue(a);
  fr = fr - (double(a * 1000.0));
  a = int(fr/100.0);
  sp100hz->setValue(a);
  fr = fr - (double(a * 100.0));
  a = int(fr/10.0);
  sp10hz->setValue(a);
  fr = fr - (double(a * 10.0));
  sp1hz->setValue(int(fr));

  wgrunddaten.ddstakt = nwt7configfile.readDouble("DDStakt", 400000000.0);
  wgrunddaten.pll = nwt7configfile.readInteger("PLLMode", 1);
  wgrunddaten.bdaempfungfa = nwt7configfile.readBool("faeichglied", false);
  setDaempfungItem();
  messtimer->stop();
  fontsize = nwt7configfile.readInteger("messlabelfontsize", 20);
  messfont.setPointSize(fontsize);
  if(!qs.isNull()){
    messfont.setFamily(qs);
    messlabel1->setFont(messfont);
    messlabel2->setFont(messfont);
    messlabel3->setFont(messfont);
    messlabel4->setFont(messfont);
    messlabel5->setFont(messfont);
    messlabel6->setFont(messfont);
    lmhz->setFont(messfont);
    lkhz->setFont(messfont);
    lhz->setFont(messfont);
  }  
  qs = nwt7configfile.readString("infofont", "");
  if(!qs.isNull()){
    infofont.setFamily(qs);
    fontsize = nwt7configfile.readInteger("infofontsize", 8);
    infofont.setPointSize(fontsize);
    mledit->setFont(infofont);
  }
  grafik->setWobbeldaten(wobdaten);
  lcdfrq1 = nwt7configfile.readDouble("LCD1", 1800000.0);
  lcdfrq2 = nwt7configfile.readDouble("LCD2", 3600000.0);
  lcdfrq3 = nwt7configfile.readDouble("LCD3", 7000000.0);
  lcdfrq4 = nwt7configfile.readDouble("LCD4", 14000000.0);
  lcdfrq5 = nwt7configfile.readDouble("LCD5", 28000000.0);
  LCD1->display(qs.sprintf("%2.6f", lcdfrq1/1000000.0));
  LCD2->display(qs.sprintf("%2.6f", lcdfrq2/1000000.0));
  LCD3->display(qs.sprintf("%2.6f", lcdfrq3/1000000.0));
  LCD4->display(qs.sprintf("%2.6f", lcdfrq4/1000000.0));
  LCD5->display(qs.sprintf("%2.6f", lcdfrq5/1000000.0));
  editzf->setText(nwt7configfile.readString("editzf", "0"));
  wobdaten.namesonde1 = nwt7configfile.readString("namesonde1", "defsonde1");
  wgrunddaten.strsonde1lin = nwt7configfile.readString("namesonde1lin", "defsonde1lin");
  wobdaten.namesonde2 = nwt7configfile.readString("namesonde2", "defsonde2");
  wobdaten.einkanalig = nwt7configfile.readBool("Einkanalig", true);
  wgrunddaten.kalibrierk1 = nwt7configfile.readBool("eichkorrk1", false);
  wgrunddaten.kalibrierk2 = nwt7configfile.readBool("eichkorrk2", false);
  kalibrier1->setChecked(wgrunddaten.kalibrierk1);
  kalibrier2->setChecked(wgrunddaten.kalibrierk2);
  wobdaten.bswriteration = nwt7configfile.readBool("SWRIteration", true);
  wobdaten.bswrrelais = nwt7configfile.readBool("SWR_Relais", false); //Relais fuer NWT500 Nachbau
  wobdaten.binvers = nwt7configfile.readBool("bandbreite_invers", true); //Darstellung der inversen Bandbreite
  checkboxinvers->setChecked(wobdaten.binvers);
  int r,g,b;
  //default rot
  r = nwt7configfile.readInteger("color1r", 255);
  g = nwt7configfile.readInteger("color1g", 0);
  b = nwt7configfile.readInteger("color1b", 0);
  a = nwt7configfile.readInteger("color1a", 255);
  wobdaten.penwidth = nwt7configfile.readInteger("penwidth", 1);
  QColor pencolor1(r,g,b,a);
  wobdaten.penkanal1.setColor(pencolor1);
  wobdaten.penkanal1.setWidth(wobdaten.penwidth);
  
  //default gruen
  r = nwt7configfile.readInteger("color2r", 85);
  g = nwt7configfile.readInteger("color2g", 255);
  b = nwt7configfile.readInteger("color2b", 0);
  a = nwt7configfile.readInteger("color2a", 255);
  QColor pencolor2(r,g,b,a);
  wobdaten.penkanal2.setColor(pencolor2);
  wobdaten.penkanal2.setWidth(wobdaten.penwidth);
  //default weiss
  r = nwt7configfile.readInteger("colorhr", 255);
  g = nwt7configfile.readInteger("colorhg", 255);
  b = nwt7configfile.readInteger("colorhb", 255);
  a = nwt7configfile.readInteger("colorha", 255);
  QColor hcolor(r,g,b,a);
  wobdaten.colorhintergrund = hcolor;
  //fuer SAV
  //wobdaten.colorhintergrunddunkel = nwt7configfile.readDouble("colorhd", 0.95);
  
  wgrunddaten.maxwobbel = nwt7configfile.readDouble("maxwobbel", 200000000.0);
  wobdaten.frqfaktor = nwt7configfile.readInteger("frequenzfaktor", 1);
  setFrqFaktorLabel();
  //Dial1mhz->setMaximum(int(weichen.maxvfo / 1000000.0)-1);
  checkboxk1->setChecked(nwt7configfile.readBool("k1checked", true));
  checkboxk2->setChecked(nwt7configfile.readBool("k2checked", false));
  wobdaten.bswrkanal2 = checkboxk2->isChecked();
  //ComboBox im Wattmeter neu aufbauen
  ldaempfungk1->clear();
  ldaempfungk1->addItem("0");
  ldaempfungk1->addItem("10");
  ldaempfungk1->addItem("20");
  ldaempfungk1->addItem("30");
  ldaempfungk1->addItem("40");
  ldaempfungk2->clear();
  ldaempfungk2->addItem("0");
  ldaempfungk2->addItem("10");
  ldaempfungk2->addItem("20");
  ldaempfungk2->addItem("30");
  ldaempfungk2->addItem("40");
  qs = nwt7configfile.readString("powerk15", "99");
  if(qs != "99")ldaempfungk1->addItem(qs);
  qs = nwt7configfile.readString("powerk16", "99");
  if(qs != "99")ldaempfungk1->addItem(qs);
  qs = nwt7configfile.readString("powerk17", "99");
  if(qs != "99")ldaempfungk1->addItem(qs);
  qs = nwt7configfile.readString("powerk18", "99");
  if(qs != "99")ldaempfungk1->addItem(qs);
  qs = nwt7configfile.readString("powerk19", "99");
  if(qs != "99")ldaempfungk1->addItem(qs);
  qs = nwt7configfile.readString("powerk25", "99");
  if(qs != "99")ldaempfungk2->addItem(qs);
  qs = nwt7configfile.readString("powerk26", "99");
  if(qs != "99")ldaempfungk2->addItem(qs);
  qs = nwt7configfile.readString("powerk27", "99");
  if(qs != "99")ldaempfungk2->addItem(qs);
  qs = nwt7configfile.readString("powerk28", "99");
  if(qs != "99")ldaempfungk2->addItem(qs);
  qs = nwt7configfile.readString("powerk29", "99");
  if(qs != "99")ldaempfungk2->addItem(qs);
  wgrunddaten.einkanalig = wobdaten.einkanalig;
  frqmarken.f1160m = nwt7configfile.readLongLong("a160m", 1810000);
  frqmarken.f2160m = nwt7configfile.readLongLong("b160m", 2000000);
  frqmarken.f180m = nwt7configfile.readLongLong("a80m", 3500000);
  frqmarken.f280m = nwt7configfile.readLongLong("b80m", 3800000);
  frqmarken.f140m = nwt7configfile.readLongLong("a40m", 7000000);
  frqmarken.f240m = nwt7configfile.readLongLong("b40m", 7200000);
  frqmarken.f130m = nwt7configfile.readLongLong("a30m", 10100000);
  frqmarken.f230m = nwt7configfile.readLongLong("b30m", 10150000);
  frqmarken.f120m = nwt7configfile.readLongLong("a20m", 14000000);
  frqmarken.f220m = nwt7configfile.readLongLong("b20m", 14350000);
  frqmarken.f117m = nwt7configfile.readLongLong("a17m", 18068000);
  frqmarken.f217m = nwt7configfile.readLongLong("b17m", 18168000);
  frqmarken.f115m = nwt7configfile.readLongLong("a15m", 21000000);
  frqmarken.f215m = nwt7configfile.readLongLong("b15m", 21450000);
  frqmarken.f112m = nwt7configfile.readLongLong("a12m", 24890000);
  frqmarken.f212m = nwt7configfile.readLongLong("b12m", 24990000);
  frqmarken.f110m = nwt7configfile.readLongLong("a10m", 28000000);
  frqmarken.f210m = nwt7configfile.readLongLong("b10m", 29700000);
  frqmarken.f16m = nwt7configfile.readLongLong("a6m", 50000000);
  frqmarken.f26m = nwt7configfile.readLongLong("b6m", 54000000);
  frqmarken.f12m = nwt7configfile.readLongLong("a2m", 144000000);
  frqmarken.f22m = nwt7configfile.readLongLong("b2m", 146000000);
  frqmarken.f1u1 = nwt7configfile.readLongLong("au1", 0);
  frqmarken.f2u1 = nwt7configfile.readLongLong("bu1", 0);
  frqmarken.f1u2 = nwt7configfile.readLongLong("au2", 0);
  frqmarken.f2u2 = nwt7configfile.readLongLong("bu2", 0);
  frqmarken.f1u3 = nwt7configfile.readLongLong("au3", 0);
  frqmarken.f2u3 = nwt7configfile.readLongLong("bu3", 0);
  frqmarken.b160m = nwt7configfile.readBool("160m", false);
  frqmarken.b80m = nwt7configfile.readBool("80m", false);
  frqmarken.b40m = nwt7configfile.readBool("40m", false);
  frqmarken.b30m = nwt7configfile.readBool("30m", false);
  frqmarken.b20m = nwt7configfile.readBool("20m", false);
  frqmarken.b17m = nwt7configfile.readBool("17m", false);
  frqmarken.b15m = nwt7configfile.readBool("15m", false);
  frqmarken.b12m = nwt7configfile.readBool("12m", false);
  frqmarken.b10m = nwt7configfile.readBool("10m", false);
  frqmarken.b6m = nwt7configfile.readBool("6m", false);
  frqmarken.b2m = nwt7configfile.readBool("2m", false);
  frqmarken.bu1 = nwt7configfile.readBool("zusatz1", false);
  frqmarken.bu2 = nwt7configfile.readBool("zusatz2", false);
  frqmarken.bu3 = nwt7configfile.readBool("zusatz3", false);
  frqmarken.st160m = nwt7configfile.readString("s160m", "160m");
  frqmarken.st80m = nwt7configfile.readString("s80m", "80m");
  frqmarken.st40m = nwt7configfile.readString("s40m", "40m");
  frqmarken.st30m = nwt7configfile.readString("s30m", "30m");
  frqmarken.st20m = nwt7configfile.readString("s20m", "20m");
  frqmarken.st17m = nwt7configfile.readString("s17m", "17m");
  frqmarken.st15m = nwt7configfile.readString("s15m", "15m");
  frqmarken.st12m = nwt7configfile.readString("s12m", "12m");
  frqmarken.st10m = nwt7configfile.readString("s10m", "10m");
  frqmarken.st6m = nwt7configfile.readString("s6m", "6m");
  frqmarken.st2m = nwt7configfile.readString("s2m", "2m");
  frqmarken.stu1 = nwt7configfile.readString("szusatz1", "Zusatz 1");
  frqmarken.stu2 = nwt7configfile.readString("szusatz2", "Zusatz 2");
  frqmarken.stu3 = nwt7configfile.readString("szusatz3", "Zusatz 3");
  if(wobdaten.einkanalig){
    emit setmenu(emEichenK2, false);
    emit setmenu(emSondeLadenK2, false);
    emit setmenu(emSondeSpeichernK2, false);
    emit setmenu(emWattEichenK2, false);
    emit setmenu(emWattEditSonde2, false);
    checkboxk2->setChecked(false);
    checkboxk2->setEnabled(false);
    progressbarmessk2->setValue(0);
    progressbarmessk2->setEnabled(false);
    ldaempfungk2->setEnabled(false);
    boxwattoffset2->setEnabled(false);
    messlabel2->setText("");
    messlabel2->setEnabled(false);
    messlabel4->setText("");
    messlabel4->setEnabled(false);
    messlabel6->setText("");
    messlabel6->setEnabled(false);
    mlabelk2->setText("");
    mlabelk2->setEnabled(false);
    labelldaempfungk2->setText("");
    checkboxmesshang2->setEnabled(false);
  }else{
    emit setmenu(emEichenK2, true);
    emit setmenu(emSondeLadenK2, true);
    emit setmenu(emSondeSpeichernK2, true);
    emit setmenu(emWattEichenK2, true);
    emit setmenu(emWattEditSonde2, true);
    checkboxk2->setEnabled(true);
    progressbarmessk2->setEnabled(true);
    ldaempfungk2->setEnabled(true);
    labelldaempfungk2->setText(tr("Daempfung (dB)","im Wattmeter"));
    checkboxmesshang2->setEnabled(true);
    mlabelk2->setEnabled(true);
    boxwattoffset2->setEnabled(true);
  }
  menuanfangsetzen();
  wobdaten.ebetriebsart = enumbetriebsart(nwt7configfile.readInteger("betriebsart", ewobbeln));
  betriebsart->setCurrentIndex(int(wobdaten.ebetriebsart));
  setBetriebsart(int(wobdaten.ebetriebsart));
  setFrqBereich();
  nwt7configfile.close();
  emit sendefrqmarken(frqmarken);
  configfile nwt7sondenfile;
  qs = wobdaten.namesonde1;
  if((qs.indexOf(".")== -1)) qs = qs + ".hfm";
  messsondenfilek1laden(qs);
  qs = wobdaten.namesonde2;
  if((qs.indexOf(".")== -1)) qs = qs + ".hfm";
  messsondenfilek2laden(qs);
  setFrqBereich();
  wobnormalisieren();
  fsendewobbeldaten();
  if(!bersterstart){
    if(!sw305){
      QMessageBox::warning( this, tr("Messondendateien alte Version","Konfiguration laden"), 
                                  tr("<B>Die Kalbrierdaten einer Messondendatei sind im alten Format! </B><BR>"
                                    "In jeder Messondendatei befinden sich die Kalibrierdaten <BR>"
                                    "des verwendeten Messkopfes. Die Kalibrierung der Messonde <br>"
                                    "hat ab der Version 4.00 ein neues Format. Die Kalibrierdaten wurden <br>"
                                    "automatisch umgerechnet. Damit ist ein Arbeiten erst einmal <br>"
                                    "moeglich. Das neue Format wurde notwendig mit der Einfuehrung <br>" 
                                    "der Skalierung der Y-Achse. <br>" 
                                    "<b>Es ist eine neue Kalibrierung durchzufuehren! </b><br>"
                                    "Das neue Format wird ab Version 3.05 verwendet. <br>"
                                    ,"Konfiguration laden"));
    }
  }
}

void Nwt7linux::configurationspeichern(const QString &filename){
  QString qs;
  int i;

  //  boxprofil->setCurrentIndex(0);
  configfile *nwt7configfile = new configfile();
  nwt7configfile->open(filename);
  qs.sprintf("%01i.%02i:V%02i",fwversion/100,fwversion-100,fwvariante);
  nwt7configfile->writeInteger("programfontsize", wgrunddaten.pfsize);
  nwt7configfile->writeString("firmware", qs);
  nwt7configfile->writeString("serielle_schnittstelle", wgrunddaten.str_tty);
  nwt7configfile->writeString("editanfang", editanfang->text());
  nwt7configfile->writeString("editende", editende->text());
  nwt7configfile->writeString("editschritte", editschritte->text());
  nwt7configfile->writeString("editdisplay", editdisplay->text());
  nwt7configfile->writeDouble("eichanfangsfrequenz", wgrunddaten.kalibrieranfang);
  nwt7configfile->writeDouble("eichendfrequenz", wgrunddaten.kalibrierende);
  nwt7configfile->writeLong("eichschritte", wgrunddaten.kalibrierstep);
  nwt7configfile->writeDouble("DDStakt", wgrunddaten.ddstakt);
  nwt7configfile->writeBool("Einkanalig", wgrunddaten.einkanalig);
  nwt7configfile->writeInteger("PLLMode", wgrunddaten.pll);
  nwt7configfile->writeDouble("maxwobbel", wgrunddaten.maxwobbel);
  nwt7configfile->writeInteger("frequenzfaktor", wobdaten.frqfaktor);
  nwt7configfile->writeBool("faeichglied", wgrunddaten.bdaempfungfa);
  if(wobdaten.ebetriebsart == eantdiagramm)wobdaten.ebetriebsart = ewobbeln;
  nwt7configfile->writeInteger("betriebsart", int(wobdaten.ebetriebsart));
  nwt7configfile->writeBool("grafikfocuswobbeln", wgrunddaten.grperwobbeln);
  nwt7configfile->writeBool("grafikfocuseinmal", wgrunddaten.grpereinmal);
  nwt7configfile->writeBool("grafikfocusstop", wgrunddaten.grperstop);
  nwt7configfile->writeBool("bwarnsavbw", wgrunddaten.bwarnsavbw);
  nwt7configfile->writeBool("bwarnsavbwmax", wgrunddaten.bwarnsavbwmax);
  nwt7configfile->writeBool("bset0hz", wgrunddaten.bset0hz);
  nwt7configfile->writeBool("bfliessend", wgrunddaten.bfliessend);
  //ab FW 1.20 neue Parameter fuer das NF Wobbeln
  nwt7configfile->writeInteger("audioztime", wgrunddaten.audioztime);
  nwt7configfile->writeInteger("audiosens", wgrunddaten.audiosens);
  nwt7configfile->writeInteger("wattmeter_prezision", wgrunddaten.wmprezision);

  //*******************************************************************
  //ab V3.04.03 Y-Skala speichern
  //*******************************************************************
  nwt7configfile->writeInteger("ydbmax", wobdaten.ydbmax);
  nwt7configfile->writeInteger("ydbmin", wobdaten.ydbmin);
  //*******************************************************************
  //ab V3.04.05 Kurven Dir abspeichern
  //*******************************************************************
  nwt7configfile->writeString("kurvendir", kurvendir.absolutePath());
  //*********************************************************************
  //Wobbelfrequenzeinstellungen merken
  //*********************************************************************
  nwt7configfile->writeDouble("mwanfang", wgrunddaten.wanfang);
  nwt7configfile->writeDouble("mwende", wgrunddaten.wende);
  nwt7configfile->writeInteger("mwschritte", wgrunddaten.wschritte);
  //*********************************************************************
  //SA-Einstellungen merken
  //*********************************************************************
  nwt7configfile->writeDouble("msa1anfang", wgrunddaten.sa1anfang);
  nwt7configfile->writeDouble("msa1ende", wgrunddaten.sa1ende);
  nwt7configfile->writeInteger("msa1schritte", wgrunddaten.sa1schritte);
  nwt7configfile->writeDouble("msa2anfang", wgrunddaten.sa2anfang);
  nwt7configfile->writeDouble("msa2ende", wgrunddaten.sa2ende);
  nwt7configfile->writeInteger("msa2schritte", wgrunddaten.sa2schritte);
  nwt7configfile->writeDouble("msa3anfang", wgrunddaten.sa3anfang);
  nwt7configfile->writeDouble("msa3ende", wgrunddaten.sa3ende);
  nwt7configfile->writeInteger("msa3schritte", wgrunddaten.sa3schritte);
  nwt7configfile->writeDouble("sa1anfang", wgrunddaten.frqa1);
  nwt7configfile->writeDouble("sa1ende", wgrunddaten.frqb1);
  nwt7configfile->writeDouble("sa2anfang", wgrunddaten.frqa2);
  nwt7configfile->writeDouble("sa2ende", wgrunddaten.frqb2);
  nwt7configfile->writeDouble("sazf1", wgrunddaten.frqzf1);
  nwt7configfile->writeDouble("sazf2", wgrunddaten.frqzf2);
  nwt7configfile->writeDouble("safrqshift", wgrunddaten.frqshift);
  nwt7configfile->writeBool("sastatus", wgrunddaten.sastatus);
  nwt7configfile->writeBool("saschrittkorr", wgrunddaten.bschrittkorr);
  nwt7configfile->writeDouble("pegelsavabs1", wgrunddaten.psavabs1);
  nwt7configfile->writeDouble("pegelsavabs2", wgrunddaten.psavabs2);
  nwt7configfile->writeDouble("pegelsavabs3", wgrunddaten.psavabs3);
  nwt7configfile->writeDouble("pegelsav300hz", wobdaten.psav300);
  nwt7configfile->writeDouble("pegelsav7khz", wobdaten.psav7k);
  nwt7configfile->writeDouble("pegelsav30khz", wobdaten.psav30k);
  nwt7configfile->writeBool("savdbmanz", wobdaten.bsavdbm);
  nwt7configfile->writeBool("savuvanz", wobdaten.bsavuv);
  nwt7configfile->writeBool("savwattanz", wobdaten.bsavwatt);
  nwt7configfile->writeBool("savdbuvanz", wobdaten.bsavdbuv);
  nwt7configfile->writeInteger("safehlermax", wobdaten.safehlermax);
  nwt7configfile->writeInteger("safehlermin", wobdaten.safehlermin);
  nwt7configfile->writeInteger("saminschritte", wgrunddaten.saminschritte);
  nwt7configfile->writeDouble("bw300_max", wgrunddaten.bw300_max);
  nwt7configfile->writeDouble("bw300_min", wgrunddaten.bw300_min);
  nwt7configfile->writeDouble("bw7kHz_max", wgrunddaten.bw7kHz_max);
  nwt7configfile->writeDouble("bw7kHz_min", wgrunddaten.bw7kHz_min);
  nwt7configfile->writeDouble("bw30kHz_max", wgrunddaten.bw30kHz_max);
  nwt7configfile->writeDouble("bw30kHz_min", wgrunddaten.bw30kHz_min);
  //*********************************************************************
  wobdaten = grafik->getWobbeldaten();
  nwt7configfile->writeDouble("LCD1", lcdfrq1);
  nwt7configfile->writeDouble("LCD2", lcdfrq2);
  nwt7configfile->writeDouble("LCD3", lcdfrq3);
  nwt7configfile->writeDouble("LCD4", lcdfrq4);
  nwt7configfile->writeDouble("LCD5", lcdfrq5);
  nwt7configfile->writeString("editzf", editzf->text());
  // VFO im Wattbereich
  nwt7configfile->writeBool("messvfoaktiv", bmessvfo);
  double fr = double(sp1hz->value() + 
              (sp10hz->value()   * 10) +
              (sp100hz->value()  * 100) +
              (sp1khz->value()   * 1000) +
              (sp10khz->value()  * 10000) +
              (sp100khz->value() * 100000) +
              (sp1mhz->value()   * 1000000) +
              (sp10mhz->value()  * 10000000) +
              (sp100mhz->value() * 100000000) +
              (sp1ghz->value()   * 1000000000));
  nwt7configfile->writeDouble("messvfo", fr);
  messfont = messlabel1->font();
  qs = messfont.family();
  if(qs.isEmpty())qs = messfont.defaultFamily();
  nwt7configfile->writeString("messlabelfont", qs);
  nwt7configfile->writeInteger("messlabelfontsize", messfont.pointSize());
  nwt7configfile->writeString("infofont", infofont.family());
  nwt7configfile->writeInteger("infofontsize", infofont.pointSize());
  nwt7configfile->writeString("namesonde1", wobdaten.namesonde1);
  nwt7configfile->writeString("namesonde1lin", wgrunddaten.strsonde1lin);
  nwt7configfile->writeString("namesonde2", wobdaten.namesonde2);
  nwt7configfile->writeBool("k1checked", checkboxk1->isChecked());
  nwt7configfile->writeBool("k2checked", checkboxk2->isChecked());
  nwt7configfile->writeBool("eichkorrk1", wgrunddaten.kalibrierk1);
  nwt7configfile->writeBool("eichkorrk2", wgrunddaten.kalibrierk2);
  nwt7configfile->writeBool("SWRIteration", wobdaten.bswriteration);
  nwt7configfile->writeBool("bandbreite_invers", wobdaten.binvers); //Darstellung der inversen Bandbreite
  
  int r,g,b,a;
  QColor pencolor1 = wobdaten.penkanal1.color();
  pencolor1.getRgb(&r, &g, &b, &a);
  nwt7configfile->writeInteger("color1r", r);
  nwt7configfile->writeInteger("color1g", g);
  nwt7configfile->writeInteger("color1b", b);
  nwt7configfile->writeInteger("color1a", a);
  nwt7configfile->writeInteger("penwidth", wobdaten.penwidth);
  
  QColor pencolor2 = wobdaten.penkanal2.color();
  pencolor2.getRgb(&r, &g, &b, &a);
  nwt7configfile->writeInteger("color2r", r);
  nwt7configfile->writeInteger("color2g", g);
  nwt7configfile->writeInteger("color2b", b);
  nwt7configfile->writeInteger("color2a", a);
  
  QColor colorh = wobdaten.colorhintergrund;
  colorh.getRgb(&r, &g, &b, &a);
  nwt7configfile->writeInteger("colorhr", r);
  nwt7configfile->writeInteger("colorhg", g);
  nwt7configfile->writeInteger("colorhb", b);
  nwt7configfile->writeInteger("colorha", a);
  
  //nwt7configfile->writeDouble("colorhd", wobdaten.colorhintergrunddunkel);

  //Speichern der profile
  nwt7configfile->writeInteger("profile", boxprofil->count());
  for(i=0; i < boxprofil->count(); i++){
    qs.sprintf("profilname%02i",i);
    nwt7configfile->writeString(qs, aprofil[i].name);
    qs.sprintf("profilanfang%02i",i);
    nwt7configfile->writeString(qs, aprofil[i].anfang);
    qs.sprintf("profilende%02i",i);
    nwt7configfile->writeString(qs, aprofil[i].ende);
    qs.sprintf("profilschritte%02i",i);
    nwt7configfile->writeString(qs, aprofil[i].schritte);
  } 
  for(i=5; i<ldaempfungk1->count(); i++){
    qs.sprintf("powerk1%i",i);
    ldaempfungk1-> setCurrentIndex(i);
    nwt7configfile->writeString(qs, ldaempfungk1->currentText());
  } 
  ldaempfungk1-> setCurrentIndex(0);
  for(i=5; i<ldaempfungk2->count(); i++){
    qs.sprintf("powerk2%i",i);
    ldaempfungk2-> setCurrentIndex(i);
    nwt7configfile->writeString(qs, ldaempfungk2->currentText());
  } 
  ldaempfungk2-> setCurrentIndex(0);
  nwt7configfile->writeLongLong("a160m", frqmarken.f1160m );
  nwt7configfile->writeLongLong("b160m", frqmarken.f2160m);
  nwt7configfile->writeLongLong("a80m", frqmarken.f180m);
  nwt7configfile->writeLongLong("b80m", frqmarken.f280m);
  nwt7configfile->writeLongLong("a40m", frqmarken.f140m);
  nwt7configfile->writeLongLong("b40m", frqmarken.f240m);
  nwt7configfile->writeLongLong("a30m", frqmarken.f130m);
  nwt7configfile->writeLongLong("b30m", frqmarken.f230m);
  nwt7configfile->writeLongLong("a20m", frqmarken.f120m);
  nwt7configfile->writeLongLong("b20m", frqmarken.f220m);
  nwt7configfile->writeLongLong("a17m", frqmarken.f117m);
  nwt7configfile->writeLongLong("b17m", frqmarken.f217m);
  nwt7configfile->writeLongLong("a15m", frqmarken.f115m);
  nwt7configfile->writeLongLong("b15m", frqmarken.f215m);
  nwt7configfile->writeLongLong("a12m", frqmarken.f112m);
  nwt7configfile->writeLongLong("b12m", frqmarken.f212m);
  nwt7configfile->writeLongLong("a10m", frqmarken.f110m);
  nwt7configfile->writeLongLong("b10m", frqmarken.f210m);
  nwt7configfile->writeLongLong("a6m", frqmarken.f16m);
  nwt7configfile->writeLongLong("b6m", frqmarken.f26m);
  nwt7configfile->writeLongLong("a2m", frqmarken.f12m);
  nwt7configfile->writeLongLong("b2m", frqmarken.f22m);
  nwt7configfile->writeLongLong("au1", frqmarken.f1u1);
  nwt7configfile->writeLongLong("bu1", frqmarken.f2u1);
  nwt7configfile->writeLongLong("au2", frqmarken.f1u2);
  nwt7configfile->writeLongLong("bu2", frqmarken.f2u2);
  nwt7configfile->writeLongLong("au3", frqmarken.f1u3);
  nwt7configfile->writeLongLong("bu3", frqmarken.f2u3);
  nwt7configfile->writeBool("160m", frqmarken.b160m );
  nwt7configfile->writeBool("80m", frqmarken.b80m );
  nwt7configfile->writeBool("40m", frqmarken.b40m );
  nwt7configfile->writeBool("30m", frqmarken.b30m );
  nwt7configfile->writeBool("20m", frqmarken.b20m );
  nwt7configfile->writeBool("17m", frqmarken.b17m );
  nwt7configfile->writeBool("15m", frqmarken.b15m );
  nwt7configfile->writeBool("12m", frqmarken.b12m );
  nwt7configfile->writeBool("10m", frqmarken.b10m );
  nwt7configfile->writeBool("6m", frqmarken.b6m );
  nwt7configfile->writeBool("2m", frqmarken.b2m );
  nwt7configfile->writeBool("zusatz1", frqmarken.bu1 );
  nwt7configfile->writeBool("zusatz2", frqmarken.bu2 );
  nwt7configfile->writeBool("zusatz3", frqmarken.bu3 );
  nwt7configfile->writeString("s160m", frqmarken.st160m );
  nwt7configfile->writeString("s80m", frqmarken.st80m );
  nwt7configfile->writeString("s40m", frqmarken.st40m );
  nwt7configfile->writeString("s30m", frqmarken.st30m );
  nwt7configfile->writeString("s20m", frqmarken.st20m );
  nwt7configfile->writeString("s17m", frqmarken.st17m );
  nwt7configfile->writeString("s15m", frqmarken.st15m );
  nwt7configfile->writeString("s12m", frqmarken.st12m );
  nwt7configfile->writeString("s10m", frqmarken.st10m );
  nwt7configfile->writeString("s6m", frqmarken.st6m );
  nwt7configfile->writeString("s2m", frqmarken.st2m );
  nwt7configfile->writeString("szusatz1", frqmarken.stu1 );
  nwt7configfile->writeString("szusatz2", frqmarken.stu2 );
  nwt7configfile->writeString("szusatz3", frqmarken.stu3 );
  nwt7configfile->close();
}

///////////////////////////////////////////////////////////////////////////////
// Menubereich

void Nwt7linux::beenden()
{
  configurationspeichern(homedir.filePath(configfilename));
  wkmanager->beenden();
  if (bttyOpen)
  {
    defaultlesenrtty(); //eventuelle Daten im UART loeschen
    picmodem->closetty();
  }
  delete grafik;
  close();
}

void Nwt7linux::druckediagramm(){
  bstrlist.clear();
  QSize size = grafik->size();
  grafik->resize(500,460);
  QStringList sl = mledit->toPlainText().split("\n", QString::KeepEmptyParts);
  bstrlist = sl;
  emit drucken(printfont, bstrlist);
  grafik->resize(size);
}

void Nwt7linux::druckediagrammpdf(){
  bstrlist.clear();
  QSize size = grafik->size();
  grafik->resize(500,460);
  QStringList sl = mledit->toPlainText().split("\n", QString::KeepEmptyParts);
  bstrlist = sl;
  emit druckenpdf(printfont, bstrlist);
  grafik->resize(size);
}

void Nwt7linux::wobbelbildspeichern(){
  bstrlist.clear();
  QStringList sl = mledit->toPlainText().split("\n", QString::KeepEmptyParts);
  bstrlist = sl;
  emit bildspeichern(nbildinfo, bstrlist);
}

void Nwt7linux::setconffilename(const QString &fname)
{
  //qDebug("Nwt7linux::setconffilename(const QString &fname)");
  bkalibrierenswr = false;
  bkalibrierenswrneu = false;
  bkalibrieren40 = false;
  bkalibrieren0 = false;
  bmkalibrierenk1_0db = false;
  bmkalibrierenk1_20db = false;
  bmkalibrierenk2_0db = false;
  bmkalibrierenk2_20db = false;
  btabelleschreiben = false;
  bkurvegeladen = false;
  //Grundkonfig erzeugen
  if(bersterstart){
    QMessageBox::information( this, tr("Erster Start des Programmes","in Dialogbox"), 
                                    tr("<B>Das Programm wird das erste mal gestartet</B><BR>"
                                       "Es wird das Verzeichnis \"hfm9\" erzeugt,<BR>"
                                       "die Konfigurationsdatei und die Sondendateien.<br>"
                                       "Das Verzeichnis wird im \"Home-bereich\" des <br>"
                                       "des jeweiligen Betriebsystemes angelegt. <br><br>"
                                       "Die Defaultdaten erlauben erste Messungen. <br>"
                                       "Bessere Werte werden durch eine Kalibrierung erreicht!","in Dialogbox"));

    if(configfilename.isEmpty())configfilename = "hfm9.hfc";
    //defaultwerte laden
    configurationladen(homedir.filePath(configfilename));
    //defaultwerte speichern
    configurationspeichern(homedir.filePath(configfilename));
    //defaultwerte speichern
    messsondespeichern1("def");
    messsondespeichern2("def");
    configfile nwt7sondenfile;
    //lineare Defsonde erzeugen
    nwt7sondenfile.open(homedir.filePath("defsonde1lin.hfm"));
    nwt7sondenfile.writeString("dateiversion", "#V3.05");
    nwt7sondenfile.writeString("Sonde", "AD8361intern");
    nwt7sondenfile.writeDouble("faktor", 0.193143);
    nwt7sondenfile.writeDouble("shift", -84.634597);
    nwt7sondenfile.writeDouble("messx", 0.193143);
    nwt7sondenfile.writeDouble("messy", -84.634597);
    nwt7sondenfile.writeBool("linear", true);
    nwt7sondenfile.close();
    bersterstart = false;
  }

  configfilename = fname;
  configurationladen(homedir.filePath(configfilename));
  wkmanager->ladenconfig();
}

QString Nwt7linux::getconffilename()
{
  return configfilename;
}

QString Nwt7linux::delPath(const QDir &s)
{
  QDir qd(s); 
  QString qs, qs1;
  
  //qDebug("Nwt7linux::delPath");
  qs = qd.dirName();
  //  qs1 = qd.filePath();
  //  qs.remove(1, qs1.length());
  //qDebug(qs);
  return qs;
}

void Nwt7linux::EinstellungLaden()
{
  QString s = QFileDialog::getOpenFileName(this,
                                           tr("NWT Konfiguration laden", "FileDialog"),
                                           nullptr,
                                           tr("NWT Konfigurationsdatei (*.hfc)", "FileDialog"));
  if (!s.isNull())
  {
    configurationladen(s);
    s = delPath(s);
#ifdef LDEBUG
    qDebug(s);
#endif
    configfilename = s;
    setAText(wgrunddaten.str_tty);
  }
}

void Nwt7linux::EinstellungSichern()
{
  QString s = QFileDialog::getSaveFileName(this,
                                           tr("NWT Konfiguration speichern", "FileDialog"),
                                           nullptr,
                                           tr("NWT Konfigurationsdatei (*.hfc)", "FileDialog"));
  if (!s.isNull())
  {
    //Datei ueberpruefen ob Sufix vorhanden
    if((s.indexOf(".")== -1)) s += ".hfc";
    configurationspeichern(s);
    wkmanager->beenden();
    s = delPath(s);
    //qDebug(s);
    configfilename = s;
    setAText(wgrunddaten.str_tty);
    configurationladen(homedir.filePath(configfilename));
  }
}

void Nwt7linux::getwkm(int i){
  switch(i){
    case 1:
      wobdaten.kurvennr = 1;
    break;
    case 2:
      wobdaten.kurvennr = 2;
    break;
    case 3:
      wobdaten.kurvennr = 3;
    break;
    case 4:
      wobdaten.kurvennr = 4;
    break;
    case 5:
      wobdaten.kurvennr = 5;
    break;
    case 6:
      wobdaten.kurvennr = 6;
  break;

  }
//  warneichkorr();
  QString qs;
  if(infoueber.count() > 0){
    qs = "";
    if(infoueber.at(0).indexOf(";") == 0)qs = qs = infoueber.at(0);
    if(infoueber.at(0).indexOf(";no_label") == 0)qs = "";
  }
  wkmanager->suebergabe(wobdaten, qs);
}

void Nwt7linux::wkmloadkurve(int i){
  
  enumkurvenversion ekv = ekvkeine;
  TWobbel walt;
  
  walt = wobdaten;
  ekv = loadkurve();
  if(ekv == ekvabbruch)return;
  switch(i){
    case 1:
      wobdaten.kurvennr = 1;
    break;
    case 2:
      wobdaten.kurvennr = 2;
    break;
    case 3:
      wobdaten.kurvennr = 3;
    break;
    case 4:
      wobdaten.kurvennr = 4;
    break;
    case 5:
      wobdaten.kurvennr = 5;
    break;
    case 6:
      wobdaten.kurvennr = 6;
  break;

  }
  wobdaten.bhintergrund = true;
  wobdaten.mousesperre = true;
//  warneichkorr();
  wkmanager->suebergabe(wobdaten, displaystr);
  wobdaten = walt;
}

enumkurvenversion Nwt7linux::loadkurve()
{
  enumkurvenversion ekv = ekvkeine;
  int i,j;
  int ikversion = 0; //neue Versionserkennung
/*
  bool v305 = false;
  bool v404 = false;
  bool v405 = false;
  bool v410 = false;
*/
  QString zeile;
  QString qs;
  QString qs1 = "";
  double faktor1, faktor2;
  double shift1, shift2;
  bool ok;
  
  QFile f;
  QStringList filters;
  filters << "Image files (*.png *.xpm *.jpg)"
          << "Text files (*.txt)"
          << "Any files (*)";

  QString s = QFileDialog::getOpenFileName(this, tr("NWT Kurve laden","FileDialog"),
                                                 kurvendir.absolutePath(),
                                                 tr("NWT Kurvendatei (*.hfd)","FileDialog")
                                                 );
/*
  
  QString s = QFileDialog::getOpenFileName(
                    kurvendir.filePath("*.hfd"),
                    tr("NWT Kurvendatei (*.hfd)","FileDialog"),
                    this, 0, tr("NWT Kurve laden","FileDialog"));
*/  
  displaystr = "";
  //String ist nicht leer
  if (!s.isNull())
  {
    displaystr = s;
    kurvendir.setPath(s);
    qs = kurvendir.absolutePath();
    //qDebug(qs);
    //Filename setzen
    f.setFileName(s);
    if(f.open( QIODevice::ReadOnly )){
      QTextStream ts(&f);
      //erste Zeile lesen
      ts >> zeile;
      ikversion = 100;
      if(zeile == "#V3.05")ikversion = 305;
      if(zeile == "#V4.04")ikversion = 404;
      if(zeile == "#V4.05")ikversion = 405;
      if(zeile == "#V4.10")ikversion = 410;
/*
      if(zeile == "#V3.05")v305 = true;
      if(zeile == "#V4.04"){
        v404 = true;
        v305 = true;
      }
      if(zeile == "#V4.05"){
        v405 = true;
        v404 = true;
        v305 = true;
      }
      if(zeile == "#V4.10"){
        v410 = true;
        v405 = true;
        v404 = true;
        v305 = true;
      }
*/
  //if(!v305){
    if(ikversion < 305){ //erste eingelesene Zeile ist keine Versionsbezeichnung
        wobdaten.anfangsfrequenz = zeile.toDouble(&ok);
      }else{
        //Es ist V4.10: diese Zeile entscheidet ob NF oder HF Wobblung
        if(ikversion == 410){
          ts >> i; // einlesen
          wobdaten.maudio2 = false;
          if(i == 1)wobdaten.maudio2 = true;
        }
        // Version >= 305 Anfangsfrequenz einlesen
        ts >> wobdaten.anfangsfrequenz;
      }
      //qDebug("a1 %li", wobdaten.anfangsfrequenz);
      ts >> wobdaten.schrittfrequenz;
      ts >> wobdaten.schritte;
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
        //if(!v305){
          if(ikversion < 305){
            //Version < 3.05 faktor + shift umrechnen
            wobdaten.faktor1 = faktor1 / 10.23;
            wobdaten.shift1 = ((((511.5 - shift1) / faktor1) * wobdaten.faktor1) + 40.0) * -1.0;
            wobdaten.faktor2 = faktor2 / 10.23;
            wobdaten.shift2 = ((((511.5 - shift2) / faktor2) * wobdaten.faktor2) + 40.0) * -1.0;
/*
            qDebug("f1 %f",faktor1);
            qDebug("s1 %f",shift1);
            qDebug("f2 %f",faktor2);
            qDebug("s2 %f",shift2);
            qDebug("f1 %f",wobdaten.faktor1);
            qDebug("s1 %f",wobdaten.shift1);
            qDebug("f2 %f",wobdaten.faktor2);
            qDebug("s2 %f",wobdaten.shift2);
*/
          }else{
            //Version >= 3.05
            wobdaten.faktor1 = faktor1;
            wobdaten.shift1 = shift1;
            wobdaten.faktor2 = faktor2;
            wobdaten.shift2 = shift2;
          }
        break;
        //lin
        case 2:
        //if(!v305){
          if(ikversion < 305){
            //Version < 3.05 faktor + shift umrechnen
            wobdaten.faktor1 = faktor1 / 9.3;
            wobdaten.shift1 = ((((465.0 - shift1) / faktor1) * wobdaten.faktor1) + 50.0) * -1.0;
            wobdaten.faktor2 = faktor2 / 9.3;
            wobdaten.shift2 = ((((465.0 - shift2) / faktor2) * wobdaten.faktor2) + 50.0) * -1.0;
/*
            qDebug("f1 %f",faktor1);
            qDebug("s1 %f",shift1);
            qDebug("f2 %f",faktor2);
            qDebug("s2 %f",shift2);
            qDebug("f1 %f",wobdaten.faktor1);
            qDebug("s1 %f",wobdaten.shift1);
            qDebug("f2 %f",wobdaten.faktor2);
            qDebug("s2 %f",wobdaten.shift2);
*/
          }else{
            //Version >= 3.05
            wobdaten.faktor1 = faktor1;
            wobdaten.shift1 = shift1;
            wobdaten.faktor2 = faktor2;
            wobdaten.shift2 = shift2;
          }
        break;
      }
      switch(i){
        case 0: //wobbeln dB
          wobdaten.ebetriebsart = ewobbeln;
          betriebsart->setCurrentIndex (0);
          wobdaten.linear1 = false;
          wobdaten.linear2 = false;
          setTabEnabled(nwt7messen, true);
        break;
        case 1: // wobbeln SWR
          wobdaten.ebetriebsart = eswrneu;
          betriebsart->setCurrentIndex (1);
          setTabEnabled(nwt7messen, true);
        break;
        case 2: // wobbeln linear
          wobdaten.ebetriebsart = ewobbeln;
          betriebsart->setCurrentIndex (0);
          wobdaten.linear1 = true;
          wobdaten.linear2 = true;
          setTabEnabled(nwt7messen, false);
        break;
        case 3: // wobbeln SWR ANT
          wobdaten.ebetriebsart = eswrant;
          betriebsart->setCurrentIndex (2);
          setTabEnabled(nwt7messen, true);
        break;
        case 4: // wobbeln Z
          wobdaten.ebetriebsart = eimpedanz;
          betriebsart->setCurrentIndex (3);
          setTabEnabled(nwt7messen, false);
        break;
        case 5: // wobbeln SAV
          wobdaten.ebetriebsart = espektrumanalyser;
          betriebsart->setCurrentIndex (4);
          wobdaten.linear1 = false;
          wobdaten.linear2 = false;
          setTabEnabled(nwt7messen, false);
        //if(v404){
          if(ikversion > 305){
            // Version > 3.05
            //noch zusaetliche Daten fuer die dBm Darstellung einlesen
            ts >> wobdaten.psavabs;
            ts >> wobdaten.psav300;
            ts >> wobdaten.psav7k;
            ts >> wobdaten.psav30k;
            ts >> j;
            switch(j){
              case 0: // keine Bandbreitenerkennung
                wobdaten.ebbsav = ekeinebandbreite;
                break;
              case 1: // 300Hz Kanal1
                wobdaten.ebbsav = e300b1;
                break;
              case 2: // 7kHz Kanal1
                wobdaten.ebbsav = e7kb1;
                break;
              case 3: // 30kHz Kanal1
                wobdaten.ebbsav = e30kb1;
                break;
              case 4: // 300Hz Kanal2
                wobdaten.ebbsav = e300b2;
                break;
              case 5: // 7kHz Kanal2
                wobdaten.ebbsav = e7kb2;
                break;
              case 6: // 30kHz Kanal2
                wobdaten.ebbsav = e30kb2;
                break;
            }
            ts >> wobdaten.displayshift;
          }else{
            //Version < 4.04
            //alte Versionen holen die fehlenden Daten aus dem Programm
            wobdaten.psavabs = wobdatenmerk.psavabs;
            wobdaten.psav300 = wobdatenmerk.psav300;
            wobdaten.psav7k = wobdatenmerk.psav7k;
            wobdaten.psav30k = wobdatenmerk.psav30k;
            wobdaten.ebbsav = wobdatenmerk.ebbsav;
            wobdaten.displayshift = wobdatenmerk.displayshift;
          }
        break;
      }
      for(i=0; i<wobdaten.schritte; i++){
        ts >> wobdaten.mess.k1[i];
      }
      for(i=0; i<wobdaten.schritte; i++){
        ts >> wobdaten.mess.k2[i];
      }
      // Beschreibung noch einlesen
    //if(!v405){
      if(ikversion < 405){
        //Version < 4.05
        qs = "";
        qs1 = "";
        while(!ts.atEnd()){
          ts >> qs1;
          qs = qs + qs1;
        }
      }else{
        ts >> qs;
        //qDebug(qs);
        //qDebug(wobdaten.layoutlabel);
        //Kalibrierung lesen
        for(i=0; i < maxmesspunkte; i++){
          ts >> wobdaten.kalibrierarray.arrayk1[i];
        }
        for(i=0; i < maxmesspunkte; i++){
          ts >> wobdaten.kalibrierarray.arrayk2[i];
        }
        for(i=0; i < maxmesspunkte; i++){
          ts >> wobdaten.swrkalibrierarray.arrayk1[i];
        }
        for(i=0; i < maxmesspunkte; i++){
          ts >> wobdaten.swrkalibrierarray.arrayk2[i];
        }
        for(i=0; i < calmaxindex; i++){
          ts >> wobdaten.savcalarray.p[i];
        }
      }
      //Test ob Label vorhanden
      if(qs.length() == 0)qs = ";no_label";
      //Ab SW 4.09 wird beim Label ein Semikolon voran gestellt
      if(qs[0] != QChar(';'))qs = ";" + qs;
      //qDebug(qs);
      f.close();
      //Infoueberschrift setzen
      mledit->clear();
      infoueber.clear();
      if(qs.contains("@;")){
        qs1 = qs.section("@",0,0);
        displaystr += " - " + qs1;
        //qDebug(qs1);
        infoueber.append(qs1);
        mledit->append(qs1);
        qs1 = qs.section("@",1,1);
        //qDebug(qs1);
        infoueber.append(qs1);
        mledit->append(qs1);
        qs1 = qs.section("@",2,2);
        //qDebug(qs1);
        infoueber.append(qs1);
        mledit->append(qs1);
        qs1 = qs.section("@",3,3);
        //qDebug(qs1);
        infoueber.append(qs1);
        mledit->append(qs1);
      }else{
        infoueber.append(qs);
        mledit->append(qs);
        displaystr += " - " + qs;
      }
      //Werte die in der Kurve nicht enthalten sind
      //muessen definiert gesetzt werden
      //kein dB Shift
      wobdaten.dbshift1 = 0.0;
      wobdaten.dbshift2 = 0.0;
      //===================
      wobdaten.mess.daten_enable = true;
/*
      if(v305)ekv = ekv305;
      if(v404)ekv = ekv404;
      if(v405)ekv = ekv405;
      if(v410)ekv = ekv410;
*/
    }
  }
  switch(ikversion){
  case 100: ekv = ekvabbruch; break;
  case 305: ekv = ekv305; break;
  case 404: ekv = ekv404; break;
  case 405: ekv = ekv405; break;
  case 410: ekv = ekv410; break;
  default: ekv = ekvabbruch; break;
  }

  return ekv;
}


void Nwt7linux::KurvenLaden()
{
  enumkurvenversion ekv;
  bool v305 = false;
  //bool v404 = false;
  //bool v405 = false;
  //bool v410 = false;
  QString s;
  
//  warneichkorr();
  if(bkurvegeladen){
    wobdaten = wobdatenmerk;
    wobdaten.mess.daten_enable = true;
    wobdaten.bhintergrund = false;
    wobdaten.mousesperre = false;
    mledit->clear();
    fsendewobbeldaten();
  }
  wobdatenmerk = wobdaten; //alte Einstellung sichern
  //Bandbreitenanzeige ausschalten
  checkbox3db->setChecked(false);
  checkbox6db->setChecked(false);
  checkboxgrafik->setChecked(false);
  checkboxinvers->setChecked(false);
  bkurvegeladen = true;
  ekv = loadkurve();
  switch(ekv){
    case ekvabbruch:
      return;
      break;
    case ekvkeine:
      v305 = false;
      //v404 = false;
      //v405 = false;
      //v410 = false;
      break;
    case ekv305:
      v305 = true;
      //v404 = false;
      //v405 = false;
      //v410 = false;
      s = " #V3.05";
      break;
    case ekv404:
      v305 = true;
      //v404 = true;
      //v405 = false;
      //v410 = false;
      s = " #V4.04";
      break;
    case ekv405:
      v305 = true;
      //v404 = true;
      //v405 = true;
      //v410 = false;
      s = " #V4.05";
      break;
    case ekv410:
      v305 = true;
      //v404 = true;
      //v405 = true;
      //v410 = true;
      s = " #V4.10";
      break;
  }
  //Werte die in der Kurve nicht enthalten sind
  //muessen definiert gesetzt werden
  //kein dB Shift
  wobdaten.dbshift1 = 0.0;
  wobdaten.dbshift2 = 0.0;
  //keine Eichkorrektur
  //wobdaten.beichkorrk1 = false;
  //wobdaten.beichkorrk2 = false;
  //===================
  wobdaten.mess.daten_enable = true;
  wobdaten.bhintergrund = false;
  wobdaten.mousesperre = false;
  //qDebug("a %li", wobdaten.anfangsfrequenz);
  fsendewobbeldaten();
  grafik->setWindowTitle(tr("Display","Wobbelfenster") + " - " + displaystr + " - " + s);
  if(!v305){
    QMessageBox::warning( this, tr("Kurvendatei alte Version","Kurvendatei laden"), 
                                tr("<B>Kurvendatei Kalibrierdaten sind im alten Format! </B><BR>"
                                  "In jeder Kurvendatei befinden sich auch die Kalibrierdaten <BR>"
                                  "des verwendeten Messkopfes. Die Kalibrierung der Messonde <br>"
                                  "hat ab der Version 4.00 ein neues Format. Die Kalibrierdaten wurden <br>"
                                  "automatisch umgerechnet. Soll diese Datei im neuen <br>"
                                  "Format gespeichert werden, so muss die Datei noch einmal <br>"
                                  "aus diesem Programm heraus abgespeichert werden.","Kurvendatei laden"));
  }
}

void Nwt7linux::MenuKurvenSichern(){
  TWobbel walt;

  walt = wobdaten;
  wobdaten = grafik->getWobbeldaten();
  KurvenSichern();
  wobdaten = walt;
}

void Nwt7linux::wkmsavekurve(const TWobbel &awobbel){
  TWobbel walt;
  
  walt = wobdaten;
  wobdaten = awobbel;
  KurvenSichern();
  wobdaten = walt;
}

void Nwt7linux::KurvenSichern()
{
  int i,j;
  QFile f;
  QString qs;

  fsendewobbeldaten();
  QString s = QFileDialog::getSaveFileName(this,
                                           tr("NWT Kurve speichern","FileDialog"),
                                           kurvendir.absolutePath(),
                                           tr("NWT Kurvendatei (*.hfd)","FileDialog")
                                           );
/*  
  QString s = QFileDialog::getSaveFileName(
                    kurvendir.filePath("*.hfd"),
                    tr("NWT Kurvendatei (*.hfd)","FileDialog"),
                    this, 0, tr("NWT Kurve speichern","FileDialog"));
*/
  if (!s.isNull())
  {
    kurvendir.setPath(s);
    qs = kurvendir.absolutePath();
    //qDebug(qs);
    //Datei ueberpruefen ob Sufix vorhanden
    if((s.indexOf(".")== -1)) s += ".hfd";
    f.setFileName(s);
    if(f.open( QIODevice::WriteOnly )){
      QTextStream ts(&f);
      ts << "#V4.10" << endl;
      if(wobdaten.maudio2){
        ts << 1 << endl;
      }else{
        ts << 0 << endl;
      }
      ts << wobdaten.anfangsfrequenz << endl;
      ts << wobdaten.schrittfrequenz << endl;
      //qDebug() << "Neu 2: " << wobdaten.schritte;
      ts << wobdaten.schritte << endl;
      ts << wobdaten.faktor1 << endl;
      ts << wobdaten.shift1 << endl;
      ts << wobdaten.faktor2 << endl;
      ts << wobdaten.shift2 << endl;
      switch(wobdaten.ebetriebsart){
       case ewobbeln:
         if(wobdaten.linear1 or wobdaten.linear2){
           //linear == 2
           ts << 2 << endl;
         }else{
           //log == 0
           ts << 0 << endl;
         }
         break;
       case eswrneu:
         //swr == 1
         ts << 1 << endl;
         break;
       case eswrant:
         //swr == 3
         ts << 3 << endl;
         break;
       case eimpedanz:
         //impedanz == 4
         ts << 4 << endl;
         break;
       case espektrumanalyser:
       case espekdisplayshift:
         //speky == 5
         ts << 5 << endl;
         ts << wobdaten.psavabs << endl;
         ts << wobdaten.psav300 << endl;
         ts << wobdaten.psav7k << endl;
         ts << wobdaten.psav30k << endl;
         switch(wobdaten.ebbsav){
           case ekeinebandbreite:
             ts << 0 << endl;
             break;
           case e300b1:
             ts << 1 << endl;
             break;
           case e7kb1:
             ts << 2 << endl;
             break;
           case e30kb1:
             ts << 3 << endl;
             break;
           case e300b2:
             ts << 4 << endl;
             break;
           case e7kb2:
             ts << 5 << endl;
             break;
           case e30kb2:
             ts << 6 << endl;
             break;
         }
         ts << wobdaten.displayshift << endl;
         break;
       case eantdiagramm:
         break;
      }
      //Messung speichern
      for(i=0; i<wobdaten.schritte; i++){
        ts << wobdaten.mess.k1[i] << endl;
      }
      for(i=0; i<wobdaten.schritte; i++){
        ts << wobdaten.mess.k2[i] << endl;
      }
      //Beschreibungstext speichern
      qDebug() << infoueber.count();
      //keine Beschreibung?
      if(infoueber.count() == 0){
        infoueber.append(";no_label");
      }else{
        // max 4 Zeilen zusammenfassen
        j = infoueber.count();
        if(j > 4)j=4;
        qs = infoueber.at(0);
        for(i=1;i<j;i++){
          qs = qs + "@" + infoueber.at(i);
        }
        //die Leerzeichen muessen entfernt werden
        qDebug() << qs;
        while(qs.contains(' ')){
          qs = qs.replace(' ', '_');
        }
        qDebug() << qs;
      }
      ts << qs << endl;
      //Kalibrierung speichern
      for(i=0; i < maxmesspunkte; i++){
        ts << wobdaten.kalibrierarray.arrayk1[i] << endl;
      }
      for(i=0; i < maxmesspunkte; i++){
        ts << wobdaten.kalibrierarray.arrayk2[i] << endl;
      }
      for(i=0; i < maxmesspunkte; i++){
        ts << wobdaten.swrkalibrierarray.arrayk1[i] << endl;
      }
      for(i=0; i < maxmesspunkte; i++){
        ts << wobdaten.swrkalibrierarray.arrayk2[i] << endl;
      }
      for(i=0; i < calmaxindex; i++){
        ts << wobdaten.savcalarray.p[i] << endl;
      }
      f.close();
    }
    QString finfo = s;
    if((finfo.indexOf(".")== -1)) s += ".hfd";
    finfo.replace(QString(".hfd"), ".info");
    f.setFileName(finfo);
    if(f.open( QIODevice::WriteOnly )){
      QTextStream ts(&f);
      for(i=0;i < infoueber.count();i++){
        ts << infoueber.at(i);
        ts << "\r\n" << flush;
      }
      for(i=0;i < mleditlist.count();i++){
        ts << mleditlist.at(i);
        ts << "\r\n" << flush;
      }
      f.close();
    }
    if(wobdaten.ebetriebsart == ewobbeln){
      if(QMessageBox::question(this, tr("CSV-Datei erzeugen","WarnungsBox"), 
                                     tr("<B>Es wird zusatzlich eine CSV-Datei mit gleichem Name erzeugt.</B><BR>"
                                        "Datei speichern?","WarnungsBox"),
                                     tr("&Ja","WarungsBox"), tr("&Nein","WarnungsBox"),
                                        QString::null, 1, 0) == 0){
        if((s.indexOf(".")== -1)) s += ".hfd";
        s.replace(QString(".hfd"), ".csv");
        f.setFileName(s);
        if(f.open( QIODevice::WriteOnly )){
          QTextStream ts(&f);
          double anfangsfrq;
          double schrittweite;
          double faktor1;
          double faktor2;
          double shift1;
          double shift2;
          //int schritte;
          int k1, k2;
          double f1, f2, db1, db2, frq;
          QString qs1;
          
          anfangsfrq = wobdaten.anfangsfrequenz;
          schrittweite = wobdaten.schrittfrequenz;
          //schritte = wobdaten.schritte;
          faktor1 = wobdaten.faktor1;
          faktor2 = wobdaten.faktor2;
          shift1 = wobdaten.shift1;
          shift2 = wobdaten.shift2;
          if(wobdaten.linear1 or wobdaten.linear2){
            ts << "Frequenz(Hz)";
            if(wobdaten.bkanal1)
              ts << ";Kanal1;Kanal1(dB)";
            if(wobdaten.bkanal2)
              ts << ";Kanal2;Kanal2(dB)";
            ts << endl;
          }else{
            ts << "Frequenz(Hz)";
            if(wobdaten.bkanal1)
              ts << ";Kanal1(dB)";
            if(wobdaten.bkanal2)
              ts << ";Kanal2(dB)";
            ts << endl;
          }
          for(i=0; i<wobdaten.schritte; i++){
            k1 = wobdaten.mess.k1[i];
            k2 = wobdaten.mess.k2[i];
            db1 = double(k1) * faktor1 + shift1;
            db2 = double(k2) * faktor2 + shift2;
            frq = anfangsfrq + (double(i) * schrittweite);
            if(wobdaten.linear1 or wobdaten.linear2){
              f1 = db1;
              f2 = db2;
              f1 = (100.0 + f1) / 100.0;
              db1 = 20.0 * log10(f1);
              f2 = (100.0 + f2) / 100.0;
              db2 = 20.0 * log10(f2);
              qs.sprintf("%9.0f", frq);
              if(wobdaten.bkanal1){
                qs1.sprintf(";%1.5f;%2.5f",f1,db1);
                qs = qs + qs1;
              }
              if(wobdaten.bkanal2){
                qs1.sprintf(";%1.5f;%2.5f",f2,db2);
                qs = qs + qs1;
              }
            }else{
              db1 = db1 - getkalibrierwertk1(frq) + wobdaten.dbshift1 + double(wobdaten.displayshift);
              db2 = db2 - getkalibrierwertk2(frq) + wobdaten.dbshift2 + double(wobdaten.displayshift);
              qs.sprintf("%9.0f", frq);
              if(wobdaten.bkanal1){
                qs1.sprintf(";%2.5f",db1);
                qs = qs + qs1;
              }
              if(wobdaten.bkanal2){
                qs1.sprintf(";%2.5f",db2);
                qs = qs + qs1;
              }
            }
            qs.replace('.',',');
            ts << qs << endl;
          }
          f.close();
        }
      }
    }
  }
}

double Nwt7linux::korrsavpegel(double p)
{
  #ifdef LDEBUG
  qDebug("Nwt7linux::korrsavpegel(double p)");
  #endif
  if(wobdaten.ebbsav == e300b1)p = p + wobdaten.psav300;
  if(wobdaten.ebbsav == e7kb1)p = p + wobdaten.psav7k;
  if(wobdaten.ebbsav == e30kb1)p = p + wobdaten.psav30k;
  if(wobdaten.ebbsav == e300b2)p = p + wobdaten.psav300;
  if(wobdaten.ebbsav == e7kb2)p = p + wobdaten.psav7k;
  if(wobdaten.ebbsav == e30kb2)p = p + wobdaten.psav30k;
  p = p + wobdaten.psavabs;
  return p;
}

double Nwt7linux::getkalibrierwertk1(double afrequenz)
{
  double j;
  int i;
  
  #ifdef LDEBUG
//    qDebug("WidgetWobbeln::getkalibrierwertk1()");
  #endif
  if(wgrunddaten.kalibrierk1 and wobdaten.ebetriebsart == ewobbeln)
  {
    //qDebug("WidgetWobbeln::getkalibrierwertk1()");
    j = round(afrequenz / eteiler);
    i = int(j);
    if(i > (maxmesspunkte-1))i = maxmesspunkte-1;
    return wobdaten.kalibrierarray.arrayk1[i];
  }else{
    return 0.0;
  }
}

double Nwt7linux::getkalibrierwertk2(double afrequenz)
{
  double j;
  int i;
  
  #ifdef LDEBUG
//    qDebug("WidgetWobbeln::getkalibrierwertk1()");
  #endif
  if(wgrunddaten.kalibrierk2 and wobdaten.ebetriebsart == ewobbeln)
  {
    //qDebug("WidgetWobbeln::getkalibrierwertk1()");
    j = round(afrequenz / eteiler);
    i = int(j);
    if(i > (maxmesspunkte-1))i = maxmesspunkte-1;
    return wobdaten.kalibrierarray.arrayk2[i];
  }else{
    return 0.0;
  }
}

double Nwt7linux::getsavcalwert(double afrequenz)
{
  int a,b;
  double pegela, pegelb, pegeldiff;
  double frequa, frequb, frequdiff;
  
  #ifdef LDEBUG
    qDebug("Nwt7linux::getsavcalwert()");
  #endif
  //aus der Frequenz den Index errechnen
  // Bereich 0 - 100 GHz
  a = int(savcalkonst * log10(afrequenz));
  b = a + 1;
  //arraygrenzen einhalten
  if(a<0)a=0;
  if(a > calmaxindex-1)a = calmaxindex-1;
  if(b > calmaxindex-1)b = calmaxindex-1;
  pegela = wobdaten.savcalarray.p[a];
  // Ist das Ende des Array erreicht?
  if(a != b){
    //Eine Iteration ist moeglich
    pegelb = wobdaten.savcalarray.p[b];
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

double Nwt7linux::getswrkalibrierwertk1(double afrequenz)
{
  double j;
  int i;
  double w1;
  
  #ifdef LDEBUG
    qDebug("Nwt7linux::getswrkalibrierwertek1()");
  #endif
  j = round(afrequenz / eteiler);
  i = int(j);
  if(i > (maxmesspunkte-3))i = maxmesspunkte-3;
//  return wswrkalibrierarray.arrayk1[i];
  w1 = (wobdaten.swrkalibrierarray.arrayk1[i-2] +
        wobdaten.swrkalibrierarray.arrayk1[i-1] +  
        wobdaten.swrkalibrierarray.arrayk1[i] +  
        wobdaten.swrkalibrierarray.arrayk1[i+1] +  
	wobdaten.swrkalibrierarray.arrayk1[i+2]) / 5.0;
  return (w1);
}


///////////////////////////////////////////////////////////////////////////////
// Funktionen im Wobbelbereich
void Nwt7linux::infosetfont(){
  bool ok;

  infofont = QFontDialog::getFont( &ok, infofont, this);
  if(ok){
    mledit->setFont(infofont);
  }
}

void Nwt7linux::messsondenfilek1laden(const QString &filename){
  int i;
  QFile f;
  QString qs;
  QString s;
  QString snum;
  QString version;
  double faktor;
  double shift;
  
  s = filename;
  if (!s.isNull()){
    if(bkurvegeladen){
      wobdaten = wobdatenmerk;
      wobdaten.mess.daten_enable = true;
      wobdaten.bhintergrund = false;
      wobdaten.mousesperre = false;
      bkurvegeladen = false;
      fsendewobbeldaten();
    }
//    betriebsart->setCurrentIndex (0);
    QFileInfo fi(s);
    filenamesonde1 = fi.baseName();
    configfile nwt7sondenfile;
    nwt7sondenfile.open(homedir.filePath(s));
    qs = nwt7sondenfile.readString("Sonde", "AD8307intern");
    checkboxk1->setText(filenamesonde1+".hfm");
    wobdaten.beschreibung1 = qs;
    qs = nwt7sondenfile.readString("dateiversion", "#V3.05");
    if(qs == "#V3.05")sw305 = true;
    faktor = nwt7sondenfile.readDouble("faktor", 0.191187);
    shift = nwt7sondenfile.readDouble("shift", -87.139634);
    wobdaten.linear1 = nwt7sondenfile.readBool("linear", false);
    //Ab Firmware 1.20
    wobdaten.maudio1 = nwt7sondenfile.readBool("audio_nf", false);
    //Umrechnung der Messsondendaten wenn nicht die richtige Version
    if(!wobdaten.linear1){
      //log
      if(sw305){
        wobdaten.faktor1 = faktor;
        wobdaten.shift1 = shift;
      }else{
        wobdaten.faktor1 = faktor / 10.23;
        wobdaten.shift1 = ((((511.5 - shift) / faktor) * wobdaten.faktor1) + 40.0) * -1.0;
      }
    }else{
      //lin
      if(sw305){
        wobdaten.faktor1 = faktor;
        wobdaten.shift1 = shift;
      }else{
        wobdaten.faktor1 = faktor / 9.3;
        wobdaten.shift1 = ((((465.0 - shift) / faktor) * wobdaten.faktor1) + 50.0) * -1.0;
      }
    }
    /*
    qDebug(qs);
    qDebug("f %f",wobdaten.faktor1);
    qDebug("s %f",wobdaten.shift1);
    */
    wobdaten.mfaktor1 = nwt7sondenfile.readDouble("messx", 0.193143);
    if(fwversion == 120){
      if(wobdaten.mfaktor1 > 0.01)wobdaten.mfaktor1 = wobdaten.mfaktor1 / 32.0;
    }
    wobdaten.mshift1 = nwt7sondenfile.readDouble("messy", -84.634597);
    //wobdaten.beichkorrk1 = nwt7sondenfile.readBool("eichkorrk1", false);
    wattoffsetk1[0].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr0", "0 - 10 MHz +/-0,0dB");
    wattoffsetk1[1].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr1", "10 - 20 MHz  0,8dB");
    wattoffsetk1[2].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr2", "20 - 50 MHz  1,0dB");
    wattoffsetk1[3].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr3", "50 - 100 MHz  2,0dB");
    wattoffsetk1[4].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr4", "100 - 150 MHz  2,5dB");
    wattoffsetk1[5].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr5", "150 - 200 MHz  3,0dB");
    wattoffsetk1[6].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr6", "200 - 300 MHz  4,0dB");
    wattoffsetk1[7].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr7", "300 - 400 MHz  6,0dB");
    wattoffsetk1[8].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr8", "400 - 500 MHz  9,0dB");
    wattoffsetk1[9].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr9", "NN");
    wattoffsetk1[10].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr10", "NN");
    wattoffsetk1[11].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr11", "NN");
    wattoffsetk1[12].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr12", "NN");
    wattoffsetk1[13].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr13", "NN");
    wattoffsetk1[14].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr14", "NN");
    wattoffsetk1[15].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr15", "NN");
    wattoffsetk1[16].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr16", "NN");
    wattoffsetk1[17].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr17", "NN");
    wattoffsetk1[18].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr18", "NN");
    wattoffsetk1[19].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr19", "NN");
    wattoffsetk1[20].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr20", "NN");
    wattoffsetk1[21].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr21", "NN");
    wattoffsetk1[22].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr22", "NN");
    wattoffsetk1[23].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr23", "NN");
    wattoffsetk1[24].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr24", "NN");
    wattoffsetk1[25].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr25", "NN");
    wattoffsetk1[26].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr26", "NN");
    wattoffsetk1[27].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr27", "NN");
    wattoffsetk1[28].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr28", "NN");
    wattoffsetk1[29].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr29", "NN");
    boxwattoffset1->clear();
    for(i=0;i<30;i++){
      snum.sprintf("%2i| ",i+1);
      boxwattoffset1->addItem(snum + wattoffsetk1[i].str_offsetwatt);
    }
    wattoffsetk1[0].offsetwatt = nwt7sondenfile.readDouble("wattoffset0", 0.0);
    wattoffsetk1[1].offsetwatt = nwt7sondenfile.readDouble("wattoffset1", 0.8);
    wattoffsetk1[2].offsetwatt = nwt7sondenfile.readDouble("wattoffset2", 1.0);
    wattoffsetk1[3].offsetwatt = nwt7sondenfile.readDouble("wattoffset3", 2.0);
    wattoffsetk1[4].offsetwatt = nwt7sondenfile.readDouble("wattoffset4", 2.5);
    wattoffsetk1[5].offsetwatt = nwt7sondenfile.readDouble("wattoffset5", 3.0);
    wattoffsetk1[6].offsetwatt = nwt7sondenfile.readDouble("wattoffset6", 4.0);
    wattoffsetk1[7].offsetwatt = nwt7sondenfile.readDouble("wattoffset7", 6.0);
    wattoffsetk1[8].offsetwatt = nwt7sondenfile.readDouble("wattoffset8", 9.0);
    wattoffsetk1[9].offsetwatt = nwt7sondenfile.readDouble("wattoffset9", 0.0);
    wattoffsetk1[10].offsetwatt = nwt7sondenfile.readDouble("wattoffset10", 0.0);
    wattoffsetk1[11].offsetwatt = nwt7sondenfile.readDouble("wattoffset11", 0.0);
    wattoffsetk1[12].offsetwatt = nwt7sondenfile.readDouble("wattoffset12", 0.0);
    wattoffsetk1[13].offsetwatt = nwt7sondenfile.readDouble("wattoffset13", 0.0);
    wattoffsetk1[14].offsetwatt = nwt7sondenfile.readDouble("wattoffset14", 0.0);
    wattoffsetk1[15].offsetwatt = nwt7sondenfile.readDouble("wattoffset15", 0.0);
    wattoffsetk1[16].offsetwatt = nwt7sondenfile.readDouble("wattoffset16", 0.0);
    wattoffsetk1[17].offsetwatt = nwt7sondenfile.readDouble("wattoffset17", 0.0);
    wattoffsetk1[18].offsetwatt = nwt7sondenfile.readDouble("wattoffset18", 0.0);
    wattoffsetk1[19].offsetwatt = nwt7sondenfile.readDouble("wattoffset19", 0.0);
    wattoffsetk1[20].offsetwatt = nwt7sondenfile.readDouble("wattoffset20", 0.0);
    wattoffsetk1[21].offsetwatt = nwt7sondenfile.readDouble("wattoffset21", 0.0);
    wattoffsetk1[22].offsetwatt = nwt7sondenfile.readDouble("wattoffset22", 0.0);
    wattoffsetk1[23].offsetwatt = nwt7sondenfile.readDouble("wattoffset23", 0.0);
    wattoffsetk1[24].offsetwatt = nwt7sondenfile.readDouble("wattoffset24", 0.0);
    wattoffsetk1[25].offsetwatt = nwt7sondenfile.readDouble("wattoffset25", 0.0);
    wattoffsetk1[26].offsetwatt = nwt7sondenfile.readDouble("wattoffset26", 0.0);
    wattoffsetk1[27].offsetwatt = nwt7sondenfile.readDouble("wattoffset27", 0.0);
    wattoffsetk1[28].offsetwatt = nwt7sondenfile.readDouble("wattoffset28", 0.0);
    wattoffsetk1[29].offsetwatt = nwt7sondenfile.readDouble("wattoffset29", 0.0);
    nwt7sondenfile.close();
    wobbelungueltig();
    testLogLin();
    s.replace(QString(".hfm"), "k1.hfe");
    //Filename setzen
    f.setFileName(homedir.filePath(s));
    if(f.open( QIODevice::ReadOnly )){
      QTextStream ts(&f);
      ts >> version;
      if(version != "#V2.02"){
        wgrunddaten.kalibrierk1 = false;
        QMessageBox::warning( this, tr("Mathematische-Kalibrierkorrektur Kanal 1","Warnung"),
                                    tr("<B>Kalibrierkorrekturdatei ist ungueltig</B><BR>"
                                       "Die Datei hat eine kleinere Version als V2.02. "
                                       "Bitte den Kanal 1 neu mit Math-Kalibrierkorrektur kalibrieren. "
                                       "Die Datei ist nicht mehr verwendbar, da "
                                       "die mathematischen Funktionen neu programmiert "
                                       "wurden. Nach einer erneuten Kalibrierung mit Math- "
                                       "Kalibrierkorrektur erscheint dieses Meldungsfenster "
                                       "nicht mehr","Warnung") );
        setColorBlackk1();
      }else{ 
        for(i=0; i<maxmesspunkte; i++){
          ts >> wobdaten.kalibrierarray.arrayk1[i];
        }
      }  
      f.close();
      fsendewobbeldaten();
    }
    if(wgrunddaten.kalibrierk1 and !wobdaten.linear1){
      setColorRedk1();
    }else{
      setColorBlackk1();
    }
    s.replace(QString("k1.hfe"), "k1.hfs");
    //Filename setzen fuer SWV Eichdatei lesen
    f.setFileName(homedir.filePath(s));
    if(f.open( QIODevice::ReadOnly )){
      QTextStream ts(&f);
      for(i=0; i<maxmesspunkte; i++){
        ts >> wobdaten.swrkalibrierarray.arrayk1[i];
      }
      f.close();
      emit sendewobbeldaten(wobdaten);
    }else{
      wobdaten.bkalibrierswrk1 = false;
    }
    f.setFileName(homedir.filePath("sav.cal"));
    bool bsavcal;
    bsavcal = f.open( QIODevice::ReadOnly);
    if(bsavcal){
      //qDebug("Laden SAV Kalibrierung");
      QTextStream ts(&f);
      ts >> qs;
      if(qs == "#V4.04"){
        bsavcal = true;
      }else{
        bsavcal = false;
      }
      for(i=0; i < calmaxindex; i++){
        ts >> wobdaten.savcalarray.p[i];
      }
      f.close();
    }
    if(!bsavcal){
      //qDebug("kein Laden SAV Kalibrierung");
      for(i=0; i < calmaxindex; i++){
        wobdaten.savcalarray.p[i] = 0.0;
      }
    }
    emit sendewobbeldaten(wobdaten);
    //Daten ins Grafikfenster uebermitteln und neu darstellen
    fsendewobbeldaten();
  }
};

void Nwt7linux::messsondenfilek2laden(const QString &filename){
  int i;
  QFile f;
  QString qs;
  QString s;
  QString snum;
  QString version;
  double faktor;
  double shift;
  
  s = filename;
  if (!s.isNull()){
    if(bkurvegeladen){
      wobdaten = wobdatenmerk;
      wobdaten.mess.daten_enable = true;
      wobdaten.bhintergrund = false;
      wobdaten.mousesperre = false;
      bkurvegeladen = false;
      fsendewobbeldaten();
    }
    //betriebsart->setCurrentIndex (0);
    QFileInfo fi(s);
    filenamesonde2 = fi.baseName();
    checkboxk2->setText(filenamesonde2+".hfm");
    configfile nwt7sondenfile;
    nwt7sondenfile.open(homedir.filePath(s));
    qs = nwt7sondenfile.readString("Sonde", "AD8307externK2");
    //checkboxk2->setText(qs);
    wobdaten.beschreibung2 = qs;
    //Wenn es die Versionsangabe noch nicht gibt, alte Version einblenden
    qs = nwt7sondenfile.readString("dateiversion", "#V3.05");
    faktor = nwt7sondenfile.readDouble("faktor", 0.191187);
    shift = nwt7sondenfile.readDouble("shift", -87.139634);
    wobdaten.linear2 = nwt7sondenfile.readBool("linear", false);
    //Ab Firmware 1.20
    wobdaten.maudio2 = nwt7sondenfile.readBool("audio_nf", false);
    if(!wobdaten.linear2){
      if(qs == "#V3.05"){
        wobdaten.faktor2 = faktor;
        wobdaten.shift2 = shift;
      }else{
        wobdaten.faktor2 = faktor / 10.23;
        wobdaten.shift2 = ((((511.5 - shift) / faktor) * wobdaten.faktor2) + 40.0) * -1.0;
      }
    }else{
      if(qs == "#V3.05"){
        wobdaten.faktor2 = faktor;
        wobdaten.shift2 = shift;
      }else{
        wobdaten.faktor2 = faktor / 9.3;
        wobdaten.shift2 = ((((465.0 - shift) / faktor) * wobdaten.faktor2) + 50.0) * -1.0;
      }
    }
    wobdaten.mfaktor2 = nwt7sondenfile.readDouble("messx", 0.193143);
    if(fwversion == 120){
      if(wobdaten.mfaktor2 > 0.01)wobdaten.mfaktor2 = wobdaten.mfaktor2 /32.0;
    }
    wobdaten.mshift2 = nwt7sondenfile.readDouble("messy", -84.634597);
    //wobdaten.beichkorrk2 = nwt7sondenfile.readBool("eichkorrk2", false);
    wattoffsetk2[0].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr0", "0 - 10 MHz +/-0,0dB");
    wattoffsetk2[1].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr1", "10 - 20 MHz  0,8dB");
    wattoffsetk2[2].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr2", "20 - 50 MHz  1,0dB");
    wattoffsetk2[3].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr3", "50 - 100 MHz  2,0dB");
    wattoffsetk2[4].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr4", "100 - 150 MHz  2,5dB");
    wattoffsetk2[5].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr5", "150 - 200 MHz  3,0dB");
    wattoffsetk2[6].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr6", "200 - 300 MHz  4,0dB");
    wattoffsetk2[7].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr7", "300 - 400 MHz  6,0dB");
    wattoffsetk2[8].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr8", "400 - 500 MHz  9,0dB");
    wattoffsetk2[9].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr9", "NN");
    wattoffsetk2[10].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr10", "NN");
    wattoffsetk2[11].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr11", "NN");
    wattoffsetk2[12].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr12", "NN");
    wattoffsetk2[13].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr13", "NN");
    wattoffsetk2[14].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr14", "NN");
    wattoffsetk2[15].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr15", "NN");
    wattoffsetk2[16].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr16", "NN");
    wattoffsetk2[17].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr17", "NN");
    wattoffsetk2[18].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr18", "NN");
    wattoffsetk2[19].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr19", "NN");
    wattoffsetk2[20].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr20", "NN");
    wattoffsetk2[21].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr21", "NN");
    wattoffsetk2[22].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr22", "NN");
    wattoffsetk2[23].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr23", "NN");
    wattoffsetk2[24].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr24", "NN");
    wattoffsetk2[25].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr25", "NN");
    wattoffsetk2[26].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr26", "NN");
    wattoffsetk2[27].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr27", "NN");
    wattoffsetk2[28].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr28", "NN");
    wattoffsetk2[29].str_offsetwatt = nwt7sondenfile.readString("wattoffsetstr29", "NN");
    boxwattoffset2->clear();
    for(i=0;i<30;i++){
      snum.sprintf("%2i| ",i+1);
      boxwattoffset2->addItem(snum + wattoffsetk2[i].str_offsetwatt);
    }
    wattoffsetk2[0].offsetwatt = nwt7sondenfile.readDouble("wattoffset0", 0.0);
    wattoffsetk2[1].offsetwatt = nwt7sondenfile.readDouble("wattoffset1", 0.8);
    wattoffsetk2[2].offsetwatt = nwt7sondenfile.readDouble("wattoffset2", 1.0);
    wattoffsetk2[3].offsetwatt = nwt7sondenfile.readDouble("wattoffset3", 2.0);
    wattoffsetk2[4].offsetwatt = nwt7sondenfile.readDouble("wattoffset4", 2.5);
    wattoffsetk2[5].offsetwatt = nwt7sondenfile.readDouble("wattoffset5", 3.0);
    wattoffsetk2[6].offsetwatt = nwt7sondenfile.readDouble("wattoffset6", 4.0);
    wattoffsetk2[7].offsetwatt = nwt7sondenfile.readDouble("wattoffset7", 6.0);
    wattoffsetk2[8].offsetwatt = nwt7sondenfile.readDouble("wattoffset8", 9.0);
    wattoffsetk2[9].offsetwatt = nwt7sondenfile.readDouble("wattoffset9", 0.0);
    wattoffsetk2[10].offsetwatt = nwt7sondenfile.readDouble("wattoffset10", 0.0);
    wattoffsetk2[11].offsetwatt = nwt7sondenfile.readDouble("wattoffset11", 0.0);
    wattoffsetk2[12].offsetwatt = nwt7sondenfile.readDouble("wattoffset12", 0.0);
    wattoffsetk2[13].offsetwatt = nwt7sondenfile.readDouble("wattoffset13", 0.0);
    wattoffsetk2[14].offsetwatt = nwt7sondenfile.readDouble("wattoffset14", 0.0);
    wattoffsetk2[15].offsetwatt = nwt7sondenfile.readDouble("wattoffset15", 0.0);
    wattoffsetk2[16].offsetwatt = nwt7sondenfile.readDouble("wattoffset16", 0.0);
    wattoffsetk2[17].offsetwatt = nwt7sondenfile.readDouble("wattoffset17", 0.0);
    wattoffsetk2[18].offsetwatt = nwt7sondenfile.readDouble("wattoffset18", 0.0);
    wattoffsetk2[19].offsetwatt = nwt7sondenfile.readDouble("wattoffset19", 0.0);
    wattoffsetk2[20].offsetwatt = nwt7sondenfile.readDouble("wattoffset20", 0.0);
    wattoffsetk2[21].offsetwatt = nwt7sondenfile.readDouble("wattoffset21", 0.0);
    wattoffsetk2[22].offsetwatt = nwt7sondenfile.readDouble("wattoffset22", 0.0);
    wattoffsetk2[23].offsetwatt = nwt7sondenfile.readDouble("wattoffset23", 0.0);
    wattoffsetk2[24].offsetwatt = nwt7sondenfile.readDouble("wattoffset24", 0.0);
    wattoffsetk2[25].offsetwatt = nwt7sondenfile.readDouble("wattoffset25", 0.0);
    wattoffsetk2[26].offsetwatt = nwt7sondenfile.readDouble("wattoffset26", 0.0);
    wattoffsetk2[27].offsetwatt = nwt7sondenfile.readDouble("wattoffset27", 0.0);
    wattoffsetk2[28].offsetwatt = nwt7sondenfile.readDouble("wattoffset28", 0.0);
    wattoffsetk2[29].offsetwatt = nwt7sondenfile.readDouble("wattoffset29", 0.0);
    //if(wobdaten.linear2)wobdaten.beichkorrk2 = false;
    nwt7sondenfile.close();
    wobbelungueltig();
    //Daten ins Grafikfenster uebermitteln und neu darstellen
    testLogLin();
    //Filename setzen
    s.replace(QString(".hfm"), "k2.hfe");
    f.setFileName(homedir.filePath(s));
    if(f.open( QIODevice::ReadOnly )){
      QTextStream ts(&f);
      ts >> version;
      if(version != "#V2.02"){
        wgrunddaten.kalibrierk2 = false;
        QMessageBox::warning( this, tr("Mathematische-Kalibrierkorrektur Kanal 2","Warnung"),
                                    tr("<B>Kalibrierkorrekturdatei ist ungueltig</B><BR>"
                                       "Die Datei hat eine kleinere Version als V2.02. "
                                       "Bitte den Kanal 2 neu mit Math-Kalibrierkorrektur kalibrieren. "
                                       "Die Datei ist nicht mehr verwendbar, da "
                                       "die mathematischen Funktionen neu programmiert "
                                       "wurden. Nach einer erneuten Kalibrierung mit Math- "
                                       "Kalibrierkorrektur erscheint dieses Meldungsfenster "
                                       "nicht mehr","Warnung") );
        setColorBlackk2();
      }else{
        for(i=0; i<maxmesspunkte; i++){
          ts >> wobdaten.kalibrierarray.arrayk2[i];
        }
      }
      f.close();
      emit sendewobbeldaten(wobdaten);
    }
    if(wgrunddaten.kalibrierk2 and !wobdaten.linear2){
      setColorRedk2();
    }else{
      setColorBlackk2();
    }
    //Filename setzen fuer SWV Eichdatei lesen
    s.replace(QString("k2.hfe"), "k2.hfs");
    f.setFileName(homedir.filePath(s));
    if(f.open( QIODevice::ReadOnly )){
      qDebug("k2 lesen");
      QTextStream ts(&f);
      for(i=0; i<maxmesspunkte; i++){
        ts >> wobdaten.swrkalibrierarray.arrayk2[i];
      }
      f.close();
      emit sendewobbeldaten(wobdaten);
    }else{
      wobdaten.bkalibrierswrk2 = false;
    }
    //Daten ins Grafikfenster uebermitteln und neu darstellen
    fsendewobbeldaten();
  }
};

void Nwt7linux::messsondeladen1(){
  QString s1;
  s1 = tr("NWT Sondendatei (*.hfm)","FileDialog");
  //qDebug(s1);
  QString s = QFileDialog::getOpenFileName(this,
                                           tr("NWT Sondendatei laden Kanal 1", "FileDialog"),
                                           nullptr,
                                           tr("NWT Sondendatei (*.hfm)", "FileDialog"));
  messsondenfilek1laden(s);
  QFileInfo fi(s);
  filenamesonde1 = fi.baseName();

}

void Nwt7linux::messsondeladen2(){
  QString s1;
  s1 = tr("NWT Sondendatei (*.hfm)","FileDialog");
  QString s = QFileDialog::getOpenFileName(this,
                                           tr("NWT Sondendatei laden Kanal 2", "FileDialog"),
                                           nullptr,
                                           tr("NWT Sondendatei (*.hfm)", "FileDialog"));
  messsondenfilek2laden(s);
  QFileInfo fi(s);
  filenamesonde2 = fi.baseName();
}

void Nwt7linux::messsondespeichern1(const QString &sa){
  QString s1 = sa;
  QString s2;
  QString beschr;
  QString sondenname;
  QString s, smerk;
  QFile f;
  int i;
  bool ok = false;
  
  //kein erster Start normales abspeichern
  if(!bersterstart){
    beschr = tr("<b>Name der Messsonde am Checkbutton</b><br>"
                "Gib der Messonde einen Name oder<br>"
                "den vorgegebenen Name uebernehmen:","FileDialog");
  }
  //default Dateiname verwenden
  if(s1 == "def"){
    if(wobdaten.ebetriebsart == ewobbeln){
      s1 = wobdaten.namesonde1;
    }
  } 
  s2 = tr("NWT Sondendatei (*.hfm)","FileDialog");

  //Datei ueberpruefen ob Sufix vorhanden
  if((s1.indexOf(".hfm")== -1)) s1 += ".hfm";
  if(!bersterstart){
    /*
    sondenname = QInputDialog::getText(tr("Messsondenname","InputDialog"), beschr,
                 QLineEdit::Normal,
                 checkboxk1->text(), &ok, this);
    */
    ok = true;
  }else{
    sondenname = checkboxk1->text();
    ok = true;
  }
  if(ok){
    //kein erster Start normales abspeichern
    if(!bersterstart){
      s = QFileDialog::getSaveFileName(this,
                                       tr("NWT Sondendatei speichern", "FileDialog"),
                                       nullptr,
                                       s2);
    }else{
      s = homedir.filePath(s1);
    }
    if (!s.isNull())
    {
      //Datei ueberpruefen ob Sufix vorhanden
      if((s.indexOf(".hfm")== -1)) s += ".hfm";
    }
    smerk = s; //Dateinamen merken
    QFileInfo fi(s);
    filenamesonde1 = fi.baseName();
    //qDebug()<< "messsondespeichern1:"<<s;
    configfile nwt7sondenfile;
    nwt7sondenfile.open(s);
    if((wobdaten.ebetriebsart == ewobbeln)  
       or (wobdaten.ebetriebsart == eswrneu)
       ){
      //wobdaten = grafik->getWobbeldaten();
      nwt7sondenfile.writeString("dateiversion", "#V3.05");
      nwt7sondenfile.writeString("Sonde", sondenname);
      nwt7sondenfile.writeDouble("faktor", wobdaten.faktor1);
      nwt7sondenfile.writeDouble("shift", wobdaten.shift1);
      nwt7sondenfile.writeBool("linear", wobdaten.linear1);
      nwt7sondenfile.writeBool("audio_nf", wobdaten.maudio1);
      nwt7sondenfile.writeDouble("messx", wobdaten.mfaktor1);
      nwt7sondenfile.writeDouble("messy", wobdaten.mshift1);
      //wobdaten.beichkorrk1 = weichen.eichk1;
      //nwt7sondenfile.writeBool("eichkorrk1", wobdaten.beichkorrk1);
      nwt7sondenfile.writeString("wattoffsetstr0", wattoffsetk1[0].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr1", wattoffsetk1[1].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr2", wattoffsetk1[2].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr3", wattoffsetk1[3].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr4", wattoffsetk1[4].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr5", wattoffsetk1[5].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr6", wattoffsetk1[6].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr7", wattoffsetk1[7].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr8", wattoffsetk1[8].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr9", wattoffsetk1[9].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr10", wattoffsetk1[10].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr11", wattoffsetk1[11].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr12", wattoffsetk1[12].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr13", wattoffsetk1[13].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr14", wattoffsetk1[14].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr15", wattoffsetk1[15].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr16", wattoffsetk1[16].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr17", wattoffsetk1[17].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr18", wattoffsetk1[18].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr19", wattoffsetk1[19].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr20", wattoffsetk1[20].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr21", wattoffsetk1[21].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr22", wattoffsetk1[22].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr23", wattoffsetk1[23].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr24", wattoffsetk1[24].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr25", wattoffsetk1[25].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr26", wattoffsetk1[26].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr27", wattoffsetk1[27].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr28", wattoffsetk1[28].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr29", wattoffsetk1[29].str_offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset0", wattoffsetk1[0].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset1", wattoffsetk1[1].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset2", wattoffsetk1[2].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset3", wattoffsetk1[3].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset4", wattoffsetk1[4].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset5", wattoffsetk1[5].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset6", wattoffsetk1[6].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset7", wattoffsetk1[7].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset8", wattoffsetk1[8].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset9", wattoffsetk1[9].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset10", wattoffsetk1[10].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset11", wattoffsetk1[11].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset12", wattoffsetk1[12].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset13", wattoffsetk1[13].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset14", wattoffsetk1[14].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset15", wattoffsetk1[15].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset16", wattoffsetk1[16].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset17", wattoffsetk1[17].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset18", wattoffsetk1[18].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset19", wattoffsetk1[19].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset20", wattoffsetk1[20].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset21", wattoffsetk1[21].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset22", wattoffsetk1[22].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset23", wattoffsetk1[23].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset24", wattoffsetk1[24].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset25", wattoffsetk1[25].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset26", wattoffsetk1[26].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset27", wattoffsetk1[27].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset28", wattoffsetk1[28].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset29", wattoffsetk1[29].offsetwatt);
      nwt7sondenfile.close();
      checkboxk1->setText(filenamesonde1 + ".hfm");
      //Filename setzen
      //Eichkorrektur bei log. Messkopf
      if((wgrunddaten.kalibrierk1) and (!wobdaten.linear1)){
        s.replace(QString(".hfm"), "k1.hfe");
        f.setFileName(homedir.filePath(s));
        if(f.open(QIODevice::WriteOnly)){
          QTextStream ts(&f);
          ts << "#V2.02" << endl;
          for(i=0; i<maxmesspunkte; i++){
            ts << wobdaten.kalibrierarray.arrayk1[i] << endl;
          }
          f.close();
        }
      }
    }
    if(wobdaten.ebetriebsart == eswrneu){
      s = smerk;
      //Filename setzen
      s.replace(QString(".hfm"), "k1.hfs");
      f.setFileName(homedir.filePath(s));
      if(f.open(QIODevice::WriteOnly)){
        #ifdef LDEBUG
          qDebug(s);
        #endif
        QTextStream ts(&f);
        for(i=0; i<maxmesspunkte; i++){
          ts << wobdaten.swrkalibrierarray.arrayk1[i] << endl;
        }
        f.close();
      }
    }
  }
}

void Nwt7linux::messsondespeichern2(const QString &sa){
  QString s1 = sa;
  QString s2;
  QString beschr;
  QString sondenname;
  QString s, smerk;
  QFile f;
  int i;
  bool ok = false;

  if(!bersterstart){
    beschr = tr("<b>Name der Messsonde am Checkbutton</b><br>"
                "Gib der Messonde einen Name oder<br>"
                "den vorgegebenen Name uebernehmen:","InputDialog");
  }
  if(s1 == "def"){
    s1 = wobdaten.namesonde2;
  } 
  s2 = tr("NWT Sondendatei (*.hfm)");
  //Datei ueberpruefen ob Sufix vorhanden
  if((s1.indexOf(".hfm")== -1)) s1 += ".hfm";
  //wenn keine Erstinitialisierung Dialog oeffnen
  if(!bersterstart){
    /*
    sondenname = QInputDialog::getText(tr("Messsondenname","InputDialog"), beschr,
                               QLineEdit::Normal,
                               checkboxk2->text(), &ok, this);
    */
    ok = true;
  }else{
    sondenname = checkboxk2->text();
    ok = true;
  }
  if(ok){
    //wenn keine Erstinitialisierung Dialog oeffnen
    if(!bersterstart){
      s = QFileDialog::getSaveFileName(this,
                                       tr("NWT Sondendatei speichern", "FileDialog"),
                                       nullptr,
                                       s2);
    }else{
      s = homedir.filePath(s1);
    }
    if (!s.isNull())
    {
      //Datei ueberpruefen ob Sufix vorhanden
      if((s.indexOf(".hfm")== -1)) s += ".hfm";
    }
    smerk = s;
    QFileInfo fi(s);
    filenamesonde2 = fi.baseName();
    //qDebug() << filenamesonde2;
    configfile nwt7sondenfile;
    nwt7sondenfile.open(s);
    if(wobdaten.ebetriebsart == ewobbeln){
      //wobdaten = grafik->getWobbeldaten();
      nwt7sondenfile.writeString("dateiversion", "#V3.05");
      nwt7sondenfile.writeString("Sonde", sondenname);
      nwt7sondenfile.writeDouble("faktor", wobdaten.faktor2);
      nwt7sondenfile.writeDouble("shift", wobdaten.shift2);
      nwt7sondenfile.writeBool("linear", wobdaten.linear2);
      nwt7sondenfile.writeBool("audio_nf", wobdaten.maudio2);
      nwt7sondenfile.writeDouble("messx", wobdaten.mfaktor2);
      nwt7sondenfile.writeDouble("messy", wobdaten.mshift2);
      //wobdaten.beichkorrk2 = weichen.eichk2;
      //nwt7sondenfile.writeBool("eichkorrk2", wobdaten.beichkorrk2);
      nwt7sondenfile.writeString("wattoffsetstr0", wattoffsetk2[0].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr1", wattoffsetk2[1].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr2", wattoffsetk2[2].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr3", wattoffsetk2[3].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr4", wattoffsetk2[4].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr5", wattoffsetk2[5].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr6", wattoffsetk2[6].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr7", wattoffsetk2[7].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr8", wattoffsetk2[8].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr9", wattoffsetk2[9].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr10", wattoffsetk2[10].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr11", wattoffsetk2[11].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr12", wattoffsetk2[12].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr13", wattoffsetk2[13].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr14", wattoffsetk2[14].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr15", wattoffsetk2[15].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr16", wattoffsetk2[16].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr17", wattoffsetk2[17].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr18", wattoffsetk2[18].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr19", wattoffsetk2[19].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr20", wattoffsetk2[20].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr21", wattoffsetk2[21].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr22", wattoffsetk2[22].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr23", wattoffsetk2[23].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr24", wattoffsetk2[24].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr25", wattoffsetk2[25].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr26", wattoffsetk2[26].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr27", wattoffsetk2[27].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr28", wattoffsetk2[28].str_offsetwatt);
      nwt7sondenfile.writeString("wattoffsetstr29", wattoffsetk2[29].str_offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset0", wattoffsetk2[0].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset1", wattoffsetk2[1].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset2", wattoffsetk2[2].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset3", wattoffsetk2[3].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset4", wattoffsetk2[4].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset5", wattoffsetk2[5].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset6", wattoffsetk2[6].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset7", wattoffsetk2[7].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset8", wattoffsetk2[8].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset9", wattoffsetk2[9].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset10", wattoffsetk2[10].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset11", wattoffsetk2[11].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset12", wattoffsetk2[12].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset13", wattoffsetk2[13].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset14", wattoffsetk2[14].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset15", wattoffsetk2[15].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset16", wattoffsetk2[16].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset17", wattoffsetk2[17].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset18", wattoffsetk2[18].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset19", wattoffsetk2[19].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset20", wattoffsetk2[20].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset21", wattoffsetk2[21].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset22", wattoffsetk2[22].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset23", wattoffsetk2[23].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset24", wattoffsetk2[24].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset25", wattoffsetk2[25].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset26", wattoffsetk2[26].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset27", wattoffsetk2[27].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset28", wattoffsetk2[28].offsetwatt);
      nwt7sondenfile.writeDouble("wattoffset29", wattoffsetk2[29].offsetwatt);
      nwt7sondenfile.close();
      checkboxk2->setText(filenamesonde2 + ".hfm");
      //Filename setzen
      if((wgrunddaten.kalibrierk2) and (!wobdaten.linear2)){
        s.replace(QString(".hfm"), "k2.hfe");
        f.setFileName(homedir.filePath(s));
        if(f.open( QIODevice::WriteOnly )){
          QTextStream ts(&f);
          ts << "#V2.02" << endl;
          for(i=0; i<maxmesspunkte; i++){
            ts << wobdaten.kalibrierarray.arrayk2[i] << endl;
          }
          f.close();
        }
      }	
    }  
    if(wobdaten.ebetriebsart == eswrneu){
      //Filename setzen
      s = smerk;
      s.replace(QString(".hfm"), "k2.hfs");
      f.setFileName(homedir.filePath(s));
      if(f.open( QIODevice::WriteOnly )){
        QTextStream ts(&f);
        for(i=0; i<maxmesspunkte; i++){
          ts << wobdaten.swrkalibrierarray.arrayk2[i] << endl;
        }
        f.close();
      }
    }
  }  
}

void Nwt7linux::kalibrierenkanal1(){
  QString s;
  int i;

  if(!bttyOpen){
    //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
      QMessageBox::warning( this, tr("Serielle Schnittstelle"), 
                                     "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!"));
      #ifdef LDEBUG
        qDebug("Nwt7linux::kalibrierenkanal1()");
      #endif
      return;
    }
  }
  if (bttyOpen)
  for(i=0;i<maxmesspunkte;i++)
  {
    //Entsprechendes Array loeschen
    if(wobdaten.ebetriebsart==ewobbeln)wobdaten.kalibrierarray.arrayk1[i] = 0.0;
    if(wobdaten.ebetriebsart==eswrneu)wobdaten.swrkalibrierarray.arrayk1[i] = -10.0;
  }
  if(wobdaten.ebetriebsart==ewobbeln)emit sendewobbeldaten(wobdaten);
  if(wobdaten.ebetriebsart==eswrneu)emit sendewobbeldaten(wobdaten);
  switch(wobdaten.ebetriebsart){
    case eantdiagramm:
      break;
    case espekdisplayshift:
      break;
    case espektrumanalyser:
      break;
    case ewobbeln:
      switch( QMessageBox::information( this, tr("Art der Messsonde","InformationsBox"),
            tr("<B>Die Messonde bitte auswaehlen</B><BR>"
            "Ist die Messsonde linear oder logarithmisch?","InformationsBox"),
            "L&in.", "L&og.",
            0,
            1 ) ) {
        case 0:
            // Linear
            //qDebug("Linear");
            wobdaten.linear1 = true;
            testLogLin();
            emit sendewobbeldaten(wobdaten);
            break;
        case 1:
            // Logarithmisch
            //qDebug("Log");
            wobdaten.linear1 = false;
            wobdaten.linear2 = false;
            testLogLin();
            emit sendewobbeldaten(wobdaten);
            break;
      }
      checkboxk1->setChecked(true);
      checkboxk2->setChecked(false);
      buttoneinmal->setEnabled(false);
      buttonwobbeln->setEnabled(false);
      buttonstop->setEnabled(false);
      emit setmenu(emEinmal, false);
      emit setmenu(emWobbeln, false);
      emit setmenu(emStop, false);
      groupzoom->setEnabled(false);
      groupwobbel->setEnabled(false);
      groupbetriebsart->setEnabled(false);
      if(wgrunddaten.fwfalsch){
        boxzwischenzeit->setCurrentIndex(0);
        boxzwischenzeit->setEnabled(false);
        labelzwischenzeit->setEnabled(false);
//        editscantime->setEnabled(false);
//        checkboxtime->setEnabled(false);
      }
      bkalibrieren40 = true;
      if(!wobdaten.linear1){
        //math. Korrektur aktiv ?
        if(wgrunddaten.kalibrierk1){
          //beim Kalibrieren andere Frequenzbereiche zulassen
          //wenn der Faktor > 1 ist
          if(wobdaten.frqfaktor == 1){
            //Frequenzbereiche ins Wobbelfenster uebertragen
            setanfang(wgrunddaten.kalibrieranfang);
            setende(wgrunddaten.kalibrierende);
          }
          setschritte(double(wgrunddaten.kalibrierstep));
          wobnormalisieren();
        }
        if(wgrunddaten.bdaempfungfa){
          setDaempfung(19); // -40dB
          daempfung1->setCurrentIndex(19);
          daempfung2->setCurrentIndex(19);
          daempfung3->setCurrentIndex(19);
          setDaempfung(19); // -40dB
        }else{  
          setDaempfung(4); // -40dB
          daempfung1->setCurrentIndex(4);
          daempfung2->setCurrentIndex(4);
          daempfung3->setCurrentIndex(4);
          setDaempfung(4); // -40dB
        }
        QMessageBox::warning( this, tr("Kalibrieren Kanal 1","InformationsBox"),
                                    tr("<B>Ermittlung der -40dB Linie</B><BR>" 
                                       "-40db-Glied einschleifen. <BR>"
                                       "<em>Bitte nur wenn kein automatisches <BR>"
                                       "Daempfungsglied vorhanden ist.</em>","InformationsBox"));
      }else{
        if(wgrunddaten.bdaempfungfa){
          setDaempfung(2); // -6dB
          daempfung1->setCurrentIndex(2);
          daempfung2->setCurrentIndex(2);
          daempfung3->setCurrentIndex(2);
          setDaempfung(2); // -6dB
        }
        QMessageBox::warning( this, tr("Kalibrieren Kanal 1","InformationsBox"),
                                    tr("<B>Ermittlung der 0,5 Linie</B><BR>" 
                                       "-6db-Glied einschleifen. <BR>"
                                       "<em>Bitte nur wenn kein automatisches <BR>"
                                       "Daempfungsglied vorhanden ist, was diese <br>"
                                       "Aufloesung beherrscht.</em>","InformationsBox"));
      }
      break;
    //SWR kalibrieren
    case eswrneu:
      s.sprintf("%li",wgrunddaten.kalibrierstep);
      editschritte->setText(s);
      wobnormalisieren();
      bkalibrierenswr = true;
      bkalibrieren0 = true;
      QMessageBox::warning( this, tr("Kalibrieren SWV","InformationsBox"),
                                  tr("<B>Ermitteln SWV = unendlich</B><BR>"
                                     "SWV-Bruecke offen lassen oder kurzschliessen.","InformationsBox") );
      break;
    case eswrant:
    case eimpedanz:
      break;
  }
  wobbelstop = true;
//  fsendewobbeldaten();
  sendwobbeln();
}

void Nwt7linux::kalibrierenkanal2(){
  QString s;
  int i;

  if(!bttyOpen){
    //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
       QMessageBox::warning( this, tr("Serielle Schnittstelle"), 
                                     "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!"));
      #ifdef LDEBUG
        qDebug("Nwt7linux::kalibrierenkanal2()");
      #endif
      return;
    }
  }
  if (bttyOpen)
  for(i=0;i<maxmesspunkte;i++)
  {
    //Entsprechendes Array loeschen
    if(wobdaten.ebetriebsart==ewobbeln)wobdaten.kalibrierarray.arrayk2[i] = 0.0;
    if(wobdaten.ebetriebsart==eswrneu)wobdaten.swrkalibrierarray.arrayk2[i] = -10.0;
  }
  if(wobdaten.ebetriebsart==ewobbeln)emit sendewobbeldaten(wobdaten);
  if(wobdaten.ebetriebsart==eswrneu)emit sendewobbeldaten(wobdaten);
  switch(wobdaten.ebetriebsart){
    case eantdiagramm:
      break;
    case espekdisplayshift:
      break;
    case espektrumanalyser:
      break;
    case ewobbeln:
      switch( QMessageBox::information( this, tr("Art der Messsonde","InformationsBox"),
                                        tr("<B>Die Messonde bitte auswaehlen</B><BR>"
                                           "Ist die Messsonde linear oder logarithmisch?","InformationsBox"),
                                        "L&in.", "L&og.",
                                        0,
                                        1 ) ) {
       case 0:
        // Linear
        // fprintf(stderr, "Linear\n");
        wobdaten.linear2 = true;
        testLogLin();
        emit sendewobbeldaten(wobdaten);
        break;
       case 1:
        // Logarithmisch
        // fprintf(stderr, "Log\n");
        wobdaten.linear1 = false;
        wobdaten.linear2 = false;
        testLogLin();
        emit sendewobbeldaten(wobdaten);
        break;
      }
      if(fwversion == 120){
        if(QMessageBox::question(this, tr("Messkopf Kanal 2","WarnungsBox"),
                                 tr("<B>Art des Messkopfes</B><BR>"
                                    "Ist das ein Audio-NF Messkopf?","WarungsBox"),
                                 tr("&Ja","WarungsBox"), tr("&Nein","WarungsBox"),
                                 QString::null, 1, 0) == 0){
          wobdaten.maudio2 = true;
          //boxzwischenzeit->setCurrentIndex(12); // Zwischenzeit auf maximum
        }
      }
      checkboxk1->setChecked(false);
      checkboxk2->setChecked(true);
      buttoneinmal->setEnabled(false);
      buttonwobbeln->setEnabled(false);
      buttonstop->setEnabled(false);
      emit setmenu(emEinmal, false);
      emit setmenu(emWobbeln, false);
      emit setmenu(emStop, false);
      groupzoom->setEnabled(false);
      groupwobbel->setEnabled(false);
      groupbetriebsart->setEnabled(false);
      if(wgrunddaten.fwfalsch){
        boxzwischenzeit->setCurrentIndex(0);
        boxzwischenzeit->setEnabled(false);
        labelzwischenzeit->setEnabled(false);
//        editscantime->setEnabled(false);
//        checkboxtime->setEnabled(false);
      }
      bkalibrieren40 = true;
      if(!wobdaten.linear2){
        if(wgrunddaten.kalibrierk2){
          //beim Kalibrieren andere Frequenzbereiche zulassen
          //wenn der Faktor > 1 ist
          if(wobdaten.frqfaktor == 1){
          //Frequenzbereiche ins Wobbelfenster uebertragen
            setanfang(wgrunddaten.kalibrieranfang);
            setende(wgrunddaten.kalibrierende);
          }
          setschritte(double(wgrunddaten.kalibrierstep));
          wobnormalisieren();
        }
        if(wgrunddaten.bdaempfungfa){
          setDaempfung(19); // -40dB
          daempfung1->setCurrentIndex(19);
          daempfung2->setCurrentIndex(19);
          daempfung3->setCurrentIndex(19);
        }else{  
          setDaempfung(4); // -40dB
          daempfung1->setCurrentIndex(4);
          daempfung2->setCurrentIndex(4);
          daempfung3->setCurrentIndex(4);
        }
        QMessageBox::warning( this, tr("Kalibrieren Kanal 2","InformationsBox"),
                                    tr("<B>Ermittlung der -40dB Linie</B><BR>" 
                                       "-40db-Glied einschleifen. <BR>"
                                       "<em>Bitte nur wenn kein automatisches <BR>"
                                       "Daempfungsglied vorhanden ist.</em>","InformationsBox"));
      }else{
        if(wgrunddaten.bdaempfungfa){
          setDaempfung(2); // -6dB
          daempfung1->setCurrentIndex(2);
          daempfung2->setCurrentIndex(2);
          daempfung3->setCurrentIndex(2);
        }
        QMessageBox::warning( this, tr("Kalibrieren Kanal 2","InformationsBox"),
                                    tr("<B>Ermittlung der 0,5 Linie</B><BR>" 
                                       "-6db-Glied einschleifen. <BR>"
                                       "<em>Bitte nur wenn kein automatisches <BR>"
                                       "Daempfungsglied vorhanden ist, was diese <br>"
                                       "Aufloesung beherrscht.</em>","InformationsBox"));
      }
      break;
    //SWR kalibrieren
    case eswrneu:
      s.sprintf("%li",wgrunddaten.kalibrierstep);
      editschritte->setText(s);
      wobnormalisieren();
      bkalibrierenswr = true;
      bkalibrieren0 = true;
      QMessageBox::warning( this, tr("Kalibrieren SWV Kanal 2","InformationsBox"),
                                  tr("<B>Ermitteln SWV = unendlich</B><BR>"
                                     "SWV-Bruecke offen lassen oder kurzschliessen.","InformationsBox") );
      break;
    case eswrant:
    case eimpedanz:
      break;
  }
  wobbelstop = true;
//  fsendewobbeldaten();
  sendwobbeln();
}

void Nwt7linux::vtimerbehandlung(){
  unsigned char a;
  
  if (bttyOpen){
    if((picmodem->readttybuffer(&a, 1)) == 1){
      fwversion = int (a);
      #ifdef LDEBUG
      qDebug("daten");
      #endif
    }else{
      #ifdef LDEBUG
      qDebug("keine daten");
      #endif
      fwversion = 0;
      defaultlesenrtty(); //eventuelle Daten im UART loeschen
    };
  }
  #ifdef LDEBUG
  qDebug("vtimerbehandlung()");
  qDebug("version %i",fwversion);
  #endif
  setAText(wgrunddaten.str_tty);
  vtimer->stop();
}

void Nwt7linux::stimerbehandlung(){
  char abuffer[4];
  unsigned char c1, c2;
  if (bttyOpen){
    if((picmodem->readttybuffer(abuffer, 4)) == 4){
      fwvariante = int(abuffer[0]);
      fwportb = int(abuffer[1]);
      c1 = abuffer[2];
      c2 = abuffer[3];
      fwsav = int(c1 + c2 * 256);
    }else{
      fwvariante = 0;
      fwportb = 0;
      fwsav = 0;
      defaultlesenrtty(); //eventuelle Daten im UART loeschen
    }
  }
  
  //  qDebug("stimerbehandlung()");
  //  qDebug("variante %i",fwvariante);
  //  qDebug("portb %i",fwportb);
  //  qDebug("c1 %x",c1);
  //  qDebug("c2 %x",c2);
  //  qDebug("sav %i",fwsav);
  stimer->stop();
}

void Nwt7linux::idletimerbehandlung(){
  //Kalibrierwerte fuer die Erkennung der eingestellten Werte am SAV
  int sa75_003  = 353;
  int sa75_05   = 446;
  int sa75_30   = 544;
  int sa148_003 = 674;
  int sa148_05  = 853;
  
  int pb;
  int daempf[8] = {0,1,2,3,0,0,4,5};
                      //0  1  2  3  4  5  6  7  8  9
  int daempf_fa[64] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
                        3, 4, 5, 6, 0, 8, 7, 0, 0, 0,//1
                        0, 0, 0, 0, 9,10,11,12,13,14,//2
                        0,16,15, 0, 0, 0, 0, 0, 0, 0,//3
                       17,18,19,20,21,22, 0,24,23, 0,//4
                        0, 0, 0, 0, 0, 0,25,26,27,28,//5
                       29,30,31,32                   //6
                       };

  versionskontrolle(); // Start vtimer und stimer abwechselnt
  //Wenn nicht gewobbelt wird 0 Hz ausgeben am DDS
  setparkfrq();
  //Vorbereitung fuer Farbeinstellung
  QPalette palette = labelnwt->palette();
  if(fwversion != 0){
    //Dunkel Gruene Schrift
    palette.setColor(QPalette::Active, QPalette::WindowText, Qt::darkGreen);
    labelnwt->setPalette(palette);
    labelnwt->setText(tr("Online","Label"));
    fwversioncounter = 3;
    setTabEnabled(nwt7vfo, true);
    //setTabEnabled(berechnung, true);
    if(wobdaten.linear2 or wobdaten.linear1){
      setTabEnabled(nwt7messen, false);
    }else{
      setTabEnabled(nwt7messen, true);
    };
    if((fwversion > 114) and (fwversion < 150)){
      if(wobdaten.maudio2){
        if(!checkboxk1->isChecked() and checkboxk2->isChecked()){
          kalibrier1->setEnabled(false);
          kalibrier2->setChecked(false);
          kalibrier2->setEnabled(false);
          boxzwischenzeit->setCurrentIndex(0);
          boxzwischenzeit->setEnabled(false);
          labelzwischenzeit->setEnabled(false);
        }else{
          boxzwischenzeit->setEnabled(true);
          labelzwischenzeit->setEnabled(true);
          kalibrier1->setEnabled(true);
          kalibrier2->setEnabled(true);
        }
      }else{
        boxzwischenzeit->setEnabled(true);
        labelzwischenzeit->setEnabled(true);
      }
      //checkboxtime->setEnabled(true);
      //      editscantime->setEnabled(true);
      wgrunddaten.fwfalsch = false;
      //fwportb kommt von der HW NWT01
      pb = fwportb;
      //pb = pb >> 1; //PortB ein BIT nach links
      //qDebug("portb2 %X %i", pb, pb);
      //Rueckwaertzauswertung des Daempfunggliedes
      //keine Versionsabfrage sondern Statusabfrage wurde duerchgefuehrt
      if(!vabfrage){
        if(fwportb != fwportbalt){
          fwportbalt = fwportb;
          fwportbcounter = 1;
        }else{
          if(fwportbcounter > 0)fwportbcounter--;
        }
        if(fwportbcounter == 0){
          //qDebug("Daempfungskorrektur");
          if(wgrunddaten.bdaempfungfa){
            daempfung1->setCurrentIndex(daempf_fa[pb]);
            daempfung2->setCurrentIndex(daempf_fa[pb]);
            daempfung3->setCurrentIndex(daempf_fa[pb]);
          }else{
            daempfung1->setCurrentIndex(daempf[pb]);
            daempfung2->setCurrentIndex(daempf[pb]);
            daempfung3->setCurrentIndex(daempf[pb]);
          }
        }
        //SA Status richtig einstellen
        if(fwsav < 10){
          labelsafrqbereich->setText(tr("kein Spektrumanalyser","Label"));
          labelsabandbreite->setText(tr("angeschlossen","Label"));
          sabw = bwkein;
          if(!bkurvegeladen)wobdaten.ebbsav = ekeinebandbreite;
        }
        if((fwsav >= 10) and (fwsav < sa75_003)){
          labelsafrqbereich->setText(tr("Scanbereich 1","Label"));
          labelsabandbreite->setText(tr("Bandwidth: 30 kHz","Label"));
          if(wgrunddaten.sastatus){
            sabereich1->setChecked(true);
            sabereich2->setChecked(false);
            sabereich = bsa1;
            sabw = bw30kHz;
            if(!bkurvegeladen)wobdaten.ebbsav = e30kb1;
            //qDebug("30k");
          }
        }
        if((fwsav >= sa75_003) and (fwsav < sa75_05)){
          labelsafrqbereich->setText(tr("Scanbereich 1","Label"));
          labelsabandbreite->setText(tr("Bandwidth: 7 kHz","Label"));
          if(wgrunddaten.sastatus){
            sabereich1->setChecked(true);
            sabereich2->setChecked(false);
            sabereich = bsa1;
            sabw = bw7kHz;
            if(!bkurvegeladen)wobdaten.ebbsav = e7kb1;
            //qDebug("7k");
          }
        }
        if((fwsav >= sa75_05) and (fwsav < sa75_30)){
          labelsafrqbereich->setText(tr("Scanbereich 1","Label"));
          labelsabandbreite->setText(tr("Bandwidth: 300 Hz","Label"));
          if(wgrunddaten.sastatus){
            sabereich1->setChecked(true);
            sabereich2->setChecked(false);
            sabereich = bsa1;
            sabw = bw300Hz;
            if(!bkurvegeladen)wobdaten.ebbsav = e300b1;
            //qDebug("300");
          }
        }
        if((fwsav >= sa75_30) and (fwsav < sa148_003)){
          labelsafrqbereich->setText(tr("Scanbereich 2","Label"));
          labelsabandbreite->setText(tr("Bandwidth: 30 kHz","Label"));
          if(wgrunddaten.sastatus){
            sabereich1->setChecked(false);
            sabereich2->setChecked(true);
            sabereich = bsa2;
            sabw = bw30kHz;
            if(!bkurvegeladen)wobdaten.ebbsav = e30kb2;
            //qDebug("30k");
          }
        }
        if((fwsav >= sa148_003) and (fwsav < sa148_05)){
          labelsafrqbereich->setText(tr("Scanbereich 2","Label"));
          labelsabandbreite->setText(tr("Bandwidth: 7 kHz","Label"));
          if(wgrunddaten.sastatus){
            sabereich1->setChecked(false);
            sabereich2->setChecked(true);
            sabereich = bsa2;
            sabw = bw7kHz;
            if(!bkurvegeladen)wobdaten.ebbsav = e7kb2;
            //qDebug("7k");
          }
        }
        if(fwsav >= sa148_05){
          labelsafrqbereich->setText(tr("Scanbereich 2","Label"));
          labelsabandbreite->setText(tr("Bandwidth: 300 Hz","Label"));
          if(wgrunddaten.sastatus){
            sabereich1->setChecked(false);
            sabereich2->setChecked(true);
            sabereich = bsa2;
            sabw = bw300Hz;
            if(!bkurvegeladen)wobdaten.ebbsav = e300b2;
            //qDebug("300");
          }
        }
      }
      if((wobdaten.ebetriebsart == espektrumanalyser/* or wobdaten.ebetriebsart == espekdisplayshift*/) and wgrunddaten.sastatus){
        //nur Frequenz umspeichern wenn HW Rueckmeldung und Betriebsart SA
        setFrqBereich();
      } 
    }
  }else{
    if(fwversioncounter == 0){
      // Rote Schrift
      palette.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
      labelnwt->setText(tr("Offline","Label"));
      labelnwt->setPalette(palette);
      boxzwischenzeit->setCurrentIndex(0);
      boxzwischenzeit->setEnabled(false);
      labelzwischenzeit->setEnabled(false);
      //nicht die richtige FW erkannt oder keine Verbindung zum NWT
      wgrunddaten.fwfalsch = true;
      //SAV Bandbreitenerkennung AUS
      if(!bkurvegeladen)wobdaten.ebbsav = ekeinebandbreite;
    }
    fwversioncounter--;
    if(fwversioncounter < 0)fwversioncounter = 0;
  }
  // Windostext setzen
  setAText(wgrunddaten.str_tty);
  // Timmer laeuft 1 Sekunde
  idletimer->start(wgrunddaten.idletime);
}

void Nwt7linux::clickwobbeln(){
  wobnormalisieren(); //Abgleich aller Eingabedaten
  if(wobbelabbruch)return;
  if(wobdaten.einkanalig){
    emit setmenu(emEichenK2, false);
    emit setmenu(emSondeLadenK2, false);
    emit setmenu(emSondeSpeichernK2, false);
    emit setmenu(emWattEichenK2, false);
    emit setmenu(emWattEditSonde2, false);
  }else{
    emit setmenu(emEichenK2, true);
    emit setmenu(emSondeLadenK2, true);
    emit setmenu(emSondeSpeichernK2, true);
    emit setmenu(emWattEichenK2, true);
    emit setmenu(emWattEditSonde2, true);
  }
  //versionskontrolle();
  if(!bttyOpen){
    //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
      QMessageBox::warning( this, tr("Serielle Schnittstelle","WarungsBox"), 
                                  "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","WarungsBox"));
      #ifdef LDEBUG
      qDebug("Nwt7linux::clickwobbeln()");
      #endif    
      return;
    }
  }
  grafik->setWindowTitle(tr("Display","Wobbelfenster"));
  if (bkurvegeladen){
    setAText(wgrunddaten.str_tty);
    bkurvegeladen = false; //Modus ausschalten
    wobdaten = wobdatenmerk;//Alte einstellungen wieder zurueckladen
    checkboxk1->setChecked(wobdaten.bkanal1);
    checkboxk2->setChecked(wobdaten.bkanal2);
    wobbelungueltig();
    switch(wobdaten.ebetriebsart){
      case eantdiagramm:
      break;
      case ewobbeln:
      betriebsart->setCurrentIndex (0);
      break;
      case eswrneu:
      betriebsart->setCurrentIndex (1);
      break;
      case eswrant:
      betriebsart->setCurrentIndex (2);
      break;
      case eimpedanz:
      betriebsart->setCurrentIndex (3);
      break;
      case espektrumanalyser:
      betriebsart->setCurrentIndex (4);
      break;
      case espekdisplayshift:
      betriebsart->setCurrentIndex (5);
      break;
    }
    checkbox3db->setChecked(wobdaten.bandbreite3db);
    checkbox6db->setChecked(wobdaten.bandbreite6db);
    checkboxgrafik->setChecked(wobdaten.bandbreitegrafik);
    checkboxinvers->setChecked(wobdaten.binvers);
    mledit->clear();
  }
  if (bttyOpen)
  {
    bdauerwobbeln = true;
    labellupe->setEnabled(false);
    buttonlupeplus->setEnabled(false);
    buttonlupeminus->setEnabled(false);
    buttonlupemitte->setEnabled(false);
    buttoneinmal->setEnabled(false);
    buttonwobbeln->setEnabled(false);
    buttonstop->setEnabled(true);
//    groupdaempfung->setEnabled(false);
//    daempfung2->setEnabled(false);
    emit setmenu(emEinmal, false);
    emit setmenu(emWobbeln, false);
    emit setmenu(emStop, true);
    groupzoom->setEnabled(false);
    groupwobbel->setEnabled(false);
    groupbetriebsart->setEnabled(false);
    if(!wgrunddaten.fwfalsch){
      //keine Zwischenzeit bei ungeeigneter FW
      boxzwischenzeit->setEnabled(false);
      labelzwischenzeit->setEnabled(false);
    }
    wobbelstop = false;
    setTabEnabled(wkmanager, false);
    setTabEnabled(nwt7vfo, false);
    setTabEnabled(berechnung, false);
    setTabEnabled(nwt7messen, false);
    sendwobbeln();
    wobdaten.mousesperre = true; //Kursoraktivitaeten sperren
    fsendewobbeldaten();
  }
  //Zaehler fuer Abbruch des Datenverkehrs
  abbruchlevel=0;
  bdatenruecklauf=false;
  if(wgrunddaten.grperwobbeln){
    grafik->setFocus();
    grafik->activateWindow ();
  }
  //emit setmenu(emmainhidde, true);
}

void Nwt7linux::clickwobbelneinmal(){
  wobnormalisieren(); //Abgleich aller Eingabedaten
  if(wobbelabbruch)return;
  if(!bttyOpen){
    //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
      QMessageBox::warning( this, tr("Serielle Schnittstelle","WarungsBox"), 
                                     "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","WarungsBox"));
      #ifdef LDEBUG
      qDebug("Nwt7linux::clickwobbelneinmal()\n");
      #endif    
      return;
    }
  }
  grafik->setWindowTitle(tr("Display","Wobbelfenster"));
  if (bkurvegeladen){
    setAText(wgrunddaten.str_tty);
    bkurvegeladen = false; //Modus ausschalten
    wobdaten = wobdatenmerk;//Alte einstellungen wieder zurueckladen
    checkboxk1->setChecked(wobdaten.bkanal1);
    checkboxk2->setChecked(wobdaten.bkanal2);
    wobbelungueltig();
    switch(wobdaten.ebetriebsart){
      case eantdiagramm:
      break;
      case ewobbeln:
      betriebsart->setCurrentIndex (0);
      break;
      case eswrneu:
      betriebsart->setCurrentIndex (1);
      break;
      case eswrant:
      betriebsart->setCurrentIndex (2);
      break;
      case eimpedanz:
      betriebsart->setCurrentIndex (3);
      break;
      case espektrumanalyser:
      betriebsart->setCurrentIndex (4);
      break;
      case espekdisplayshift:
      betriebsart->setCurrentIndex (5);
      break;
    }
    checkbox3db->setChecked(wobdaten.bandbreite3db);
    checkbox6db->setChecked(wobdaten.bandbreite6db);
    checkboxgrafik->setChecked(wobdaten.bandbreitegrafik);
    checkboxinvers->setChecked(wobdaten.binvers);
    mledit->clear();
  }
  if (bttyOpen)
  {
    buttoneinmal->setEnabled(false);
    buttonwobbeln->setEnabled(false);
    buttonstop->setEnabled(false);
    groupdaempfung->setEnabled(false);
    daempfung2->setEnabled(false);
    emit setmenu(emEinmal, false);
    emit setmenu(emWobbeln, false);
    emit setmenu(emStop, false);
    groupzoom->setEnabled(false);
    groupwobbel->setEnabled(false);
    groupbetriebsart->setEnabled(false);
    if(!wgrunddaten.fwfalsch){
      //keine Zwischenzeit bei ungeeigneter FW
      boxzwischenzeit->setEnabled(false);
      labelzwischenzeit->setEnabled(false);
    }
    wobbelstop = true;
    setTabEnabled(wkmanager, false);
    setTabEnabled(nwt7vfo, false);
    setTabEnabled(berechnung, false);
    setTabEnabled(nwt7messen, false);
    sendwobbeln();
    wobdaten.mousesperre = true; //Kursoraktivitaeten sperren
    fsendewobbeldaten();
  }
  abbruchlevel=0;
  bdatenruecklauf=false;
  if(wgrunddaten.grpereinmal){
    grafik->setFocus();
    grafik->activateWindow ();
  }
}

void Nwt7linux::clickwobbelnstop(){
  if (bttyOpen)
  {
    wobbelstop = true;
    buttonstop->setEnabled(false);
    emit setmenu(emStop, false);
  }
  if(wgrunddaten.grperstop){
    grafik->setFocus();
    grafik->activateWindow ();
  }
}

void Nwt7linux::sendwobbeln(){
  QString qs;
  double dfanfang, dfschrittfrequenz;
  long fanfang;
  long fschrittfrequenz;
  double zf;
  int frqfaktor;
  
  frqfaktor = wobdaten.frqfaktor;
  //qDebug("frqfaktor %i",frqfaktor);
  if(bdauerwobbeln and bsetdaempfung){
    if(bttyOpen){
      bsetdaempfung = false;
      picmodem->writeChar(0x8F);
      picmodem->writeChar('r');
      picmodem->writeChar(fwportb);
      #ifdef LDEBUG
      qs.sprintf("r%i",fwportb); 
      qDebug(qs);
      #endif    
    }
  }
  
  wcounter = 3;
  dfanfang =  wobdaten.anfangsfrequenz;
  dfschrittfrequenz = wobdaten.schrittfrequenz;
  switch(wobdaten.ebetriebsart){
    case eantdiagramm:
      break;
    case ewobbeln:
    case eswrneu:
    case eswrant:
    case eimpedanz:
      if(frqfaktor > 1){
        dfanfang = round(dfanfang / double(frqfaktor));
        dfschrittfrequenz = round(dfschrittfrequenz / double(frqfaktor));
        //qDebug("dfanfang %f",dfanfang);
      }else{
        if(checkboxzf->isChecked()){
          qs = editzf->text();
          zf = linenormalisieren(qs);
          // nur wenn keine Frequenzvervielfachung
          dfanfang += zf;
        }
      }
    break;
    case espektrumanalyser:
      if(sabereich1->isChecked()){
        dfanfang += wgrunddaten.frqzf1;
        wobdaten.psavabs = wgrunddaten.psavabs1;
      }
      //Bereich 2 Pegelabweichung addieren
      if(sabereich2->isChecked()){
        dfanfang += wgrunddaten.frqzf2;
        wobdaten.psavabs = wgrunddaten.psavabs2;
      }
    break;
    case espekdisplayshift:
      //SA-Konverter Frequenzverschiebung addieren
      dfanfang += wgrunddaten.frqshift;
      if(sabereich1->isChecked()){
        dfanfang += wgrunddaten.frqzf1;
        wobdaten.psavabs = wgrunddaten.psavabs1;
      }
      //Bereich 2 Pegelabweichung addieren
      if(sabereich2->isChecked()){
        dfanfang += wgrunddaten.frqzf2;
        wobdaten.psavabs = wgrunddaten.psavabs2;
      }
      //SAshift Pegelabweichung addieren
      wobdaten.psavabs = wobdaten.psavabs + wgrunddaten.psavabs3;
    break;
  }
  
  
/*  
  wcounter = 3;
  fanfang =  wobdaten.anfangsfrequenz;
  //SAshift Frequenzverschiebung addieren
  if(wobdaten.ebetriebsart == espekdisplayshift){
    fanfang += weichen.frqshift;
  }
  //SA ZF addieren
  if((wobdaten.ebetriebsart == espektrumanalyser) or 
     (wobdaten.ebetriebsart == espekdisplayshift)){
    //Bereich 1 Pegelabweichung addieren
    if(sabereich1->isChecked()){
      fanfang += weichen.frqzf1;
      wobdaten.psavabs = weichen.psavabs1;
    }
    //Bereich 2 Pegelabweichung addieren
    if(sabereich2->isChecked()){
      fanfang += weichen.frqzf2;
      wobdaten.psavabs = weichen.psavabs2;
    }
    //SAshift Pegelabweichung addieren
    if(wobdaten.ebetriebsart == espekdisplayshift){
      wobdaten.psavabs = wobdaten.psavabs + weichen.psavabs3;
    } 
  }else{
    // alle anderen Betriebsarten
    // eventuell ZF addieren
    if(checkboxzf->isChecked()){
      qs = editzf->text();
      zf = linenormalisieren(qs);
      // nur wenn keine Frequenzvervielfachung
      if(frqfaktor == 1.0)fanfang += long(zf);
    }
  }
  fschrittfrequenz = wobdaten.schrittfrequenz;
  // bei Frequenzvervielfachung Frequenzen umrechnen
  if(frqfaktor > 1.0 and wobdaten.ebetriebsart == ewobbeln){
    fanfang = fanfang / frqfaktor;
    fschrittfrequenz = wobdaten.schrittfrequenz / frqfaktor;
  }
*/
  //Daten in LONG umrechen
  fanfang = long(round(dfanfang));
  fschrittfrequenz = long(round(dfschrittfrequenz));
  //NF Wobbeln
  if((checkboxk2->isChecked() == true) and (checkboxk1->isChecked() == false) and
     (wobdaten.maudio2 == true) and (fwversion == 120)){
    int ztime = wgrunddaten.audioztime / wobdaten.schritte;
    if(ztime > 999)ztime = 999;
    //qDebug() << "wgrunddaten.audioztime :"  << wgrunddaten.audioztime;
    //qDebug() << "ztime :"  << ztime;
    qs.sprintf("%09li%08li%04i%03i%02i", fanfang, fschrittfrequenz, wobdaten.schritte, ztime, wgrunddaten.audiosens);
    if(wobdaten.linear1 or wobdaten.linear2){
      qs = "d" + qs; //linear einschalten
    }else{
      qs = "c" + qs; //log einschalten
    };
  }else{
    if(boxzwischenzeit->currentIndex() != 0){
      qs.sprintf("%09li%08li%04i%03i", fanfang,
                                      fschrittfrequenz,
                                      wobdaten.schritte,
                                      wobdaten.ztime);
      if(wobdaten.linear1 or wobdaten.linear2){
        qs = "b" + qs; //linear einschalten
      }else{
        qs = "a" + qs; //log einschalten
      }
    }else{
      qs.sprintf("%09li%08li%04i", fanfang,
                                   fschrittfrequenz,
                                   wobdaten.schritte);
      if(wobdaten.linear1 or wobdaten.linear2){
        qs = "w" + qs; //linear einschalten
      }else{
        qs = "x" + qs; //log einschalten
      }
    }
  }
  //qDebug(qs);
  ttyindex = 0; //index zum lesen der Daten vom NWT auf 0
  if(!bttyOpen){
    //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
      QMessageBox::warning( this, tr("Serielle Schnittstelle","WarungsBox"),
                                  "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","WarungsBox"));
      #ifdef LDEBUG
      qDebug() << "Nwt7linux::sendwobbeln()\n";
      #endif    
      return;
    }
  }
  if (bttyOpen)
  {
    defaultlesenrtty(); //eventuelle Daten im UART loeschen
    //zuerst 8fh senden Vektor fuer LinuxSW im PIC
    picmodem->writeChar(143);
    picmodem->writeLine(qs);
    //qDebug() << "sendwobbeln()";
    //qDebug() << qs;
    wcounter = 3;
    idletimer->stop();
    stimer->stop();
    vtimer->stop();
    wobbeltimer->start(10);
    progressbar->setMaximum(wobdaten.schritte * 4);
    buttoneinmal->setEnabled(false);
    buttonwobbeln->setEnabled(false);
//    groupdaempfung->setEnabled(false);
//    daempfung2->setEnabled(false);
    emit setmenu(emEinmal, false);
    emit setmenu(emWobbeln, false);
    synclevel=0;
    bdauerwobbeln = true;
  }
}

void Nwt7linux::setparkfrq()
{
  char spark[20];
  
  if(bttyOpen and wgrunddaten.bset0hz){
    sprintf(spark,"f%09.0f", 0.0);
    picmodem->writeChar(143);
    picmodem->writeLine(spark);
  }
  //qDebug("Nwt7linux::setparkfrq");
}


void Nwt7linux::readtty(){
  int  i, j;
  qint64 a;
  char abuffer[(maxmesspunkte * 4)];
  double yf=0.0, ys=0.0;
  unsigned char c, c1;
  //bufferlaengenmultiplikator festlegen
  int bufferfaktor = 4;
  double afrequenz;
  double findex;
  QString s;
  
  //Daten aus der API lesen
  if (bttyOpen){
    //a = picmodem->size();
    a = picmodem->readttybuffer(abuffer, ((wobdaten.schritte) * bufferfaktor));
    #ifdef LDEBUG
    qDebug("readtty()");
    #endif
    if(a>0){
      #ifdef LDEBUG
       qDebug("a>0");
      #endif
      bdatenruecklauf = true;
    }
    if(a == 0){
      synclevel++; //es lagen keine Daten an
    }else{
      abbruchlevel = 0;
      synclevel = 0; //es lagen Daten an
    }
    for(i=0, j=ttyindex; i<a; i++, j++){
      ttybuffer[j] = abuffer[i]; //Die gelesenen Daten in den richtigen Buffer kopieren
    }
    ttyindex = ttyindex + a; // und den Index auf die richtige Stelle setzen
    progressbar->setValue(ttyindex);
    for(i=0, j=0; i < (ttyindex / bufferfaktor); i++, j++){
      c = ttybuffer[4 * i];
      c1 = ttybuffer[(4 * i) + 1];
      wobdaten.mess.k1[i] = c + (c1 * 256);
      c = ttybuffer[(4 * i) + 2];
      c1 = ttybuffer[(4 * i) + 3];
      wobdaten.mess.k2[i] = c + (c1 * 256);
    }
    if(bdatenruecklauf){
      wobdaten.mess.daten_enable = true;
      wcounter--;
      if(wcounter==0){
        if(wgrunddaten.bfliessend)emit sendewobbelkurve(wobdaten.mess);
        wcounter=3;
      }
    }
  }
  if(((wobdaten.schritte) * bufferfaktor) == ttyindex){
    //Alle Daten sind vom NWT7 gelesen und werden gesplittet Kanal 1+2
    for(i=0, j=0; i < (ttyindex / bufferfaktor); i++, j++){
      c = ttybuffer[4 * i];
      c1 = ttybuffer[(4 * i) + 1];
      wobdaten.mess.k1[i] = c + (c1 * 256);
      c = ttybuffer[(4 * i) + 2];
      c1 = ttybuffer[(4 * i) + 3];
      wobdaten.mess.k2[i] = c + (c1 * 256);
    }
    //Wobbeldaten sind gueltig
    if(bdatenruecklauf){
      wobdaten.mess.daten_enable = true;
    }else{
      wobdaten.mess.daten_enable = false;
    }
    //Wobbeldaten ins Grafikfenster kopieren
    fsendewobbeldaten();
    if(wobbelstop){
      //DDS Ausgang auf 0Hz setzen
      setparkfrq();
      //Button + Menue wieder aktivieren
      wobdaten.mousesperre = false; //mousekursor wieder freigeben
      fsendewobbeldaten();
      bdauerwobbeln = false;
      setKursorFrequenz(KursorFrequenz);
      buttoneinmal->setEnabled(true);
      buttonwobbeln->setEnabled(true);
      buttonstop->setEnabled(false);
      idletimer->start(wgrunddaten.idletime);
      if((wobdaten.ebetriebsart == eswrneu) or (wobdaten.ebetriebsart == eswrant)){
        groupdaempfung->setEnabled(false);
        daempfung2->setEnabled(false);
      }
      if((wobdaten.ebetriebsart == ewobbeln) or 
         (wobdaten.ebetriebsart == espektrumanalyser) or 
         (wobdaten.ebetriebsart == espekdisplayshift)){
        groupdaempfung->setEnabled(true);
        daempfung2->setEnabled(true);
      }
      emit setmenu(emEinmal, true);
      emit setmenu(emWobbeln, true);
      emit setmenu(emStop, false);
      groupzoom->setEnabled(true);
      groupwobbel->setEnabled(true);
      groupbetriebsart->setEnabled(true);
      if(!wgrunddaten.fwfalsch){
        //Zwischenzeit bei geeigneter FW
        boxzwischenzeit->setEnabled(true);
        labelzwischenzeit->setEnabled(true);
      }
      setTabEnabled(wkmanager, true);
      setTabEnabled(nwt7vfo, true);
      setTabEnabled(berechnung, true);
      // K1 oder K2 linear = Messfenster deaktivieren
      if(wobdaten.linear2 || wobdaten.linear1){
        setTabEnabled(nwt7messen, false);
      }else{
        setTabEnabled(nwt7messen, true);
      }
      if(bkalibrieren0){
        ym_0db = 0.0;
        //Kanal 1
        if(checkboxk1->isChecked()){
          //  Mittelwert bilden
          for(i=0; i<wobdaten.schritte; i++){
            ym_0db = ym_0db + double(wobdaten.mess.k1[i]);
          }
          ym_0db = ym_0db / double(wobdaten.schritte);
          //qDebug("ym_0db = %f",ym_0db); 
          //SWV eichen
          if(bkalibrierenswr){
            if(wobdaten.ebetriebsart == eswrneu){
              afrequenz = wobdaten.anfangsfrequenz;
              for(i=0; i<wobdaten.schritte; i++){
                yf = double(wobdaten.mess.k1[i]) * wobdaten.faktor1 + wobdaten.shift1;
                findex = (afrequenz + (wobdaten.schrittfrequenz * i)) / wobdaten.frqfaktor;
                j = int(findex / eteiler);
                wobdaten.swrkalibrierarray.arrayk1[j] = yf;
              }
              emit sendewobbeldaten(wobdaten);
              bkalibrierenswr = false;
              //qDebug("faktor1 %f shift1 %f", wobdaten.faktor1, wobdaten.shift1);	      
            }
          //Wobbeln kalibrieren
          }else{
            // Lineare Sonde kalibrieren
            if(wobdaten.linear1){
              yf = 50.0 / (ym_0db - ym_40db);
              ys = ((ym_0db * yf) - maxkalibrier) * -1.0;
              //yf = (yaufloesung * 5.0)/(11.0 * (ym_0db - ym_40db));
              //ys = (yaufloesung * 5.0)/11.0 - yf * ym_40db;
              //logaritmische Sonde Eichen
            }else{
              //qDebug("ym_0dB = %f; ym_40dB = %f", ym_0db, ym_40db);
              //Neue Berechnung ab V3.05
              yf = (40.0 + maxkalibrier) / (ym_0db - ym_40db);
              //yf = (yaufloesung * (4.0 - maxkalibrier))/(ydisplayteilung_log * (ym_0db - ym_40db));
              //Neue Berechnung ab V3.05
              ys = ((ym_0db * yf) - maxkalibrier) * -1.0;
              //ys = (yaufloesung * 5.0)/ydisplayteilung_log - yf * ym_40db;
            }
            //qDebug("faktor = %f; shift = %f", yf, ys);
            wobdaten.faktor1 = yf;
            wobdaten.shift1 = ys;
            // Eichkorretur nur bei log. Sonde
            if(!wobdaten.linear1){
              afrequenz = wobdaten.anfangsfrequenz;
              for(i=0; i<wobdaten.schritte; i++){
                //aus der Frequenz den Index j errechnen
                //Neue Berechnung ab V3.05
                findex = (afrequenz + (wobdaten.schrittfrequenz * i)) / wobdaten.frqfaktor;
                j = int(findex / eteiler);
                wobdaten.kalibrierarray.arrayk1[j] = (double(wobdaten.mess.k1[i]) * wobdaten.faktor1 + wobdaten.shift1) - maxkalibrier;
                //qDebug(" nkalibrierarray.arrayk1[j] %f", nkalibrierarray.arrayk1[j]);
                //yf = double(wobdaten.mess.k1[i]) * wobdaten.faktor1 + wobdaten.shift1;
                //nkalibrierarray.arrayk1[j] = (yaufloesung * (0.9 - maxkalibrier/10.0)) - yf;
              }
              emit sendewobbeldaten(wobdaten);
            }
          }
        }
        if(checkboxk2->isChecked()){
          //  Mittelwert bilden
          for(i=0; i<wobdaten.schritte; i++){
            ym_0db = ym_0db + double(wobdaten.mess.k2[i]);
          }
          ym_0db = ym_0db / double(wobdaten.schritte);
          if(wobdaten.maudio2)ym_0db = ym_0db / 32.0;
          //qDebug("ym_0db = %f",ym_0db); 
          //SWV kalibrieren
          if(bkalibrierenswr){
            if(wobdaten.ebetriebsart == eswrneu){
              afrequenz = wobdaten.anfangsfrequenz;
              for(i=0; i<wobdaten.schritte; i++){
                yf = double(wobdaten.mess.k2[i]) * wobdaten.faktor2 + wobdaten.shift2;
                findex = (afrequenz + (wobdaten.schrittfrequenz * i)) / wobdaten.frqfaktor;
                j = int(findex / eteiler);
                wobdaten.swrkalibrierarray.arrayk2[j] = yf;
              }
              emit sendewobbeldaten(wobdaten);
              bkalibrierenswr = false;
              //qDebug("faktor1 %f shift1 %f", wobdaten.faktor1, wobdaten.shift1);	      
            }
          //Wobbeln kalibrieren
          }else{
            // Lineare Sonde kalibrieren
            if(wobdaten.linear2){
              yf = 50.0 / (ym_0db - ym_40db);
              ys = ((ym_0db * yf) - maxkalibrier) * -1.0;
              //yf = (yaufloesung * 5.0)/(11.0 * (ym_0db - ym_40db));
              //ys = (yaufloesung * 5.0)/11.0 - yf * ym_40db;
              //logaritmische Sonde Eichen
            }else{
              //qDebug("ym_0dB = %f; ym_40dB = %f", ym_0db, ym_40db);
              //Neue Berechnung ab V3.05
              yf = (40.0 + maxkalibrier) / (ym_0db - ym_40db);
              //yf = (yaufloesung * (4.0 - maxkalibrier))/(ydisplayteilung_log * (ym_0db - ym_40db));
              //Neue Berechnung ab V3.05
              ys = ((ym_0db * yf) - maxkalibrier) * -1.0;
              //ys = (yaufloesung * 5.0)/ydisplayteilung_log - yf * ym_40db;
            }
            //qDebug("faktor = %f; shift = %f", yf, ys);
            wobdaten.faktor2 = yf;
            wobdaten.shift2 = ys;
            // Eichkorretur nur bei log. Sonde
            if(!wobdaten.linear2){
              afrequenz = wobdaten.anfangsfrequenz;
              for(i=0; i<wobdaten.schritte; i++){
                //aus der Frequenz den Index j errechnen
                //Neue Berechnung ab V3.05
                findex = (afrequenz + (wobdaten.schrittfrequenz * i)) / wobdaten.frqfaktor;
                j = int(findex / eteiler);
                wobdaten.kalibrierarray.arrayk2[j] = (double(wobdaten.mess.k2[i]) * wobdaten.faktor2 + wobdaten.shift2) - maxkalibrier;
                //qDebug(" nkalibrierarray.arrayk1[j] %f", nkalibrierarray.arrayk1[j]);
                //yf = double(wobdaten.mess.k1[i]) * wobdaten.faktor1 + wobdaten.shift1;
                //nkalibrierarray.arrayk1[j] = (yaufloesung * (0.9 - maxkalibrier/10.0)) - yf;
              }
              emit sendewobbeldaten(wobdaten);
            }
          }
        }
        //Wobbeldaten sind gueltig
        if(bdatenruecklauf){
          wobdaten.mess.daten_enable = true;
        }else{
          wobdaten.mess.daten_enable = false;
        }
        //Wobbeldaten ins Grafikfenster kopieren
        fsendewobbeldaten();
        //qDebug(wobdaten.namesonde1);
        if(QMessageBox::question(this, tr("Kalibrierung speichern? -- NWT","WarungsBox"), 
                                       tr("<B>Speichern der Werte in der Messsondendatei</B><BR>"
                                          "Die ermittelten Werte sofort abspeichern?","WarungsBox"),
                                       tr("&Ja","WarungsBox"), tr("&Nein","WarungsBox"),
                                 QString::null, 0, 1) == 0){
          if(wobdaten.ebetriebsart == ewobbeln){
            if(checkboxk1->isChecked()){
              if(wobdaten.linear1){
                messsondespeichern1(wgrunddaten.strsonde1lin);
              }else{
                messsondespeichern1(wobdaten.namesonde1);
              }
            }
            if(checkboxk2->isChecked()){
              messsondespeichern2(wobdaten.namesonde2);
	    }
          }
          if(wobdaten.ebetriebsart == eswrneu){
            if(checkboxk1->isChecked()){
              messsondespeichern1(filenamesonde1);
            }
            if(checkboxk2->isChecked()){
              messsondespeichern2(filenamesonde2);
	    }
          }
        }
        bkalibrieren0 = false;
        bkalibrierenswr = false;
      }
      if(bkalibrieren40){
        ym_40db = 0.0;
        if(checkboxk1->isChecked()){
          for(i=0; i<wobdaten.schritte; i++){
            ym_40db = ym_40db + double(wobdaten.mess.k1[i]);
          }
          ym_40db = ym_40db / double(wobdaten.schritte);
          setDaempfung(0); // 0 dB
          daempfung1->setCurrentIndex(0);
          daempfung2->setCurrentIndex(0);
          daempfung3->setCurrentIndex(0);
          maxkalibrier = 0.0;
          if(!wobdaten.linear1){          
            QString beschr(tr("<B>Ermitteln der zweiten dB Linie Kanal1</B><BR>"
                          "Ausgang mit Eingang verbinden oder ein <br>"
                          "kleines Daempfungsglied einschleifen."
                          "<BR><BR>"
                          "Eingabe des dB_Wertes oder default lassen: ","InputDialog"));
            bool ok;
            double l = QInputDialog::getDouble(this, tr("Kalibrieren Kanal 1","InputDialog"),
                                                    beschr, 0.0, -20.0, 0.0, 0, &ok); 
            if(ok){
              maxkalibrier = l;
              //maxkalibrier = l / -10.0;
            }
          }else{
          QMessageBox::warning( this, tr("Kalibrieren Kanal 1","InformationsBox"), 
                                      tr("<B>Ermitteln der 0 dB Linie</B><BR>"
                                         "Ausgang mit Eingang verbinden!","InformationsBox"));
          }
        }
        if(checkboxk2->isChecked()){
          for(i=0; i<wobdaten.schritte; i++){
            ym_40db = ym_40db + double(wobdaten.mess.k2[i]);
          }
          ym_40db = ym_40db / double(wobdaten.schritte);
          if(wobdaten.maudio2)ym_40db = ym_40db / 32.0;
          setDaempfung(0); // 0 dB
          daempfung1->setCurrentIndex(0);
          daempfung2->setCurrentIndex(0);
          daempfung3->setCurrentIndex(0);
          maxkalibrier = 0.0;
          if(!wobdaten.linear2){          
            QString beschr(tr("<B>Ermitteln der zweiten dB Linie Kanal2</B><BR>"
                          "Ausgang mit Eingang verbinden oder ein <br>"
                          "kleines Daempfungsglied einschleifen"
                          "<BR><BR>"
                          "Eingabe des dB_Wertes oder default lassen: ","InputDialog"));
            bool ok;
            double l = QInputDialog::getDouble(this, tr("Kalibrieren Kanal 2"),
                                                    beschr, 0.0, -20.0, 0.0, 0, &ok); 
            if(ok){
              maxkalibrier = l;
              //maxkalibrier = l / -10.0;
            }
          }else{
          QMessageBox::warning( this, tr("Kalibrieren Kanal 2","InformationsBox"), 
                                      tr("<B>Ermitteln der 0 dB Linie</B><BR>"
                                         "Ausgang mit Eingang verbinden!","InformationsBox"));
          }
        }
        bkalibrieren40 = false;
        bkalibrieren0 = true;
        //erneut wobbeln
        sendwobbeln();
        //Timer starten fuer lesen der NWT7 Daten
        idletimer->stop();
        stimer->stop();
        vtimer->stop();
        wobbeltimer->start(10);
      }
    }else{
      //erneut wobbeln
      sendwobbeln();
      //Timer starten fuer lesen der NWT7 Daten
      idletimer->stop();
      stimer->stop();
      vtimer->stop();
      wobbeltimer->start(10);
    }  
  }else{
    //Alle Daten sind noch nicht gelesen Timer neu starten
    if(synclevel >=200){
      abbruchlevel++;
      if(abbruchlevel > 5){
        //Abbruch der Funktionen
        QMessageBox::warning( this, tr("Serielle Schnittstelle","InformationsBox"), 
                                    tr("Es kommen keine Daten vom NWT!","InformationsBox"));
        #ifdef LDEBUG
        qDebug("Nwt7linux::readtty()");
        #endif
        idletimer->start(wgrunddaten.idletime);
        wobbelstop = true;
        buttoneinmal->setEnabled(true);
        buttonwobbeln->setEnabled(true);
        buttonstop->setEnabled(false);
        if((wobdaten.ebetriebsart == eswrneu) or 
	   (wobdaten.ebetriebsart == eswrant) or
	   (wobdaten.ebetriebsart == eimpedanz)){
          groupdaempfung->setEnabled(false);
          daempfung2->setEnabled(false);
        }
        if((wobdaten.ebetriebsart == ewobbeln) or
           (wobdaten.ebetriebsart == espektrumanalyser) or
           (wobdaten.ebetriebsart == espekdisplayshift)){
          groupdaempfung->setEnabled(true);
          daempfung2->setEnabled(true);
        }
        emit setmenu(emEinmal, true);
        emit setmenu(emWobbeln, true);
        emit setmenu(emStop, false);
        groupzoom->setEnabled(true);
        groupwobbel->setEnabled(true);
        groupbetriebsart->setEnabled(true);
        if(!wgrunddaten.fwfalsch){
          //Zwischenzeit bei geeigneter FW
          boxzwischenzeit->setEnabled(true);
          labelzwischenzeit->setEnabled(true);
        }
        setTabEnabled(wkmanager, true);
        setTabEnabled(berechnung, true);
        setTabEnabled(nwt7vfo, true);
        // K1 oder K2 linear = Messfenster deaktivieren
        if(wobdaten.linear2 || wobdaten.linear1){
          setTabEnabled(nwt7messen, false);
        }else{
          if(wobdaten.ebetriebsart == ewobbeln){
            setTabEnabled(nwt7messen, true);
          }
        }
        return;
      }else{
        synclevel = 0;
        sendwobbeln();
        idletimer->stop();
        stimer->stop();
        vtimer->stop();
        wobbeltimer->start(10);
        return;
      }
    }
    idletimer->stop();
    stimer->stop();
    vtimer->stop();
    wobbeltimer->start(10);
  }
}

void Nwt7linux::checkboxtime_checked(int b){
  if(b == Qt::Unchecked){
//    editscantime->setText("0");
//    wobdaten.ztime = 0;
  }else{
//    editscantime->setText("100.0");
  }
  wobnormalisieren();
}

void Nwt7linux::checkboxinvers_checked(bool b){
  //geaenderter Darstellung
  wobdaten.binvers = b;
  //Daten ins Grafikfenster uebermitteln und neu darstellen
  fsendewobbeldaten();
}

void Nwt7linux::checkboxgrafik_checked(bool b){
  //geaenderter Darstellung
  wobdaten.bandbreitegrafik = b;
  //Daten ins Grafikfenster uebermitteln und neu darstellen
  fsendewobbeldaten();
}

void Nwt7linux::checkbox6db_checked(bool b){
  //geaenderter Darstellung
  wobdaten.bandbreite6db = b;
  if(b)wobdaten.bandbreitegrafik = b;
  if(b)checkboxgrafik->setChecked(b);
  //Daten ins Grafikfenster uebermitteln und neu darstellen
  fsendewobbeldaten();
}

void Nwt7linux::checkbox3db_checked(bool b){
  //geaenderter Darstellung
  wobdaten.bandbreite3db = b;
  if(b)wobdaten.bandbreitegrafik = b;
  if(b)checkboxgrafik->setChecked(b);
  //Daten ins Grafikfenster uebermitteln und neu darstellen
  fsendewobbeldaten();
}

void Nwt7linux::checkboxk1_checked(bool b){
  //geaenderter Darstellung Kanal 1
  wobdaten.bkanal1 = b;
  switch(wobdaten.ebetriebsart){
    case eantdiagramm:
      break;
    case ewobbeln:
    break;
    case eswrneu:
    case eswrant:
    case eimpedanz:
      if(b){
        wobdaten.bkanal2 = false;
        checkboxk2->setChecked(false);
        wobdaten.bswrkanal2 = false;
      }else{
        wobdaten.bkanal2 = true;
        checkboxk2->setChecked(true);
        wobdaten.bswrkanal2 = true;
      }
    break;
    case espektrumanalyser:
    case espekdisplayshift:
    break;
  }
  //Daten ins Grafikfenster uebermitteln und neu darstellen
  fsendewobbeldaten();
}

void Nwt7linux::checkboxk2_checked(bool b){
  //geaenderter Darstellung Kanal 2
  wobdaten.bkanal2 = b;
  switch(wobdaten.ebetriebsart){
    case eantdiagramm:
    break;
    case ewobbeln:
    break;
    case eswrneu:
    case eswrant:
    case eimpedanz:
      if(b){
        wobdaten.bkanal1 = false;
        checkboxk1->setChecked(false);
        wobdaten.bswrkanal2 = true;
      }else{
        wobdaten.bkanal1 = true;
        checkboxk1->setChecked(true);
        wobdaten.bswrkanal2 = false;
      }
    break;
    case espektrumanalyser:
    case espekdisplayshift:
    break;
  }
  
  //Daten ins Grafikfenster uebermitteln und neu darstellen
  fsendewobbeldaten();
}

void Nwt7linux::setSaBereich1(){
  sabereich = bsa1;
  setFrqBereich();
}

void Nwt7linux::setSaBereich2(){
  sabereich = bsa2;
  setFrqBereich();
}

void Nwt7linux::setFrqBereich(){
  QString qs;
  
  //nur wenn sich Beriebsart oder SA-Bereich aendert
  //nicht beim Laden der Kurvendatei
  if(!bkurvegeladen){
    if(sabereich != sabereichalt){
      if(wobdaten.ebetriebsart == espektrumanalyser){
        if(sabereich1->isChecked()){
          setanfang(double(wgrunddaten.sa1anfang));
          setende(double(wgrunddaten.sa1ende));
          setschritte(double(wgrunddaten.sa1schritte));
          sabereich = bsa1;
        }
        if(sabereich2->isChecked()){
          setanfang(double(wgrunddaten.sa2anfang));
          setende(double(wgrunddaten.sa2ende));
          setschritte(double(wgrunddaten.sa2schritte));
          sabereich = bsa2;
        }
      }else{
        if(wobdaten.ebetriebsart == espekdisplayshift){
          if(sabereich1->isChecked()){
            setanfang(double(wgrunddaten.sa3anfang));
            setende(double(wgrunddaten.sa3ende));
            setschritte(double(wgrunddaten.sa3schritte));
            sabereich = bsa3;
          }
          if(sabereich2->isChecked()){
            /*setanfang(double(wgrunddaten.sa2anfang- wgrunddaten.frqshift));
            setende(double(wgrunddaten.sa2ende- wgrunddaten.frqshift));
            setschritte(double(wgrunddaten.sa2schritte));
            sabereich = bsa2;*/
          }
        }else{
          setanfang(double(wgrunddaten.wanfang));
          setende(double(wgrunddaten.wende));
          setschritte(double(wgrunddaten.wschritte));
          sabereich = bwobbeln;
        }
      }
      wobnormalisieren();
      sabereichalt = sabereich;
    }
  }
}


void Nwt7linux::setBetriebsart(int i){
  QString qs;

  switch(i){
    case 0:
      qs = "o0"; //SWR Relais aus
      wobdaten.ebetriebsart = ewobbeln;
      sabereich = bwobbeln;
      emit setmenu(emEichenK1, true);
      if(!wobdaten.einkanalig){
        emit setmenu(emEichenK2, true);
        checkboxk2->setEnabled(true);
        emit setmenu(emSondeSpeichernK2, true);
        emit setmenu(emSondeLadenK2, true);
        kalibrier2->show();
      }	
      kalibrier1->show();
      checkboxk1->setEnabled(true);
      checkboxgrafik->setEnabled(true);
      checkbox3db->setEnabled(true);
      checkbox6db->setEnabled(true);
      checkboxinvers->setEnabled(true);
      groupdaempfung->setEnabled(true);
      daempfung2->setEnabled(true);
      aufloesung->hide();
      labelaufloesung->hide();
      //SA
      groupsa->hide();
      labelsafrqbereich->hide();
      labelsabandbreite->hide();
      sabereich1->hide();
      sabereich2->hide();
      //setTabEnabled(nwt7messen, true);
      labela_100->hide();
      labelkabellaenge->hide();
      edita_100->hide();
      editkabellaenge->hide();
      boxdbshift1->setEnabled(true);
      boxdbshift2->setEnabled(true);
      labelboxdbshift1->setEnabled(true);
      labelboxdbshift2->setEnabled(true);
      break;
    case 1:  
      qs = "o1"; //SWR Relais ein
      wobdaten.ebetriebsart = eswrneu;
      sabereich = bwobbeln;
      emit setmenu(emEichenK1, true);
      emit setmenu(emEichenK2, true);
      emit setmenu(emSondeSpeichernK2, true);
      emit setmenu(emSondeLadenK2, true);
      if(checkboxk1->isChecked() and checkboxk2->isChecked()){
        checkboxk1->setChecked(true);
        checkboxk2->setChecked(false);
        wobdaten.bswrkanal2 = false;
      }
      if(!checkboxk1->isChecked() and !checkboxk2->isChecked()){
        checkboxk1->setChecked(true);
        checkboxk2->setChecked(false);
        wobdaten.bswrkanal2 = false;
      }
//      checkboxk1->setChecked(true);
//      checkboxk2->setChecked(false);
//      checkboxk1->setEnabled(false);
      if(!wobdaten.einkanalig){
        checkboxk2->setEnabled(true);
      }
      kalibrier1->hide();
      kalibrier2->hide();
      checkboxgrafik->setEnabled(false);
      checkbox3db->setEnabled(false);
      checkbox6db->setEnabled(false);
      checkboxinvers->setEnabled(false);
      //Relais auf 0dB schalten
      daempfung1->setCurrentIndex(0); 
      groupdaempfung->setEnabled(false);
      daempfung2->setCurrentIndex(0); 
      daempfung3->setCurrentIndex(0); 
      daempfung2->setEnabled(false);
      aufloesung->show();
      labelaufloesung->show();
      //SA
      groupsa->hide();
      labelsafrqbereich->hide();
      labelsabandbreite->hide();
      sabereich1->hide();
      sabereich2->hide();
      //setTabEnabled(nwt7messen, false);
      labela_100->hide();
      labelkabellaenge->hide();
      edita_100->hide();
      editkabellaenge->hide();
      boxdbshift1->setCurrentIndex(20);
      boxdbshift2->setCurrentIndex(20);
      boxdbshift1->setEnabled(false);
      boxdbshift2->setEnabled(false);
      labelboxdbshift1->setEnabled(false);
      labelboxdbshift2->setEnabled(false);
      break;
    case 2:  
      qs = "o1"; //SWR Relais ein
      wobdaten.ebetriebsart = eswrant;
      sabereich = bwobbeln;
      emit setmenu(emEichenK1, false);
      emit setmenu(emEichenK2, false);
      emit setmenu(emSondeSpeichernK2, false);
      emit setmenu(emSondeLadenK2, false);
      if(checkboxk1->isChecked() and checkboxk2->isChecked()){
        checkboxk1->setChecked(true);
        checkboxk2->setChecked(false);
        wobdaten.bswrkanal2 = false;
      }
      if(!checkboxk1->isChecked() and !checkboxk2->isChecked()){
        checkboxk1->setChecked(true);
        checkboxk2->setChecked(false);
        wobdaten.bswrkanal2 = false;
      }
      kalibrier1->hide();
      kalibrier2->hide();
//     checkboxk1->setChecked(true);
//     checkboxk2->setChecked(false);
//     checkboxk1->setEnabled(false);
      if(!wobdaten.einkanalig)checkboxk2->setEnabled(true);
      checkboxgrafik->setEnabled(false);
      checkbox3db->setEnabled(false);
      checkbox6db->setEnabled(false);
      checkboxinvers->setEnabled(false);
      //Relais auf 0dB schalten
      daempfung1->setCurrentIndex(0); 
      groupdaempfung->setEnabled(false);
      daempfung2->setCurrentIndex(0); 
      daempfung3->setCurrentIndex(0); 
      daempfung2->setEnabled(false);
      aufloesung->show();
      labelaufloesung->show();
      labela_100->show();
      labelkabellaenge->show();
      edita_100->show();
      editkabellaenge->show();
      boxdbshift1->setCurrentIndex(20);
      boxdbshift2->setCurrentIndex(20);
      boxdbshift1->setEnabled(false);
      boxdbshift2->setEnabled(false);
      labelboxdbshift1->setEnabled(false);
      labelboxdbshift2->setEnabled(false);
      //SA
      groupsa->hide();
      labelsafrqbereich->hide();
      labelsabandbreite->hide();
      sabereich1->hide();
      sabereich2->hide();
      //setTabEnabled(nwt7messen, false);
      break;
    case 3:  
      qs = "o1"; //SWR Relais ein
      wobdaten.ebetriebsart = eimpedanz;
      sabereich = bwobbeln;
      emit setmenu(emEichenK1, false);
      emit setmenu(emEichenK2, false);
      emit setmenu(emSondeSpeichernK2, false);
      emit setmenu(emSondeLadenK2, false);
      if(checkboxk1->isChecked() and checkboxk2->isChecked()){
        checkboxk1->setChecked(true);
        checkboxk2->setChecked(false);
        wobdaten.bswrkanal2 = false;
      }
      if(!checkboxk1->isChecked() and !checkboxk2->isChecked()){
        checkboxk1->setChecked(true);
        checkboxk2->setChecked(false);
        wobdaten.bswrkanal2 = false;
      }
      kalibrier1->hide();
      kalibrier2->hide();
//      checkboxk1->setChecked(true);
//      checkboxk2->setChecked(false);
//      checkboxk1->setEnabled(false);
      if(!wobdaten.einkanalig)checkboxk2->setEnabled(true);
      checkboxgrafik->setEnabled(false);
      checkbox3db->setEnabled(false);
      checkbox6db->setEnabled(false);
      checkboxinvers->setEnabled(false);
      //Relais auf 0dB schalten
      daempfung1->setCurrentIndex(0); 
      groupdaempfung->setEnabled(false);
      daempfung2->setCurrentIndex(0); 
      daempfung3->setCurrentIndex(0); 
      daempfung2->setEnabled(false);
      aufloesung->hide();
      labelaufloesung->hide();
      labela_100->hide();
      labelkabellaenge->hide();
      edita_100->hide();
      editkabellaenge->hide();
      //SA
      groupsa->hide();
      labelsafrqbereich->hide();
      labelsabandbreite->hide();
      sabereich1->hide();
      sabereich2->hide();
      //setTabEnabled(nwt7messen, false);
      boxdbshift1->setCurrentIndex(20);
      boxdbshift2->setCurrentIndex(20);
      boxdbshift1->setEnabled(false);
      boxdbshift2->setEnabled(false);
      labelboxdbshift1->setEnabled(false);
      labelboxdbshift2->setEnabled(false);
      if(!bkurvegeladen){
        QMessageBox::warning( this, tr("Impedanzmessung mit Serienwiderstand 50 Ohm","InformationsBox"), 
                                    tr("<B>Impedanzmessung Z (absolut)</B><BR>"
                                       "Fuer die richtige Anzeige des absoluten Z-Wertes "
                                       "muss ein Widerstand von 50 Ohm am Messkopfausgang "
                                       "in Serie eingeschleift werden.","InformationsBox"));
      }
      break;
    case 4:
      qs = "o0"; //SWR Relais aus
      wobdaten.ebetriebsart = espektrumanalyser;
      emit setmenu(emEichenK1, false);
      if(!wobdaten.einkanalig){
        emit setmenu(emEichenK2, false);
        checkboxk2->setEnabled(true);
        emit setmenu(emSondeSpeichernK2, true);
        emit setmenu(emSondeLadenK2, true);
      }	
      kalibrier1->hide();
      kalibrier2->hide();
      checkboxk1->setEnabled(true);
      checkboxgrafik->setEnabled(true);
      checkbox3db->setEnabled(true);
      checkbox6db->setEnabled(true);
      checkboxinvers->setEnabled(true);
      groupdaempfung->setEnabled(true);
      daempfung2->setEnabled(true);
      aufloesung->hide();
      labelaufloesung->hide();
      //SA
      if(wgrunddaten.sastatus){
        groupsa->setTitle(tr("Status","SpectrumAnalyser"));
        groupsa->show();
        labelsafrqbereich->show();
        labelsabandbreite->show();
        sabereich1->hide();
        sabereich2->hide();
      }else{
        groupsa->setTitle(tr("Bereich","SpectrumAnalyser"));
        groupsa->show();
        labelsafrqbereich->hide();
        labelsabandbreite->hide();
        sabereich1->show();
        sabereich2->show();
        if(sabereich1->isChecked())sabereich = bsa1;
        if(sabereich2->isChecked())sabereich = bsa2;
      }
      labela_100->hide();
      labelkabellaenge->hide();
      edita_100->hide();
      editkabellaenge->hide();
      boxdbshift1->setEnabled(true);
      boxdbshift2->setEnabled(true);
      labelboxdbshift1->setEnabled(true);
      labelboxdbshift2->setEnabled(true);
      break;
    case 5:
      qs = "o0"; //SWR Relais aus
      wobdaten.ebetriebsart = espekdisplayshift;
      emit setmenu(emEichenK1, false);
      if(!wobdaten.einkanalig){
        emit setmenu(emEichenK2, false);
        checkboxk2->setEnabled(true);
        emit setmenu(emSondeSpeichernK2, true);
        emit setmenu(emSondeLadenK2, true);
      }	
      kalibrier1->hide();
      kalibrier2->hide();
      checkboxk1->setEnabled(true);
      checkboxgrafik->setEnabled(false);
      checkbox3db->setEnabled(false);
      checkbox6db->setEnabled(false);
      checkboxinvers->setEnabled(false);
      groupdaempfung->setEnabled(true);
      daempfung2->setEnabled(true);
      aufloesung->hide();
      labelaufloesung->hide();
      //SA
      if(wgrunddaten.sastatus){
        groupsa->setTitle(tr("Status + FrequenzShift","SpectrumAnalyser"));
        groupsa->show();
        labelsafrqbereich->show();
        labelsabandbreite->show();
        sabereich1->hide();
        sabereich2->hide();
      }else{
        groupsa->setTitle(tr("Bereich + FrequenzShift","SpectrumAnalyser"));
        groupsa->show();
        labelsafrqbereich->hide();
        labelsabandbreite->hide();
        sabereich1->show();
        sabereich2->show();
      }
      labela_100->hide();
      labelkabellaenge->hide();
      edita_100->hide();
      editkabellaenge->hide();
      boxdbshift1->setEnabled(true);
      boxdbshift2->setEnabled(true);
      labelboxdbshift1->setEnabled(true);
      labelboxdbshift2->setEnabled(true);
      sabereich = bsa3;
      break;
  }
  setFrqBereich();
  if(!bkurvegeladen)wobnormalisieren();
  //nur Befehl zum PIC senden, wenn Befehl im PIC implementiert
  if(wobdaten.bswrrelais){
    if(!bttyOpen){
      //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
      if(!bttyOpen){
        QMessageBox::warning( this, tr("Serielle Schnittstelle","InformationsBox"), 
                                      "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","InformationsBox"));
        #ifdef LDEBUG
        qDebug("Nwt7linux::setBetriebsart()");
        #endif    
        return;
      }
    }
    if(bttyOpen)
    {
      picmodem->writeChar(143);
      picmodem->writeLine(qs);
    }
  }
  //Daten ins Grafikfenster uebermitteln und neu darstellen
  fsendewobbeldaten();
}

void Nwt7linux::setanfang(double a)
{
  QString s;
  s.sprintf("%1.0f",a);
  //qDebug(s);
  editanfang->setText(s);
}

void Nwt7linux::setende(double a)
{
  QString s;
  s.sprintf("%1.0f",a);
  editende->setText(s);
}

void Nwt7linux::setschritte(double a)
{
  QString s;
  s.sprintf("%1.0f",a);
  editschritte->setText(s);
}

void Nwt7linux::readberechnung(double a)
{
  QString s;
  s.sprintf("%f",a/1000000.0);
//  qDebug(s);
  editf1->setText(s);
  editf2->setText(s);
}

void Nwt7linux::schwingkreisfc()
{
  QString sf;
  QString sl;
  QString sc;
  double f;
  double l;
  double c;
  double xc;
  bool ok;
  
  //qDebug("schwingkreisfc()");
  sf = editf2->text();
  //qDebug(sf);
  f = sf.toDouble(&ok);
  if(!ok)return;
  sl = editl2->text();
  //qDebug(sl);
  l = sl.toDouble(&ok);
  if(!ok)return;
  f = f * 1000000.0;
  //qDebug("f=%g",f);
  l = l / 1000000.0;
  //qDebug("c=%g",l);
  c = 1.0/(4.0*3.1415*3.1415*l*f*f);
  xc = 1.0/(2.0*3.1415*f*c);
  //qDebug("c=%g",c);
  //sl.sprintf("%03.3f",l);
  sc = QString("%1").arg(c*1000000000000.0);
  //qDebug(sc);
  ergebnisc->setText(sc);
  sc = QString("%1").arg(xc);
  //qDebug(sc);
  ergebnisxc->setText(sc);
}

void Nwt7linux::alwindungen()
{
  QString sal;
  QString sl;
  QString sw;
  double al;
  double l;
  double w;
  bool ok;
  
  sal = edital3->text();
  al = sal.toDouble(&ok);
  if(!ok)return;
  sl = editl3->text();
  l = sl.toDouble(&ok);
  if(!ok)return;
  l = l * 1000.0;
  //  qDebug("l=%g",l);
  //  qDebug("al=%g",al);
  w = sqrt(l/al);
  //  qDebug("al * l=%g",al * l);
  //  qDebug("w=%g",w);
  sw = QString("%1").arg(w);
  if((sal.length() > 0) and (sl.length() > 0))ergebnisw->setText(sw);
}

void Nwt7linux::schwingkreisfl()
{
  QString sf;
  QString sl;
  QString sc;
  QString sn;
  double f;
  double l;
  double c;
  double n;
  double al;
  double xl;
  bool ok;
  
  sf = editf1->text();
  f = sf.toDouble(&ok);
  if(!ok)return;
  sc = editc1->text();
  c = sc.toDouble(&ok);
  if(!ok)return;
  f = f * 1000000.0;
  c = c / 1000000000000.0;
  l = 1.0/(4.0*3.1415*3.1415*c*f*f);
  xl = 2.0*3.1415*f*l;
  sl = QString("%1").arg(l*1000000.0);
  ergebnisl->setText(sl);
  if(sl.length() > 0){
    sn = editn->text();
    n = sn.toDouble(&ok);
    sl = QString("%1").arg(xl);
    ergebnisxl->setText(sl);
    if(!ok)return;
    al = l / (n * n);
    sl = QString("%1").arg(al*1000000000.0);
    ergebnisal->setText(sl);
    //AL-Wert mit ins naechste Berechnungsfenster kopieren
    edital3->setText(sl);
  }
}

double Nwt7linux::linenormalisieren(const QString &line)
{
  bool ok;
  double faktor=1.0;
  double ergebnis = 0.0;
  bool gefunden = false;
  
  QString aline(line);
  
  aline = aline.toLower();	//alles auf Kleinschreibung
  aline.remove(QChar(' ')); // Leerzeichen entfernen
  if(aline.contains('g')){
    if(aline.contains(',') or aline.contains('.')){
      aline.replace('g',' ');
    }else{
      aline.replace('g','.');
    }
    faktor = 1000000000.0;
    gefunden = true;
  }
  if(aline.contains('m')){
    if(aline.contains(',') or aline.contains('.')){
      aline.replace('m',' ');
    }else{
      aline.replace('m','.');
    }
    faktor = 1000000.0;
    gefunden = true;
  }
  if(aline.contains('k')){
    if(aline.contains(',') or aline.contains('.')){
      aline.replace('k',' ');
    }else{
      aline.replace('k','.');
    }
    faktor = 1000.0;
    gefunden = true;
  }
  ergebnis = aline.toDouble(&ok);
  if(!ok)ergebnis = 0.0;
  if(gefunden){
    ergebnis *= faktor;
  }
  if(ergebnis < 0.0)ergebnis = 0.0; // negative Zahl verhindern
  if(ergebnis >= 10000000000.0)ergebnis = 0.0; // groesser als 9,999.. GHz verhindern
  return ergebnis;
}

void Nwt7linux::swrantaendern(){
  QString qs;
  double a_100 = 0.0;
  double laenge = 0.0;
  bool ok;

  qs = edita_100->text();
  a_100 = qs.toDouble(&ok);
  if(ok){
    wobdaten.a_100 = a_100;
  }else{
    wobdaten.a_100 = 0.0;
  }
  qs = editkabellaenge->text();
  laenge = qs.toDouble(&ok);
  if(ok){
    wobdaten.kabellaenge = laenge;
  }else{
    wobdaten.kabellaenge = 0.0;
  }
  fsendewobbeldaten();
}

void Nwt7linux::wobnormalisieren(){
  double scantime,  //Zwischenzeit der Wobbelpunkte
         anfang,  //Anfangsfrequenz
         ende,  //Endfrequenz
         schritte, //Anzahl der Schritte
         schritteneu, //Anzahl der Schritte neu
         schrittweite, //Schrittweite
         displayshift, //dB Verschiebnung im Display
         bw=0.0,  //Spektrumanalyser Bandbreitenkontrolle
         frqfaktor, //Faktor fuer Frequenzvervielfachung
         neueschrittweite,
         neueschritte,
         schrittmax = double(maxmesspunkte);

  QString sscantime,
          sanfang,
          sende,
          sschritte,
          sschrittweite,
          sdbshift1,
          sdbshift2,
          sdisplayshift,
          sbw;

  bool ok,
       saschrittkorr,
       sakorrnachoben;

  int button;

  wobbelabbruch = false;
  //qDebug("wobnormalisieren()");
  /////////////////////////////////////////////////////////////////////////////  
  //Werte aus den Editfenster lesen
  //Zwischenzeit in uSek zwischen den Messungen
  /////////////////////////////////////////////////////////////////////////////  
  sscantime = boxzwischenzeit->currentText();
  scantime = sscantime.toDouble(&ok);
  if(ok){scantime = scantime / 10.0;}else{scantime=0.0;}
  sanfang = editanfang->text();
  sende = editende->text();
  anfang = linenormalisieren(sanfang);
  ende = linenormalisieren(sende);
  frqfaktor = double(wobdaten.frqfaktor);
  //qDebug("frqfaktor: %f",frqfaktor);
  //qDebug("Ende  : %f",ende);
  sschritte = editschritte->text();
  sschritte.remove(QChar(' '));
  schritte = sschritte.toDouble(&ok);
  if(!ok)schritte = 1000.0;
  //schrittweite = editschrittweite->text();
  //schrittweite = sschrittweite.toDouble(&ok);
  schrittweite = qRound((ende - anfang)/(schritte-1));
  if(wobdaten.maudio2){
      neueschritte = qRound(ende - anfang + 1.0);
      /*
      if(neueschritte > wgrunddaten.audioschrittkorrgrenze){
        neueschritte = wgrunddaten.audioschrittkorrgrenze;
      }
      */
      neueschrittweite = qRound((ende - anfang)/(neueschritte-1));
      if(neueschrittweite < 1.0)neueschrittweite = 1.0;
      if(neueschrittweite > 1.0){
        schrittweite = qRound((ende - anfang)/(neueschritte-1));
      }else{
        schrittweite = 1.0;
      }
      schritte = neueschritte;
  }
  if(schrittweite == 0.0)schrittweite=1.0;
  //qDebug() << "Schrittweite2:" << schrittweite;
  //qDebug() << "Schritte2:" << schritte;
  switch(wobdaten.ebetriebsart){
    case ewobbeln:
    case eswrneu:
    case eswrant:
    case eimpedanz:
      if(frqfaktor > 1.0){
        //ganzzahliger Wert wird gebraucht
        anfang = round(anfang / frqfaktor) * frqfaktor;
        schrittweite = qRound(schrittweite / frqfaktor) * frqfaktor;
      }
    break;
    case eantdiagramm:
    case espektrumanalyser:
    case espekdisplayshift:
    break;
  }
  //Anzahl der Schritte aendern wenn Frequenzabstand zu klein
  if(schrittweite == 1){
    if(ende - anfang < schritte) schritte = ende - anfang + 1.0;
  }
  //ende noch einmal neu berechnen
  ende = anfang + (schritte - 1.0) * schrittweite;
  //qDebug("anfang %f",anfang);
  //qDebug("ende %f",ende);
  //qDebug("schritte %f",schritte);
  //qDebug("schrittweite %f",schrittweite);
  // ============ Displayshift behandlung
  sdisplayshift = editdisplay->text();
  displayshift = sdisplayshift.toDouble(&ok);;
  if(!ok)displayshift = 0.0;
  displayshift = qRound(displayshift/10)*10;
  sdisplayshift.sprintf("%1.0f",displayshift);
  editdisplay->setText(sdisplayshift);
  wobdaten.displayshift = int(displayshift);
  // ============ Displayshift behandlung
  sdbshift1 = boxdbshift1->currentText();
  sdbshift2 = boxdbshift2->currentText();
  /////////////////////////////////////////////////////////////////////////////  
  //Alle wichtigen werte sind gefuellt ?
  /////////////////////////////////////////////////////////////////////////////  
  if((sanfang.length() > 0) & (sende.length() > 0) & (sschritte.length() > 0)){
    //qDebug("wobnormalisieren() 1");
    ///////////////////////////////////////////////////////////////////////////  
    //Verschiebung auf der Y-Achse einstellen
    ///////////////////////////////////////////////////////////////////////////  
    wobdaten.dbshift1 = sdbshift1.toDouble(&ok);
    if(!ok)wobdaten.dbshift1 = 0.0;
    wobdaten.dbshift2 = sdbshift2.toDouble(&ok);
    if(!ok)wobdaten.dbshift2 = 0.0;
    ///////////////////////////////////////////////////////////////////////////  
    //Korrektur der Schritte
    ///////////////////////////////////////////////////////////////////////////  
    if(schritte < 2.0){
      schritte = 2.0;
      sschritte.sprintf("%1.0f",schritte);
      editschritte->setText(sschritte);
    }
    if(schritte > schrittmax){
      schritte = schrittmax;
      sschritte.sprintf("%1.0f",schritte);
      editschritte->setText(sschritte);
    }
    ///////////////////////////////////////////////////////////////////////////  
    //Korrektur der Frequenzdaten Wobbeln, SA-Bereich 1 und SA-Bereich 2 wenn Ueberschreitung
    ///////////////////////////////////////////////////////////////////////////  
    switch(wobdaten.ebetriebsart){
      case eantdiagramm:
        break;
      case ewobbeln:
      case eswrneu:
      case eswrant:
      case eimpedanz:
        if(ende > (wgrunddaten.maxwobbel * frqfaktor)){
          ende = wgrunddaten.maxwobbel * frqfaktor;
          if(anfang > (wgrunddaten.maxwobbel * frqfaktor)){
            anfang = ende - ((schritte - 1.0) * schrittweite);
          }
        }
        break;
      case espektrumanalyser:
        if(sabereich1->isChecked()){
          wobdaten.psavabs = wgrunddaten.psavabs1;
          if(ende > wgrunddaten.frqb1){
            ende = wgrunddaten.frqb1;
            if(anfang >= ende)anfang = wgrunddaten.frqa1;
          }
        }
        if(sabereich2->isChecked()){
          wobdaten.psavabs = wgrunddaten.psavabs2;
          if(anfang < wgrunddaten.frqa2){
            anfang = wgrunddaten.frqa2;
          }
          if(ende > wgrunddaten.frqb2){
            ende = wgrunddaten.frqb2;
            if(anfang >= ende)anfang = wgrunddaten.frqa2;
          }
        }
        break;
      case espekdisplayshift:
        if(sabereich1->isChecked()){
          wobdaten.psavabs = wgrunddaten.psavabs1 + wgrunddaten.psavabs3;
          //qDebug("anfang %f",anfang);
          //qDebug("ende %f",ende);
          if((anfang + wgrunddaten.frqshift) < wgrunddaten.frqa1){
            anfang = wgrunddaten.frqa1 - wgrunddaten.frqshift;
            //qDebug("anfang1 %f",anfang);
          }
          if((ende + wgrunddaten.frqshift) >= wgrunddaten.frqb1){
            ende = wgrunddaten.frqb1 - wgrunddaten.frqshift;
            //qDebug("ende1 %f",ende);
            if((anfang + wgrunddaten.frqshift) >= ende)
              anfang = wgrunddaten.frqa1 - wgrunddaten.frqshift;
              //qDebug("anfang2 %f",anfang);
          }
        }
        if(sabereich2->isChecked()){
          wobdaten.psavabs = wgrunddaten.psavabs2  + wgrunddaten.psavabs3;
          if((anfang + wgrunddaten.frqshift) < wgrunddaten.frqa2){
            anfang = wgrunddaten.frqa2 - wgrunddaten.frqshift;
          }
          if((ende + wgrunddaten.frqshift) > wgrunddaten.frqb2){
            ende = wgrunddaten.frqb2 - wgrunddaten.frqshift;
            if((anfang + wgrunddaten.frqshift) >= ende)
              anfang = wgrunddaten.frqa2 - wgrunddaten.frqshift;
          }
        }
        break;
    }
    ///////////////////////////////////////////////////////////////////////////  
    //Korrektur von Anfang, Ende bei allen Betriebsarten
    ///////////////////////////////////////////////////////////////////////////  
    if(anfang >= ende){
      ende = anfang + 1.0; //min um 1  erhoehen
    }
    ///////////////////////////////////////////////////////////////////////////  
    //Ueberpruefung und Korrektur der Bandbreite und Schrittweite bei SA 
    //Nur wenn keine Kurvendatei geladen
    ///////////////////////////////////////////////////////////////////////////  
    if(sabw != bwkein and ((wobdaten.ebetriebsart == espektrumanalyser) or (wobdaten.ebetriebsart == espekdisplayshift))){
      saschrittkorr = false; //Schrittanzahl fuer den SA nicht OK
      sakorrnachoben = false; //keine Schrittweite nach oben korrigieren
      // Test ob Schrittweite erhoeht werden muss
      if(sabw == bw30kHz and schrittweite > wgrunddaten.bw30kHz_max){
        saschrittkorr = true; bw = wgrunddaten.bw30kHz_max;
      }
      //if(saschrittkorr){qDebug("saschrittkorr true");}else{qDebug("saschrittkorr false");}
      if(sabw == bw7kHz and schrittweite > wgrunddaten.bw7kHz_max){
        saschrittkorr = true; bw = wgrunddaten.bw7kHz_max;
      }
      //if(saschrittkorr){qDebug("saschrittkorr true");}else{qDebug("saschrittkorr false");}
      if(sabw == bw300Hz and schrittweite > wgrunddaten.bw300_max){
        saschrittkorr = true; bw = wgrunddaten.bw300_max;
      }
      //if(saschrittkorr){qDebug("saschrittkorr true");}else{qDebug("saschrittkorr false");}
      if(!saschrittkorr){
        //Test ob Schrittanzahl verringert werden kann, BW festlegen
        //keine Korr nach oben
        if(sabw == bw30kHz and schrittweite < wgrunddaten.bw30kHz_min){
          saschrittkorr = true; bw = wgrunddaten.bw30kHz_min; sakorrnachoben = true;
        }
        if(sabw == bw7kHz and schrittweite < wgrunddaten.bw7kHz_min){
          saschrittkorr = true; bw = wgrunddaten.bw7kHz_min; sakorrnachoben = true;
        }
        if(sabw == bw300Hz and schrittweite < wgrunddaten.bw300_min){
          saschrittkorr = true; bw = wgrunddaten.bw300_min; sakorrnachoben = true;
        }
      }
      //if(sakorrnachoben){qDebug("sakorrnachoben true");}else{qDebug("sakorrnachoben false");}
      //qDebug("bw %f",bw);
      if(saschrittkorr){
        if(bw < 1000.0){
          sbw.sprintf(" %1.0fHz !!!",bw);
        }else{
          sbw.sprintf(" %1.2fkHz !!!",bw/1000.0);
        }
        //Schrittanzahl mit neuer BW korrigieren
        schritteneu = (ende - anfang) / bw;
        int j = int(schritteneu / 100.0) + 1;
        schritteneu = 100.0 * j + 1.0;
        if(schritteneu > schrittmax)schritteneu = schrittmax;
        if((schritteneu < wgrunddaten.saminschritte) and wgrunddaten.bschrittkorr)schritteneu = wgrunddaten.saminschritte;
        //Die neuen Werte schon mal anzeigen
        schrittweite = (ende - anfang) / (schritteneu - 1.0);
        //sind die Schritte kleiner als 1.0
        if(schrittweite < 1.0){
          schrittweite = 1.0;
        }
        ende = anfang + (schritteneu - 1) * schrittweite;
        setanfang(anfang);
        setende(ende);
        setschritte(schritteneu);
        sschrittweite.sprintf("%1.0f",schrittweite);
        editschrittweite->setText(sschrittweite);
        //Warnung nur bei Obergrenze und Schrittanzahlerhoehung und Warnung ist erlaubt
        if((schritteneu == schrittmax) and wgrunddaten.bwarnsavbwmax and !sakorrnachoben){
          button = QMessageBox::Abort;
          button = QMessageBox::warning(this, tr("SAV-Bandbreitenueberpruefung","InformationsBox Bandbreitenueberpruefung max"), 
                                              tr("Die Schrittweite ist hoeher als","InformationsBox Bandbreitenueberpruefung max")
                                                 + sbw + "<br><br> " +
                                              tr("Die maximale Schrittanzahl ist erreicht und voreingestellt.<br>"
                                                 "<b>Bitte die Scanweite verringern oder die Bandbreite erhoehen!</b>",
                                                 "InformationsBox Bandbreitenueberpruefung max"),
                                                 QMessageBox::Ok, QMessageBox::Abort);
          if(button == QMessageBox::Abort)wobbelabbruch = true;
          schritte = schritteneu;
        }else{
          //Schrittmaximum noch nicht erreicht
          button = QMessageBox::No;
          if(wgrunddaten.bschrittkorr){
            //automatische Schrittkorrektur
            schritte = schritteneu;
          }else{
            //keine automatische Schrittkorrektur
            if(wgrunddaten.bwarnsavbw and !sakorrnachoben){
              //Warnung erlaubt und Schrittweite verringern und korr Schrittanzahl nach oben
              button = QMessageBox::question(this, tr("SAV-Bandbreitenueberpruefung","InformationsBox Bandbreitenueberpruefung"), 
                                                   tr("Die Schrittweite ist hoeher als","InformationsBox Bandbreitenueberpruefung")
                                                   + sbw + "<br><br> " +
                                                   tr("Die neuen Werte sind schon eingetragen. <br>"
                                                      "Sollen die Werten uebernommen werden?","InformationsBox Bandbreitenueberpruefung"),
                                                   QMessageBox::Yes, QMessageBox::No);
            }
            //qDebug("button %x",button);
            if(button == QMessageBox::Yes){
              //Die Werte von der Vorberechnung nehmen und Schritte korrigieren
              schritte = schritteneu;
              //qDebug("button %x",button);
            }
          }
        }
      }
    }
    ///////////////////////////////////////////////////////////////////////////  
    //Korrektur der Schrittweite bei allen Betriebsarten 
    ///////////////////////////////////////////////////////////////////////////  
    switch(wobdaten.ebetriebsart){
      case eantdiagramm:
        break;
      case ewobbeln:
        if(frqfaktor > 1.0){
          anfang = qRound(anfang / frqfaktor) * frqfaktor;
        }
        schrittweite = (ende - anfang)/(schritte - 1.0);
        if(frqfaktor > 1.0){
          schrittweite = qRound(schrittweite / frqfaktor) * frqfaktor;
        }
        break;
      case eswrneu:
      case eswrant:
      case eimpedanz:
      case espektrumanalyser:
      case espekdisplayshift:
        schrittweite = qRound((ende - anfang) / (schritte - 1.0));
        break;
    }
    //sind die Schritte kleiner als 1.0
    if(schrittweite < 1.0){
      if(frqfaktor > 1){
        schrittweite = double(frqfaktor);
      }else{
        schrittweite = 1.0;
      }
    }
    ///////////////////////////////////////////////////////////////////////////  
    //Berechnung der Zwischenzeit von jedem Schritt wenn Scanntime gesetzt wurde
    ///////////////////////////////////////////////////////////////////////////  
    wobdaten.ztime = int(scantime);
    ///////////////////////////////////////////////////////////////////////////  
    //Berechnung der Scanntime zur Anzeige ohne Zwischenzeit
    ///////////////////////////////////////////////////////////////////////////  
    ///////////////////////////////////////////////////////////////////////////  
    //Setzen der Endfrequenz nach allen Berechnungen
    ///////////////////////////////////////////////////////////////////////////  
    ende = anfang + ((schritte-1.0) * schrittweite);
    //wieder alle Werte zurueckschreiben
    setanfang(anfang);
    setende(ende);
    setschritte(schritte);
    sschrittweite.sprintf("%1.0f",schrittweite);
    editschrittweite->setText(sschrittweite);
    //Daten ins Grafikfenster uebermitteln
    ok = true;
    
    if(wobdaten.anfangsfrequenz != anfang)ok = false;
    wobdaten.anfangsfrequenz = anfang;
    if(wobdaten.schritte != int(schritte))ok = false;
    wobdaten.schritte = int(schritte);
    if(wobdaten.schrittfrequenz != schrittweite)ok = false;
    wobdaten.schrittfrequenz = schrittweite;
    if(!ok)wobbelungueltig();
    //Je nach Betriebsart Frequenzeckpunkte merken
    switch(wobdaten.ebetriebsart){
      case eantdiagramm:
        break;
      case espektrumanalyser:
        if(sabereich1->isChecked()){
          wgrunddaten.sa1anfang = anfang;
          wgrunddaten.sa1ende = ende;
          wgrunddaten.sa1schritte = int(schritte);
        }
        if(sabereich2->isChecked()){
          wgrunddaten.sa2anfang = anfang;
          wgrunddaten.sa2ende = ende;
          wgrunddaten.sa2schritte = int(schritte);
        }
        break;
      case espekdisplayshift: 
        if(sabereich1->isChecked()){
          wgrunddaten.sa3anfang = anfang;
          wgrunddaten.sa3ende = ende;
          wgrunddaten.sa3schritte = int(schritte);
        }
        if(sabereich2->isChecked()){
          /*wgrunddaten.sa2anfang = anfang;
          wgrunddaten.sa2ende = ende;
          wgrunddaten.sa2schritte = int(schritte);*/
        }
        break;
      case ewobbeln:
      case eswrneu:
      case eswrant:
      case eimpedanz:
        wgrunddaten.wanfang = anfang;
        wgrunddaten.wende = ende;
        wgrunddaten.wschritte = int(schritte);
        break;
    }
    fsendewobbeldaten();
  }
}

void Nwt7linux::mleditloeschen(){
  QString qs;
  
  QStringList sl = mledit->toPlainText().split("\n", QString::KeepEmptyParts);
  mleditlist.clear();
  infoueber.clear();
  
  for(int i=0; i<sl.count(); i++){
    //qDebug(sl.at(i));
    //if((sl.at(i).indexOf("Kanal") == 0) or (sl.at(i).indexOf("Kursor") == 0) or (sl.at(i).indexOf(" ") == 0))break;
    if(sl.at(i).indexOf(";") == 0)infoueber.append(sl.at(i));
  }
  mleditlist.append(" ");
  if(infoueber.count() == 0){
    infoueber.append(";no_label");
  }
  mledit->clear();
}

void Nwt7linux::mleditinsert(const QString &s){
  int i;
  QString qs;
  
  mleditlist.append(s);
  mledit->clear();
  for(i=0;i < infoueber.count();i++){
    mledit->append(infoueber.at(i));
  }
  for(i=0;i < mleditlist.count();i++){
    mledit->append(mleditlist.at(i));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Funktionen im Messfenster

void Nwt7linux::tabellespeichern(){
  QFile f;
  QString s = QFileDialog::getSaveFileName(this,
                                           tr("NWT Messwerte speichern", "FileDialog"),
                                           kurvendir.filePath("messtab.txt"),
                                           "Textdatei (*.txt)");
  if (!s.isNull())
  {
    //Datei ueberpruefen ob Sufix vorhanden
    if((s.indexOf(".")== -1)) s += ".txt";
    f.setFileName(s);
    if(f.open( QIODevice::WriteOnly )){
      QTextStream ts(&f);
      ts << messedit->document()->toPlainText();
      messedit->document()->setModified(false);
      f.close();
    }
  }
}

void Nwt7linux::tabelleschreiben(){
  btabelleschreiben = true;
}

void Nwt7linux::messsetfont(){
  bool ok;
  
  messfont = QFontDialog::getFont( &ok, messfont, this);
  if(ok){
    messlabel1->setFont(messfont);
    messlabel2->setFont(messfont);
    messlabel3->setFont(messfont);
    messlabel4->setFont(messfont);
    messlabel5->setFont(messfont);
    messlabel6->setFont(messfont);
    lmhz->setFont(messfont);
    lkhz->setFont(messfont);
    lhz->setFont(messfont);
  }
}

//Funktion Befehl zur Messwertausgabe an die Baugruppe senden


void Nwt7linux::setmessen(){
  QString qs;

  if(bttyOpen)defaultlesenrtty();
  if(fwversion == 120){
    messtime = messtimeneu; //ab FW 1.20 die Zwischenzeit fuer
                            //das Wattmetermessen anders setzen
    qs="n";
  }else{
    qs= "m";
  }
  if(!bttyOpen){
    //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
      QMessageBox::warning( this, tr("Wattmeter","InformationsBox"), 
                                     "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","InformationsBox"));
#ifdef LDEBUG
      qDebug("Nwt7linux::setmessen()");
#endif    
      return;
    }
  }
  if (bttyOpen)
  {
    if(fwversion == 120){
      progressbarmessk1->setMaximum(maxmessstep*32);
      progressbarmessk2->setMaximum(maxmessstep*32);
    }else{
      progressbarmessk1->setMaximum(maxmessstep);
      progressbarmessk2->setMaximum(maxmessstep);
    }
    picmodem->writeChar(143);

    picmodem->writeLine(qs);
    messtimer->setSingleShot(true);
    messtimer->start(messtime); //Timer starten
  }  
}

//Nach Timerablauf Messdaten holen

void Nwt7linux::getmessdaten(){
  char abuffer[10];
  unsigned char c1=0, c2=0;
  int i1, i2, a;
  QString qs, qsline;
  double w, wzu;
  double uv;
  double p;
  bool ok;
  int maxcount;
  bool anzeigeneu;

  if(fwversion == 120){
    maxcount = 1;
  }else{
    maxcount = 100 / messtime;
  }
  if(maxcount<1)maxcount=1;
  QWidget *wg = this->currentWidget();
  if(wg == nwt7messen){
  if (bttyOpen){
    a = picmodem->readttybuffer(abuffer, 10);
    //qDebug("a = %i",a);
    if(a < 4){
      messsync++;
      setmessen(); //und Messung neu starten
      if(messsync > 100){
        messtimer->stop();
        QMessageBox::warning( this, tr("Wattmeter","InformationsBox"), 
                                    tr("Es kommen keine Daten vom NWT!","InformationsBox"));
        setCurrentIndex(0);
        return;
      }
    }else{
      messsync=0;
      c1 = abuffer[0];
      c2 = abuffer[1];
      i1 = int(c1 + c2 * 256);
      c1 = abuffer[2];
      c2 = abuffer[3];
      i2 = int(c1 + c2 * 256);
      messcounter++;
      if(fwversion == 120){
        if(i1 < 32*1024)messungk1 += double(i1);
        if(i2 < 32*1024)messungk2 += double(i2);
      }else{
        if(i1 < 1024)messungk1 += double(i1);
        if(i2 < 1024)messungk2 += double(i2);
      }
      anzeigeneu = false;
      //qDebug()<<messcounter;
      //qDebug()<<maxcount;
      //qDebug()<<messtime;
      if(messcounter==maxcount){
        messcounter=0;
        if(!checkboxmesshang1->isChecked()){
          anzeigeneu = true;
        }
        if((messungk1/maxcount) > messungk1alt){
          anzeigeneu = true;
          messungk1alt = messungk1/maxcount;
          counterspitzek1 = 0;
        }else{
          counterspitzek1 ++;
          if(counterspitzek1 > 8){
            anzeigeneu = true;
            counterspitzek1 = 0;
            messungk1alt = 0.0;
          }
        }
        if(anzeigeneu){
        //Kanal 1
          qs.sprintf("%i",int(messungk1/maxcount));
          mlabelk1->setText(qs);
          if(fwversion == 120){
            if((messungk1/maxcount)>maxmessstep*32){
              progressbarmessk1->setValue(maxmessstep*32);
            }else{
              progressbarmessk1->setValue(int(messungk1/maxcount));
            }
          }else{
            if((messungk1/maxcount)>maxmessstep){
              progressbarmessk1->setValue(maxmessstep);
            }else{
              progressbarmessk1->setValue(int(messungk1/maxcount));
            }
          }
        }
        //aus den Eichfaktoren den richtigen Wert errechnen
        w = wobdaten.mfaktor1 * (messungk1/maxcount) + wobdaten.mshift1;
        qs = ldaempfungk1->currentText();
        wzu = qs.toDouble(&ok);
        if(ok)w += wzu;
        //offset ermitten 21.07.12
        setwattoffset1(boxwattoffset1->currentIndex());
        w = w + woffset1;
        qs.sprintf("%2.1f dBm", w);
        if(wgrunddaten.wmprezision == 2)qs.sprintf("%2.2f dBm", w);
        qsline = qs;
        if(anzeigeneu)messlabel1->setText(qs);
        //aus dBm Volt errechnen
        uv = sqrt((pow(10.0 , w/10.0) * 0.001) * 50.0);
        p = uv * uv / 50.0;
        //Messbereche automatisch umschalten
        if (uv > 999){
          qs.sprintf("%3.3f kV", uv/1000);
        }
        if (uv < 1000){
          qs.sprintf("%3.1f V", uv);
        }
        if (uv < 1.0){
          uv = uv * 1000.0;
          qs.sprintf("%3.1f mV", uv);
        }
        if (uv < 1.0){
          uv = uv * 1000.0;
          qs.sprintf("%3.1f uV", uv);
        }
        if(anzeigeneu)messlabel3->setText(qs);
        qsline = qsline + "; " + qs;
        if (p > 999){
          qs.sprintf("%3.3f kW", p/1000);
        }
        if (p < 1000){
          qs.sprintf("%3.1f W", p);
        }
        if (p < 1.0){
          p = p * 1000.0;
          qs.sprintf("%3.1f mW", p);
        }
        if (p < 1.0){
          p = p * 1000.0;
          qs.sprintf("%3.1f uW", p);
        }
        if (p < 1.0){
          p = p * 1000.0;
          qs.sprintf("%3.1f nW", p);
        }
        if (p < 1.0){
          p = p * 1000.0;
          qs.sprintf("%3.1f pW", p);
        }
        if(anzeigeneu)messlabel5->setText(qs);
        qsline = qsline + "; " + qs;
        //Kanal 2 
        if(!wobdaten.einkanalig){ 
          if(!checkboxmesshang2->isChecked()){
            anzeigeneu = true;
          }else{
            anzeigeneu = false;
          }
          if((messungk2/maxcount) > messungk2alt){
            anzeigeneu = true;
            messungk2alt = messungk2/maxcount;
            counterspitzek2 = 0;
          }else{
            counterspitzek2 ++;
            if(counterspitzek2 > 8){
              anzeigeneu = true;
              counterspitzek2 = 0;
              messungk2alt = 0.0;
            }  
          }
          if(anzeigeneu){
            qs.sprintf("%i",int(messungk2/maxcount));
            mlabelk2->setText(qs);
            if(fwversion == 120){
              if((messungk2/maxcount)>maxmessstep*32){
                progressbarmessk2->setValue(maxmessstep*32);
              }else{
                progressbarmessk2->setValue(int(messungk2/maxcount));
              }
            }else{
              if((messungk2/maxcount)>maxmessstep){
                progressbarmessk2->setValue(maxmessstep);
              }else{
                progressbarmessk2->setValue(int(messungk2/maxcount));
              }
            }
          }
          //aus den Eichfaktoren den richtigen Wert errechnen
          w = wobdaten.mfaktor2 * (messungk2/maxcount) + wobdaten.mshift2;
          qs = ldaempfungk2->currentText();
          wzu = qs.toDouble(&ok);
          if(ok)w += wzu;
          //offset ermitten 21.07.12
          setwattoffset2(boxwattoffset2->currentIndex());
          w = w + woffset2;
          if(wobdaten.maudio2){
            qs.sprintf("%2.2f dBV", w);
          }else{
            qs.sprintf("%2.1f dBm", w);
            if(wgrunddaten.wmprezision == 2)qs.sprintf("%2.2f dBm", w);
          }
          if(anzeigeneu)messlabel2->setText(qs);
          qsline = qsline + "\t" + qs;
          //aus dBm Volt errechnen
          if(wobdaten.maudio2){
            uv = pow(10.0, w/20);
          }else{
            uv = sqrt((pow(10.0 , w/10.0) * 0.001) * 50.0);
            p = uv * uv / 50.0;
          }
          //Messbereche automatisch umschalten
          if (uv > 999){
            qs.sprintf("%3.3f kV", uv/1000);
          }
          if (uv < 1000){
            qs.sprintf("%3.1f V", uv);
          }
          if (uv < 1.0){
            uv = uv * 1000.0;
            qs.sprintf("%3.2f mV", uv);
          }
          if (uv < 1.0){
            uv = uv * 1000.0;
            qs.sprintf("%3.2f uV", uv);
          }
          if(anzeigeneu)messlabel4->setText(qs);
          qsline = qsline + "; " + qs;
          if(wobdaten.maudio2){
            qs= "        ";
          }else{
            if (p > 999){
              qs.sprintf("%3.3f kW", p/1000);
            }
            if (p < 1000){
              qs.sprintf("%3.1f W", p);
            }
            if (p < 1.0){
              p = p * 1000.0;
              qs.sprintf("%3.1f mW", p);
            }
            if (p < 1.0){
              p = p * 1000.0;
              qs.sprintf("%3.1f uW", p);
            }
            if (p < 1.0){
              p = p * 1000.0;
              qs.sprintf("%3.1f nW", p);
            }
            if (p < 1.0){
              p = p * 1000.0;
              qs.sprintf("%3.1f pW", p);
            }
          }
          if(anzeigeneu)messlabel6->setText(qs);
          qsline = qsline + "; " + qs;
        }
        if (bmkalibrierenk1_20db){
          bmkalibrierenk1_20db = false;
          ym_40db = messungk1/maxcount;

          //qDebug("ym_0db:%f",ym_0db);
          //qDebug("ym_40db:%f",ym_40db);
          //qDebug("ym_daempf:%f",ym_daempf);
          //qDebug("ym_gen:%f",ym_gen);
          //jetzt wobdaten.mfaktor1 und wobdaten.mshift1 ausrechnen
          wobdaten.mfaktor1 = ym_daempf/(ym_0db - ym_40db);
          wobdaten.mshift1 = ((ym_0db * wobdaten.mfaktor1) * -1.0) + ym_gen;
          //qDebug("wobdaten.mfaktor1:%f",wobdaten.mfaktor1);
          //qDebug("wobdaten.mshift1:%f",wobdaten.mshift1);
          //qDebug("Kanal 1: faktor=%f shift=%f", wobdaten.mfaktor1, wobdaten.mshift1);
          if(QMessageBox::question(this, tr("Kalibrierung speichern? -- NWT","InformationsBox"), 
                                          tr("<B>Speichern der Werte in der Messsondendatei</B><BR>"
                                            "Die ermittelten Werte sofort abspeichern?","InformationsBox"),
                                          tr("&Ja","InformationsBox"), tr("&Nein","InformationsBox"),
                                          QString::null, 0, 1) == 0){
              messsondespeichern1(filenamesonde1);
          }else{
            QMessageBox::warning( this, tr("Speicher der Ergebnisse","InformationsBox"),
                                        tr("<B>Speichern der Werte in der Messsondendatei</B><BR>"
                                            "Die ermittelten Daten koennen unter<BR>"
                                            "(Wobbeln) (Eigenschaften Messsonde Kanal 1 speichern)<BR>"
                                            "abgespeichert werden !","InformationsBox" ));
          }
        }
        if (bmkalibrierenk1_0db){
          bmkalibrierenk1_0db = false;
          ym_0db = messungk1/maxcount;
          QString beschr(tr("<b>Kalibrieren Kanal 1</b><br>"
                          "1. Den Pegel mit einem Daempfungsglied verringern.<br>"
                          "   Das Daempfungsglied sollte min. 20dB betragen.<br>"
                          "   <br><br>"
                          "Werte des Daempfungsgliedes (dB):","InformationsBox"));
          ym_daempf = QInputDialog::getDouble(this, tr("Kalibrieren Kanal 1","InputBox"),
                                                    beschr, 20.0, 20, 90, 2, &ok);
          //qDebug("ym_daempfung:%f",ym_daempf);
          if(ok){
            bmkalibrierenk1_20db = true;
            if(wgrunddaten.bdaempfungfa){
              setDaempfung(9); // -20dB
              daempfung1->setCurrentIndex(9);
              daempfung2->setCurrentIndex(9);
              daempfung3->setCurrentIndex(9);
            }else{
              setDaempfung(2); // -20dB
              daempfung1->setCurrentIndex(2);
              daempfung2->setCurrentIndex(2);
              daempfung3->setCurrentIndex(2);
            }
          }
        }
        if (bmkalibrierenk2_20db){
          bmkalibrierenk2_20db = false;
          ym_40db = messungk2/maxcount;
          //jetzt wobdaten.mfaktor2 und wobdaten.mshift2 ausrechnen
          wobdaten.mfaktor2 = ym_daempf/(ym_0db - ym_40db);
          wobdaten.mshift2 = ((ym_0db * wobdaten.mfaktor2) * -1.0) + ym_gen;
          //          qDebug("Kanal 2: faktor=%f shift=%f", wobdaten.mfaktor2, wobdaten.mshift2);
          if(QMessageBox::question(this, tr("Kalibrierung speichern? -- NWT","InputBox"), 
                                          tr("<B>Speichern der Werte in der Messsondendatei</B><BR>"
                                            "Die ermittelten Werte sofort abspeichern?","InputBox"),
                                    tr("&Ja","InputBox"), tr("&Nein","InputBox"),
                                    QString::null, 0, 1) == 0){
              messsondespeichern2(filenamesonde2);
          }else{
            QMessageBox::warning( this, tr("Speicher der Ergebnisse","InformationsBox"),
                                        tr("<B>Speichern der Werte in der Messsondendatei</B><BR>"
                                            "Die ermittelten Daten koennen unter<BR>"
                                            "(Wobbeln) (Eigenschaften Messsonde Kanal 2 speichern)<BR>"
                                            "abgespeichert werden !","InformationsBox" ));
          }
        }
        if (bmkalibrierenk2_0db){
          bmkalibrierenk2_0db = false;
          ym_0db = messungk2/maxcount;
          QString beschr(tr("<b>Kalibrieren Kanal 2</b><br>"
                          "1. Den Pegel mit einem Daempfungsglied verringern.<br>"
                          "   Das Daempfungsglied sollte min. 20dB betragen.<br>"
                          "   <br><br>"
                          "Werte des Daempfungsgliedes (dB):","InputBox"));
          ym_daempf = QInputDialog::getDouble(this, tr("Kalibrieren Kanal 2","InputBox"),
                                                    beschr, 20.0, 20, 90, 2, &ok);
          if(ok)bmkalibrierenk2_20db = true;
        }
        if(btabelleschreiben){
          btabelleschreiben = false;
          messedit->moveCursor(QTextCursor::End);
          messedit->insertPlainText(qsline);
          messedit->moveCursor(QTextCursor::End);
        }
        messungk1 = 0.0;
        messungk2 = 0.0;
      }
      if(bmessen){
        setmessen(); //und Messung neu starten
      }
    }
  }
  }
}

void Nwt7linux::mkalibrierenk1(){
  bool ok;
  ym_gen = 4.25;
  QString beschr(tr("<b>Kalibrieren Kanal 1</b><br>"
                 "1. Einen bekannten HF-Pegel an den Messeingang anlegen.<br>"
                 "   Das kann auch der HF-Ausgang des Netzwerktesters sein.<br>"
                 "   Beim NWT01 betraegt der Ausgangspegel bei 1MHz +4,25dBm.<br>"
                 "   Dieser Wert wird als Default unten eingeblendet.<br>"
                 "<br>"
                 "2. Im Ablauf der Kalibrieren wird ein genau bekanntes Daempfungsglied<br>"
                 "   eingeschleift. Das Daempfungsglied sollte min. 20dB betragen.<br>"
                 "   Aus diesen beiden Messergebnissen werden die Funktionskonstanden <br>"
                 "   errechnet die anschliessend in der Messsondendatei abgespeichert <br>"
                 "   werden muessen.<br>"
                 "<br>"
                 "<br>"
                 "Pegel des HF-Generators (dBm):","InputBox"));
  ym_gen = QInputDialog::getDouble(this, tr("Kalibrieren Kanal 1","InputBox"),
                                           beschr, ym_gen, -20, 10, 2, &ok); 
  if(ok)bmkalibrierenk1_0db = true;
  ym_0db = 0.0;
  ym_40db = 0.0;
  setDaempfung(0); // 0 dB
  daempfung1->setCurrentIndex(0);
  daempfung2->setCurrentIndex(0);
  daempfung3->setCurrentIndex(0);
}

void Nwt7linux::mkalibrierenk2(){
  bool ok;
  ym_gen = 4.25;
  QString beschr(tr("<b>Kalibrieren Kanal 2</b><br>"
                 "1. Einen bekannten HF-Pegel an den Messeingang anlegen.<br>"
                 "   Das kann auch der HF-Ausgang des Netzwerktesters sein.<br>"
                 "   Beim NWT01 betraegt der Ausgangspegel bei 1MHz +4,25dBm.<br>"
                 "   Dieser Wert wird als Default unten eingeblendet.<br>"
                 "<br>"
                 "2. Im Ablauf der Kalibrierung wird ein genau bekanntes Daempfungsglied<br>"
                 "   eingeschleift. Das Daempfungsglied sollte min. 20dB betragen.<br>"
                 "   Aus diesen beiden Messergebnissen werden die Funktionskonstanden <br>"
                 "   errechnet die anschliessend in der Messsondendatei abgespeichert <br>"
                 "   werden muessen.<br>"
                 "<br>"
                 "<br>"
                 "Pegel des HF-Generators (dBm):","InputBox"));
  ym_gen = QInputDialog::getDouble(this, tr("Kalibrieren Kanal 2","InputBox"),
                                           beschr, ym_gen, -20, 10, 2, &ok); 
  if(ok)bmkalibrierenk2_0db = true;
  ym_0db = 0.0;
  ym_40db = 0.0;
}


///////////////////////////////////////////////////////////////////////////////
// Funktionen im Hauptfenster

void Nwt7linux::defaultlesenrtty(){
  char abuffer[(maxmesspunkte * 4)];
  int a;

  if (bttyOpen){
    a = picmodem->readttybuffer(abuffer, (maxmesspunkte * 4));
  }
  
  (void) a;
}

/*
void Nwt7linux::resizeEvent( QResizeEvent * ){
  //nach Groessenaenderung grafische Elemente anpassen
  qDebug("Nwt7linux::resizeEvent");
  qDebug("breite %i hoehe %i", width(), height());
  nwt7wobbeln->resize(width(), height());
  grafiksetzen();
}
*/

void Nwt7linux::tabumschalten( QWidget *wg ){
  if(wg != wdiagramm){
    setBetriebsart(betriebsart->currentIndex());
  }
  if(wg == nwt7wobbeln){
    diagrammstop();
    emit setmenu(emmenuKurven, true);
    emit setmenu(emmenuwobbel, true);
//    emit setmenu(emmenuvfo, false);
    emit setmenu(emmenumessen, false);
    emit setmenu(emDrucken, true);
    emit setmenu(emDruckenpdf, true);
    bmessen = false;
    idletimer->start(wgrunddaten.idletime);
  }
  if(wg == nwt7vfo){
    diagrammstop();
    emit setmenu(emmenuKurven, false);
    emit setmenu(emmenuwobbel, false);
    //    emit setmenu(emmenuvfo, true);
    emit setmenu(emmenumessen, false);
    emit setmenu(emDrucken, false);
    emit setmenu(emDruckenpdf, false);
    setVfo();
    //und die Drehgeber setzen
    setdrehgebervonlcd();
    bmessen = false;
    //    Dial1hz->setFocus();
    idletimer->stop();
    stimer->stop();
    vtimer->stop();
  }
  if(wg == nwt7messen){
    diagrammstop();
    emit setmenu(emmenuKurven, false);
    emit setmenu(emmenuwobbel, false);
    //    emit setmenu(emmenuvfo, false);
    emit setmenu(emmenumessen, true);
    emit setmenu(emDrucken, false);
    emit setmenu(emDruckenpdf, false);
    //    defaultlesenrtty(); //eventuelle Daten im UART loeschen
    idletimer->stop();
    stimer->stop();
    vtimer->stop();
    if(bmessvfo)setmessvfo(); 
    setmessen();
    bmessen = true;
  }
  if(wg == berechnung){
    bmessen = false;
    diagrammstop();
    idletimer->start(wgrunddaten.idletime);
  }
  if(wg == wkmanager){
    bmessen = false;
    diagrammstop();
    idletimer->start(wgrunddaten.idletime);
  }
  if(wg == wimpedanz){
    bmessen = false;
    diagrammstop();
    idletimer->start(wgrunddaten.idletime);
  }
  if(wg == wdiagramm){
    bmessen = false;
    //qDebug()<< "Diagramm";
    wobdaten.ebetriebsart = eantdiagramm;
    idletimer->start(wgrunddaten.idletime);
    //wobdaten.mess.daten_enable = false;
    diagrammdim(0);
    emit sendewobbeldaten(wobdaten);
  }
}

void Nwt7linux::grafiksetzen(){

  ///////////////////////////////////////////////
  // zuerst den VFO bereich

  nwt7vfo->setGeometry(0,0,5000,5000);
  editvfo->setGeometry(30,10,150,30);
  labelvfo->setGeometry(190,10,90,30);
  LCD1->setGeometry(30,50,200,45);
  rb1->setGeometry(240,50,20,40);
  LCD2->setGeometry(30,100,200,45);
  rb2->setGeometry(240,100,20,40);
  LCD3->setGeometry(30,150,200,45);
  rb3->setGeometry(240,150,20,40);
  LCD4->setGeometry(30,200,200,45);
  rb4->setGeometry(240,200,20,40);
  LCD5->setGeometry(30,250,200,45);
  rb5->setGeometry(240,250,20,40);
  editzf->setGeometry(60,300,100,20);
  checkboxzf->setGeometry(500,15,250,20);
  checkboxiqvfo->setGeometry(500,35,250,20);
  labelzf->setGeometry(30,300,25,20);
  labelhz->setGeometry(170,300,25,20);
  daempfung2->setGeometry(250,14,60,20);
  labeldaempfung1->setGeometry(320,14,170,20);
  
  int vpos1 = 320;
  int vpos2 = vpos1+20;
  int vpos3 = vpos1+40;
  int vab = 40;
  int hpos1 = 265;
  
  labelfrqfaktorv->setGeometry(vpos1+20,hpos1-80,190,30);

  lmhz->setGeometry(vpos1+20,hpos1-40,90,30);
  lkhz->setGeometry(vpos2+30+3*vab,hpos1-40,90,30);
  lhz->setGeometry(vpos3+30+6*vab,hpos1-40,90,30);
  
  
  vsp1ghz->setGeometry(vpos1-40,hpos1,40,30);
  
  vsp100mhz->setGeometry(vpos1,hpos1,40,30);
  vsp10mhz->setGeometry(vpos1+vab,hpos1,40,30);
  vsp1mhz->setGeometry(vpos1+2*vab,hpos1,40,30);
  
  vsp100khz->setGeometry(vpos2+3*vab,hpos1,40,30);
  vsp10khz->setGeometry(vpos2+4*vab,hpos1,40,30);
  vsp1khz->setGeometry(vpos2+5*vab,hpos1,40,30);
  
  vsp100hz->setGeometry(vpos3+6*vab,hpos1,40,30);
  vsp10hz->setGeometry(vpos3+7*vab,hpos1,40,30);
  vsp1hz->setGeometry(vpos3+8*vab,hpos1,40,30);

  //wobbelfenster
  // grafik hat jetzt ein eigenes Window
  //  grafik->setGeometry(0, 0, wgrafik->width(), wgrafik->height());

  int gbreite, ghoehe;
  int spalte1;
  int spalte2;
  int spalte3;
  int abstand = 21;
  int i = 0;
  
  gbreite = width();
  ghoehe = height() - 50;
  
  if(gbreite < 750){
    spalte1 = 500;
    spalte2 = 0;
    spalte3 = 255;
  }else{
    spalte1 = 10;
    spalte2 = 250;
    spalte3 = 505;
  }
  
  mledit->setGeometry(spalte1, 10, 250, ghoehe);
 
  groupwobbel->setGeometry(spalte2+20, 10, 220, 185);
  editanfang->setGeometry(100, 15 + i*abstand,100,20);
  labelanfang->setGeometry(5, 17 + i*abstand,90,20);
  labelanfang->setAlignment(Qt::AlignRight);
  i++;
  editende->setGeometry(100, 15 + i*abstand,100,20);
  labelende->setGeometry(5, 17 + i*abstand, 90, 20);
  labelende->setAlignment(Qt::AlignRight);
  i++;
  editschrittweite->setGeometry(120, 15 + i*abstand,80,20);
  labelschrittweite->setGeometry(5, 17 + i*abstand, 110, 20);
  labelschrittweite->setAlignment(Qt::AlignRight);
  i++;
  editschritte->setGeometry(120, 15 + i*abstand,80,20);
  labelschritte->setGeometry(5, 17 + i*abstand, 110, 20);
  labelschritte->setAlignment(Qt::AlignRight);
  i++;
  boxzwischenzeit->setGeometry(120, 15 + i*abstand,80,20);
  labelzwischenzeit->setGeometry(5, 17 + i*abstand, 110, 20);
  labelzwischenzeit->setAlignment(Qt::AlignRight);
//  checkboxtime->setGeometry(5, 15 + i*abstand, 150, 20);
  i++;
  editdisplay->setGeometry(120, 15 + i*abstand,80,20);
  labeldisplayshift->setGeometry(5, 17 + i*abstand, 110, 20);
  labeldisplayshift->setAlignment(Qt::AlignRight);
  i++;
  boxprofil->setGeometry(50, 15 + i*abstand,150,20);
  labelprofil->setGeometry(5, 17 + i*abstand, 40, 20);
  labelprofil->setAlignment(Qt::AlignRight);
  i++;
  labelfrqfaktor->setGeometry(5, 17 + i*abstand, 190, 20);
  labelfrqfaktor->setAlignment(Qt::AlignRight);
  
  groupdaempfung->setGeometry(spalte2+20, 200, 220, 40);
  daempfung1->setGeometry(120, 15,80,20);
  labeldaempfung->setGeometry(5, 17, 100, 20);
  labeldaempfung->setAlignment(Qt::AlignRight);
    
  groupbetriebsart->setGeometry(spalte2+20, 245, 220, 105);
  i=0;
  betriebsart->setGeometry(80, 15 + i*abstand, 130, 20);
  labelbetriebsart->setGeometry(5, 18 + i*abstand,70, 20);
  labelbetriebsart->setAlignment(Qt::AlignRight);
  i++;
  kalibrier1->setGeometry(5, 18 + i*abstand, 200, 20);
  kalibrier1->setLayoutDirection(Qt::RightToLeft);
  aufloesung->setGeometry(100, 15 + i*abstand, 110, 20);
  labelaufloesung->setGeometry(5, 18 + i*abstand, 90, 20);
  labelaufloesung->setAlignment(Qt::AlignRight);
  
  groupsa->setGeometry(30, 15 + i*abstand, 180, 60);
  labelsafrqbereich->setGeometry(10, 15, 160, 20);
  sabereich1->setGeometry(10, 15, 160, 20);
  labelsabandbreite->setGeometry(10, 35, 160, 20);
  sabereich2->setGeometry(10, 35, 160, 20);
  i++;
  kalibrier2->setGeometry(5, 18 + i*abstand, 200, 20);
  kalibrier2->setLayoutDirection(Qt::RightToLeft);
  labela_100->setGeometry(5, 18 + i*abstand, 110, 20);
  edita_100->setGeometry(120, 15 + i*abstand, 90, 20);
  labela_100->setAlignment(Qt::AlignRight);
  i++;
  labelkabellaenge->setGeometry(5, 18 + i*abstand, 110, 20);
  editkabellaenge->setGeometry(120, 15 + i*abstand, 90, 20);
  labelkabellaenge->setAlignment(Qt::AlignRight);
  
  //  labelprogressbar->setAlignment(Qt::AlignRight);
  

  buttonwobbeln->setGeometry(spalte3+160, 10, 70, 30);
  buttoneinmal->setGeometry(spalte3+160, 45, 70, 30);
  buttonstop->setGeometry(spalte3+160, 80, 70, 30);
  
  i = 0;
  groupbandbreite->setGeometry(spalte3+10, 10, 140, 100);
  checkbox3db->setGeometry(5, 15, 130, 20);
  checkbox3db->setLayoutDirection(Qt::RightToLeft);
  checkbox6db->setGeometry(5, 35, 130, 20);
  checkbox6db->setLayoutDirection(Qt::RightToLeft);
  checkboxgrafik->setGeometry(5, 55, 130, 20);
  checkboxgrafik->setLayoutDirection(Qt::RightToLeft);
  checkboxinvers->setGeometry(5, 75, 130, 20);
  checkboxinvers->setLayoutDirection(Qt::RightToLeft);
  
  groupzoom->setGeometry(spalte3+10, 111,220,48);
  labellupe->setGeometry(5, 24, 95, 20);
  labellupe->setAlignment(Qt::AlignRight);
  buttonlupeplus->setGeometry(110, 15, 30, 30);
  buttonlupeminus->setGeometry(140, 15, 30, 30);
  buttonlupemitte->setGeometry(170, 15, 30, 30);
  
  groupkanal->setGeometry(spalte3+10, 160, 220, 58);
  checkboxk1->setGeometry(5, 15,200,20);
  checkboxk1->setLayoutDirection(Qt::RightToLeft);
  checkboxk2->setGeometry(5, 35,200,20);
  checkboxk2->setLayoutDirection(Qt::RightToLeft);
  
  groupshift->setGeometry(spalte3+10, 218, 220, 132);
  labelboxydbmax->setGeometry(5, 31, 50, 20);
  labelboxydbmax->setAlignment(Qt::AlignLeft);
  labelboxydbmin->setGeometry(5, 56, 50, 20);
  labelboxydbmin->setAlignment(Qt::AlignLeft);
  boxydbmax->setGeometry(65, 30, 55, 20);
  boxydbmin->setGeometry(65, 55, 55, 20);
  labelboxdbshift1->setGeometry(127, 31, 40, 20);
  labelboxdbshift1->setAlignment(Qt::AlignRight);
  labelboxdbshift2->setGeometry(127, 56, 40, 20);
  labelboxdbshift2->setAlignment(Qt::AlignRight);
  boxdbshift1->setGeometry(170, 30, 45, 20);
  boxdbshift2->setGeometry(170, 55, 45, 20);
  
  //  groupkursor->setGeometry(spalte3+10, 276, 220, 38);
  labelkursornr->setGeometry(5, 81, 150, 20);
  labelkursornr->setAlignment(Qt::AlignRight);
  boxkursornr->setGeometry(170, 80, 45, 20);

  //  groupbar->setGeometry(spalte3+10, 314, 220, 35);
  progressbar->setGeometry(160, 109, 56, 10);
  labelnwt->setGeometry(5, 106, 40, 20);
  labelnwt->setAlignment(Qt::AlignRight);
  labelprogressbar->setGeometry(45, 106, 100, 20);
  labelprogressbar->setAlignment(Qt::AlignRight);

  //messfenster
  progressbarmessk1->setGeometry(10,60,600,20);
  progressbarmessk2->setGeometry(10,160,600,20);
  
  boxwattoffset1->setGeometry(10,81,180,20);
  labelldaempfungk1->setGeometry(220,81,110,20);
  ldaempfungk1->setGeometry(330,81,100,20);
  checkboxmesshang1->setGeometry(450,81,100,20);
  
  boxwattoffset2->setGeometry(10,181,180,20);
  labelldaempfungk2->setGeometry(220,181,110,20);
  ldaempfungk2->setGeometry(330,181,100,20);
  checkboxmesshang2->setGeometry(450,181,100,20);
  
  mlabelk1->setGeometry(610,60,40,20);
  mlabelk2->setGeometry(610,160,40,20);
  messlabel1->setGeometry(10,10,185,50);
  messlabel2->setGeometry(10,110,185,50);
  messlabel3->setGeometry(200,10,185,50);
  messlabel4->setGeometry(200,110,185,50);
  messlabel5->setGeometry(390,10,185,50);
  messlabel6->setGeometry(390,110,185,50);
  messedit->setGeometry(10,220,600,ghoehe-210);
  
  buttonmesssave->setGeometry(620,220,100,35);
  buttonmess->setGeometry(620,260,100,35);
  buttonvfo->setGeometry(620,300,100,35);
  
//  labelspghz->setGeometry(30,240,90,30);
  sp1ghz->setGeometry(10,270,40,30);
  
  labelfrqfaktorm->setGeometry(70,210,190,30);
  
  labelspmhz->setGeometry(70,240,90,30);
  sp100mhz->setGeometry(50,270,40,30);
  sp10mhz->setGeometry(90,270,40,30);
  sp1mhz->setGeometry(130,270,40,30);
  
  labelspkhz->setGeometry(220,240,90,30);
  sp100khz->setGeometry(190,270,40,30);
  sp10khz->setGeometry(230,270,40,30);
  sp1khz->setGeometry(270,270,40,30);
  
  labelsphz->setGeometry(370,240,90,30);
  sp100hz->setGeometry(330,270,40,30);
  sp10hz->setGeometry(370,270,40,30);
  sp1hz->setGeometry(410,270,40,30);
  
  daempfung3->setGeometry(460,270,60,20);
  labeldaempfung3->setGeometry(460,240,170,20);

  //berechnungsfenster
  groupschwingkreisc->setGeometry(20,20,250,110);
  editf2->setGeometry(20, 20, 100, 20);
  labelf2->setGeometry(130, 20, 90, 20);
  editl2->setGeometry(20, 40, 100, 20);
  labell2->setGeometry(130, 40, 90, 20);
  ergebnisc->setGeometry(25, 60, 95, 20);
  labelc2->setGeometry(130, 60, 90, 20);
  ergebnisxc->setGeometry(25, 80, 95, 20);
  labelxc->setGeometry(130, 80, 90, 20);
  
  groupschwingkreisl->setGeometry(20,160,250,150);
  editf1->setGeometry(20, 20, 100, 20);
  labelf1->setGeometry(130, 20, 90, 20);
  editc1->setGeometry(20, 40, 100, 20);
  labelc1->setGeometry(130, 40, 90, 20);
  ergebnisl->setGeometry(25, 60, 95, 20);
  labell1->setGeometry(130, 60, 90, 20);
  editn->setGeometry(20,100, 100, 20);
  labeln->setGeometry(130, 100, 90, 20);
  ergebnisal->setGeometry(25, 120, 95, 20);
  labelal->setGeometry(130, 120, 90, 20);
  ergebnisxl->setGeometry(25, 80, 95, 20);
  labelxl->setGeometry(130, 80, 90, 20);
    
  groupwindungen->setGeometry(290,20,250,110);
  edital3->setGeometry(20, 20, 100, 20);
  labeledital3->setGeometry(130, 20, 90, 20);
  editl3->setGeometry(20, 40, 100, 20);
  labeleditl3->setGeometry(130, 40, 90, 20);
  ergebnisw->setGeometry(25, 60, 95, 20);
  labelergebnisw->setGeometry(130, 60, 90, 20);

  gimp->setGeometry(20,20,250,50);
  rbr->setGeometry(20,20,100,20);
  rblc->setGeometry(120,20,100,20);
  
  bild1->setGeometry(280,110,444,156);
  gzr->setGeometry(20,100,250,220);
  editz1->setGeometry(20, 20, 50, 20);
  labeleditz1->setGeometry(90, 20, 90, 20);
  editz2->setGeometry(20, 40, 50, 20);
  labeleditz2->setGeometry(90, 40, 90, 20);
  editz3->setGeometry(20, 60, 50, 20);
  labeleditz3->setGeometry(90, 60, 90, 20);
  editz4->setGeometry(20, 80, 50, 20);
  labeleditz4->setGeometry(90, 80, 90, 20);
  lr1->setGeometry(20, 100, 50, 20);
  lbeschrr1->setGeometry(90, 100, 90, 20);
  lr2->setGeometry(20, 120, 50, 20);
  lbeschrr2->setGeometry(90, 120, 90, 20);
  lr3->setGeometry(20, 140, 50, 20);
  lbeschrr3->setGeometry(90, 140, 90, 20);
  lr4->setGeometry(20, 160, 50, 20);
  lbeschrr4->setGeometry(90, 160, 90, 20);
  ldaempfung->setGeometry(20, 180, 50, 20);
  lbeschrdaempfung->setGeometry(90, 180, 160, 20);

  bild2->setGeometry(280,110,444,209);
  gzlc->setGeometry(20,100,250,220);
  editzlc1->setGeometry(20, 20, 50, 20);
  labeleditzlc1->setGeometry(90, 20, 90, 20);
  editzlc2->setGeometry(20, 40, 50, 20);
  labeleditzlc2->setGeometry(90, 40, 90, 20);
  editzlc3->setGeometry(20, 60, 50, 20);
  labeleditzlc3->setGeometry(90, 60, 90, 20);
  ll->setGeometry(20, 80, 50, 20);
  lbeschrl->setGeometry(90, 80, 90, 20);
  lc->setGeometry(20, 100, 50, 20);
  lbeschrc->setGeometry(90, 100, 90, 20);

  gdiagramm->setGeometry(10, 10, 160, 200);
  bstart->setGeometry(10, 20, 100, 20);
  bstop->setGeometry(10, 50, 100, 20);
  bspeichern->setGeometry(10, 80, 100, 20);
  lanzeige->setGeometry(10, 110, 200, 20);
  bsim->setGeometry(10, 170, 100, 20);

  gdiagrammdim->setGeometry(180, 10, 230, 150);
  ldbmax->setGeometry(10, 20, 50, 20);
  boxdbmax->setGeometry(80, 20, 80, 20);
  ldbmin->setGeometry(10, 40, 50, 20);
  boxdbmin->setGeometry(80, 40, 80, 20);
  lbegin->setGeometry(10, 70, 60, 20);
  spinadbegin->setGeometry(80, 70, 50, 20);
  spingradbegin->setGeometry(130, 70, 50, 20);
  lbegin1->setGeometry(180, 70, 50, 20);
  lend->setGeometry(10, 100, 60, 20);
  spinadend->setGeometry(80, 100, 50, 20);
  spingradend->setGeometry(130, 100, 50, 20);
  lend1->setGeometry(180, 100, 50, 20);

}

///////////////////////////////////////////////////////////////////////////////
// Widget VFO
void Nwt7linux::vfoedit(){
  double l;
  QString s;
  QString s1;
  
  s = editvfo->text();
  l = linenormalisieren(s);
  //Frequenzen zwischen 10 Hz und 200 MHz
  if(l > 10.0 && l < 9999999999.0){
    //Drehgeber auslesen
    s1.sprintf("%4.6f", l / 1000000.0);
    //auf Display darstellen
    switch(lcdauswahl)
    {
      case 1:
        LCD1->display(s1);
        lcdfrq1 = l;
        break;
      case 2:
        LCD2->display(s1);
        lcdfrq2 = l;
        break;
      case 3:
        LCD3->display(s1);
        lcdfrq3 = l;
        break;
      case 4:
        LCD4->display(s1);
        lcdfrq4 = l;
        break;
      case 5:
        LCD5->display(s1);
        lcdfrq5 = l;
        break;
    }
    setdrehgebervonlcd();
  }
}

void Nwt7linux::LCDaendern(){
  double l;
  QString s;

  //Drehgeber auslesen
  //Werte zusammenfuegen
  l = getdrehgeber();
  // komma einfuegen nach Mhz
  s.sprintf("%4.6f", l / 1000000.0);
  //auf Display darstellen
  switch(lcdauswahl)
  {
    case 1:
      LCD1->display(s);
      lcdfrq1 = l;
      break;
    case 2:
      LCD2->display(s);
      lcdfrq2 = l;
      break;
    case 3:
      LCD3->display(s);
      lcdfrq3 = l;
      break;
    case 4:
      LCD4->display(s);
      lcdfrq4 = l;
      break;
    case 5:
      LCD5->display(s);
      lcdfrq5 = l;
      break;
  }
  s.sprintf("%3.0f", l);
  editvfo->setText(s);
}

void Nwt7linux::setVfo(){
  //timer starten mit 10 mSek Laufzeit
  vfotimer->start(10);
}

void Nwt7linux::senddaten(){
  if(!bersterstart){
    QString qs;
    char s[20];
    double fr = 1000000.0;
    double zf = 0.0;
    //nach ablauf des vfoTimers wird Frequenz zum NWT gesendet
    if(!bttyOpen){
      //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
      if(!bttyOpen){
        QMessageBox::warning( this, tr("Serielle Schnittstelle"), 
                                      "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!"));
        #ifdef LDEBUG
        qDebug("Nwt7linux::senddaten()");
        #endif    
        return;
      }
    }
    if (bttyOpen)
    {
      switch(lcdauswahl)
      {
        case 1:fr = lcdfrq1;break;
        case 2:fr = lcdfrq2;break;
        case 3:fr = lcdfrq3;break;
        case 4:fr = lcdfrq4;break;
        case 5:fr = lcdfrq5;break;
      }
      //Kommadarstellung wieder wegrechnen
      //ZF auslesen mit Abkurzungen
      qs = editzf->text();
      zf = linenormalisieren(qs);
      qs.sprintf("%1.0f", zf);
      editzf->setText(qs);
      //in Hz umrechnen
      //fr *= 1000000.0;
      //Frequenzfaktor einrechnen
      fr = qRound(fr / double(wobdaten.frqfaktor));
      //ZF addieren
      fr += zf;
      // I/Q Frequenz * 4 
      if(checkboxiqvfo->isChecked())fr = fr * 4.0;
      if((fr < 1000000000.0) and fr >= 0.0){
        sprintf(s, "f%09.0f", fr);
        //qDebug(s);
        defaultlesenrtty(); //eventuelle Daten im UART loeschen
        picmodem->writeChar(143);
        picmodem->writeLine(s);
      }
    }
  }
  //timer stoppen da Aufgabe erledigt
  vfotimer->stop();
}


double Nwt7linux::getdrehgeber()
{
  double wert = double(vsp1hz->value()) + 
                double(vsp10hz->value())   * 10.0 +
                double(vsp100hz->value())  * 100.0 +
                double(vsp1khz->value())   * 1000.0 +
                double(vsp10khz->value())  * 10000.0 +
                double(vsp100khz->value()) * 100000.0 +
                double(vsp1mhz->value())   * 1000000.0 +
                double(vsp10mhz->value())  * 10000000.0 +
                double(vsp100mhz->value()) * 100000000.0 +
                double(vsp1ghz->value())   * 1000000000.0;
  return wert;  
}

void Nwt7linux::setdrehgebervonlcd()
{
  double fr = 10000000.0;

  switch(lcdauswahl)
  {
    case 1:
      fr = lcdfrq1;
      break;
    case 2:
      fr = lcdfrq2;
      break;
    case 3:
      fr = lcdfrq3;
      break;
    case 4:
      fr = lcdfrq4;
      break;
    case 5:
      fr = lcdfrq5;
      break;
  }
  setdrehgeber(fr);
}

void Nwt7linux::change1hz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp1hz->setValue(0);
    int b = vsp10hz->value();
    b++;
    vsp10hz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(vsp10mhz->value() != 0)isnull = false;
    if(vsp1mhz->value() != 0)isnull = false;
    if(vsp100khz->value() != 0)isnull = false;
    if(vsp10khz->value() != 0)isnull = false;
    if(vsp1khz->value() != 0)isnull = false;
    if(vsp100hz->value() != 0)isnull = false;
    if(vsp10hz->value() != 0)isnull = false;
    if(isnull){
      vsp1hz->setValue(0);
    }else{
      vsp1hz->setValue(9);
      int b = vsp10hz->value();
      b--;
      vsp10hz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change10hz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp10hz->setValue(0);
    int b = vsp100hz->value();
    b++;
    vsp100hz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(vsp10mhz->value() != 0)isnull = false;
    if(vsp1mhz->value() != 0)isnull = false;
    if(vsp100khz->value() != 0)isnull = false;
    if(vsp10khz->value() != 0)isnull = false;
    if(vsp1khz->value() != 0)isnull = false;
    if(vsp100hz->value() != 0)isnull = false;
    if(isnull){
      vsp10hz->setValue(0);
    }else{
      vsp10hz->setValue(9);
      int b = vsp100hz->value();
      b--;
      vsp100hz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change100hz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp100hz->setValue(0);
    int b = vsp1khz->value();
    b++;
    vsp1khz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(vsp10mhz->value() != 0)isnull = false;
    if(vsp1mhz->value() != 0)isnull = false;
    if(vsp100khz->value() != 0)isnull = false;
    if(vsp10khz->value() != 0)isnull = false;
    if(vsp1khz->value() != 0)isnull = false;
    if(isnull){
      vsp100hz->setValue(0);
    }else{
      vsp100hz->setValue(9);
      int b = vsp1khz->value();
      b--;
      vsp1khz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change1khz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp1khz->setValue(0);
    int b = vsp10khz->value();
    b++;
    vsp10khz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(vsp10mhz->value() != 0)isnull = false;
    if(vsp1mhz->value() != 0)isnull = false;
    if(vsp100khz->value() != 0)isnull = false;
    if(vsp10khz->value() != 0)isnull = false;
    if(isnull){
      vsp1khz->setValue(0);
    }else{
      vsp1khz->setValue(9);
      int b = vsp10khz->value();
      b--;
      vsp10khz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change10khz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp10khz->setValue(0);
    int b = vsp100khz->value();
    b++;
    vsp100khz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(vsp10mhz->value() != 0)isnull = false;
    if(vsp1mhz->value() != 0)isnull = false;
    if(vsp100khz->value() != 0)isnull = false;
    if(isnull){
      vsp10khz->setValue(0);
    }else{
      vsp10khz->setValue(9);
      int b = vsp100khz->value();
      b--;
      vsp100khz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change100khz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp100khz->setValue(0);
    int b = vsp1mhz->value();
    b++;
    vsp1mhz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(vsp10mhz->value() != 0)isnull = false;
    if(vsp1mhz->value() != 0)isnull = false;
    if(isnull){
      vsp100khz->setValue(0);
    }else{
      vsp100khz->setValue(9);
      int b = vsp1mhz->value();
      b--;
      vsp1mhz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change1mhz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp1mhz->setValue(0);
    int b = vsp10mhz->value();
    b++;
    vsp10mhz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(vsp10mhz->value() != 0)isnull = false;
    if(isnull){
      vsp1mhz->setValue(0);
    }else{
      vsp1mhz->setValue(9);
      int b = vsp10mhz->value();
      b--;
      vsp10mhz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change10mhz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp10mhz->setValue(0);
    int b = vsp100mhz->value();
    b++;
    vsp100mhz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(vsp100mhz->value() != 0)isnull = false;
    if(isnull){
      vsp10mhz->setValue(0);
    }else{
      vsp10mhz->setValue(9);
      int b = vsp100mhz->value();
      b--;
      vsp100mhz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change100mhz(int a)
{
  bool isnull = true;
  
  if(a==10){
    vsp100mhz->setValue(0);
    int b = vsp1ghz->value();
    b++;
    vsp1ghz->setValue(b);
  }
  if(a==-1){
    if(vsp1ghz->value() != 0)isnull = false;
    if(isnull){
      vsp100mhz->setValue(0);
    }else{
      vsp100mhz->setValue(9);
      int b = vsp1ghz->value();
      b--;
      vsp1ghz->setValue(b);
    }
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::change1ghz(int a)
{
  if(a==10){
    vsp1ghz->setValue(9);
  }
  if(a==-1){
    vsp1ghz->setValue(0);
  }
  LCDaendern();
  setVfo();
}

void Nwt7linux::setdrehgeber(double wert)
{
  double fr;
  int a;
  
  fr = wert;
  a = int(fr/1000000000.0);
  vsp1ghz->setValue(a);
  fr = fr - double(a) * 1000000000.0;
  a = int(fr/100000000.0);
  vsp100mhz->setValue(a);
  fr = fr - double(a) * 100000000.0;
  a = int(fr/10000000.0);
  vsp10mhz->setValue(a);
  fr = fr - double(a) * 10000000.0;
  a = int(fr/1000000.0);
  vsp1mhz->setValue(a);
  fr = fr - double(a) * 1000000.0;
  a = int(fr/100000.0);
  vsp100khz->setValue(a);
  fr = fr - double(a) * 100000.0;
  a = int(fr/10000.0);
  vsp10khz->setValue(a);
  fr = fr - double(a) * 10000.0;
  a = int(fr/1000.0);
  vsp1khz->setValue(a);
  fr = fr - double(a) * 1000.0;
  a = int(fr/100.0);
  vsp100hz->setValue(a);
  fr = fr - double(a) * 100.0;
  a = int(fr/10.0);
  vsp10hz->setValue(a);
  fr = fr - double(a) * 10.0;
  vsp1hz->setValue(int(fr));
}

void Nwt7linux::lcd1clicked(){
  QPalette dp;
  dp.setColor(QPalette::Background, Qt::yellow);
  QPalette dpd;
  dpd.setColor(QPalette::Background, Qt::darkYellow);

  rb1->setChecked(true);
  rb2->setChecked(false);
  rb3->setChecked(false);
  rb4->setChecked(false);
  rb5->setChecked(false);
  
  LCD1->setPalette(dp);
  LCD2->setPalette(dpd);
  LCD3->setPalette(dpd);
  LCD4->setPalette(dpd);
  LCD5->setPalette(dpd);

  //  Dial1hz->setFocus();
  lcdauswahl = 1;
  setdrehgebervonlcd();
}

void Nwt7linux::lcd2clicked(){
  QPalette dp;
  dp.setColor(QPalette::Background, Qt::yellow);
  QPalette dpd;
  dpd.setColor(QPalette::Background, Qt::darkYellow);

  rb1->setChecked(false);
  rb2->setChecked(true);
  rb3->setChecked(false);
  rb4->setChecked(false);
  rb5->setChecked(false);
  
  LCD1->setPalette(dpd);
  LCD2->setPalette(dp);
  LCD3->setPalette(dpd);
  LCD4->setPalette(dpd);
  LCD5->setPalette(dpd);

//  Dial1hz->setFocus();
  lcdauswahl = 2;
  setdrehgebervonlcd();
}

void Nwt7linux::lcd3clicked(){
  QPalette dp;
  dp.setColor(QPalette::Background, Qt::yellow);
  QPalette dpd;
  dpd.setColor(QPalette::Background, Qt::darkYellow);

  rb1->setChecked(false);
  rb2->setChecked(false);
  rb3->setChecked(true);
  rb4->setChecked(false);
  rb5->setChecked(false);
  
  LCD1->setPalette(dpd);
  LCD2->setPalette(dpd);
  LCD3->setPalette(dp);
  LCD4->setPalette(dpd);
  LCD5->setPalette(dpd);

  //  Dial1hz->setFocus();
  lcdauswahl = 3;
  setdrehgebervonlcd();
}

void Nwt7linux::lcd4clicked(){
  QPalette dp;
  dp.setColor(QPalette::Background, Qt::yellow);
  QPalette dpd;
  dpd.setColor(QPalette::Background, Qt::darkYellow);

  rb1->setChecked(false);
  rb2->setChecked(false);
  rb3->setChecked(false);
  rb4->setChecked(true);
  rb5->setChecked(false);
  
  LCD1->setPalette(dpd);
  LCD2->setPalette(dpd);
  LCD3->setPalette(dpd);
  LCD4->setPalette(dp);
  LCD5->setPalette(dpd);

//  Dial1hz->setFocus();
  lcdauswahl = 4;
  setdrehgebervonlcd();
}

void Nwt7linux::lcd5clicked(){
  QPalette dp;
  dp.setColor(QPalette::Background, Qt::yellow);
  QPalette dpd;
  dpd.setColor(QPalette::Background, Qt::darkYellow);

  rb1->setChecked(false);
  rb2->setChecked(false);
  rb3->setChecked(false);
  rb4->setChecked(false);
  rb5->setChecked(true);
  
  LCD1->setPalette(dpd);
  LCD2->setPalette(dpd);
  LCD3->setPalette(dpd);
  LCD4->setPalette(dpd);
  LCD5->setPalette(dp);

  //  Dial1hz->setFocus();
  lcdauswahl = 5;
  setdrehgebervonlcd();
}
/*
void Nwt7linux::changedrehgeber(){
  double fr = 1000000.0, fr1;

  switch(lcdauswahl)
  {
    case 1:fr = LCD1->value();break;
    case 2:fr = LCD2->value();break;
    case 3:fr = LCD3->value();break;
    case 4:fr = LCD4->value();break;
    case 5:fr = LCD5->value();break;
  }
  //Kommadarstellung wieder wegrechnen
  fr = fr * 1000000.0; //Darstellung in Herz
  fr1 = floor(fr / 1000000.0);
  Dial1mhz->setValue(int(fr1));
  fr = fr - fr1 * 1000000.0;
  fr1 = floor(fr / 1000.0);
  Dial1khz->setValue(int(fr1));
  fr = fr - fr1 * 1000.0;
  Dial1hz->setValue(int(fr));
}
*/
void Nwt7linux::setFrequenzmarken(){
  int r;

  fmdlg *dlg = new fmdlg(this);
  //Frequenzmarken in den Dialog
  dlg->setdaten(frqmarken);
  QFont font("Helvetica", wgrunddaten.pfsize);
  dlg->setFont(font);
  r = dlg->exec();
  if(r == QDialog::Accepted){
    //    qDebug("ok");
    //Frequenzmarken aus den Dialog holen
    frqmarken = dlg->getdaten();
    //Frequenzmarken ins Wobbelfenster 
    emit sendefrqmarken(frqmarken);
  }
  delete dlg;
}

void Nwt7linux::setHLine(){
  bool ok;
  wobdaten.hline = QInputDialog::getDouble(this, "dB-Linie", "Eingabe in dB", wobdaten.hline, -120.0, 50.0, 2, &ok); 
  emit sendewobbeldaten(wobdaten);
}

void Nwt7linux::editProfile(){
  int i, j, r;
  TProfil cprofil;
  
  profildlg *dlg = new profildlg(this);
  //Frequenzmarken in den Dialog
  QFont font("Helvetica", wgrunddaten.pfsize);
  dlg->setFont(font);
  dlg->tip(btip);
  j = boxprofil->count();
  if(j>1){
    for(i=0;i<j-1;i++){
      cprofil.anfang = aprofil[i+1].anfang;
      cprofil.ende = aprofil[i+1].ende;
      cprofil.schritte = aprofil[i+1].schritte;
      cprofil.beschr = aprofil[i+1].name;
      //qDebug("S " + cprofil.beschr);
      dlg->setprofil(cprofil,i);
    }
  }
  r = dlg->exec();
  if(r == QDialog::Accepted){
    // alle Profile ausser default loeschen
    for(i=1; i<profilanz; i++){
      //qDebug(aprofil[i].name);
      aprofil[i].name = "NN";
    }
    for(i=0;i<profilanz;i++){
      cprofil = dlg->getprofil(i);
      aprofil[i+1].anfang = cprofil.anfang;
      aprofil[i+1].ende = cprofil.ende;
      aprofil[i+1].name = cprofil.beschr;
      aprofil[i+1].schritte = cprofil.schritte;
      //qDebug("G " + cprofil.beschr);
    }
    boxprofil->clear();
    TFprofil bprofil[profilanz+1];
    j=0;
    for(i=0; i<=profilanz; i++){
      bprofil[i].name="NN";
    }
    for(i=0; i<=profilanz; i++){
      if(aprofil[i].name != "NN"){
        bprofil[j]=aprofil[i];
        j++;
      }
    }
    for(i=0; i<=profilanz; i++){
      aprofil[i]=bprofil[i];
    }
    for(i=0; i<=profilanz; i++){
      //qDebug(aprofil[i].name);
      if(aprofil[i].name != "NN")boxprofil->addItem(aprofil[i].name);
    }
  }
  delete dlg;
}

void Nwt7linux::option(){
  int r;
  double addstakt;
  int apll;
  QString stty;
  double dkonst;
  long double ddskonst;
  qulonglong test;
  QString qs;
  QString qpll;
  
  addstakt = wgrunddaten.ddstakt;
  apll = wgrunddaten.pll;
  odlg *dlg = new odlg(this);
  dlg->psetfontsize(wgrunddaten.pfsize);
  //wgrunddaten.kalibrierk1 = wobdaten.bkalibrierkorrk1;
  //wgrunddaten.kalibrierk2 = wobdaten.bkalibrierkorrk2;
  wgrunddaten.einkanalig = wobdaten.einkanalig;
  wgrunddaten.strsonde1 = wobdaten.namesonde1;
  wgrunddaten.strsonde2 = wobdaten.namesonde2;
  wgrunddaten.bswriteration = wobdaten.bswriteration;
  wgrunddaten.penkanal1 = wobdaten.penkanal1;
  wgrunddaten.penkanal2 = wobdaten.penkanal2;
  wgrunddaten.colorhintergrund = wobdaten.colorhintergrund;
  wgrunddaten.psav300 = wobdaten.psav300;
  wgrunddaten.psav7k = wobdaten.psav7k;
  wgrunddaten.psav30k = wobdaten.psav30k;
  wgrunddaten.bsavdbm = wobdaten.bsavdbm;
  wgrunddaten.bsavuv = wobdaten.bsavuv;
  wgrunddaten.bsavwatt = wobdaten.bsavwatt;
  wgrunddaten.bsavdbuv = wobdaten.bsavdbuv;
  wgrunddaten.safehlermax = wobdaten.safehlermax;
  wgrunddaten.safehlermin = wobdaten.safehlermin;
  wgrunddaten.frqfaktor = wobdaten.frqfaktor;
  wgrunddaten.penwidth = wobdaten.penwidth;
  stty = wgrunddaten.str_tty;
  dlg->setdaten(wgrunddaten);
  dlg->setsavcal(wobdaten.savcalarray);
  dlg->setkalibrierarray(wobdaten.kalibrierarray);
  dlg->tip(btip);
  r = dlg->exec();
  if(r == QDialog::Accepted){
    wgrunddaten = dlg->getdaten();
    setDaempfungItem(); //Daempfungsglied einstellen
    messtimer->stop();
    if(wgrunddaten.kalibrierende > long(wgrunddaten.maxwobbel)){
        QMessageBox::warning( this, tr("Frequenzgrenzen","InformationsBox"), 
                                    tr("<center><b>Die Endfrequenz der Kalibrierkorrektur ist"
                                       " groesser als die max. Wobbelfrequenz!</b></center><br>"
                                       "<center>Die Endfrequenz der Kalibrierkorrektur wird angepasst.</center>","InformationsBox"));
      wgrunddaten.kalibrierende = long(wgrunddaten.maxwobbel);
    }
    if(wobdaten.ebetriebsart == espektrumanalyser){
      setBetriebsart(4);
    }
    if(wobdaten.ebetriebsart == espekdisplayshift){
      setBetriebsart(5);
    }
    //wobdaten.bkalibrierkorrk1 = wgrunddaten.kalibrierk1;
    //wobdaten.bkalibrierkorrk2 = wgrunddaten.kalibrierk2;
    wobdaten.einkanalig = wgrunddaten.einkanalig;
    wobdaten.namesonde1 = wgrunddaten.strsonde1;
    wobdaten.namesonde2 = wgrunddaten.strsonde2;
    wobdaten.bswriteration = wgrunddaten.bswriteration;
    wobdaten.penkanal1 = wgrunddaten.penkanal1;
    wobdaten.penkanal2 = wgrunddaten.penkanal2;
    wobdaten.penwidth = wgrunddaten.penwidth;
    wobdaten.penkanal1.setWidth(wobdaten.penwidth);
    wobdaten.penkanal2.setWidth(wobdaten.penwidth);
    wobdaten.colorhintergrund = wgrunddaten.colorhintergrund;
    wobdaten.psav300 = wgrunddaten.psav300;
    wobdaten.psav7k = wgrunddaten.psav7k;
    wobdaten.psav30k = wgrunddaten.psav30k;
    wobdaten.bsavdbm = wgrunddaten.bsavdbm;
    wobdaten.bsavuv = wgrunddaten.bsavuv;
    wobdaten.bsavwatt = wgrunddaten.bsavwatt;
    wobdaten.bsavdbuv = wgrunddaten.bsavdbuv;
    wobdaten.safehlermax = wgrunddaten.safehlermax;
    wobdaten.safehlermin = wgrunddaten.safehlermin;
    wobdaten.frqfaktor = wgrunddaten.frqfaktor;
    
    setFrqFaktorLabel();
    wobdaten.savcalarray = dlg->getsavcal();
    emit sendewobbeldaten(wobdaten);
    qs = "sav.cal";
    QFile f;
    int i;
    f.setFileName(homedir.filePath(qs));
    if(f.open(QIODevice::WriteOnly)){
      #ifdef LDEBUG
        qDebug(s);
      #endif
      QTextStream ts(&f);
      //zuerst die Versionsnummer der SAV-Kalibrierung
      ts << "#V4.04" << endl;
      for(i=0; i< calmaxindex; i++){
        ts << wobdaten.savcalarray.p[i] << endl;
      }
      f.close();
    }
    //    Dial1mhz->setMaximum(int(wgrunddaten.maxvfo / 1000000.0)-1);
    if(wobdaten.einkanalig){
      if(wgrunddaten.kalibrierk1 and !wobdaten.linear1){
        setColorRedk1();
        wgrunddaten.kalibrierstep = maxmesspunkte;
      }else{
        setColorBlackk1();
      }  
      checkboxk2->setChecked(false);
      checkboxk2->setEnabled(false);
      progressbarmessk2->setValue(0);
      progressbarmessk2->setEnabled(false);
      ldaempfungk2->setEnabled(false);
      boxwattoffset2->setEnabled(false);
      messlabel2->setText("");
      messlabel2->setEnabled(false);
      messlabel4->setText("");
      messlabel4->setEnabled(false);
      messlabel6->setText("");
      messlabel6->setEnabled(false);
      mlabelk2->setText("");
      mlabelk2->setEnabled(false);
      labelldaempfungk2->setText("");
      checkboxmesshang2->setEnabled(false);
//      checkboxM10dbK2->setEnabled(false);
//      checkboxM20dbK2->setEnabled(false);
      emit setmenu(emEichenK2, false);
      emit setmenu(emSondeLadenK2, false);
      emit setmenu(emSondeSpeichernK2, false);
      emit setmenu(emWattEichenK2, false);
      emit setmenu(emWattEditSonde2, false);
    }else{
      if(wgrunddaten.kalibrierk1 and !wobdaten.linear1){
        setColorRedk1();
        wgrunddaten.kalibrierstep = maxmesspunkte;
      }else{
        setColorBlackk1();
      }
      if(wgrunddaten.kalibrierk2 and !wobdaten.linear2){
        setColorRedk2();
        wgrunddaten.kalibrierstep = maxmesspunkte;
      }else{
        setColorBlackk2();
      }
      //Aenderung nur erlauben wenn Betriebsart Wobbeln, sonst wird Eichen K2 frei obwohl SWV eingestellt ist
      if(wobdaten.ebetriebsart == ewobbeln){
        checkboxk2->setEnabled(true);
        progressbarmessk2->setEnabled(true);
        ldaempfungk2->setEnabled(true);
        labelldaempfungk2->setText(tr("Daempfung (dB)","im Wattmeter"));
        checkboxmesshang2->setEnabled(true);
        boxwattoffset2->setEnabled(true);
        messlabel2->setEnabled(true);
        messlabel4->setEnabled(true);
        messlabel6->setEnabled(true);
        mlabelk2->setEnabled(true);
        //      checkboxM10dbK2->setEnabled(true);
        //      checkboxM20dbK2->setEnabled(true);
        emit setmenu(emEichenK2, true);
        emit setmenu(emSondeLadenK2, true);
        emit setmenu(emSondeSpeichernK2, true);
        emit setmenu(emWattEichenK2, true);
        emit setmenu(emWattEditSonde2, true);
      }
    }
    //    QFont font("Helvetica", weichen.pfsize);
    emit setFontSize(wgrunddaten.pfsize);
    fsendewobbeldaten();
  }
  if((addstakt != wgrunddaten.ddstakt) or (apll != wgrunddaten.pll)){
    switch(wgrunddaten.pll){
      case 1: qpll="00"; break; 
      case 2: qpll="17"; break; 
      case 3: qpll="1F"; break; 
      case 4: qpll="27"; break; 
      case 5: qpll="2F"; break; 
      case 6: qpll="37"; break; 
      case 7: qpll="3F"; break; 
      case 8: qpll="47"; break; 
      case 9: qpll="4F"; break; 
      case 10: qpll="57"; break; 
      case 11: qpll="5F"; break; 
      case 12: qpll="67"; break; 
      case 13: qpll="6F"; break; 
      case 14: qpll="77"; break; 
      case 15: qpll="7F"; break; 
      case 16: qpll="87"; break; 
      case 17: qpll="8F"; break; 
      case 18: qpll="97"; break; 
      case 19: qpll="9F"; break; 
      case 20: qpll="A7"; break; 
    }
    dkonst = 4294967296.0;  // 2 hoch 32
    ddskonst = dkonst * dkonst / wgrunddaten.ddstakt;
    test = qulonglong(ddskonst);
    qs = QString("%1").arg(test,0,16);
    if(qs.length() < 10)qs="0"+qs;
    #ifdef LDEBUG
    qDebug("%f", wgrunddaten.ddstakt);
    qDebug("%llf", ddskonst);
    qDebug("%lli", test);
    qDebug(qs);
    #endif    
    qs = qs.toUpper();
    qs = "e" + qs + qpll;
    #ifdef LDEBUG
    qDebug(qs);
    return;
    #endif    
    if(!bttyOpen){
      //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
      if(!bttyOpen){
        QMessageBox::warning( this, tr("Serielle Schnittstelle","InformationsBox"), 
	                               "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","InformationsBox"));
        return;
      }
    }
    if (bttyOpen)
    {
      defaultlesenrtty(); //eventuelle Daten im UART loeschen
      picmodem->writeChar(143);
      picmodem->writeLine(qs);
    }
  }
  if(stty != wgrunddaten.str_tty){
    picmodem->closetty();
    bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
      QMessageBox::warning( this, tr("Serielle Schnittstelle","InformationsBox"), 
                                     "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","InformationsBox"));
    }
  }
  vfotimer->stop();
  messvfotimer->stop();
  // VFO Tab aktiv ?
  if(this->currentIndex() == 2)vfotimer->start(10);
  // Wattmeter VFO aktiv ?
  if(this->currentIndex() == 3){
    this->setCurrentIndex(0);
  }
  delete dlg;
}

void Nwt7linux::setAText(QString stty){
  QString qs;
  if(fwversion > 0){
    qs.sprintf("FW:%01i.%02i:V%02i",fwversion/100,fwversion-100,fwvariante);
  }else{
    qs = "FW:0.00";
  }
  QString astty = qs + " - " + configfilename + " - " + stty;
  emit setAppText(astty);
}

void Nwt7linux::versionskontrolle(){
  QString qs;
  
  //qDebug("versionskontrolle()");
  if(vabfrage){
    vabfrage = false;
    if (bttyOpen)
    {
      defaultlesenrtty(); //eventuelle Daten im UART loeschen
      picmodem->writeChar(143);
      picmodem->writeLine("v");
      vtimer->start(wgrunddaten.vtime);
      //qDebug("v --> rs232");
    }
  }else{  
    vabfrage = true;
    if (bttyOpen and (fwversion > 113) and (fwversion < 150))
    {
      defaultlesenrtty(); //eventuelle Daten im UART loeschen
      picmodem->writeChar(143);
      picmodem->writeLine("s");
      stimer->start(wgrunddaten.stime);
      //qDebug("s --> rs232");
    }
  }
}

void Nwt7linux::version(){
  int a=0, i=0;
  QString qs;

  if (bttyOpen)
  {
    defaultlesenrtty(); //eventuelle Daten im UART loeschen
    picmodem->writeChar(143);
    picmodem->writeLine("v");
    while((picmodem->readttybuffer(&a, 1)) != 1){
      i++;
      if(i==20000){
        a=0;
      break;
      }
    };
    #ifdef LDEBUG
    qDebug("%i",a);
    #endif    
    fwversion = a;
    qs = tr("<B>Abfrage der Firmware im PIC</B><BR>"
            "Das Ergebnis muss groesser 100 sein.<BR><BR>"
            "Version: ");
    QString as;
    as.sprintf("%03i",a);
    QMessageBox::about(this, "Firmware", qs+as);
  }
}

void Nwt7linux::firmupdate(){
  char c;
  QFile f;
  int i;
  QString qs, qs1, zeile;
  QString beschr(tr("<b>Ablauf des Firmwareupdate</b><br>"
                    "1. Netzwerktester ausschalten<br>"
                    "2. Bruecke zwischen GND und RB0 stecken<br>"
                    "   Der Netzwerktester bleibt <B><EM>ausgeschaltet</EM></B><BR>"
                    "3. Laden der HEX-Datei fuer den PIC<BR><BR>"
                    "   <B>Weiter Infos folgen!!!</B>","InformationsBox"));
  QString beschr1(tr("<b>Weiterer Ablauf des Firmwareupdate</b><br>"
                     "1. Nach druecken des <EM><B>OK</B></EM>-Button den Netzwerktester einschalten<br>"
                     "2. Nach dem Erkennen der Anfangskennung beginnt das Update<br>"
                     "Funktioniert das nicht, den Netzwerktester ausschalten und wieder von vorn beginnen<BR>"
                     "<B>Durch das stecken der Bruecke + PowerON ist es unbedingt notwendig eine neue Firmware"
                     " in den PIC zu laden!!!</B>","InformationsBox"));
  firmwidget firmw(this);
 
  QMessageBox::about(this, tr(" Firmware Update ","InformationsBox"), beschr);
  QString s = QFileDialog::getOpenFileName(this,
                                           "Firmware Updaten",
                                           homedir.filePath("*.hex"),
                                           "Firmware Hexdatei (*.hex)");
  if (!s.isNull())
  {
    firmw.show();
    QMessageBox::about(this, tr(" Firmware Update "), beschr1); 
    firmw.setText("HEX-Datei "+s);
    #ifdef LDEBUG
    qDebug(s);
    #endif    
    f.setFileName(s);
    if(f.open( QIODevice::ReadOnly )){
      idletimer->stop();
      stimer->stop();
      vtimer->stop();
      QTextStream ts(&f);
      qs = "";
      while((qs.indexOf("HFM9") == -1)and(qs.indexOf("NWT7") == -1)){
        if(picmodem->readttybuffer(&c, 1) == 1){
          firmw.setText("Kennung RX"); 
          qs = qs + c;
          //qDebug(qs);
        }
      }
      qs = "";
      i = 0;
      while(qs.indexOf("OK") == -1){
        ts >> zeile;
        #ifdef LDEBUG
        qDebug(zeile);
        #endif	
        picmodem->writeLine(zeile);
        c = ' ';
        while(picmodem->readttybuffer(&c, 1) != 1) ;
        i++;
        qs1.sprintf(qPrintable(tr("Line: %i")), i);
        firmw.setText(qs1);
        if(c == 'O'){
          qs = qs + c;
          while(picmodem->readttybuffer(&c, 1) != 1) ;
        }
        qs = qs + c;
        #ifdef LDEBUG
        qDebug(qs);
        #endif	
      }
      idletimer->start(wgrunddaten.idletime);
    }
    QMessageBox::about(this, tr("Firmware Update","InformationsBox"), 
                             tr("<CENTER><B>Update erfolgreich!</B></CENTER><BR>"
                                "Die Bruecke wieder entfernen!","InformationsBox"));
  }
}

void Nwt7linux::setDaempfungWob(int index)
{
  //qDebug("Nwt7linux::setDaempfungWob");
  daempfung2->setCurrentIndex(index);
  daempfung3->setCurrentIndex(index);
  setDaempfung(index);
}

void Nwt7linux::setDaempfungVfo(int index)
{
  //qDebug("Nwt7linux::setDaempfungVfo");
  daempfung1->setCurrentIndex(index);
  daempfung3->setCurrentIndex(index);
  setDaempfung(index);
}

void Nwt7linux::setDaempfungWatt(int index)
{
  //qDebug("Nwt7linux::setDaempfungWatt");
  daempfung1->setCurrentIndex(index);
  daempfung2->setCurrentIndex(index);
  setDaempfung(index);
}

void Nwt7linux::setDaempfung(int index)
{
  bool bidletimer = false;
  bool bstimer = false;
  bool bvtimer = false;
  bool bmesstimer = false;
  int portb=0;
  int db;
  bool r1 = false, // -2dB
       r2 = false, // -4dB
       r3 = false, // -8db
       r4 = false, // -4dB
       r5 = false, // -16dB
       r6 = false; // -32db
  QString qs;
  //es wird nicht gewobbelt
  
  //qDebug("Nwt7linux::setDaempfung");

  if(idletimer->timerId() != -1){
    idletimer->stop();
    bidletimer = true;
  }
  if(stimer->timerId() != -1){
    stimer->stop();
    bstimer = true;
  }
  if(vtimer->timerId() != -1){
    vtimer->stop();
    bvtimer = true;
  }
  if(messtimer->timerId() != -1){
    messtimer->stop();
    bmesstimer = true;
  }
  if(wgrunddaten.bdaempfungfa){
    if(index > 0){
      db = (index+1) * 2;
      //qDebug("db: %i", db);
      if(db >= 32){
        r6 = true;
        db = db - 32;
        if((db !=0) and (db < 4)){
          r6 = false;
          db = db + 32;
        } 
        //qDebug("db: %i", db);
      } 
      if(db >= 16){
        r5 = true;
        db = db - 16;
        if((db !=0) and (db < 4)){
          r5 = false;
          db = db + 16;
        } 
        //qDebug("db: %i", db);
      } 
      if(db != 0){
        db = db - 4;
        r4 = true;
        if(db >= 8){
          r3 = true;
          db = db - 8;
          //qDebug("db: %i", db);
        } 
        if(db >= 4){
          r2 = true;
          db = db - 4;
          //qDebug("db: %i", db);
        } 
        if(db >= 2){
          r1 = true;
          db = db - 2;
          //qDebug("db: %i", db);
        } 
      }
    }
  portb = 0;
  if(r1){
    //qDebug("r1 ein -2");
    portb = portb + 1;
  }
  if(r2){
    //qDebug("r2 ein -4");
    portb = portb + 2;
  }
  if(r3){
    //qDebug("r3 ein -8");
    portb = portb + 4;
  }
  if(r4){
    //qDebug("r4 ein -4");
    portb = portb + 8;
  }
  if(r5){
    //qDebug("r5 ein -16");
    portb = portb + 16;
  }
  if(r6){
    //qDebug("r6 ein -32");
    portb = portb + 32;
  }

  }else{
    switch(index){
      case 0:
          portb = 0;
          break;
      case 1:
          portb = 1;
          break;
      case 2:
          portb = 2;
          break;
      case 3:
          portb = 3;
          break;
      case 4:
          portb = 6;
          break;
      case 5:
          portb = 7;
          break;
    }
  }
  //qDebug("index  : %i", index);
  //qDebug("portb  : %x hex", portb);
  //qDebug("fwportb: %x hex", fwportb >> 1);
  //Sofort Systemwert richtig einstellen
  fwportb = portb;
  //es wird nicht gewobbelt
  if(!bdauerwobbeln){
    if(bttyOpen){
      picmodem->writeChar(0x8F);
      picmodem->writeChar('r');
      picmodem->writeChar(portb);
      #ifdef LDEBUG
      qs.sprintf("r%i",portb); 
      qDebug(qs);
      #endif    
    }
  }else{
    bsetdaempfung = true;
  }
  if(bidletimer)idletimer->start(wgrunddaten.idletime);
  if(bstimer)stimer->start(wgrunddaten.stime);
  if(bvtimer)vtimer->start(wgrunddaten.vtime);
  if(bmesstimer and bmessen){
    messtimer->setSingleShot(false);
    messtimer->start(messtime);
  }
}

void Nwt7linux::setwattoffset1(int index)
{
  //  qDebug("%i",index);
  switch(index){
  case 0: woffset1 = wattoffsetk1[0].offsetwatt; break;
  case 1: woffset1 = wattoffsetk1[1].offsetwatt; break;
  case 2: woffset1 = wattoffsetk1[2].offsetwatt; break;
  case 3: woffset1 = wattoffsetk1[3].offsetwatt; break;
  case 4: woffset1 = wattoffsetk1[4].offsetwatt; break;
  case 5: woffset1 = wattoffsetk1[5].offsetwatt; break;
  case 6: woffset1 = wattoffsetk1[6].offsetwatt; break;
  case 7: woffset1 = wattoffsetk1[7].offsetwatt; break;
  case 8: woffset1 = wattoffsetk1[8].offsetwatt; break;
  case 9: woffset1 = wattoffsetk1[9].offsetwatt; break;
  case 10: woffset1 = wattoffsetk1[10].offsetwatt; break;
  case 11: woffset1 = wattoffsetk1[11].offsetwatt; break;
  case 12: woffset1 = wattoffsetk1[12].offsetwatt; break;
  case 13: woffset1 = wattoffsetk1[13].offsetwatt; break;
  case 14: woffset1 = wattoffsetk1[14].offsetwatt; break;
  case 15: woffset1 = wattoffsetk1[15].offsetwatt; break;
  case 16: woffset1 = wattoffsetk1[16].offsetwatt; break;
  case 17: woffset1 = wattoffsetk1[17].offsetwatt; break;
  case 18: woffset1 = wattoffsetk1[18].offsetwatt; break;
  case 19: woffset1 = wattoffsetk1[19].offsetwatt; break;
  case 20: woffset1 = wattoffsetk1[20].offsetwatt; break;
  case 21: woffset1 = wattoffsetk1[21].offsetwatt; break;
  case 22: woffset1 = wattoffsetk1[22].offsetwatt; break;
  case 23: woffset1 = wattoffsetk1[23].offsetwatt; break;
  case 24: woffset1 = wattoffsetk1[24].offsetwatt; break;
  case 25: woffset1 = wattoffsetk1[25].offsetwatt; break;
  case 26: woffset1 = wattoffsetk1[26].offsetwatt; break;
  case 27: woffset1 = wattoffsetk1[27].offsetwatt; break;
  case 28: woffset1 = wattoffsetk1[28].offsetwatt; break;
  case 29: woffset1 = wattoffsetk1[29].offsetwatt; break;
  }
}

void Nwt7linux::setwattoffset2(int index)
{
  switch(index){
  case 0: woffset2 = wattoffsetk2[0].offsetwatt; break;
  case 1: woffset2 = wattoffsetk2[1].offsetwatt; break;
  case 2: woffset2 = wattoffsetk2[2].offsetwatt; break;
  case 3: woffset2 = wattoffsetk2[3].offsetwatt; break;
  case 4: woffset2 = wattoffsetk2[4].offsetwatt; break;
  case 5: woffset2 = wattoffsetk2[5].offsetwatt; break;
  case 6: woffset2 = wattoffsetk2[6].offsetwatt; break;
  case 7: woffset2 = wattoffsetk2[7].offsetwatt; break;
  case 8: woffset2 = wattoffsetk2[8].offsetwatt; break;
  case 9: woffset2 = wattoffsetk2[9].offsetwatt; break;
  case 10: woffset2 = wattoffsetk2[10].offsetwatt; break;
  case 11: woffset2 = wattoffsetk2[11].offsetwatt; break;
  case 12: woffset2 = wattoffsetk2[12].offsetwatt; break;
  case 13: woffset2 = wattoffsetk2[13].offsetwatt; break;
  case 14: woffset2 = wattoffsetk2[14].offsetwatt; break;
  case 15: woffset2 = wattoffsetk2[15].offsetwatt; break;
  case 16: woffset2 = wattoffsetk2[16].offsetwatt; break;
  case 17: woffset2 = wattoffsetk2[17].offsetwatt; break;
  case 18: woffset2 = wattoffsetk2[18].offsetwatt; break;
  case 19: woffset2 = wattoffsetk2[19].offsetwatt; break;
  case 20: woffset2 = wattoffsetk2[20].offsetwatt; break;
  case 21: woffset2 = wattoffsetk2[21].offsetwatt; break;
  case 22: woffset2 = wattoffsetk2[22].offsetwatt; break;
  case 23: woffset2 = wattoffsetk2[23].offsetwatt; break;
  case 24: woffset2 = wattoffsetk2[24].offsetwatt; break;
  case 25: woffset2 = wattoffsetk2[25].offsetwatt; break;
  case 26: woffset2 = wattoffsetk2[26].offsetwatt; break;
  case 27: woffset2 = wattoffsetk2[27].offsetwatt; break;
  case 28: woffset2 = wattoffsetk2[28].offsetwatt; break;
  case 29: woffset2 = wattoffsetk2[29].offsetwatt; break;
  }
}

void Nwt7linux::setProfil(int index)
{
  if(boxprofil->count() != 0){
    editanfang->setText(aprofil[index].anfang);
    editende->setText(aprofil[index].ende);
    editschritte->setText(aprofil[index].schritte);
  }
  wobnormalisieren();
}

void Nwt7linux::setAufloesung(int index)
{
  int aufloesung=4;
  
  switch(index){
    case 0:
        aufloesung = 4;
        break;
    case 1:
        aufloesung = 1;
        break;
    case 2:
        aufloesung = 2;
        break;
    case 3:
        aufloesung = 3;
        break;
    case 4:
        aufloesung = 4;
        break;
    case 5:
        aufloesung = 5;
        break;
    case 6:
        aufloesung = 6;
        break;
    case 7:
        aufloesung = 7;
        break;
    case 8:
        aufloesung = 8;
        break;
    case 9:
        aufloesung = 9;
        break;
  }
  wobdaten.swraufloesung = aufloesung;
  fsendewobbeldaten();
}

void Nwt7linux::tip(bool atip)
{
QString tip_kalibrierkorrk1 = tr(
  "<b>Aktivieren der mathematischen Kalibrierkorrektur (nur log. Messkopf)</B><BR>"
  "Kalibrierkorrektur bedeutet eine mathematische Korrektur der Wobbelkurve.<br> "
  "Bei der Kalibrierung der 0 dB Linie wird die Abweichung pro Frequenzpunkt<br>"
  "in eine Datei abgespeichert. Bei Darstellen von den Wobbelkurven<br> "
  "wird mit Hilfe der Abweichungs-Datei die Kurvendarstellung korrigiert.<br>"
  "Achtung das funktioniert nur mit der <b>log. Sonde!!!.</b><br>"
  ,"tooltip text");
QString tip_grafik = Nwt7linux::tr( 
  "<b>Linke Maustaste:</b> Kursor setzen <br>"
  "<b>Rechte Maustaste:</b> Menue (erst nach linker Maustaste) <br>"
  "<br>"
  "<b>Bedienung mit der Tastatur:</b> <br>"
  "<em><b>Return</b></em> setzen des Kursors <br>"
  "<em><b>Space</b></em> der aktive Kursor wird die Mitte des Frequenzbereiches und setzt Frequenzanfang und Frequenzende neu. Anschliessend wird EINMAL WOBBELN ausgeloest<br>"
  "<em><b>Up</b></em> Kursor in die Mitte und Lupeplus einmal Wobbeln <br>"
  "<em><b>Down</b></em> Kursor in die Mitte und Lupeminus einmal Wobbeln <br>"
  "<em><b>Left</b></em> Kursor 5 Pixel nach links <br>"
  "<em><b>Right</b></em> Kursor 5 Pixel nach rechts <br>"
  "<em><b>1</b></em> Umschalten auf Kursor 1 mit Return setzen des Kursors <br>"
  "<em><b>2</b></em> Umschalten auf Kursor 2 mit Return setzen des Kursors <br>"
  "<em><b>3</b></em> Umschalten auf Kursor 3 mit Return setzen des Kursors <br>"
  "<em><b>4</b></em> Umschalten auf Kursor 4 mit Return setzen des Kursors <br>"
  "<em><b>5</b></em> Umschalten auf Kursor 5 mit Return setzen des Kursors <br>"
  "<em><b>0</b></em> alle Kursor loeschen <br>"
  "<em><b>E</b></em> Einmal (wie Button) <br>"
  "<em><b>W</b></em> Wobbeln (wie Button) <br>"
  "<em><b>S</b></em> Stop (wie Button) <br>"
  ,"tooltip text");
QString tip_edit = Nwt7linux::tr(
  "<b>Eingabe:</b><br>"
  "3m5 oder 3,5m oder 3.5m = 3 500 000 Hz<br>"
  "<b>Eingabe:</b><br>"
  "350k5 oder 350,5k oder 350.5k = 350 500 Hz"
  ,"tooltip text");
QString tip_k2 = Nwt7linux::tr(
  "<b>externe Messsonde</b><br>"
  "Anschluss an PIN7/RA5 am PIC<br>"
  "<b>aktivieren</b><br>"
  "in \"Option\"<br>"
  "einkanalig deaktivieren"
  ,"tooltip text");
QString tip_relais = Nwt7linux::tr(
  "<b>Relais fuer Daempfungsglied</b><br>"
  "Anschluss an PortB am PIC<br>"
  "<b>Siehe: </b>Dokumentation"
  ,"tooltip text");
QString tip_zf = Nwt7linux::tr(
  "<b>ZF Einstellung</b><br>"
  "0 = keine ZF<br>"
  "4915000 = VFO + ZF<br>"
  "-4915000 = VFO - ZF"
  ,"tooltip text");
QString tip_dial = Nwt7linux::tr(
  "<b>Einzelschritte</b><br>"
  "mit den Pfeiltasten oder mit dem Mausrad. <br>"
  "<b>Der VFO kann jetzt auch mit der Tastatur verstellt werden</b><br>"
  "______Q__W_____E__R_____T__Z<br>"
  "VFO + 10 1 MHz 10 1 kHz 10 1 Hz<br>"
  "VFO - 10 1 MHz 10 1 kHz 10 1 Hz<br>"
  "______A__S_____D__F_____G__H" 
  ,"tooltip text");
QString tip_checkmess = Nwt7linux::tr(
  "<b>Leistungsdaempfung</b><br>"
  "wird benutzt fuer<br>"
  "Leistungsabschlusswiderstand<br>"
  "mit Messausgang -10 bis -40dB.<br>"
  "Werden Zwischenwerte gebraucht<br>"
  "einfach Wert hinzufuegen.<br>"
  "<br>"
  "Alle zusaetzlichen Eintrage<br>"
  "koennen ueber das Menue wieder<br>"
  "geloescht werden.<br>"
  "(Daempfung auf default setzen)"
  ,"tooltip text");
QString tip_mledit = Nwt7linux::tr(
  "<b>Infofenster</b><br>"
  "wird benutzt fuer Darstellung verschiedener<br>"
  "Textausgaben beim <em>Wobbeln.</em><br>"
  "<br>"
  "Die erste Zeile dient als Beschreibung<br>"
  "beim Speichern der Kurve. Die erste Zeile<br>"
  "muss mit einem Semikolon beginnen.<br>"
  "Beispiel:<br>"
  "<em>\";Bandfilter fuer 80m\"</em> <br>"
  "<br>"
  "Das DEFAULT-Label ist immer:<br>"
  "<em>\";no_label\"</em>"
  );
QString tip_betriebsart = Nwt7linux::tr(
  "<b>Betriebsart Umschalter</b><br>"
  "<u>Wobbeln</u> normales Wobbeln<br>"
  "<u>SWV</u> SWV anzeigen Kanal 1 oder 2<br>"
  "<u>SWV_ant</u> zusaetzliches SWV direkt "
  "an der Antenne ermitteln<br>"
  "<u>Impedanz</u> Anzeige der Impedanz Z. "
  "Richtige Anzeige mit einem Serienwiderstand 50 Ohm <br>"
  "<u>Spektrumanalyser</u> Spektrumanalyser vom FA "
  "<u>Spek.FRQ-shift</u> SA. vom FA + Konverter"
  ,"tooltip text");
QString tip_3db = Nwt7linux::tr(
  "<b>Ermittlung der 3dB Grenzen einer Durchlasskurve</B><BR>"
  "Als erstes wird das Maximum gesucht und anschliessend <br>"
  "die 3dB Grenze der Durchlasskurve. Daten werden ins Infofenster <br>"
  "geschrieben. Ab Version 2.05 wird auch eine inverse <br>"
  "Durchlasskurve gesucht ab Minimum (z.B.: Resonanz eines Saugkreises). <br>"
  "Hintergrund ist die Bestimmung der Guete Q aus den 3 dB Werten. <br>"
  ,"tooltip text");
QString tip_6db = Nwt7linux::tr(
  "<b>Ermittlung der 6dB/60dB Grenzen + <br>"
  "Shape-Faktor einer Durchlasskurve</B><BR>"
  "Als erstes wird das Maximum gesucht und anschliessend <br>"
  "die 6dB/60dB Grenze der Durchlasskurve. Daten werden <br>"
  "ins Infofenster geschrieben. War die Suche erfolgreich, <br>"
  "wir noch der Shape-Faktor errechnet."
  ,"tooltip text");
QString tip_dbgrafik = Nwt7linux::tr(
  "<b>Grafische Anzeige der dB Grenzen einer Durchlasskurve</B><BR>"
  "Es werden senkrechte unterbrochene Linien in die Wobbelkurve "
  "eingeblendet. Beim Aktivieren von 3dB,6dB/60dB wird die <br>"
  "grafische Darstellung automatisch mit aktiv."
  ,"tooltip text");
QString tip_schritte = Nwt7linux::tr(
  "<b>Schritte:</b><br>"
  "Die maximale Anzahl der Messpunkte betraegt 9999. Um auf eine <br>"
  "runde Schrittweite zu kommen ist es guenstig einen Schritt mehr <br>"
  "einzugeben (z.B: 201). <br>"
  "Die Anfangsfrequenz ist ja auch schon ein Messpunkt. Somit ergeben sich "
  "200 Schritte."
  ,"tooltip text");
QString tip_pbar = Nwt7linux::tr(
  "<b>Datenruecklauf</b><br>"
  "Anzeige der Daten, die beim Wobbeln von <br>"
  "der Baugruppe zurueck in den PC fliessen."
  ,"tooltip text");
QString tip_messtext = Nwt7linux::tr(
  "<b>Textfenster fuer Messwertspeicherung</b><br>"
  "Das Textfenster ist voll editierbar und <br>"
  "kann als Textdatei abgespeichert werden"
  ,"tooltip text");
QString tip_messoffset = Nwt7linux::tr(
  "<b>Anpassung des Messkopfes an die Messfrequenz</b><br>"
  "Wenn die zu messende Frequenz bekannt ist, kann hier<br>"
  "der Messfehler des Messkopfes ausgeglichen werden.<br>"
  "Beim NWT01 ist das der AD8307.<br>"
  "Es koennen 10 verschiedene Korrekturen eingegeben werden.<br>"
  "Dazu ist der Menuepunkt <em>Editieren Sonde</em> vorgesehen.<br>"
  ,"tooltip text");
QString tip_swra_100 = Nwt7linux::tr(
  "<b>Eingabe der Kabeldaempfung</b><br>"
  "Die Daempfung wird in dB/100m eingegeben nur die Zahl"
  ,"tooltip text");
QString tip_swrlaenge = Nwt7linux::tr(
  "<b>Eingabe der Kabellaenge</b><br>"
  "Die Kabellaenge wird in Meter eingegeben nur die Zahl"
  ,"tooltip text");
QString tip_lupeplus = Nwt7linux::tr(
  "<b>Hineinzoomen in den Frequenzbereich</B><BR>"
  "Ausgangspunkt ist der Kursor. Er bildet auch <br>"
  "gleichzeitig den ungefaehren Mittelpunkt des <br"
  "Wobbelfensters. Der Frequenzbereich wird halbiert <br>"
  "und anschliessend <b>ein</b> Wobbeldurchlauf gestartet. <br>"
  "Frequenzgrenzen bilden die Kalibrierfrequenzangaben in der Option."
  ,"tooltip text");
QString tip_lupeminus = Nwt7linux::tr(
  "<b>Herauszoomen aus den Frequenzbereich</B><BR>"
  "Ausgangspunkt ist der Kursor. Er bildet auch <br>"
  "gleichzeitig den ungefaehren Mittelpunkt des <br>"
  "Wobbelfensters. Der Frequenzbereich wird verdopplet <br>"
  "und anschliessend <b>ein</b> Wobbeldurchlauf gestartet. <br>"
  "Frequenzgrenzen bilden die Kalibrierfrequenzangaben in der Option."
  ,"tooltip text");
QString tip_lupemitte = Nwt7linux::tr(
  "<b>Frequenzbereich mitteln</B><BR>"
  "Ausgangspunkt ist der Kursor. <br>"
  "Der Frequenzbereich bleibt bestehen <br>"
  "und anschliessend <b>ein</b> Wobbeldurchlauf gestartet. <br>"
  "Frequenzgrenzen bilden die Kalibrierfrequenzangaben in der Option."
  ,"tooltip text");
QString tip_offset = Nwt7linux::tr(
  "<b>Verschieben der Kurve im Y-Bereich</B><BR>"
  "Mit dem Mausrad oder durch Auswahl kann die <br>"
  "Kurve nach oben oder nach unten verschoben werden <br>"
  "Alle Analysergebnisse werden angepasst. Die <br>"
  "Y-Beschriftung aendert ihre Farbe in Rot."
  ,"tooltip text");
QString tip_kursor = Nwt7linux::tr(
  "<b>Einstellung der Kursornummer</B><BR>"
  "Mit dem Mausrad oder durch Auswahl kann die <br>"
  "Nummer des Kusors geaendert werden. Bereich 1-5.<br>"
  "Es ist immer nur die eingestellte Nummer aktiv"
  ,"tooltip text");
QString tip_zfwobbel = Nwt7linux::tr(
  "<b>Wobbeln mit ZF</B><BR>"
  "Dieser Haken aktiviert das Wobbeln mit Frequenzablage.<br>"
  "4915000 = VFO + ZF<br>"
  "-4915000 = VFO - ZF"
  ,"tooltip text");
QString tip_vfo_x4 = Nwt7linux::tr(
  "<b>Ansteuerung von I/Q Mischern</B><BR>"
  "Dieser Haken aktiviert die VFO-Ausgabe x 4.<br>"
  "Damit ist es moeglich SDR-Radio Softrock anzusteuern<br>"
  "Die Anzeige der Maximalfrequenz reduziert sich<br>"
  "entsprechend.<br>"
  ,"tooltip text");
QString tip_messabs = Nwt7linux::tr(
  "<b>Anzeige des nummerischer Wertes vom A/D Wandler</B><BR>"
  "Diese Zahl ist der Mittelwert vom meheren A/D Wandlungen <br>"
  "des NWT-PIC <br>"
  ,"tooltip text");
QString tip_displayshift = Nwt7linux::tr(
  "<b>Anpassung der dB-Beschriftung</B><BR>"
  "Der Messeingang arbeitet max mit etwa +10dBm. Wird ein <br>"
  "Senderausgang gemessen so muss ein Daempfungsglied vor dem <br>"
  "Messkopf geschaltet werden. Zum Beispiel ein Leistungsabschluss <br>"
  "mit -40dB Messausgang. Ich trage <em>40</em> ein und die <br>"
  "0dB Linie wandert nach unten und die Beschriftung wird auch <br>"
  "richtig angezeigt. Alle Werte werden mit 40dB addiert. <br>"
  "In diesem Beispiel ist jetzt der Bereich bis +50dB sichtbar.<br>"
  ,"tooltip text");
QString tip_scantime = Nwt7linux::tr(
  "<b>Zeit zwischen den Messpunkten</B><BR>"
  "Auswahl der Zeit zwischen den einzelnen Messpunkten. <br>"
  "Die Masseinheit ist uSekunden. Diese Funktion ist nur <br>"
  "nutzbar mit der passenden FW. <br>"
  ,"tooltip text");
QString tip_profil = Nwt7linux::tr(
  "<b>Profil</B><BR>"
  "Mit der Combobox koennen Voreinstellungen fuer Frequenzanfang, <br>"
  "Frequenzende und Schrittanzahl ausgewaehlt werden. Zum Bearbeiten <br>"
  "der Profile gibt es einen Menuepunkt. <br>"
  ,"tooltip text");
QString tip_invers = Nwt7linux::tr(
  "<b>Inverse Bandbreitenermittlung</B><BR>"
  "Mit diesem Schalter kann die Bandbreitenermittlung 3dB/Q <br>"
  "Ein- oder Ausgeschaltet werden. Die inverse Bandbreite braucht <br>"
  "man um zum Beispiel aus einem Serienschwingkreis das Q zu ermitteln. <br>"
  ,"tooltip text");
QString tip_sastatus = Nwt7linux::tr(
  "<b>Bereich oder Status des Spektrumanalysers</B><BR>"
  "Es gibt 2 Funktionen. <br>"
  "1. Funktion: Einstellung des Frequenzbereiches des SA. <br>"
  "2. Funktion: Mit der richtigen FW kann die Rueckmeldung <br>"
  "   vom SA aktiviert werden (in OPTION). Ich sehe die SA-Einstellungen <br>"
  "   in der SW. Die Rueckmeldung erfolgt durch Spannungswerte <br>"
  "   die beim Umschalten im SA entstehen. <br>"
  ,"tooltip text");
QString tip_bfrq = Nwt7linux::tr(
  "<b>Frequenzeingabe/Uebernahme</B><BR>"
  "Die Frequenz kann ueber das Kursormenue gesetzt <br>"
  "werden oder auch direkt eingegeben werden. <br>"
  "Einheit ist MHz. <br>"
  ,"tooltip text");
QString tip_bl = Nwt7linux::tr(
  "<b>Eingabe Induktivitaet</B><BR>"
  "Hier wird die Induktivitaet in uH eingegeben. <br>"
  "Daraus errechnet sich die Kapazitaet in pF. <br>"
  ,"tooltip text");
QString tip_bc = Nwt7linux::tr(
  "<b>Eingabe Kapazitaet</B><BR>"
  "Hier wird die Kapazitaet in pF eingegeben. <br>"
  "Daraus errechnet sich die Induktivitaet in uH. <br>"
  ,"tooltip text");
QString tip_bn = Nwt7linux::tr(
  "<b>Eingabe Windungen</B><BR>"
  "Hier werden die Windungen der Testinduktivitaet eingegeben. <br>"
  "Daraus errechnet sich der AL-Wert in nH/Windungen ins Quadrat. <br>"
  ,"tooltip text");
QString tip_mhang = Nwt7linux::tr(
  "<b>Haengefunktion ein/aus</B><BR>"
  "Im Wattmeter ist eine Haengefunktion inplementiert. Nur hoehere Werte <br>"
  "werden sofort angezeigt. Faellt der Wert wieder werden 8 Messungen kontrolliert. <br>"
  "Ist der Wert nach 8 Messungen immer niedriger als der angezeigte Wert, wird <br>"
  "der neue Wert zur Anzeige gebracht. <br>"
  ,"tooltip text");
QString tip_alw = Nwt7linux::tr(
  "<b>Berechnung der Windungen aus dem AL-Wert</B><BR>"
  "Der AL-Wert wird entweder automatisch gefuellt durch die AL-Berechnung oder kann <br>"
  "auch per Hand eingegeben werden. Die Induktivitaet wird nur per Hand eingegeben.<br>"
  ,"tooltip text");
QString tip_ymax = Nwt7linux::tr(
  "<b>Skalierung der Y-Achse (Ymax oberster Wert in dB)</B><BR>"
  "In dieser Combobox wird der oberste Wert der Y-Achse eingestellt. <br>"
  ,"tooltip text");
QString tip_ymin = Nwt7linux::tr(
  "<b>Skalierung der Y-Achse (Ymin unterster Wert in dB)</B><BR>"
  "In dieser Combobox wird der unterste Wert der Y-Achse eingestellt. <br>"
  ,"tooltip text");
  
  btip = atip;
  
  wkmanager->tip(btip);
  if(btip){
    grafik->setToolTip(tip_grafik);
    editanfang->setToolTip(tip_edit);
    editende->setToolTip(tip_edit);
    editdisplay->setToolTip(tip_displayshift);
    checkboxk2->setToolTip(tip_k2);
    daempfung1->setToolTip(tip_relais);
    daempfung2->setToolTip(tip_relais);
    daempfung3->setToolTip(tip_relais);
    editzf->setToolTip(tip_zf);
    editvfo->setToolTip(tip_edit);
    vsp1hz->setToolTip(tip_dial);
    vsp10hz->setToolTip(tip_dial);
    vsp100hz->setToolTip(tip_dial);
    vsp1khz->setToolTip(tip_dial);
    vsp10khz->setToolTip(tip_dial);
    vsp100khz->setToolTip(tip_dial);
    vsp1mhz->setToolTip(tip_dial);
    vsp10mhz->setToolTip(tip_dial);
    vsp100mhz->setToolTip(tip_dial);
    vsp1ghz->setToolTip(tip_dial);
    ldaempfungk1->setToolTip(tip_checkmess);
    ldaempfungk2->setToolTip(tip_checkmess);
    progressbarmessk2->setToolTip(tip_k2);
    mledit->setToolTip(tip_mledit);
    betriebsart->setToolTip(tip_betriebsart);
    checkbox3db->setToolTip(tip_3db);
    checkbox6db->setToolTip(tip_6db);
    checkboxgrafik->setToolTip(tip_dbgrafik);
    editschritte->setToolTip(tip_schritte);
    progressbar->setToolTip(tip_pbar);
    messedit->setToolTip(tip_messtext);
    edita_100->setToolTip(tip_swra_100);
    labela_100->setToolTip(tip_swra_100);
    editkabellaenge->setToolTip(tip_swrlaenge);
    labelkabellaenge->setToolTip(tip_swrlaenge);
    buttonlupeplus->setToolTip(tip_lupeplus);
    buttonlupeminus->setToolTip(tip_lupeminus);
    buttonlupemitte->setToolTip(tip_lupemitte);
    boxdbshift1->setToolTip(tip_offset);
    boxdbshift2->setToolTip(tip_offset);
    boxkursornr->setToolTip(tip_kursor);
    boxwattoffset1->setToolTip(tip_messoffset);
    boxwattoffset2->setToolTip(tip_messoffset);
    checkboxzf->setToolTip(tip_zfwobbel);
    checkboxiqvfo->setToolTip(tip_vfo_x4);
    mlabelk1->setToolTip(tip_messabs);
    mlabelk2->setToolTip(tip_messabs);
    boxzwischenzeit->setToolTip(tip_scantime);
    boxprofil->setToolTip(tip_profil);
    checkboxinvers->setToolTip(tip_invers);
    groupsa->setToolTip(tip_sastatus);
    editf1->setToolTip(tip_bfrq);
    editf2->setToolTip(tip_bfrq);
    editc1->setToolTip(tip_bc);
    editl2->setToolTip(tip_bl);
    editn->setToolTip(tip_bn);
    groupwindungen->setToolTip(tip_alw);
    checkboxmesshang1->setToolTip(tip_mhang); 
    checkboxmesshang2->setToolTip(tip_mhang);
    boxydbmax->setToolTip(tip_ymax); 
    boxydbmin->setToolTip(tip_ymin);
    kalibrier1->setToolTip(tip_kalibrierkorrk1); 
    kalibrier2->setToolTip(tip_kalibrierkorrk1); 
  }else{
    grafik->setToolTip("");
    editanfang->setToolTip("");
    editende->setToolTip("");
    editdisplay->setToolTip("");
    checkboxk2->setToolTip("");
    daempfung1->setToolTip("");
    daempfung2->setToolTip("");
    daempfung3->setToolTip("");
    editzf->setToolTip("");
    editvfo->setToolTip("");
    vsp1hz->setToolTip("");
    vsp10hz->setToolTip("");
    vsp100hz->setToolTip("");
    vsp1khz->setToolTip("");
    vsp10khz->setToolTip("");
    vsp100khz->setToolTip("");
    vsp1mhz->setToolTip("");
    vsp10mhz->setToolTip("");
    vsp100mhz->setToolTip("");
    vsp1ghz->setToolTip("");
    ldaempfungk1->setToolTip("");
    ldaempfungk2->setToolTip("");
    progressbarmessk2->setToolTip("");
    mledit->setToolTip("");
    betriebsart->setToolTip("");
    checkbox3db->setToolTip("");
    checkbox6db->setToolTip("");
    checkboxgrafik->setToolTip("");
    editschritte->setToolTip("");
    progressbar->setToolTip("");
    messedit->setToolTip("");
    edita_100->setToolTip("");
    labela_100->setToolTip("");
    editkabellaenge->setToolTip("");
    labelkabellaenge->setToolTip("");
    buttonlupeplus->setToolTip("");
    buttonlupeminus->setToolTip("");
    buttonlupemitte->setToolTip("");
    boxdbshift1->setToolTip("");
    boxdbshift2->setToolTip("");
    boxkursornr->setToolTip("");
    boxwattoffset1->setToolTip("");
    boxwattoffset2->setToolTip("");
    checkboxzf->setToolTip("");
    checkboxiqvfo->setToolTip("");
    mlabelk1->setToolTip("");
    mlabelk2->setToolTip("");
    boxzwischenzeit->setToolTip("");
    boxprofil->setToolTip("");
    checkboxinvers->setToolTip("");
    groupsa->setToolTip("");
    editf1->setToolTip("");
    editf2->setToolTip("");
    editc1->setToolTip("");
    editl2->setToolTip("");
    editn->setToolTip("");
    groupwindungen->setToolTip("");
    checkboxmesshang1->setToolTip(""); 
    checkboxmesshang2->setToolTip(""); 
    boxydbmax->setToolTip(""); 
    boxydbmin->setToolTip(""); 
    kalibrier1->setToolTip(""); 
    kalibrier2->setToolTip(""); 
  }
}

void Nwt7linux::setColorRedk1(){
  QPalette pred;
  pred.setColor(QPalette::WindowText, Qt::red);
  checkboxk1->setPalette(pred);
}

void Nwt7linux::setColorBlackk1(){
  QPalette pbl;
  pbl.setColor(QPalette::WindowText, Qt::black);
  checkboxk1->setPalette(pbl);
}

void Nwt7linux::setColorRedk2(){
  QPalette pred;
  pred.setColor(QPalette::WindowText, Qt::red);
  checkboxk2->setPalette(pred);
}

void Nwt7linux::setColorBlackk2(){
  QPalette pbl;
  pbl.setColor(QPalette::WindowText, Qt::black);
  checkboxk2->setPalette(pbl);
}

void Nwt7linux::setDaempfungdefault(){
  configfile nwt7configfile;
  QString qs;
  
  nwt7configfile.open(homedir.filePath(configfilename));
  while(nwt7configfile.clearItem("powerk")) ;
  nwt7configfile.close();
  while(ldaempfungk1->count() > 5)ldaempfungk1->removeItem(5) ;
  while(ldaempfungk2->count() > 5)ldaempfungk2->removeItem(5) ;
}

void Nwt7linux::fzaehlen(){
  QString qs;
  unsigned long zeit, ergebnis;
  char abuffer[4];
  int a=0, i, j;

  //zeit = 624998;
  zeit = 62499;

  if(!bttyOpen){
    //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
    if(!bttyOpen){
      QMessageBox::warning( this, tr("Serielle Schnittstelle","InformationsBox"), 
                                     "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","InformationsBox"));
      //qDebug("Nwt7linux::fzaehlen()");
      return;
    }
  }
  if (bttyOpen)
  {
    qs = QString("%1").arg(zeit,0,16);
    while(qs.length() < 8)qs = '0' + qs;
    //return;
    qs = "z" + qs;
    //qDebug(qs);
    defaultlesenrtty(); //eventuelle Daten im UART loeschen
    picmodem->writeChar(143);
    picmodem->writeLine(qs);
    ttyindex = 0;
    do{
      a = picmodem->readttybuffer(abuffer, 4);
      //if(a>0)qDebug("%i",a);
      for(i=0, j=ttyindex; i<a; i++, j++){
        ttybuffer[j] = abuffer[i]; // Die gelesenen Daten in den richtigen Buffer kopieren
      }
      ttyindex = ttyindex + a; // und den Index auf die richtige Stelle setzen
    }while(ttyindex != 4);
    ergebnis = ttybuffer[0] + ttybuffer[1]*256 + ttybuffer[2]*256*256 + ttybuffer[3]*256*256*256;
    //qDebug("ergebnis= %li", ergebnis);
  }
  (void)ergebnis;
}

void Nwt7linux::keyPressEvent(QKeyEvent *event){
  double wert = getdrehgeber();
  switch(event->key()){
    case Qt::Key_Q: wert+=10000000.0; break;
    case Qt::Key_A: wert-=10000000.0; break;
    case Qt::Key_W: wert+=1000000.0; break;
    case Qt::Key_S: wert-=1000000.0; break;
    case Qt::Key_E: wert+=10000.0; break;
    case Qt::Key_D: wert-=10000.0; break;
    case Qt::Key_R: wert+=1000.0; break;
    case Qt::Key_F: wert-=1000.0; break;
    case Qt::Key_T: wert+=10.0; break;
    case Qt::Key_G: wert-=10.0; break;
    case Qt::Key_Z: wert+=1.0; break;
    case Qt::Key_H: wert-=1.0; break;
    default:QWidget::keyPressEvent(event);
  }
  if(wert<0.0)wert=0.0;
  setdrehgeber(wert);
}

void Nwt7linux::openKurvenManager(){
  #ifdef LDEBUG
  qDebug("Nwt7linux::openKurvenManager()");
  #endif    
  wkmanager->show();
}



void Nwt7linux::fsendewobbeldaten(){
  //festlegen der Wobbelgrenzen, Bezug fuer die Hintergrundkurven
  wobdaten.absanfang = wobdaten.anfangsfrequenz;
  wobdaten.absende = wobdaten.anfangsfrequenz
                   + wobdaten.schrittfrequenz 
                   * ((double)wobdaten.schritte -1);
  if(wobdaten.linear1 or wobdaten.linear2){
    wobdaten.bkalibrierkorrk1 = false;
  }else{
    wobdaten.bkalibrierkorrk1 = wgrunddaten.kalibrierk1;
  }
  emit sendewobbeldaten(wobdaten);
}

void Nwt7linux::readSettings()
{
  //Position vom Grafikfenster nachladen
  QSettings settings("AFU", "NWT");
  QPoint pos = settings.value("wpos", QPoint(0,0)).toPoint();
  QSize size = settings.value("wsize", QSize(700, 400)).toSize();
  grafik->resize(size);
  grafik->move(pos);
}

void Nwt7linux::writeSettings()
{
  //Position vom Grafikfenster abspeichern
  QSettings settings("AFU", "NWT");
  settings.setValue("wpos", grafik->pos());
  settings.setValue("wsize", grafik->size());
}

void Nwt7linux::vfoum()
{
  if(bmessvfo){
    bmessvfo = false;
  }else{
    bmessvfo = true;
  }
  vfoeinaus();
}

void Nwt7linux::vfoeinaus()
{
  if(bmessvfo){
    labelsphz->show();
    labelspkhz->show();
    labelspmhz->show();
    sp1hz->show();
    sp10hz->show();
    sp100hz->show();
    sp1khz->show();
    sp10khz->show();
    sp100khz->show();
    sp1mhz->show();
    sp10mhz->show();
    sp100mhz->show();
    sp1ghz->show();
    daempfung3->show();
    labeldaempfung3->show();
    messedit->hide();
    buttonmess->hide();
    buttonmesssave->hide();
    
  }else{
    labelsphz->hide();
    labelspkhz->hide();
    labelspmhz->hide();
    sp1hz->hide();
    sp10hz->hide();
    sp100hz->hide();
    sp1khz->hide();
    sp10khz->hide();
    sp100khz->hide();
    sp1mhz->hide();
    sp10mhz->hide();
    sp100mhz->hide();
    sp1ghz->hide();
    daempfung3->hide();
    labeldaempfung3->hide();
    messedit->show();
    buttonmess->show();
    buttonmesssave->show();
  }
}

void Nwt7linux::setmessvfo()
{
  int i;
  double fr;
    
  //qDebug("setmessvfo()");
  if(messedit->isHidden()){
    if(sp1hz->value()==10){
      sp1hz->setValue(0);
      i = sp10hz->value();
      i++;
      sp10hz->setValue(i);
    }
    if(sp10hz->value()==10){
      sp10hz->setValue(0);
      i = sp100hz->value();
      i++;
      sp100hz->setValue(i);
    }
    if(sp100hz->value()==10){
      sp100hz->setValue(0);
      i = sp1khz->value();
      i++;
      sp1khz->setValue(i);
    }
    if(sp1khz->value()==10){
      sp1khz->setValue(0);
      i = sp10khz->value();
      i++;
      sp10khz->setValue(i);
    }
    if(sp10khz->value()==10){
      sp10khz->setValue(0);
      i = sp100khz->value();
      i++;
      sp100khz->setValue(i);
    }
    if(sp100khz->value()==10){
      sp100khz->setValue(0);
      i = sp1mhz->value();
      i++;
      sp1mhz->setValue(i);
    }
    if(sp1mhz->value()==10){
      sp1mhz->setValue(0);
      i = sp10mhz->value();
      i++;
      sp10mhz->setValue(i);
    }
    if(sp10mhz->value()==10){
      sp10mhz->setValue(0);
      i = sp100mhz->value();
      i++;
      sp100mhz->setValue(i);
    }
    if(sp100mhz->value()==10){
      sp100mhz->setValue(0);
      i = sp1ghz->value();
      i++;
      sp1ghz->setValue(i);
    }
    fr = double(sp1hz->value()) + 
         double(sp10hz->value())   * 10.0 +
         double(sp100hz->value())  * 100.0 +
         double(sp1khz->value())   * 1000.0 +
         double(sp10khz->value())  * 10000.0 +
         double(sp100khz->value()) * 100000.0 +
         double(sp1mhz->value())   * 1000000.0 +
         double(sp10mhz->value())  * 10000000.0 +
         double(sp100mhz->value()) * 100000000.0 +
         double(sp1ghz->value())   * 1000000000.0;
    if(fr < 0.0){
      if(sp1hz->value()==-1)sp1hz->setValue(0);
      if(sp10hz->value()==-1)sp10hz->setValue(0);
      if(sp100hz->value()==-1)sp100hz->setValue(0);
      if(sp1khz->value()==-1)sp1khz->setValue(0);
      if(sp10khz->value()==-1)sp10khz->setValue(0);
      if(sp100khz->value()==-1)sp100khz->setValue(0);
      if(sp1mhz->value()==-1)sp1mhz->setValue(0);
      if(sp10mhz->value()==-1)sp10mhz->setValue(0);
      if(sp100mhz->value()==-1)sp100mhz->setValue(0);
      if(sp1ghz->value()==-1)sp1ghz->setValue(0);
    }
    if(sp1hz->value()==-1){
      sp1hz->setValue(9);
      i = sp10hz->value();
      i--;
      sp10hz->setValue(i);
    }
    if(sp10hz->value()==-1){
      sp10hz->setValue(9);
      i = sp100hz->value();
      i--;
      sp100hz->setValue(i);
    }
    if(sp100hz->value()==-1){
      sp100hz->setValue(9);
      i = sp1khz->value();
      i--;
      sp1khz->setValue(i);
    }
    if(sp1khz->value()==-1){
      sp1khz->setValue(9);
      i = sp10khz->value();
      i--;
      sp10khz->setValue(i);
    }
    if(sp10khz->value()==-1){
      sp10khz->setValue(9);
      i = sp100khz->value();
      i--;
      sp100khz->setValue(i);
    }
    if(sp100khz->value()==-1){
      sp100khz->setValue(9);
      i = sp1mhz->value();
      i--;
      sp1mhz->setValue(i);
    }
    if(sp1mhz->value()==-1){
      sp1mhz->setValue(9);
      i = sp10mhz->value();
      i--;
      sp10mhz->setValue(i);
    }
    if(sp10mhz->value()==-1){
      sp10mhz->setValue(9);
      i = sp100mhz->value();
      i--;
      sp100mhz->setValue(i);
    }
    if(sp100mhz->value()==-1){
      sp100mhz->setValue(9);
      i = sp1ghz->value();
      i--;
      sp1ghz->setValue(i);
    }
    if(sp1ghz->value()==-1){
      sp1ghz->setValue(0);
    }
    messtimer->stop();
    messvfotimer->start(100);
  }
}

void Nwt7linux::messvfotimerende()
{
  //qDebug("messvfotimerende()");
  if(!bersterstart){
    QString qs;
    char s[20];
    double fr;
    //nach ablauf des Timers wird Frequenz zum NWT gesendet
    if(!bttyOpen){
      //bttyOpen = picmodem->opentty(wgrunddaten.str_tty);
      if(!bttyOpen){
        QMessageBox::warning( this, tr("Serielle Schnittstelle","InformationsBox"), 
                                      "\"" + wgrunddaten.str_tty + tr("\" Laesst sich nicht oeffnen!","InformationsBox"));
        #ifdef LDEBUG
        qDebug("Nwt7linux::senddaten()");
        #endif    
        return;
      }
    }
    if (bttyOpen)
    {
      fr = double(sp1hz->value()) + 
           double(sp10hz->value()) *   10.0 +
           double(sp100hz->value()) *  100.0 +
           double(sp1khz->value()) *   1000.0 +
           double(sp10khz->value()) *  10000.0 +
           double(sp100khz->value()) * 100000.0 +
           double(sp1mhz->value()) *   1000000.0 +
           double(sp10mhz->value()) *  10000000.0 +
           double(sp100mhz->value()) * 100000000.0 +
           double(sp1ghz->value()) *   1000000000.0;
      //Frequenzfaktor anwenden
      fr = qRound(fr / double(wobdaten.frqfaktor));
      if((fr < 1000000000.0) and fr >= 0.0){
        sprintf(s, "f%09.0f", fr);
        //qDebug(s);
        picmodem->writeChar(143);
        picmodem->writeLine(s);
      }
    }
  }
  //timer stoppen da Aufgabe erledigt
  messvfotimer->stop();
  messtimer->setSingleShot(true);
  messtimer->start(messtime);
}

void Nwt7linux::setDisplayYmax(const QString &sa)
{
  bool ok;
  int ydbmax;
  QString s = sa;
  //  qDebug(s);
  ydbmax = s.toInt(&ok);
  if(ok){
    wobdaten.ydbmax = ydbmax;
    fsendewobbeldaten();
//    emit sendewobbeldaten(wobdaten);
  }
}

void Nwt7linux::setDisplayYmin(const QString &sa)
{
  bool ok;
  int ydbmin;
  QString s = sa;
  //  qDebug(s);
  ydbmin = s.toInt(&ok);
  if(ok){
    wobdaten.ydbmin = ydbmin;
    fsendewobbeldaten();
//    emit sendewobbeldaten(wobdaten);
  }
}

/*
void Nwt7linux::setkdir()
{
  wkmanager->setkdir(kurvendir.absolutePath());
}

void Nwt7linux::getkdir(QString s)
{
  kurvendir.setPath(s);
}

void Nwt7linux::warnkalibrierkorr()
{
  if(wobdaten.bkalibrierkorrk1 or wobdaten.bkalibrierkorrk2){
      
    if(wgrunddaten.bwarnekorr){
      //keine Warnung bei SA
      if((wobdaten.ebetriebsart != espektrumanalyser) and (wobdaten.ebetriebsart != espekdisplayshift)){
        if( QMessageBox::warning( this, 
            tr("Grafische Darstellung","InformationsBox"),
            tr("<B>Kurvendatei Darstellung</B><BR>"
                "Durch die aktiverte mathematische Kalibrierkorrektur "
                "weicht die Darstellung der Kurvendatei etwas ab. Auf "
                "der Y-Achse entstehen Differenzen zwischen der "
                "realen Wobbelkurven und den Kurvendateinen.<br><br>"
                "<B>Zur genauen Darstellung ist es notwendig "
                "die mathematische Kalibrierkorrektur zu deaktivieren!</B>" ,"WarnungsBox"),
                tr("Weiter", "WarnungsBox"),
                tr("Deaktivieren", "WarnungsBox")
                ,0,1) == 1)
        {
          wobdaten.bkalibrierkorrk1 = false;
          wobdaten.bkalibrierkorrk2 = false;
          emit sendewobbeldaten(wobdaten);
        }
      }
    }
      //qDebug(Nwt7linux::warnkalibrierkorr()");
    if(wobdaten.bkalibrierkorrk1){
      setColorRedk1();
    }else{
      setColorBlackk1();
    }
    if(wobdaten.bkalibrierkorrk2){
      setColorRedk2();
    }else{
      setColorBlackk2();
    }
  }
}
*/

void Nwt7linux::kalibrier1_checked(bool b){
  wgrunddaten.kalibrierk1 = b;
  wobdaten.bkalibrierkorrk1 = b;
  if(wgrunddaten.kalibrierk1 and !wobdaten.linear1){
    setColorRedk1();
  }else{
    setColorBlackk1();
  }
  if(wgrunddaten.kalibrierk2 and !wobdaten.linear2){
    setColorRedk2();
  }else{
    setColorBlackk2();
  }
  emit sendewobbeldaten(wobdaten);
}

void Nwt7linux::kalibrier2_checked(bool b){
  wgrunddaten.kalibrierk2 = b;
  wobdaten.bkalibrierkorrk2 = b;
  if(wgrunddaten.kalibrierk1 and !wobdaten.linear1){
    setColorRedk1();
  }else{
    setColorBlackk1();
  }
  if(wgrunddaten.kalibrierk2 and !wobdaten.linear2){
    setColorRedk2();
  }else{
    setColorBlackk2();
  }
  emit sendewobbeldaten(wobdaten);
}

void Nwt7linux::setProgramPath(const QString &ap){
  //qDebug("Nwt7linux::setProgramPath(QString s)");
  programpath = ap;
  //qDebug(programpath);
  homedir.setPath(programpath);
  //der Programname ist noch mit im Pfad: deshalb cdUp
  homedir.cdUp();
  //QString s = homedir.absolutePath();
  //qDebug(s);
  bersterstart = false;
  //Test ob im Progrannverzeichnis das Verzeichnis nwt befindet
  if(!homedir.cd("nwt")){
    //KEIN nwt Verzeichnis: HOME ist das neue Verzeichnis
    homedir = QDir::home();
    if(!homedir.cd("hfm9")){
      //Befindet sich im HOME kein hfm9-Verzeichnis
      //hfm9 erzeugen
      homedir.mkdir("hfm9");
      //in das hfm9 gehen
      homedir.cd("hfm9");
      //das ist der erste Start des Programmes
      bersterstart = true;
    }
  }
  QDir dir=homedir;
  dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
  dir.setSorting(QDir::Size | QDir::Reversed);

  /*
  QFileInfoList list = dir.entryInfoList();
  std::cout << "     Bytes Filename" << std::endl;
  for (int i = 0; i < list.size(); ++i) {
      QFileInfo fileInfo = list.at(i);
      std::cout << qPrintable(QString("%1 %2").arg(fileInfo.size(), 10)
                                              .arg(fileInfo.fileName()));
      std::cout << std::endl;
  }
  */
  //s = homedir.absolutePath();
  //qDebug(s);
}

void Nwt7linux::setFrqFaktorLabel(){
  QString qs, s;
  QPalette pred;
  QPalette pbl;
  pred.setColor(QPalette::WindowText, Qt::red);
  pbl.setColor(QPalette::WindowText, Qt::black);
  
  s = tr("Frequenzvervielfachung");
  qs.sprintf(" x %i", wobdaten.frqfaktor);
  qs = s + qs;
  labelfrqfaktor->setText(qs);
  labelfrqfaktorv->setText(qs);
  labelfrqfaktorm->setText(qs);
  if(wobdaten.frqfaktor > 1){
    labelfrqfaktor->setPalette(pred);
    labelfrqfaktorv->setPalette(pred);
    labelfrqfaktorm->setPalette(pred);
    labelfrqfaktor->show();
    labelfrqfaktorv->show();
    labelfrqfaktorm->show();
  }else{
    labelfrqfaktor->setPalette(pbl);
    labelfrqfaktorv->setPalette(pbl);
    labelfrqfaktorm->setPalette(pbl);
    labelfrqfaktor->hide();
    labelfrqfaktorv->hide();
    labelfrqfaktorm->hide();
  }
}

void Nwt7linux::nkurve_loeschen(){
  wobbelungueltig();
  fsendewobbeldaten();
}

void Nwt7linux::zrausrechnen(){
  double r1;
  double r2;
  double r3;
  double r4;
  double daempf;
  double z1;
  double z2;
  double z3;
  double z4;
  double d1;
  double d2;
  QString qs;
  bool ok;
  
  qs = editz1->text();
  z1 = qs.toDouble(&ok);
  if(!ok)return;
  qs = editz2->text();
  z2 = qs.toDouble(&ok);
  if(!ok)return;
  qs = editz3->text();
  z3 = qs.toDouble(&ok);
  if(!ok)return;
  qs = editz4->text();
  z4 = qs.toDouble(&ok);
  if(!ok)return;
  r1 = z1/(sqrt(1.0 - (z1/z2)));
  r2 = z2*sqrt(1.0 - (z1/z2));
  r3 = z3*sqrt(1.0 - (z4/z3));
  r4 = z4/(sqrt(1.0 - (z4/z3)));
  
  qs.sprintf("%3.2f",r1);
  lr1->setText(qs);
  qs.sprintf("%3.2f",r2);
  lr2->setText(qs);
  qs.sprintf("%3.2f",r3);
  lr3->setText(qs);
  qs.sprintf("%3.2f",r4);
  lr4->setText(qs);
  
  d1 = 10.0 * log10(((r1*z1)/(r1+z1))/(2.0*r2+(r1*z1/(r1+z1))));
  //qDebug("%f",d1);
  d2 = 10.0 * log10(((r4*z4)/(r4+z4))/(2.0*r3+(r4*z4/(r4+z4))));
  //qDebug("%f",d2);
  
  daempf = d1+d2;
  qs.sprintf("%3.2f",daempf);
  ldaempfung->setText(qs);
}

void Nwt7linux::zlcausrechnen(){
  double xl;
  double xc;
  double l;
  double c;
  double f;
  double z1;
  double z2;
  QString qs;
  bool ok;
  
  qs = editzlc1->text();
  z1 = qs.toDouble(&ok);
  if(!ok)return;
  qs = editzlc2->text();
  z2 = qs.toDouble(&ok);
  if(!ok)return;
  qs = editzlc3->text();
  f = qs.toDouble(&ok);
  if(!ok)return;
  f = f*1000000.0;
  
  xl = sqrt(z1*z2-z1*z1);
  xc = (z1*z2)/xl;
  
  //qDebug("xl %f",xl);
  //qDebug("xc %f",xc);
  
  l = (xl/(2.0*3.1415*f))*1000000.0;
  c = (1.0/(2.0*3.1415*f*xc))*1000000000000.0;
  
  //qDebug("l %f",l);
  //qDebug("c %f",c);

  qs.sprintf("%3.2f",l);
  ll->setText(qs);
  qs.sprintf("%3.2f",c);
  lc->setText(qs);
}

void Nwt7linux::setimp(){
  if(rbr->isChecked()){
    gzr->show();
    bild1->show();
    gzlc->hide();
    bild2->hide();
  }else{
    gzr->hide();
    bild1->hide();
    gzlc->show();
    bild2->show();
  }
} 

void Nwt7linux::testLogLin(){
  if(wobdaten.linear2 or wobdaten.linear1){
    setColorBlackk1();
    setColorBlackk2();
    checkboxgrafik->setEnabled(true);
    checkbox6db->setEnabled(true);
    checkbox3db->setEnabled(true);
    checkboxinvers->setEnabled(true);
    betriebsart->setEnabled(false);
    labelbetriebsart->setEnabled(false);
    labelboxdbshift1->setEnabled(false);
    labelboxdbshift2->setEnabled(false);
    boxdbshift1->setEnabled(false);
    boxdbshift2->setEnabled(false);
    kalibrier1->setEnabled(false);
    kalibrier2->setEnabled(false);
    boxydbmax->setEnabled(false);
    boxydbmin->setEnabled(false);
    labelboxydbmax->setEnabled(false);
    labelboxydbmin->setEnabled(false);
    editdisplay->setEnabled(false);
    labeldisplayshift->setEnabled(false);
  }else{
    if(kalibrier1->isChecked())setColorRedk1();
    if(kalibrier2->isChecked())setColorRedk2();
    checkboxgrafik->setEnabled(true);
    checkbox6db->setEnabled(true);
    checkbox3db->setEnabled(true);
    checkboxinvers->setEnabled(true);
    betriebsart->setEnabled(true);
    labelbetriebsart->setEnabled(true);
    labelboxdbshift1->setEnabled(true);
    labelboxdbshift2->setEnabled(true);
    boxdbshift1->setEnabled(true);
    boxdbshift2->setEnabled(true);
    kalibrier1->setEnabled(true);
    kalibrier2->setEnabled(true);
    boxydbmax->setEnabled(true);
    boxydbmin->setEnabled(true);
    labelboxydbmax->setEnabled(true);
    labelboxydbmin->setEnabled(true);
    editdisplay->setEnabled(true);
    labeldisplayshift->setEnabled(true);
  }
}

void Nwt7linux::diagrammstart(){
  bstop->setEnabled(true);
  bstart->setEnabled(false);
  bspeichern->setEnabled(false);
  idletimer->stop();
  stimer->stop();
  vtimer->stop();
  for(int i=0; i<1024;i++){
    wobdaten.mess.k1[i]=0;
  }
  defaultlesenrtty();
  wobdaten.mess.daten_enable = true;
  emit sendewobbeldaten(wobdaten);
  picmodem->writeChar(143);
  picmodem->writeLine("m");
  tdiagramm->start(10);
  dummycounter = 50;
  counterkurve = 0;
}

void Nwt7linux::diagrammstop(){
  bstart->setEnabled(true);
  bspeichern->setEnabled(true);
  bstop->setEnabled(false);
  tdiagramm->stop();
  idletimer->start(wgrunddaten.idletime);
  wobdaten.mess.daten_enable = true;
  emit sendewobbelkurve(wobdaten.mess);
}

void Nwt7linux::diagrammspeichern(){
  QString qs;
  QFile f;
  double w;

  QString s = QFileDialog::getSaveFileName(this,
                                           tr("NWT Kurve speichern","FileDialog"),
                                           kurvendir.filePath(""),
                                           tr("CSV-Datei (*.csv)","FileDialog")
                                           );
  if (!s.isNull())
  {
    kurvendir.setPath(s);
    qs = kurvendir.absolutePath();
    //qDebug(qs);
    //Datei ueberpruefen ob Sufix vorhanden
    if((s.indexOf(".")== -1)) s += ".csv";
    f.setFileName(s);
    if(f.open( QIODevice::WriteOnly )){
      QTextStream ts(&f);
      ts << "index; Volt; dBm" << "\r\n" << flush;
      for(int i=0; i<1024; i++){
        if(wobdaten.mess.k1[i] != 0){
          w = wobdaten.mfaktor1 * wobdaten.mess.k1[i] + wobdaten.mshift1;
          qs.sprintf("%i; %f; %f\r\n", i, 5.0 / 1024 * (double) i, w);
          qs.replace('.',',');
          ts << qs;
        }
      }
    }
  }
}

void Nwt7linux::tdiagrammbehandlung(){
  unsigned char abuffer[10];
  unsigned char c1=0, c2=0;
  int i1, i2, a;
  QString qs, qsline;
  double w;


  if (bttyOpen){
    a = picmodem->readttybuffer(abuffer, 10);
    //qDebug("a = %i",a);
    if(a != 4){
      messsync++;
      picmodem->writeChar(143);
      picmodem->writeLine("m");
      tdiagramm->start(10);
      if(messsync > 100){
        diagrammstop();
        QMessageBox::warning( this, tr("Antennendiagramm","InformationsBox"),
                                    tr("Es kommen keine Daten vom NWT!","InformationsBox"));
        setCurrentIndex(0);
        return;
      }
    }else{
      messsync=0;
      c1 = abuffer[0];
      c2 = abuffer[1];
      i1 = int(c1 + c2 * 256);
      c1 = abuffer[2];
      c2 = abuffer[3];
      i2 = int(c1 + c2 * 256);
      if(i2 < 1024){
        if(dummycounter == 0){
          wobdaten.mess.k1[i2]=i1;
          w = wobdaten.mfaktor1 * i1 + wobdaten.mshift1;
          qs.sprintf("idx:%i, %0.2fV, %0.2f dBm", i2, 5.0/1024 * (double)i2, w);
        }else{
          dummycounter--;
          qs = "Warten!";
        }
      }
      lanzeige->setText(qs);
      picmodem->writeChar(143);
      picmodem->writeLine("m");
      tdiagramm->start(10);
      counterkurve++;
      if(counterkurve==50){
        emit sendewobbelkurve(wobdaten.mess);
        counterkurve = 0;
      }
    }
  }
}

void Nwt7linux::diagrammsimulieren(){
  //initialisieren
  int w;
  w = 250;
  for(int i=wobdaten.antdiagramm.adbegin; i<=wobdaten.antdiagramm.adend; i++){
    if(i % 4 == 0){
      if(i<512){w++;}else{w--;}
    }
    wobdaten.mess.k1[i] = w;
  }
  wobdaten.mess.daten_enable = true;
  emit sendewobbelkurve(wobdaten.mess);
}

void Nwt7linux::diagrammdim(int){
  int idmin = boxdbmin->currentIndex();
  int idmax = boxdbmax->currentIndex();
  switch(idmin){
  case 0: wobdaten.antdiagramm.dbmin = 10; break;
  case 1: wobdaten.antdiagramm.dbmin = 0; break;
  case 2: wobdaten.antdiagramm.dbmin = -10; break;
  case 3: wobdaten.antdiagramm.dbmin = -20; break;
  case 4: wobdaten.antdiagramm.dbmin = -30; break;
  case 5: wobdaten.antdiagramm.dbmin = -40; break;
  case 6: wobdaten.antdiagramm.dbmin = -50; break;
  case 7: wobdaten.antdiagramm.dbmin = -60; break;
  case 8: wobdaten.antdiagramm.dbmin = -70; break;
  case 9: wobdaten.antdiagramm.dbmin = -80; break;
  }
  switch(idmax){
  case 0: wobdaten.antdiagramm.dbmax = 10; break;
  case 1: wobdaten.antdiagramm.dbmax = 0; break;
  case 2: wobdaten.antdiagramm.dbmax = -10; break;
  case 3: wobdaten.antdiagramm.dbmax = -20; break;
  case 4: wobdaten.antdiagramm.dbmax = -30; break;
  case 5: wobdaten.antdiagramm.dbmax = -40; break;
  case 6: wobdaten.antdiagramm.dbmax = -50; break;
  case 7: wobdaten.antdiagramm.dbmax = -60; break;
  case 8: wobdaten.antdiagramm.dbmax = -70; break;
  case 9: wobdaten.antdiagramm.dbmax = -80; break;
  }
  wobdaten.antdiagramm.adbegin = int(qRound(spinadbegin->value()*1023.0/5.0));
  //qDebug()<<"qRound(spinadbegin->value()*1023.0/5.0: "<<qRound(spinadbegin->value()*1023.0/5.0);
  //qDebug()<<wobdaten.antdiagramm.adbegin;
  wobdaten.antdiagramm.adend = int(qRound(spinadend->value()*1023.0/5.0));
  //qDebug()<<"qRound(spinadend->value()*1023.0/5.0: "<<qRound(spinadend->value()*1023.0/5.0);
  //qDebug()<<wobdaten.antdiagramm.adend;
  wobdaten.antdiagramm.diabegin = spingradbegin->value();
  //qDebug()<<wobdaten.antdiagramm.diabegin;
  wobdaten.antdiagramm.diaend = spingradend->value();
  //qDebug()<<wobdaten.antdiagramm.diaend;

  emit sendewobbeldaten(wobdaten);
}

void Nwt7linux::diagrammdimd(double){
  diagrammdim(0);
}

void Nwt7linux::setTabEnabled(QWidget *widget, bool flag)
{
  QWidget *widgets[] = {nwt7wobbeln, wkmanager, nwt7vfo, nwt7messen, berechnung, nullptr};

  for (int i = 0; widgets[i] != nullptr; ++i)
  {
    if (widget == widgets[i])
    {
      QTabWidget::setTabEnabled(i, flag);
      return;
    }
  }

  Q_ASSERT(false);
}