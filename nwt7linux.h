/***************************************************************************
                          nwt7linux.h  -  description
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

//#define LDEBUG

#ifndef NWT7LINUX_H
#define NWT7LINUX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QTabWidget>
#include <QDial>
#include <QLCDNumber>
#include <QRadioButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QFileDialog>
#include <QPainter>
#include <QPixmap>
#include <QCheckBox>
#include <QObject>
#include <QProgressBar>
#include <QDialog>
#include <Q3MultiLineEdit>
#include <QFile>
#include <QFontDialog>
#include <QString>
#include <QDir>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef Q_WS_WIN
#include "win_modem.h"
#else
#include "lin_modem.h"
#endif

#include "widgetwobbeln.h"
#include "optiondlg.h"
#include "fmarkedlg.h"
#include "firmwidget.h"
#include "wkmanager.h"

#include "konstdef.h"

class QLabel;
class QMenuBar;


/** Nwt7linux is the base class of the project */
class Nwt7linux : public QTabWidget
{
  Q_OBJECT 
  public:
    /** construtor */
    Nwt7linux(QWidget* parent=0);
    /** destructor */
    ~Nwt7linux();

    Modem *picmodem;

//////////////////////////////////////////////////////////////////////////////
//Objecte fuer Messen
    QWidget *nwt7messen;
    QWidget *wgrafik;
    QProgressBar *progressbarmessk1;
    QProgressBar *progressbarmessk2;
    QTimer *messtimer;
    QLabel *messlabel1;
    QLabel *messlabel2;
    QLabel *messlabel3;
    QLabel *messlabel4;
    QLabel *messlabel5;
    QLabel *messlabel6;
    Q3MultiLineEdit *messedit;
    QPushButton *buttonmess;
    QPushButton *buttonmesssave;
    QComboBox *ldaempfungk1;
    QComboBox *ldaempfungk2;
    QLabel *labelldaempfungk1;
    QLabel *labelldaempfungk2;
    
    QPushButton *buttonvfo;
    QSpinBox *sp1hz;
    QSpinBox *sp10hz;
    QSpinBox *sp100hz;
    QSpinBox *sp1khz;
    QSpinBox *sp10khz;
    QSpinBox *sp100khz;
    QSpinBox *sp1mhz;
    QSpinBox *sp10mhz;
    QSpinBox *sp100mhz;
    QSpinBox *sp1ghz;
    QLabel *labelsphz;
    QLabel *labelspkhz;
    QLabel *labelspmhz;
//    QLabel *labelspghz;
    
//////////////////////////////////////////////////////////////////////////////
//Objecte fuer Wobbeln
    owkmanager *wkmanager;
    QWidget *nwt7wobbeln;
    WidgetWobbeln *grafik;
    
    QGroupBox *groupkanal,
              *groupbandbreite,
              *groupshift,
              *groupbetriebsart,
              *groupwobbel,
              *groupdaempfung,
              *groupzoom,
              *groupkursor,
              *groupbar,
              *groupsa;
    
    QLineEdit *editanfang,
              *editende,
              *editschrittweite,
              *editschritte,
              *editdisplay,
              *editvfo;

    QLabel *labelanfang,
           *labelende,
           *labelschrittweite,
           *labelschritte,
           *labeldisplayshift,
           *labellupe,
           *labelprogressbar,
           *labelnwt,
           *labelvfo,
           *labelsafrqbereich,
           *labelsabandbreite,
           *labelfrqfaktor,
           *labelfrqfaktorv,
           *labelfrqfaktorm;

    QRadioButton *sabereich1;
    QRadioButton *sabereich2;

    QCheckBox *checkboxk1,
              *checkboxk2,
              *checkbox3db,
              *checkbox6db,
              *checkboxgrafik,
              *checkboxinvers,
              //*checkboxtime,
              *checkboxmesshang1,
              *checkboxmesshang2;
    
    QCheckBox *kalibrier1;
    QCheckBox *kalibrier2;

