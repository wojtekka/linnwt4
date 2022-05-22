//
// C++ Interface: konstdef
//
// Description: 
//
//
// Author: Andreas Lindenau <DL4JAL@darc.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
// * Edit dc5pi, 2016-03-24 /dev/ttypACM[0:3]
//
#ifndef KONSTDEF_H
#define KONSTDEF_H

#include <QPen>
#include <QColor>
#include <QDir>

const int maxmessstep = 600; //Grenze fuer Progressmessbar Wattmeter
const int maxmesspunkte = 9999; //maximale Messpunkte von 0 - 9998
const int profilanz = 20; // maximal 20 Profile von 0 - 19
const int calmaxindex = 10000; //indexmaximum des SAV Kalibrierungsarrays 0 - 9999
const double savcalkonst = 755.0; //Konstande fue die Indexberechnung
const double eteiler=60000.0; //Fuer Indexberechnung Frequenzkalibrierungskorrektur
const double ydisplayteilung_lin=11.0; //Displayteilung Y fuer lin. Messonden
const double yaufloesung = 1023.0;//maximale Aufloesung des AD Wandlers
const int antdia_db_teilung = 5; // 5 db Schritte in Antennendiagrammdarstellung

const char schnittstelle1[] ="/dev/ttyS0";
const char schnittstelle2[] ="/dev/ttyS1";
const char schnittstelle3[] ="/dev/ttyS2";
const char schnittstelle4[] ="/dev/ttyS3";
const char schnittstelle5[] ="/dev/ttyUSB0";
const char schnittstelle6[] ="/dev/ttyUSB1";
const char schnittstelle7[] ="/dev/ttyUSB2";
const char schnittstelle8[] ="/dev/ttyUSB3";
const char schnittstelle9[] ="/dev/ttyACM0";
const char schnittstelle10[]="/dev/ttyACM1";
const char schnittstelle11[]="/dev/ttyACM2";
const char schnittstelle12[]="/dev/ttyACM3";

enum efrqbereich {hz, khz, mhz, ghz};
enum esabereich {bkein, bsa1, bsa2, bsa3, bwobbeln};
enum esabandwith {bwkein, bw30kHz, bw7kHz, bw300Hz};
enum enumbetriebsart {ewobbeln, eswrneu, eswrant, eimpedanz, espektrumanalyser, espekdisplayshift, eantdiagramm};
enum enumsavbandbreite {ekeinebandbreite, e300b1, e7kb1, e30kb1, e300b2, e7kb2, e30kb2};
enum enumkurvenversion {ekvkeine, ekvabbruch, ekv305, ekv404, ekv405, ekv410};
enum emenuenable{
  emDrucken,
  emDruckenpdf,
  emSpeichernpng,
  emBeenden,
  emEinstellungLaden,
  emEinstellungSichern,
  emInfo,
  emOption,
  emFirmware,
  emKurvenLaden,
  emKurvenSichern,
  emWobbeln,
  emEinmal,
  emStop,
  emEichenK1,
  emEichenK2,
  emSondeLadenK1,
  emSondeLadenK2,
  emSondeSpeichernK1,
  emSondeSpeichernK2,
  emFontInfo,
  emFrequenzmarke,
  emWattFont,
  emWattEichenK1,
  emWattEichenK2,
  emWattSchreiben,
  emWattEditSonde1,
  emWattEditSonde2,
  emTabelleSpeichern,
  emDaempfungdefault,
  emVersion,
  emTooltip,
#ifdef fzaehlen
  emZaehlen,
#endif
  emmenuDatei,
  emmenuEinstellung,
  emmenuKurven,
  emmenuwobbel,
//  emmenuvfo,
  emmenumessen,
  emmainhidde,
  emmainshow
};

typedef struct {
  int posx;
  int posy;
  int fontsize;
  bool infoanzeigen;
}TBildinfo;

