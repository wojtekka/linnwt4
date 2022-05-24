/****************************************************************************
**
** Copyright (C) 2004-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
//#define	LDEBUG

#include <QtGui>
#include <QObject>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QStatusBar>

#include "mainwindow.h"


MainWindow::MainWindow()
{
  QLocale::setDefault(QLocale::German);
///////////////////////////////////////////////////////////////////////////////
// Info Version usw. 
  infoversion = "4.11.10";
  infodatum = "28.03.2016";
  infotext = "<h2>LinNWT und WinNWT</h2>"
             "<p><B>Version "+infoversion+"</B>     "+infodatum+" "
             "<p>Entwickelt unter Linux mit QT5 von Trolltech"
             "<p>(c) Andreas Lindenau DL4JAL<br>"
             "DL4JAL@darc.de<br>"
             "http://www.dl4jal.eu";
  nwt7linux = new Nwt7linux(this);
  setCentralWidget(nwt7linux);
  connect(nwt7linux, SIGNAL(setAppText(QString)), this, SLOT(setText(QString)));
  //Schriftgroesse des Programmes einstellen
  connect(nwt7linux, SIGNAL(setFontSize(int)), this, SLOT(setFontSize(int)));

  createActions();
  createToolBars();
  createStatusBar();
  createMenus();
  connect( nwt7linux, SIGNAL(setmenu(emenuenable,bool)), this,SLOT(msetmenu(emenuenable,bool)));
  setWindowIcon(QIcon(":/images/appicon.png"));
  
  setMaximumSize (2000, 2000);
//  setMinimumSize (750, 480);
  setMinimumSize (500, 480);
  menuKurven->setEnabled ( true );
  menuwobbel->setEnabled ( true );
  mStop->setEnabled ( false );
//  menuvfo->setEnabled ( false );
  menumessen->setEnabled ( false );
  mDrucken->setEnabled ( true );
  readSettings();
  createToolTip(mTooltip->isChecked());
  nwt7linux->grafiksetzen();
//  nwt7linux->menuanfangsetzen();
}

void MainWindow::setFontSize(int s)
{
  QFont font("Helvetica", s);
  this->setFont(font);
  nwt7linux->setFont(font);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  writeSettings();
  nwt7linux->beenden();
  event->accept();
}


void MainWindow::createMenus()
{
  menuDatei = menuBar()->addMenu(tr("&Datei", "Menu"));
  menuDatei->addAction(mDrucken);
  menuDatei->addAction(mDruckenpdf);
  menuDatei->addAction(mSpeichernpng);
  menuDatei->addSeparator();
  menuDatei->addAction(mBeenden);

  menuEinstellung = menuBar()->addMenu(tr("&Einstellung", "Menu"));
  menuEinstellung->addAction(mEinstellungLaden);
  menuEinstellung->addAction(mEinstellungSichern);
  menuEinstellung->addSeparator();
  menuEinstellung->addAction(mOption);
  menuEinstellung->addSeparator();
  menuEinstellung->addAction(mFirmware);
  
  //menuGeraet = menuBar()->addMenu(tr("&Geraet", "Menu"));
  //menuGeraet->addAction(mGeraetNeu);
  
  menuKurven = menuBar()->addMenu(tr("&Kurven", "Menu"));
  menuKurven->addAction(mKurvenLaden);
  menuKurven->addAction(mKurvenSichern);

  menuwobbel = menuBar()->addMenu(tr("&Wobbeln", "Menu"));
  menuwobbel->addAction(mWobbeln);
  menuwobbel->addAction(mEinmal);
  menuwobbel->addAction(mStop);
  menuwobbel->addSeparator();
  menuwobbel->addAction(mEichenK1);
  menuwobbel->addAction(mEichenK2);
  menuwobbel->addAction(mSondeLadenK1);
  menuwobbel->addAction(mSondeLadenK2);
  menuwobbel->addAction(mSondeSpeichernK1);
  menuwobbel->addAction(mSondeSpeichernK2);
  menuwobbel->addSeparator();
  menuwobbel->addAction(mFontInfo);
  menuwobbel->addAction(mFrequenzmarke);
  menuwobbel->addAction(mHLine);
  menuwobbel->addAction(mProfile);
  menuwobbel->addSeparator();
  menuwobbel->addAction(mBreite);

//   = menuBar()->addMenu(tr("&VFO", "Menu"));
  
  menumessen = menuBar()->addMenu(tr("&Messen", "Menu"));
  menumessen->addAction(mWattFont);
  menumessen->addAction(mWattEichenK1);
  menumessen->addAction(mWattEichenK2);
  menumessen->addSeparator();
  menumessen->addAction(mWattSchreiben);
  menumessen->addAction(mTabelleSpeichern);
  menumessen->addSeparator();
  menumessen->addAction(mDaempfungdefault);
  menumessen->addSeparator();
  menumessen->addAction(mWattEditSonde1);
  menumessen->addAction(mWattEditSonde2);

  menuhilfe = menuBar()->addMenu(tr("&Hilfe", "Menu"));
  menuhilfe->addAction(mTooltip);
  menuhilfe->addSeparator();
  menuhilfe->addAction(mInfo);
  menuhilfe->addAction(mVersion);
}

void MainWindow::createActions()
{
  mDrucken = new QAction(QIcon(":/images/drucken.png"), tr("&Drucken","Menue"), this);
  mDrucken->setStatusTip(tr("Ausdrucken des Wobbelfensters", "Statustip"));
  connect(mDrucken, SIGNAL(triggered()), this, SLOT(druckediagramm()));

  mDruckenpdf = new QAction(QIcon(":/images/druckenpdf.png"), tr("Drucken als PDF","Menu"), this);
  mDruckenpdf->setStatusTip(tr("Ausdrucken des Wobbelfensters in PDF-Datei", "Statustip"));
  connect(mDruckenpdf, SIGNAL(triggered()), this, SLOT(druckediagrammpdf()));

  mSpeichernpng = new QAction(QIcon(":/images/savebild.png"), tr("Speichern als Bild","Menu"), this);
  mSpeichernpng->setStatusTip(tr("Speichern des Wobbelfensters als Bild-Datei", "Statustip"));
  connect(mSpeichernpng, SIGNAL(triggered()), this, SLOT(bildspeichern()));

  mBeenden = new QAction(QIcon(":/images/beenden.png"), tr("&Beenden","Menu"), this);
  mBeenden->setShortcut(Qt::CTRL + Qt::Key_Q);
  mBeenden->setStatusTip(tr("Beenden des Programmes und Speichern der Konfiguration", "Statustip"));
  connect(mBeenden, SIGNAL(triggered()), this, SLOT(beenden()));

  mEinstellungLaden = new QAction(tr("&Laden","Menu"), this);
  mEinstellungLaden->setStatusTip(tr("Laden einer neuen Programmkonfiguration", "Statustip"));
  connect(mEinstellungLaden, SIGNAL(triggered()), this, SLOT(EinstellungLaden()));

  mEinstellungSichern = new QAction(tr("&Sichern","Menu"), this);
  mEinstellungSichern->setStatusTip(tr("Sichern der Programmkonfiguration in eine Datei", "Statustip"));
  connect(mEinstellungSichern, SIGNAL(triggered()), this, SLOT(EinstellungSichern()));

  mGeraetNeu = new QAction(tr("&Neues Geraet","Menu"), this);
  mGeraetNeu->setStatusTip(tr("Neuen Geraetepfad anlegen", "Statustip"));
  connect(mGeraetNeu, SIGNAL(triggered()), this, SLOT(geraet_neu()));
  
  mKurvenLaden = new QAction(QIcon(":/images/open.png"), tr("Wobbelkurve &laden","Menu"), this);
  mKurvenLaden->setStatusTip(tr("Laden einer abgespeicherten Wobbeldatei", "Statustip"));
  connect(mKurvenLaden, SIGNAL(triggered()), this, SLOT(KurvenLaden()));

  mKurvenSichern = new QAction(QIcon(":/images/save.png"),tr("Wobbelkurve &sichern","Menu"), this);
  mKurvenSichern->setStatusTip(tr("Abspeichern des Wobbelergebnisses in eine Datei", "Statustip"));
  connect(mKurvenSichern, SIGNAL(triggered()), this, SLOT(KurvenSichern()));
  
  mInfo = new QAction(QIcon(":/images/info.png"), tr("&Info","Menu"), this);
  mInfo->setStatusTip(tr("Informationen zur Programmversion", "Statustip"));
  connect(mInfo, SIGNAL(triggered()), this, SLOT(Info()));

  mVersion = new QAction(tr("&Firmware Version","Menu"), this);
  mVersion->setStatusTip(tr("Informationen zur Firwareversion", "Statustip"));
  connect(mVersion, SIGNAL(triggered()), this, SLOT(version()));

  mFirmware = new QAction(tr("&Firmware Update","Menu"), this);
  mFirmware->setStatusTip(tr("Neue Firmware in die Baugruppe laden", "Statustip"));
  connect(mFirmware, SIGNAL(triggered()), this, SLOT(firmupdate()));

  mOption = new QAction(QIcon(":/images/option.png"), tr("&Option","Menu"), this);
  mOption->setStatusTip(tr("Einstellung verschiedener Programmoptionen", "Statustip"));
  connect(mOption, SIGNAL(triggered()), this, SLOT(option()));

  mWobbeln = new QAction(tr("Wobbeln","Menu"), this);
  mWobbeln->setStatusTip(tr("Durchgehend Wobbeln", "Statustip"));
  connect(mWobbeln, SIGNAL(triggered()), this, SLOT(clickwobbeln()));

  mEinmal = new QAction(tr("Einmal","Menu"), this);
  mEinmal->setStatusTip(tr("Nur einen Wobbeldurchlauf", "Statustip"));
  connect(mEinmal, SIGNAL(triggered()), this, SLOT(clickwobbelneinmal()));

  mStop = new QAction(tr("Stop","Menu"), this);
  mStop->setStatusTip(tr("Stoppen des Wobbelns", "Statustip"));
  connect(mStop, SIGNAL(triggered()), this, SLOT(clickwobbelnstop()));

  mEichenK1 = new QAction(tr("Kalibrieren Kanal1","Menu"), this);
  mEichenK1->setStatusTip(tr("Kalibrieren Kanal1 (Eingenschaften der Messsonde ermitteln)", "Statustip"));
  connect(mEichenK1, SIGNAL(triggered()), this, SLOT(kalibrierenkanal1()));

  mEichenK2 = new QAction(tr("Kalibrieren Kanal2","Menu"), this);
  mEichenK2->setStatusTip(tr("Kalibrieren Kanal2 (Eingenschaften der Messsonde ermitteln)", "Statustip"));
  connect(mEichenK2, SIGNAL(triggered()), this, SLOT(kalibrierenkanal2()));

  mSondeLadenK1 = new QAction(QIcon(":/images/openk1.png"), tr("Auswahl Messsonde Kanal1","Menu"), this);
  mSondeLadenK1->setStatusTip(tr("Eigenschaften der Messsonde laden fuer Kanal 1", "Statustip"));
  connect(mSondeLadenK1, SIGNAL(triggered()), this, SLOT(messsondeladen1()));

  mSondeLadenK2 = new QAction(QIcon(":/images/openk2.png"), tr("Auswahl Messsonde Kanal2","Menu"), this);
  mSondeLadenK2->setStatusTip(tr("Eigenschaften der Messsonde laden fuer Kanal 2", "Statustip"));
  connect(mSondeLadenK2, SIGNAL(triggered()), this, SLOT(messsondeladen2()));

  mSondeSpeichernK1 = new QAction(tr("Eigenschaften Messsonde Kanal 1 speichern","Menu"), this);
  mSondeSpeichernK1->setStatusTip(tr("Abspeichern der Eigenschaften der Messsonde Kanal1 in eine Datei", "Statustip"));
  connect(mSondeSpeichernK1, SIGNAL(triggered()), this, SLOT(messsondespeichern1()));

  mSondeSpeichernK2 = new QAction(tr("Eigenschaften Messsonde Kanal 2 speichern","Menu"), this);
  mSondeSpeichernK2->setStatusTip(tr("Abspeichern der Eigenschaften der Messsonde Kanal2 in eine Datei", "Statustip"));
  connect(mSondeSpeichernK2, SIGNAL(triggered()), this, SLOT(messsondespeichern2()));

  mFontInfo = new QAction(tr("Schrift im Infofenster einstellen","Menu"), this);
  mFontInfo->setStatusTip(tr("Einstellen der Schrift im Infofenster", "Statustip"));
  connect(mFontInfo, SIGNAL(triggered()), this, SLOT(infosetfont()));

  mFrequenzmarke = new QAction(QIcon(":/images/fmarken.png"), tr("Frequenzmarken","Menu"), this);
  mFrequenzmarke->setStatusTip(tr("Frequenzmarken im Wobbelfenster einblenden", "Statustip"));
  connect(mFrequenzmarke, SIGNAL(triggered()), this, SLOT(setFrequenzmarken()));
  
  mHLine = new QAction(QIcon(":/images/hline.png"), tr("dB-Line","Menu"), this);
  mHLine->setStatusTip(tr("Eine zusaetzliche dB-Line im Wobbelfenster einblenden", "Statustip"));
  connect(mHLine, SIGNAL(triggered()), this, SLOT(setHLine()));
  
  mProfile = new QAction(QIcon(":/images/profil.png"), tr("Profile","Menu"), this);
  mProfile->setStatusTip(tr("Profile editieren", "Statustip"));
  connect(mProfile, SIGNAL(triggered()), this, SLOT(editProfile()));
  
  mBreite = new QAction(QIcon(":/images/breite.png"), tr("Fensterbreite aendern","Menu"), this);
  mBreite->setStatusTip(tr("Fensterbreite des Hauptfensters veraendern", "Statustip"));
  connect(mBreite, SIGNAL(triggered()), this, SLOT(setBreite()));

  mWattFont = new QAction(tr("Schrift einstellen","Menu"), this);
  mWattFont->setStatusTip(tr("Schriftgroesse einstellen", "Statustip"));
  connect(mWattFont, SIGNAL(triggered()), this, SLOT(messsetfont()));

  mWattEichenK1 = new QAction(tr("Kalibrieren Kanal 1","Menu"), this);
  mWattEichenK1->setStatusTip(tr("Kalibrieren des Wattmeters", "Statustip"));
  connect(mWattEichenK1, SIGNAL(triggered()), this, SLOT(mkalibrierenk1()));

  mWattEichenK2 = new QAction(tr("Kalibrieren Kanal 2","Menu"), this);
  mWattEichenK2->setStatusTip(tr("Kalibrieren des Wattmeters", "Statustip"));
  connect(mWattEichenK2, SIGNAL(triggered()), this, SLOT(mkalibrierenk2()));

  mWattSchreiben = new QAction(tr("In Tabelle schreiben","Menu"), this);
  mWattSchreiben->setStatusTip(tr("Messwerte in die Tabelle schreiben", "Statustip"));
  connect(mWattSchreiben, SIGNAL(triggered()), this, SLOT(tabelleschreiben()));

  mWattEditSonde1 = new QAction(tr("Editieren Sonde 1","Menu"), this);
  mWattEditSonde1->setStatusTip(tr("Editieren der Sondendaten", "Statustip"));
  connect(mWattEditSonde1, SIGNAL(triggered()), this, SLOT(editsonde1()));
  
  mWattEditSonde2 = new QAction(tr("Editieren Sonde 2","Menu"), this);
  mWattEditSonde2->setStatusTip(tr("Editieren der Sondendaten", "Statustip"));
  connect(mWattEditSonde2, SIGNAL(triggered()), this, SLOT(editsonde2()));
  
  mTabelleSpeichern = new QAction(tr("Tabelle abspeichern","Menu"), this);
  mTabelleSpeichern->setStatusTip(tr("Tabelle als Textdatei abspeichern", "Statustip"));
  connect(mTabelleSpeichern, SIGNAL(triggered()), this, SLOT(tabellespeichern()));

  mDaempfungdefault = new QAction(tr("Daempfung auf default setzen","Menu"), this);
  mDaempfungdefault->setStatusTip(tr("Alle zusaetzlichen Eintraege in der ComboBox loeschen", "Statustip"));
  connect(mDaempfungdefault, SIGNAL(triggered()), this, SLOT(setDaempfungdefault()));
  
  mTooltip = new QAction(tr("Tip?","Menu"), this);
  mTooltip->setStatusTip(tr("Hilfe an der Maus ein/aus-belenden", "Statustip"));
  mTooltip->setCheckable(true);
  connect(mTooltip, SIGNAL(triggered()), this, SLOT(tooltip()));

}

void MainWindow::createToolBars()
{
  dateiToolBar = addToolBar("ToolBarsDatei");
  dateiToolBar->addAction(mBeenden);
  dateiToolBar->addAction(mDrucken);
  dateiToolBar->addAction(mDruckenpdf);
  dateiToolBar->addAction(mSpeichernpng);

  einstellungToolBar = addToolBar("ToolBarsEinstellung");
  einstellungToolBar->addAction(mOption);
  einstellungToolBar->addAction(mFrequenzmarke);
  einstellungToolBar->addAction(mHLine);
  einstellungToolBar->addAction(mProfile);
  einstellungToolBar->addAction(mInfo);

  sondenToolBar = addToolBar("ToolBarsSonden");
  sondenToolBar->addAction(mSondeLadenK1);
  sondenToolBar->addAction(mSondeLadenK2);
  
  kurvenToolBar = addToolBar("ToolBarsKurven");
  kurvenToolBar->addAction(mKurvenLaden);
  kurvenToolBar->addAction(mKurvenSichern);
  kurvenToolBar->addAction(mBreite);
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage("Ready");
}

void MainWindow::createToolTip(bool btip){
  nwt7linux->tip(btip);
}

void MainWindow::tooltip(){
  writeSettings();
  createToolTip(mTooltip->isChecked());
}

void MainWindow::loadConfig(char *name)
{
  QString s = name;  
  //qDebug("MainWindow::loadConfig(char *name)");
  nwt7linux->setconffilename(s);
}

void MainWindow::loadGeraet(char *name)
{
  QString s = name;  
  //qDebug("MainWindow::loadGeraet(char *name)");
  
/*  
  QDir dir=homedir;
  dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
  dir.setSorting(QDir::Size | QDir::Reversed);

  QFileInfoList list = dir.entryInfoList();
  std::cout << "     Bytes Filename" << std::endl;
  for (int i = 0; i < list.size(); ++i) {
      QFileInfo fileInfo = list.at(i);
      std::cout << qPrintable(QString("%1 %2").arg(fileInfo.size(), 10)
                                              .arg(fileInfo.fileName()));
      std::cout << std::endl;
  }
*/  
  nwt7linux->setconffilename(s);
}