    QPushButton *buttonwobbeln,
                *buttoneinmal,
                *buttonstop,
                *buttonlupeminus,
                *buttonlupeplus,
                *buttonlupemitte;

    QTimer *wobbeltimer;
    QTimer *idletimer;
    QTimer *vtimer;
    QTimer *stimer;
    QTimer *messvfotimer;
    
    
    QProgressBar *progressbar;
    
    QTextEdit *mledit;
    
    QStringList mleditlist;
    QComboBox *daempfung1,
              *daempfung2,
              *daempfung3,
              *betriebsart,
              *aufloesung,
              *boxdbshift1,
              *boxdbshift2,
              *boxkursornr,
              *boxwattoffset1,
              *boxwattoffset2,
              *boxprofil,
              *boxydbmax,
              *boxydbmin,
              *boxzwischenzeit;
    
    QLabel *labelboxydbmax;
    QLabel *labelboxydbmin;
    QLabel *labelboxdbshift1;
    QLabel *labelboxdbshift2;
    QLabel *labelkursornr;
    QLabel *labeldaempfung;
    QLabel *labeldaempfung1;
    QLabel *labeldaempfung3;
    QLabel *labelbetriebsart;
    QLabel *labelaufloesung;
    QLabel *labelprofil;
    QLabel *labelzwischenzeit;
    enumbetriebsart ebetriebsart; //Betriebart wobbeln,swr,swrneu ....

    QLabel *labela_100;
    QLineEdit *edita_100;
    QLabel *labelkabellaenge;
    QLineEdit *editkabellaenge;

//////////////////////////////////////////////////////////////////////////////
//Objecte fuer VFO
    QWidget *nwt7vfo;
    QTimer *vfotimer;
    
    QSpinBox *vsp1hz;
    QSpinBox *vsp10hz;
    QSpinBox *vsp100hz;
    QSpinBox *vsp1khz;
    QSpinBox *vsp10khz;
    QSpinBox *vsp100khz;
    QSpinBox *vsp1mhz;
    QSpinBox *vsp10mhz;
    QSpinBox *vsp100mhz;
    QSpinBox *vsp1ghz;
    
    QLCDNumber *LCD1;
    QLCDNumber *LCD2;
    QLCDNumber *LCD3;
    QLCDNumber *LCD4;
    QLCDNumber *LCD5;
    QRadioButton *rb1;
    QRadioButton *rb2;
    QRadioButton *rb3;
    QRadioButton *rb4;
    QRadioButton *rb5;
    QLineEdit *editzf;
    QCheckBox *checkboxzf;
    QCheckBox *checkboxiqvfo;
    QLabel *labelzf;
    QLabel *labelhz;
    QLabel *lmhz;
    QLabel *lkhz;
    QLabel *lhz;
    QLabel *mlabelk1;
    QLabel *mlabelk2;
    QFileDialog *IniFileDialog;
//    optiondlg *dlg;
//////////////////////////////////////////////////////////////////////////////
//Objecte fuer Berechnungen
    QWidget *berechnung;
    
    QGroupBox *groupschwingkreisl;
    QLineEdit *editf1;
    QLineEdit *editc1;
    QLabel *ergebnisl;
    QLabel *labell1;
    QLabel *labelf1;
    QLabel *labelc1;
    QLineEdit *editn;
    QLabel *labeln;
    QLabel *ergebnisal;
    QLabel *labelal;
    
    QGroupBox *groupschwingkreisc;
    QLineEdit *editf2;
    QLineEdit *editl2;
    QLabel *ergebnisc;
    QLabel *labell2;
    QLabel *labelf2;
    QLabel *labelc2;
    QLabel *labelxc;
    QLabel *ergebnisxc;
    QLabel *labelxl;
    QLabel *ergebnisxl;
    