typedef struct {
  QString name;//Text in der ComboBox
  QString anfang;//Anfangsfrequenz
  QString ende;//Endefrequenz
  QString schritte;//Anzahl der Messpunkte
}TFprofil;

typedef struct {
  double offsetwatt;      //Offset je nach Frequenz
  QString str_offsetwatt; //Bezeichnung des Offsets
}TWattOffset;

//Konstande fuer Messfenster

typedef struct {
  double kalibrieranfang;
  double kalibrierende;
  long kalibrierstep;
  double ddstakt;
  double maxwobbel;
  QString str_tty;
  bool kalibrierk1;
  bool kalibrierk2;
  bool einkanalig;
  QString strsonde1;
  QString strsonde1lin;
  QString strsonde2;
  QString kurvendir;
  int pll;
  bool pllmodeenable;
  bool bswriteration;
  QPen penkanal1;
  QPen penkanal2;
  int penwidth;
  QColor colorhintergrund;
  bool bdaempfungfa;
  bool binvers;
  double wanfang;  //Wobbeln
  double wende;    //Wobbeln
  int wschritte;   //Wobbeln
//SA
  double sa1anfang;    //SA Bereich1
  double sa1ende;      //SA Bereich1
  int sa1schritte;     //SA Bereich1
  double sa2anfang;    //SA Bereich2
  double sa2ende;      //SA Bereich2
  int sa2schritte;     //SA Bereich2
  double sa3anfang;    //SA Bereich3 Umsetzer
  double sa3ende;      //SA Bereich3 Umsetzer
  int sa3schritte;     //SA Bereich3 Umsetzer
  double frqa1;        //SA Grenzwerte Bereich1
  double frqb1;        //SA Grenzwerte Bereich1
  double frqa2;        //SA Grenzwerte Bereich2
  double frqb2;        //SA Grenzwerte Bereich2
  double frqzf1;       //SA ZF Bereich1
  double frqzf2;       //SA ZF Bereich2
  double frqshift;     //SA Frequenzverschiebung mit Konverter-Vorsatz
  double bw300_max;    //Bandweitengrenzen fuer die automatische Schrittkorrektur
  double bw300_min;    //Bandweitengrenzen fuer die automatische Schrittkorrektur
  double bw7kHz_max;   //Bandweitengrenzen fuer die automatische Schrittkorrektur
  double bw7kHz_min;   //Bandweitengrenzen fuer die automatische Schrittkorrektur
  double bw30kHz_max;  //Bandweitengrenzen fuer die automatische Schrittkorrektur
  double bw30kHz_min;  //Bandweitengrenzen fuer die automatische Schrittkorrektur
  double psavabs1;     //Pegelabweichung SAV Bereich 1
  double psavabs2;     //Pegelabweichung SAV Bereich 2
  double psavabs3;     //Pegelabweichung SAV Bereich 3
  double psav300;      //Pegelabweichung SAV 300hz
  double psav7k;       //Pegelabweichung SAV 7khz
  double psav30k;      //Pegelabweichung SAV 30khz
  int safehlermax;     //Eingrauung des oberen Bereiches
  int safehlermin;     //Eingrauung des unteren Bereiches
  int saminschritte;   //Schritteanzahl Minimum bei Schrittautomatik
  bool bsavdbm;        //dBm Anzeige
  bool bsavuv;         //uV Anzeige
  bool bsavwatt;       //Watt Anzeige
  bool bsavdbuv;       //dbuV Anzeige
  bool bschrittkorr;   //BIT fuer automatische Schrittkorrektur
  bool sastatus;       //SA Statusmeldung
  bool bwarnsavbw;     //Warnung EIN/AUS bei SAV keine auto. Schrittkorr
  bool bwarnsavbwmax;  //Warnung EIN/AUS bei SAV keine auto. Schrittkorr

  bool fwfalsch;
  int pfsize;
  bool grperwobbeln;   //BIT fuer Focusumschaltung
  bool grpereinmal;    //BIT fuer Focusumschaltung
  bool grperstop;      //BIT fuer Focusumschaltung
  bool bfliessend;     //fliessende Wobbeldarstellung
  bool bset0hz;        //NWT auf 0 Hz setzen
  int frqfaktor;
  int stime;           //Timerzeiten
  int vtime;           //Timerzeiten
  int idletime;        //Timerzeiten
  int audioztime;      //neu FW120 NF-Audio Zeit fuer einen Wobbeldurchlauf
  int audiosens;       //neu FW120 NF-Audio Sensibilitaet
  int wmprezision;     //neu FW120 Nachkommastellen im Wattmeter
}TGrunddaten;

