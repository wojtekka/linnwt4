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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "nwt7linux.h"

#include "konstdef.h"

//#define fzaehler

class QAction;
class QMenu;
//class QTextEdit;
class Nwt7linux;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    void setProgramPath(QString);

protected:
    void closeEvent(QCloseEvent *event);
    virtual void paintEvent(QPaintEvent*);
    virtual void resizeEvent( QResizeEvent *);

public slots:
    void setText(QString);
    void readSettings();
    void loadConfig(char *name);
    void loadGeraet(char *name);
    void setFontSize(int s);

private slots:
    void msetmenu(emenuenable,bool);

    void druckediagramm();
    void druckediagrammpdf();
    void bildspeichern();

    void EinstellungLaden();
    void EinstellungSichern();
    
    void geraet_neu();

    void KurvenLaden();
    void KurvenSichern();

    void Info();
    void option();
    void firmupdate();

    void clickwobbeln(); //Wobbeln starten
    void clickwobbelneinmal(); //Einen Durchlauf wobbeln
    void clickwobbelnstop(); //Wobbeln stoppen
    void kalibrierenkanal1();
    void kalibrierenkanal2();
    void messsondeladen1();
    void messsondeladen2();
    void messsondespeichern1();
    void messsondespeichern2();
    void infosetfont();
    void setBreite();
    
    void messsetfont();
    void mkalibrierenk1();
    void mkalibrierenk2();
    void editsonde1();
    void editsonde2();
    void tabelleschreiben();
    void tabellespeichern();
    void setDaempfungdefault();
    void beenden();
    void version();
    void zaehlen();
    void tooltip();
    void setFrequenzmarken();
    void editProfile();
    void setHLine();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createToolTip(bool);
    void writeSettings();

    bool bbreite;


//    QTextEdit *textEdit;
    Nwt7linux *nwt7linux;
    QString curFile;
    QString infotext;
    QString infodatum;
    QString infoversion;
    QString programpath;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;
    QMenu *menuDatei;
    QMenu *menuEinstellung;
    QMenu *menuGeraet;
    QMenu *menuKurven;
    QMenu *menuwobbel;
//    QMenu *menuvfo;
    QMenu *menumessen;
#ifdef fzaehler
    QMenu *menuzaehlen;
#endif    
    QMenu *menuhilfe;

    QToolBar *dateiToolBar;
    QToolBar *einstellungToolBar;
    QToolBar *sondenToolBar;
    QToolBar *kurvenToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QAction *mDrucken;
    QAction *mDruckenpdf;
    QAction *mSpeichernpng;
    QAction *mBeenden;
    
    QAction *mEinstellungLaden;
    QAction *mEinstellungSichern;
    
    QAction *mGeraetNeu;
    
    QAction *mInfo;
    QAction *mOption;
    QAction *mFirmware;
    QAction *mKurvenLaden;
    QAction *mKurvenSichern;
    QAction *mWobbeln;
    QAction *mEinmal;
    QAction *mStop;
    QAction *mEichenK1;
    QAction *mEichenK2;
    QAction *mSondeLadenK1;
    QAction *mSondeLadenK2;
    QAction *mSondeSpeichernK1;
    QAction *mSondeSpeichernK2;
    QAction *mFontInfo;
    QAction *mFrequenzmarke;
    QAction *mProfile;
    QAction *mVFODreh1;
    QAction *mVFODreh2;
    QAction *mVFODreh3;
    QAction *mWattFont;
    QAction *mWattEichenK1;
    QAction *mWattEichenK2;
    QAction *mWattSchreiben;
    QAction *mWattEditSonde1;
    QAction *mWattEditSonde2;
    QAction *mTabelleSpeichern;
    QAction *mDaempfungdefault;
    QAction *mVersion;
    QAction *mTooltip;
    QAction *mKurvenManager;
    QAction *mBreite;
    QAction *mHLine;
    
#ifdef fzaehler
    QAction *mZaehlen;
#endif    

    bool bstart;
};

#endif