    QGroupBox *groupwindungen;
    QLineEdit *edital3;
    QLabel *labeledital3;
    QLineEdit *editl3;
    QLabel *labeleditl3;
    QLabel *ergebnisw;
    QLabel *labelergebnisw;

//Objecte fuer Berechnungen
//////////////////////////////////////////////////////////////////////////////
    
//////////////////////////////////////////////////////////////////////////////
//Objecte fuer Impedanz
    QWidget *wimpedanz;
    
    QGroupBox *gimp;
    QRadioButton *rbr;
    QRadioButton *rblc;
    
    QGroupBox *gzr;
    QLabel *labeleditz1;
    QLineEdit *editz1;
    QLabel *labeleditz2;
    QLineEdit *editz2;
    QLabel *labeleditz3;
    QLineEdit *editz3;
    QLabel *labeleditz4;
    QLineEdit *editz4;
    QLabel *lr1;
    QLabel *lbeschrr1;
    QLabel *lr2;
    QLabel *lbeschrr2;
    QLabel *lr3;
    QLabel *lbeschrr3;
    QLabel *lr4;
    QLabel *lbeschrr4;
    QLabel *ldaempfung;
    QLabel *lbeschrdaempfung;
    QLabel *bild1;
    
    QGroupBox *gzlc;
    QLabel *labeleditzlc1;
    QLineEdit *editzlc1;
    QLabel *labeleditzlc2;
    QLineEdit *editzlc2;
    QLabel *labeleditzlc3;
    QLineEdit *editzlc3;
    QLabel *ll;
    QLabel *lbeschrl;
    QLabel *lc;
    QLabel *lbeschrc;
    QLabel *bild2;
  //Objecte fuer Impedanz Ende
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  //Objecte fuer Antennendiagramm
    QWidget *wdiagramm;
    QGroupBox *gdiagramm;
    QPushButton *bstart;
    QPushButton *bstop;
    QPushButton *bspeichern;
    QPushButton *bsim;
    QTimer *tdiagramm;
    QLabel *lanzeige;

    QGroupBox *gdiagrammdim;
    QComboBox *boxdbmax;
    QComboBox *boxdbmin;
    QLabel *ldbmax;
    QLabel *ldbmin;
    QDoubleSpinBox *spinadbegin;
    QDoubleSpinBox *spinadend;
    QSpinBox *spingradbegin;
    QSpinBox *spingradend;
    QLabel *lbegin;
    QLabel *lbegin1;
    QLabel *lend;
    QLabel *lend1;
  //Objecte fuer Antennendiagramm Ende
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  //interne Funktion zum errechnen und setzen der Positionen
    
    void grafiksetzen();
    void tip(bool);
    QString getconffilename();
    void setconffilename(const QString &fname);
    TWattOffset wattoffsetk1[30];
    TWattOffset wattoffsetk2[30];
    TFprofil aprofil[100];
    int profilindex;
    void readSettings();
    void writeSettings();
    void setparkfrq();
    void setProgramPath(const QString &);
    void setFrqFaktorLabel();
  
  public slots:
    void setimp();
    void zlcausrechnen();
    void zrausrechnen();
    void nkurve_loeschen();
    void wkmloadkurve(int);
    void wkmsavekurve(const TWobbel &);
    void menuanfangsetzen();
    void EinstellungLaden();
    void EinstellungSichern();
    void KurvenLaden();
    void KurvenSichern();
    void MenuKurvenSichern();
    //void drucklabel();
    void druckediagramm();
    void druckediagrammpdf();
    void wobbelbildspeichern();
    void option();
    void firmupdate();
    void beenden();
    void version();
    void versionskontrolle();
    void setAText(QString);
    void setShift();
    void setIQVFO();

    void clickwobbeln(); //Wobbeln starten
    void clickwobbelneinmal(); //Einen Durchlauf wobbeln
    void clickwobbelnstop(); //Wobbeln stoppen
//    void kalibrierenswr();
    void kalibrierenkanal1();
    void kalibrierenkanal2();
    void messsondeladen1();
    void messsondeladen2();
    void messsondespeichern1(const QString&);
    void messsondespeichern2(const QString&);
    void infosetfont();
    void setFrequenzmarken();
    void editProfile();
    void setHLine();
    void openKurvenManager();
    void getwkm(int);
    void resetKursor(int);
    