typedef struct {
  bool b160m;
  bool b80m;
  bool b40m;
  bool b30m;
  bool b20m;
  bool b17m;
  bool b15m;
  bool b12m;
  bool b10m;
  bool b6m;
  bool b2m;
  bool bu1;
  bool bu2;
  bool bu3;
  qlonglong f1160m;
  qlonglong f180m;
  qlonglong f140m;
  qlonglong f130m;
  qlonglong f120m;
  qlonglong f117m;
  qlonglong f115m;
  qlonglong f112m;
  qlonglong f110m;
  qlonglong f16m;
  qlonglong f12m;
  qlonglong f1u1;
  qlonglong f1u2;
  qlonglong f1u3;
  qlonglong f2160m;
  qlonglong f280m;
  qlonglong f240m;
  qlonglong f230m;
  qlonglong f220m;
  qlonglong f217m;
  qlonglong f215m;
  qlonglong f212m;
  qlonglong f210m;
  qlonglong f26m;
  qlonglong f22m;
  qlonglong f2u1;
  qlonglong f2u2;
  qlonglong f2u3;
  QString st160m;
  QString st80m;
  QString st40m;
  QString st30m;
  QString st20m;
  QString st17m;
  QString st15m;
  QString st12m;
  QString st10m;
  QString st6m;
  QString st2m;
  QString stu1;
  QString stu2;
  QString stu3;
}TFrqmarken;

typedef struct {
  bool aktiv;
  QString anfang;
  QString ende;
  QString schritte;
  QString beschr;
}TProfil;

typedef struct {
  QString dname;
  QString sname;
  double offset[30];
  QString stroffset[30];
}TSonde;

typedef struct{
  double p[calmaxindex];
}TSavcalarray; 

typedef struct{
//  QDate date; //Datum der letzten Speicherung
  double arrayk1[maxmesspunkte];
  double arrayk2[maxmesspunkte];
}TKalibrierarray; 

typedef struct{
  double arrayk1[maxmesspunkte];
  double arrayk2[maxmesspunkte];
}TSwrkalibrierarray; 

typedef struct{
  unsigned int k1[maxmesspunkte];
  unsigned int k2[maxmesspunkte];
  bool daten_enable;   //gueltige Daten in Kanal 1+2
}TMess; 

typedef struct{
  double k[1024];
}TDiagrammarray;

typedef struct{
  int dbmin; //10dBm..-80dBm
  int dbmax; //10dBm..-80dBm
  int adbegin; //0..1023
  int adend; //0..1023
  int diabegin; //0..360
  int diaend; //0..360
}TAntdiagramm;

