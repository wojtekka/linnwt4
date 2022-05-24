//
// C++ Implementation: firmwiget
//
// Description: 
//
//
// Author: Andreas Lindenau <DL4JAL@darc.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "firmwidget.h"


//#####################################################################################
// Class fuer Firmwidget 
//#####################################################################################
firmwidget::firmwidget(QWidget* parent, Qt::WindowFlags flags): QDialog(parent, flags)
{
  this->resize(450,100);
  label1 = new QLabel(this);
  label1->setGeometry(10,10,400,30);
  label1->setText(tr("Firmware Update","Label"));
  label1->setAlignment(Qt::AlignHCenter);
}

void firmwidget::setText(QString s)
{
  setFocus();
  label1->setText(s);
  repaint();
}