    void messsetfont();
    void mkalibrierenk1();
    void mkalibrierenk2();
    void editsonde1();
    void editsonde2();
    void tabelleschreiben();
    void tabellespeichern();
    void setDaempfungdefault();
    void mleditloeschen();
    void mleditinsert(const QString &);

//Funktionen fuer VFO
    void setDaempfungItem();

//Funktionen Berechnungen

    void fzaehlen();

  private:
    QString filenamesonde1;
    QString filenamesonde2;
    esabandwith sabw;
    esabereich sabereich, sabereichalt;
    int fwversion;
    int fwversioncounter;
    int fwportb;
    int fwportbalt;
    int fwportbcounter;
    int fwsav;
    int fwvariante;
    bool vabfrage;
    int pfontsize;
    long anfangalt;
    long endealt;
    int schrittealt;
    bool sw305;
    bool sw410;
    bool bsetdaempfung;
    bool wobbelabbruch;
    bool binfokeine;
    //bool bfliessend;
    
    double woffset1, woffset2;
    bool btip;
    TFrqmarken frqmarken;
    QString configfilename;
    TWobbel wobdatenmerk;
    bool bkurvegeladen;
    bool bdauerwobbeln;
    QDir homedir;
    QDir kurvendir;

    QString infotext;
    QString displaystr;
    unsigned char linienr;
    bool bkalibrierenswr;
    bool bkalibrierenswrneu;
    bool bkalibrieren40;
    bool bkalibrieren0;
    double ym_40db;
    double ym_0db;
    double ym_gen;
    double ym_daempf;
    double KursorFrequenz;
    bool wobbelstop;
    bool bttyOpen;
    int lcdauswahl;
    QMenuBar *menu;
    TWobbel wobdaten;
    char ttybuffer[maxmesspunkte * 4];
    int ttyindex;
    bool b10bit;
    int synclevel;
    int abbruchlevel;
    bool bersterstart;
    bool bmessen;
    bool bmkalibrierenk1_0db;
    bool bmkalibrierenk1_20db;
    bool bmkalibrierenk2_0db;
    bool bmkalibrierenk2_20db;
    QFont messfont;
    QFont infofont;
    QFont printfont;
    bool btabelleschreiben;
    QStringList bstrlist;
    QStringList infoueber;
    TGrunddaten wgrunddaten;
    double messungk1, messungk2;
    double messungk1alt, messungk2alt;
    int messcounter;
    int messsync;
    bool bdatenruecklauf;
    int messtime;
    int messtimeneu; //ab FW 120
    double maxkalibrier;
    int wcounter;
    double messvfofrequenz;
    bool bmessvfo;
    int counterspitzek1;
    int counterspitzek2;
    double lcdfrq1;
    double lcdfrq2;
    double lcdfrq3;
    double lcdfrq4;
    double lcdfrq5;
    TBildinfo nbildinfo;
    TDiagrammarray diagrammarray;
    int dummycounter;
    int counterkurve;

    
    void testLogLin();
    void wobbelungueltig();
    QString delPath(const QDir &s);
    void configurationladen(const QString &filename);
    void configurationspeichern(const QString &filename);
    void messsondenfilek1laden(const QString &filename);
    void messsondenfilek2laden(const QString &filename);
    enumkurvenversion loadkurve();
    void setmessen();
    double getdrehgeber();
    void setdrehgeber(double wert);
    void setdrehgebervonlcd();
    void defaultlesenrtty();
    double linenormalisieren(const QString &);
    void sendwobbeln();   //wobbelstring an NWT7 senden
    void setFrqBereich();
    void setColorRedk1();
    void setColorBlackk1();
    void setColorRedk2();
    void setColorBlackk2();
    void LCDaendern();
    void fsendewobbeldaten();
    void vfoeinaus();
    double getkalibrierwertk1(double afrequenz);
    double getkalibrierwertk2(double afrequenz);
    double getsavcalwert(double afrequenz);
    double getswrkalibrierwertk1(double afrequenz);
    double korrsavpegel(double p);
    QString programpath;

