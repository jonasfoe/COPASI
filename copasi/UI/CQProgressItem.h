// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2005 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef CQPROGRESSITEM_H
#define CQPROGRESSITEM_H

#include "ui_CQProgressItem.h"

#include <QtCore/QVariant>

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QWidget>
#include "utilities/CProcessReport.h"
#include "utilities/CVector.h"

class CQProgressItem : public QWidget, public Ui::CQProgressItem
{
  Q_OBJECT

public:
  CQProgressItem(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
  ~CQProgressItem();

  virtual bool initFromProcessReportItem(CProcessReportItem * pItem);
  virtual bool process();
  virtual bool reset();

protected:
  CProcessReportItem * mpItem;

protected slots:
};

#endif // CQPROGRESSITEM_H
