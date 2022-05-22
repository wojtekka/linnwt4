/***************************************************************************
                          configfile.h  -  description
                             -------------------
    begin                : Son Aug 17 2003
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

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qobject.h>
#include <qstringlist.h>

//#include "konstdef.h"


/**
  *@author Andreas Lindenau
  */

class QFile;
class QStringList;
class QString;

class configfile{
public: 
  configfile();
  ~configfile();
  void open( const QString &name );
  void close();
  double readDouble(const QString &name, double def);
  int readInteger(const QString &name, int def);
  long readLong(const QString &name, long def);
  qlonglong readLongLong(const QString &name, qlonglong def);
  QString readString(const QString &name, const QString &def);
  bool readBool(const QString &name, bool def);
  void writeDouble(const QString &name, double wert);
  void writeInteger(const QString &name, int wert);
  void writeLong(const QString &name, long wert);
  void writeLongLong(const QString &name, qlonglong wert);
  void writeString(const QString &name, const QString &wert);
  void writeBool(const QString &name, bool wert);
  bool clearItem(const QString &name);

protected:
  QStringList sl;
  QString fname;
  bool speichern;
};

#endif