typedef struct {
  TMess mess;
  double absanfang;    //Anfangsfrequenz im Wobbelfenster
  double absende;      //Endfrequenz im Wobbelfenster
  double anfangsfrequenz;//Anfangsfrequenz der Kurve
  double schrittfrequenz;//Frequenzsprung
  int schritte;        //Anzahl der Messpunkte
  int ztime;           //Zeit zwischen den Messpunkten
  double faktor1;      //Korrektur Kurvensteilheit
  double shift1;       //Kanal 1
  double faktor2;      //Korrektur Kurvensteilheit
  double shift2;       //Kanal 2
  bool bkanal1;        //Kanal1 darstellen
  bool bkanal2;        //Kanal2 darstellen
  bool linear1;        //ob linear AD8361 Kanal 1
  bool linear2;        //ob linear AD8361 Kanal 2
  bool bandbreite3db;  //Bandbreiteninformation 3db
  bool bandbreite6db;  //Bandbreiteninformation 6db/60dB
  bool bandbreitegrafik;//Bandbreite grafische Darstellung
  int frqfaktor;       //Einsatz von Frequenzvervielfacher
  QString namesonde1,  //Dateiname der Messsonde
          namesonde2;
  QString beschreibung1,//Name der Messsonde
          beschreibung2;
  double mfaktor1;     //Korrektur Sonde als Wattmeter Faktor Sonde1
  double mshift1;      //Korrektur Sonde als Wattmeter Shift Sonde1
  double mfaktor2;     //Korrektur Sonde als Wattmeter Faktor Sonde2
  double mshift2;      //Korrektur Sonde als Wattmeter Shift Sonde2
  double dbshift1;     //Korrektur der Wobbelkurve an 0db
  double dbshift2;     //Korrektur der Wobbelkurve an 0db
  int displayshift;    //Korrektur der Wobbelkurvenansicht
  double hline;       //horizontale Linie
  QString hlinelabel;
  bool bkalibrierkorrk1;    //Eichkorrektur Kanal 1
  bool bkalibrierkorrk2;    //Eichkorrektur Kanal 2
  bool einkanalig;     //HFM9 original verwendet nur einen Kanal
  enumbetriebsart ebetriebsart;	//Betriebsart
  bool bkalibrierswrk1;     //Eichlinie fuer SWR neu
  bool bkalibrierswrk2;     //Eichlinie fuer SWR neu
  int swraufloesung;   //Aufloesung im SWR fenster
  bool bswriteration;  //math. Korrektur der SWR-Kurve
  bool bswrrelais;     //SWR-Kurve
  bool bswrkanal2;     //SWR mit Kanal 2 ermitteln
  double a_100;        //Kabeldaempfung auf 100m
  double kabellaenge;  //Kabellaenge
  bool mousesperre;    //sperren der Mouseaktivitaet
  int kurvennr;        //Nr der Hintergrundkurve nur fuer WKManager
  QPen penkanal1;      //Stift fuer Kanal 1
  QPen penkanal2;      //Stift fuer Kanal 2
  int penwidth;        //Stiftbreite
  bool bhintergrund;   //ob Hintergrund oder nicht fuer WKManager
  QColor colorhintergrund; //Wobbelfenster Hintergrundfarbe
  int safehlermax;     // Ungenauigkeit von MAX in den Plusbereich
  int safehlermin;     // Ungenauigkeit von MIN bis Unendlich
  //QString layoutlabel; //Wobbellabel
  bool binvers;        //inverse Ermittlung der Bandbreite
  int ydbmax;          // Y-Skala obere Grenze
  int ydbmin;          // Y-Skala untere Grenze
  enumsavbandbreite ebbsav; //Bandbreitenrueckmeldung vom SAV
  double psavabs;      //Pegelabweichung SAV
  double psav300;      //Pegelabweichung SAV 300hz
  double psav7k;       //Pegelabweichung SAV 7khz
  double psav30k;      //Pegelabweichung SAV 30khz
  bool bsavdbm;        //SAV Kursorwert dBm
  bool bsavuv;         //SAV Kursorwert Volt
  bool bsavwatt;       //SAV Kursorwert Leistung an 50Ohm
  bool bsavdbuv;       //SAV Kursorwert dBuv an 50Ohm

  bool maudio1;        //Kanal1 ab FW 1.20 Audiomesskopf
  bool maudio2;        //Kanal2 ab FW 1.20 Audiomesskopf

  TSwrkalibrierarray swrkalibrierarray;
  TKalibrierarray kalibrierarray;
  TSavcalarray savcalarray;
  TAntdiagramm antdiagramm;
}TWobbel;

#endif
