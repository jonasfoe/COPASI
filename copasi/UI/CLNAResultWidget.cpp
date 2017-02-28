// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2011 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include <QGridLayout>
#include "copasi.h"

#include "CLNAResultWidget.h"
#include "CLNAResultSubwidget.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "lna/CLNATask.h"
#include "lna/CLNAMethod.h"
#include "qtUtilities.h"
#include "utilities/CCopasiVector.h"

/*
 *  Constructs a CLNAResultWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CLNAResultWidget::CLNAResultWidget(QWidget *parent, const char *name, Qt::WindowFlags fl)
  : CopasiWidget(parent, name, fl)
{
  if (!name)
    setObjectName("CLNAResultWidget");

  setWindowTitle(trUtf8("CLNAResultWidget"));
  mWidgetLayout = new QGridLayout(this);
  mWidgetLayout->setObjectName("Layout");
  mCentralWidget = new CLNAResultSubwidget(this, "CLNAResultSubwidget");
  mWidgetLayout->addWidget(mCentralWidget, 0, 0);
}

/*
 *  Destroys the object and frees any allocated resources
 */
CLNAResultWidget::~CLNAResultWidget()
{}

/* This function loads the compartments widget when its name is
  clicked in the tree   */
bool CLNAResultWidget::loadFromBackend()
{
  assert(mpDataModel != NULL);
  CLNATask *pTask =
    dynamic_cast<CLNATask *>(&mpDataModel->getTaskList()->operator[]("Linear Noise Approximation"));

  if (!pTask) return false;

  mCentralWidget->loadAll(dynamic_cast<CLNAMethod *>(pTask->getMethod()));
  return true;
}

bool CLNAResultWidget::saveToBackend()
{
  return true;
}

bool CLNAResultWidget::update(ListViews::ObjectType objectType,
                              ListViews::Action action,
                              const std::string &C_UNUSED(key))
{
  if (objectType == ListViews::MODEL && action == ListViews::ADD)
    mCentralWidget->loadAll(NULL);

  return true;
}

bool CLNAResultWidget::leave()
{
  return true;
}

bool CLNAResultWidget::enterProtected()
{
  return true;
}