  private slots:
    void schwingkreisfl();
    void schwingkreisfc();
    void alwindungen();
    void setDisplayYmax(const QString &);
    void setDisplayYmin(const QString &);

    void idletimerbehandlung();
    void vtimerbehandlung();
    void stimerbehandlung();
    
    void setDaempfungWob(int index);
    void setDaempfungVfo(int index);
    void setDaempfungWatt(int index);
    
    void setDaempfung(int index);
    void setBetriebsart(int index);
    void setSaBereich1();
    void setSaBereich2();
    void setAufloesung(int index);
    void setProfil(int index);
//    void setkdir();
//    void getkdir(QString);
/////////////////////////////////////////////////////////////////////////////
//Funktionen fuer Alles
    void tabumschalten(QWidget*);
    
/////////////////////////////////////////////////////////////////////////////
//Funktionen fuer Wobbeln
    void checkboxk1_checked(bool);
    void checkboxk2_checked(bool);
    void kalibrier1_checked(bool);
    void kalibrier2_checked(bool);
//    void checkboxswr_checked(bool);
    
    void wobnormalisieren(); //Alle Eingaben pruefen
    void checkbox3db_checked(bool);
    void checkbox6db_checked(bool);
    void checkboxgrafik_checked(bool);
    void checkboxinvers_checked(bool);
    void checkboxtime_checked(int);
    void setanfang(double);
    void setende(double);
    void setschritte(double);
    void readberechnung(double);
    void setKursorFrequenz(double);
    void swrantaendern();
    void clicklupeplus();
    void clicklupeminus();
    void clicklupegleich();
    
/////////////////////////////////////////////////////////////////////////////

    void vfoedit();
    void setVfo();
    void senddaten();
    void readtty();
    void lcd1clicked();
    void lcd2clicked();
    void lcd3clicked();
    void lcd4clicked();
    void lcd5clicked();
//    void warneichkorr();
    void change1hz(int);
    void change10hz(int);
    void change100hz(int);
    void change1khz(int);
    void change10khz(int);
    void change100khz(int);
    void change1mhz(int);
    void change10mhz(int);
    void change100mhz(int);
    void change1ghz(int);

/////////////////////////////////////////////////////////////////////////////
//Funktionen fuers Messen
    void getmessdaten();
    void setwattoffset1(int);
    void setwattoffset2(int);
    void vfoum();
    void setmessvfo();
    void messvfotimerende();
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//Funktionen fuers Antennendiagramm
    void diagrammstart();
    void diagrammstop();
    void diagrammsimulieren();
    void diagrammspeichern();
    void tdiagrammbehandlung();
    void diagrammdim(int);
    void diagrammdimd(double);
//
/////////////////////////////////////////////////////////////////////////////

  protected:
   //virtuelle Funktion muss neu definiert werden. Wird aufgerufen bei
   //Groessenaenderung des Windowshauptfensters
   virtual void keyPressEvent( QKeyEvent *event );
//   virtual void paintEvent( QPaintEvent * );
  signals:
    void sendewobbeldaten(const TWobbel &);
    void sendewobbelkurve(const TMess &);
    void sendefrqmarken(const TFrqmarken &);
    void drucken(const QFont &, const QStringList &);
    void druckenpdf(const QFont &, const QStringList &);
    void bildspeichern(const TBildinfo &, const QStringList &);
    void setmenu(emenuenable, bool);
    void setAppText(const QString &);
    void setWKursorNr(int);
    void setFontSize(int);
    void cursormitte();
        

};

#endif
