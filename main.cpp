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

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QString>
#include <QtGui>
#include <QDir>


#include "mainwindow.h"

int main(int argc, char *argv[])
{
  int indexhfc = 0, indexqm = 0;
  QString s;
  char c[]="hfm9.hfc";
  QApplication app(argc, argv);
  QTranslator appTranslator;
  
  QDir dir(argv[0]);
  QString programpath(dir.absolutePath());
  //qDebug(programpath);
  
  s = argv[1];
  if(s.contains(".qm"))indexqm = 1;
  if(s.contains(".hfc"))indexhfc = 1;
  s = argv[2];
  if(s.contains(".qm"))indexqm = 2;
  if(s.contains(".hfc"))indexhfc = 2;
  if(indexqm != 0){
    //qDebug("QM Laden");
    s = argv[indexqm];
    //qDebug(s);
    if(appTranslator.load(s))qDebug("OK");
    app.installTranslator(&appTranslator); 
  }
  app.setQuitOnLastWindowClosed(true); 
  MainWindow *mainWin = new MainWindow();
  app.setWindowIcon(QIcon(":images/appicon.png"));
  mainWin->setWindowIcon(QIcon(":images/appicon.png"));
  mainWin->show();
  //Den Pfad fuer Konfigurationsdateien setzen
  mainWin->setProgramPath(programpath);
  if(indexhfc != 0){
    mainWin->loadConfig(argv[indexhfc]);
  }else{
    mainWin->loadConfig(c);
  }
  return app.exec();
}