void MainWindow::geraet_neu(){
}

void MainWindow::readSettings()
{
  QSettings settings("AFU", "NWT");
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(750, 480)).toSize();
  resize(size);
  move(pos);
  mTooltip->setChecked(settings.value("tooltip", true).toBool());
  nwt7linux->readSettings();
  if(size.rwidth() < 750){
    bbreite = true;
  }else{
    bbreite = false;
  }
}

void MainWindow::setBreite()
{
  int breite = this->width();
  int hoehe = this->height();
  if(bbreite){
    bbreite = false;
    breite = 750;
  }else{
    bbreite = true;
    breite = 500;
  }
  resize(breite, hoehe);
}

void MainWindow::writeSettings()
{
  QSettings settings("AFU", "NWT");
  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.setValue("tooltip", mTooltip->isChecked());
  nwt7linux->writeSettings();
}

void MainWindow::druckediagramm(){
  QSize size = this->size();
//  resize(750,600);
  nwt7linux->druckediagramm();
  resize(size);
}

void MainWindow::druckediagrammpdf(){
  QSize size = this->size();
//  resize(750,600);
  nwt7linux->druckediagrammpdf();
  resize(size);
}

void MainWindow::bildspeichern(){
  nwt7linux->wobbelbildspeichern();
}

void MainWindow::zaehlen(){
  nwt7linux->fzaehlen();
}

