/***************************************************************************
                          widgetwobbeln.h  -  description
                             -------------------
    begin                : 15.Mai.2007
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
#ifndef WIDGETWOBBWELN_H
#define WIDGETWOBBWELN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QLineEdit>
#include <QObject>
#include <QPrinter>
#include <Q3StrList>
#include <QVector>

#include <Q3PaintDeviceMetrics>
//#include <Q3PopupMenu>
#include <QMouseEvent>
#include <QtGui>

#include "konstdef.h"
  
class winsinfo : public QDialog
{
  Q_OBJECT

public:
  winsinfo(QWidget* parent=0, const char* name=0, bool modal=FALSE);
  ~winsinfo();
  

public slots:
  void setBildinfo(TBildinfo);

private slots:
  void changefont(int);
  void changexpos(int);
  void changeypos(int);
  void changesetboxinfo();
  
private:
  TBildinfo bbildinfo;

  QGroupBox *ginfo;
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
  void  pixauffrischen(const TBildinfo &);
};

class WidgetWobbeln : public QWidget
{
  Q_OBJECT

private:
  QVector<QPoint> kurve;
  QVector<QPoint> kurveb;


  public:
    /** construtor */
  WidgetWobbeln(QWidget* parent=0);
    /** destructor */
  ~WidgetWobbeln();

  void setWobbeldaten(TWobbel);    
  TWobbel getWobbeldaten();
  double getkalibrierwertk1(double afrequenz);
  double getkalibrierwertk2(double afrequenz);
  double getswrkalibrierwertk1(double afrequenz);
  double getswrkalibrierwertk2(double afrequenz);
  
  public slots:
  void setWKursorNr(int);
  void setmousecursormitte();
  void neubildmitinfo(const TBildinfo &);

  protected:
  void closeEvent(QCloseEvent *event);
  virtual void paintEvent(QPaintEvent*);
  virtual void resizeEvent(QResizeEvent*);
  virtual void mouseMoveEvent(QMouseEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void keyPressEvent( QKeyEvent *event );

  private:
  QAction *akursor_clr;
  QAction *akursor_clr_all;
  QAction *akurve_clr;
  QAction *afrequenzanfang;
  QAction *afrequenzende;
  QAction *akabellaenge;
  QAction *akabeldaten;
  QAction *aberechnen;
  
//  Q3PopupMenu *menmouse;
  QMenu *popupmenu;
  
  QPixmap pix, pix1;
  TWobbel wobbeldaten;
  TWobbel kurve1;
  TWobbel kurve2;
  TWobbel kurve3;
  TWobbel kurve4;
  TWobbel kurve5;
  TWobbel kurve6;

  double dbreitealt;
  double dbreite;
  int ObererRand;
  int LinkerRand;
  int mouseposx[5];
  int mouseposx1[5];
  int mouseposy[5];
  int kxm1[5];
  int kxm2[5];
  int kursorix;
  bool bnomove;
  bool banfang;
  bool bende;
  bool bberechnung;
  bool blupeplus;
  bool blupeminus;
  double mousefrequenz;
  TFrqmarken wfmarken;
  int hoehe;  //Hoehe der Wobbeldarstellung
  int breite; //Breite der wobbeldarstellung
  int durchmesser; //Durchmesser Antennendiagramm
  int yswrteilung; //SWV Anzeige Aufloesung
  //double yswraufloesung; //ADC Aufloesung
  double xSchritt; //Schrittweite Y 
  double ySchritt; //Schrittweite X
  double xAnfang; //absoluter Anfang normal bei 0.0
  double xEnde; //absolutes Ende normal Wobbelfensterbreite
  bool bresize;
  //obere Anzeige der LOG_Y-Achse
  double ymax;//dB
  //obere Anzeige der LOG_Y-Achse
  double ymin;//dB
  //obere Anzeige der LOG_Y-Achse
  double ydisplayteilung_log;

  QStringList qusl;
  QString fname;
  QString squdaten;
  bool speichern;
  int qunr;
  
  QStringList infolist;
  TBildinfo wbildinfo;
  
  QString kursorsavstr(double);
  double getsavcalwert(double afrequenz);
  void resetmousecursor();
  void drawBeschriftung(QPainter *p);
  void drawKurve(QPainter *p);
  void refreshPixmap();
  void refreshKurve();
  void setParameterPixmap();
  double korrsavpegel(double);
  void CreateQuarzDatei();
  void WriteToQuarzDatei(const QString &);
  void CloseQuarzDatei();

  private slots:
  void setKurve(TWobbel);
  void kursor_loeschen();
  void kurve_loeschen();
  void wsetfrqanfang();
  void wsetfrqende();
  void setberechnung();
  void kabellaenge();
  void kabeldaten();
  void alle_kursor_loeschen();
  void wobbeldatenempfang(const TWobbel&);
  void wobbeldatenkurve(const TMess&);
  void printdiagramm(const QFont &, const QStringList &);
  void printdiagrammpdf(const QFont &, const QStringList &);
  void bildspeichern(const TBildinfo &, const QStringList &);
  void empfangfrqmarken(const TFrqmarken &);
  
  signals:
  void multieditloeschen();
  void multieditinsert(const QString &);
  void writeywert(int);
  void writeanfang(double);
  void writeende(double);
  void writeberechnung(double);
  void setKursorFrequenz(double);
  void resetKursor(int);
  void lupeplus();
  void lupeminus();
  void lupegleich();
  void wobbelneinmal();
  void wobbelndauer();
  void wobbelnstop();
  void wkurve_loeschen();

};

#endif