void MainWindow::EinstellungLaden(){
  nwt7linux->EinstellungLaden();
}

void MainWindow::EinstellungSichern(){
  nwt7linux->EinstellungSichern();
}

void MainWindow::KurvenLaden(){
  nwt7linux->KurvenLaden();
}

void MainWindow::KurvenSichern(){
  nwt7linux->MenuKurvenSichern();
}

void MainWindow::Info()
{
#ifdef Q_WS_WIN
  QMessageBox::about(this, "WinNWT4", infotext);
#else
  QMessageBox::about(this, "LinNWT4", infotext);
#endif
}

void MainWindow::option(){
  nwt7linux->option();
}

void MainWindow::clickwobbeln(){
  nwt7linux->clickwobbeln();
}

void MainWindow::clickwobbelneinmal(){
  nwt7linux->clickwobbelneinmal();
}

void MainWindow::clickwobbelnstop(){
  nwt7linux->clickwobbelnstop();
}

void MainWindow::kalibrierenkanal1(){
  nwt7linux->kalibrierenkanal1();
}

void MainWindow::kalibrierenkanal2(){
  nwt7linux->kalibrierenkanal2();
}

void MainWindow::messsondeladen1(){
  nwt7linux->messsondeladen1();
}

void MainWindow::messsondeladen2(){
  nwt7linux->messsondeladen2();
}

void MainWindow::messsondespeichern1(){
  nwt7linux->messsondespeichern1("def");
}

void MainWindow::messsondespeichern2(){
  nwt7linux->messsondespeichern2("def");
}

void MainWindow::infosetfont(){
  nwt7linux->infosetfont();
}

void MainWindow::setFrequenzmarken(){
  nwt7linux->setFrequenzmarken();
}

void MainWindow::setHLine(){
  nwt7linux->setHLine();
}

void MainWindow::editProfile(){
  nwt7linux->editProfile();
}

void MainWindow::messsetfont(){
  nwt7linux->messsetfont();
}

void MainWindow::mkalibrierenk1(){
  nwt7linux->mkalibrierenk1();
}

void MainWindow::mkalibrierenk2(){
  nwt7linux->mkalibrierenk2();
}

void MainWindow::editsonde1(){
  nwt7linux->editsonde1();
}

void MainWindow::editsonde2(){
  nwt7linux->editsonde2();
}

void MainWindow::version(){
  nwt7linux->version();
}

void MainWindow::tabelleschreiben(){
  nwt7linux->tabelleschreiben();
}

void MainWindow::tabellespeichern(){
  nwt7linux->tabellespeichern();
}

void MainWindow::setDaempfungdefault(){
  nwt7linux->setDaempfungdefault();
}

void MainWindow::beenden()
{
  nwt7linux->beenden();
  close();
}

void MainWindow::setText(QString stty){
  QString qs;
  #ifdef Q_WS_WIN
  qs = "WinNWT4";
  #else
  qs = "LinNWT4";
  #endif
  
  setWindowTitle(tr("%1 - V.%2 - %3","nicht uebersetzen").arg(qs).arg(infoversion).arg(stty));
}

void MainWindow::firmupdate(){
  nwt7linux->firmupdate();
}

void MainWindow::msetmenu(emenuenable menuenable, bool einaus){
  switch(menuenable){
    case emDrucken: mDrucken->setEnabled(einaus);break;
    case emDruckenpdf: mDruckenpdf->setEnabled(einaus);break;
    case emSpeichernpng: mSpeichernpng->setEnabled(einaus);break;
    case emBeenden: mBeenden->setEnabled(einaus);break;
    case emEinstellungLaden: mEinstellungLaden->setEnabled(einaus);break;
    case emEinstellungSichern: mEinstellungSichern->setEnabled(einaus);break;
    case emInfo: mInfo->setEnabled(einaus);break;
    case emOption: mOption->setEnabled(einaus);break;
    case emFirmware: mFirmware->setEnabled(einaus);break;
    case emKurvenLaden: mKurvenLaden->setEnabled(einaus);break;
    case emKurvenSichern: mKurvenSichern->setEnabled(einaus);break;
    case emWobbeln: mWobbeln->setEnabled(einaus);break;
    case emEinmal: mEinmal->setEnabled(einaus);break;
    case emStop: mStop->setEnabled(einaus);break;
    case emEichenK1: mEichenK1->setEnabled(einaus);break;
    case emEichenK2: mEichenK2->setEnabled(einaus);break;
    case emSondeLadenK1: mSondeLadenK1->setEnabled(einaus);break;
    case emSondeLadenK2: mSondeLadenK2->setEnabled(einaus);break;
    case emSondeSpeichernK1: mSondeSpeichernK1->setEnabled(einaus);break;
    case emSondeSpeichernK2: mSondeSpeichernK2->setEnabled(einaus);break;
    case emFontInfo: mFontInfo->setEnabled(einaus);break;
    case emFrequenzmarke: mFrequenzmarke->setEnabled(einaus);break;
    case emWattFont: mWattFont->setEnabled(einaus);break;
    case emWattEichenK1: mWattEichenK1->setEnabled(einaus);break;
    case emWattEichenK2: mWattEichenK2->setEnabled(einaus);break;
    case emWattEditSonde1: mWattEditSonde1->setEnabled(einaus);break;
    case emWattEditSonde2: mWattEditSonde2->setEnabled(einaus);break;
    case emWattSchreiben: mWattSchreiben->setEnabled(einaus);break;
    case emTabelleSpeichern: mTabelleSpeichern->setEnabled(einaus);break;
    case emDaempfungdefault: mDaempfungdefault->setEnabled(einaus);break;
    case emVersion: mVersion->setEnabled(einaus);break;
    case emTooltip: mTooltip->setEnabled(einaus);break;
    case emmenuDatei: menuDatei->setEnabled(einaus);break;
    case emmenuEinstellung: menuEinstellung->setEnabled(einaus);break;
    case emmenuKurven: menuKurven->setEnabled(einaus);break;
    case emmenuwobbel: menuwobbel->setEnabled(einaus);break;
//    case emmenuvfo: menuvfo->setEnabled(einaus);break;
    case emmenumessen: menumessen->setEnabled(einaus);break;
    case emmainhidde: showMinimized ();break;
    case emmainshow: showNormal ();break;
  }
}  


void MainWindow::paintEvent(QPaintEvent*){
#ifdef LDEBUG
  qDebug("MainWindow::paintEvent");
#endif
  nwt7linux->grafiksetzen();
}

void MainWindow::resizeEvent( QResizeEvent * ){
  //nach Groessenaenderung grafische Elemente anpassen
#ifdef LDEBUG
  qDebug("MainWindow::resizeEvent");
  qDebug("breite %i hoehe %i", width(), height());
#endif
  resize(this->rect().size());
  nwt7linux->grafiksetzen();
}

void MainWindow::setProgramPath(QString s){
  //qDebug("MainWindow::setProgramPath(QString s)");
  programpath = s;
  nwt7linux->setProgramPath(programpath);
}
