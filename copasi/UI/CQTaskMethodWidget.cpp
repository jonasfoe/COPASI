// Copyright (C) 2011 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "CQTaskMethodWidget.h"
#include "qtUtilities.h"

#include "copasi.h"

#include "utilities/CCopasiTask.h"
#include "utilities/CCopasiMethod.h"
#include "utilities/utility.h"
#include "optimization/COptMethod.h"

CQTaskMethodWidget::CQTaskMethodWidget(QWidget* parent, Qt::WindowFlags f):
  QWidget(parent, f),
  mpTask(NULL),
  mpMethod(NULL),
  mpActiveMethod(NULL),
  mMethodHistory(),
  mShowMethods(false),
  mShowMethodParameters(false)
{
  setupUi(this);

  mpTableParameter->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpTableParameter->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpTableParameter->horizontalHeader()->hide();

  mpLblMethod->hide();
  mpBoxMethod->hide();
  mpLblMethodProtocol->hide();
  mpBoxMethodProtocol->hide();

  mpLblParameter->hide();
  mpTableParameter->hide();
}

CQTaskMethodWidget::~CQTaskMethodWidget()
{
  clearHistory();
}

void CQTaskMethodWidget::changeMethod(int /* index */)
{
  if (mpTask == NULL)
    return;

  size_t logParameterIndex = mpActiveMethod->getIndex("#LogDetail");

  // We update the active methods parameters
  if (mShowMethodParameters)
    {
      unsigned C_INT32 i, j;
      QString Value;

      for (i = 0; i < mpActiveMethod->size(); i++)
        {
          if (!mpTableParameter->item(i, 0))
            continue;

          Value = mpTableParameter->item(i, 0)->text();
          j = (i < logParameterIndex) ? i : i + 1;
          setParameterValue(mpActiveMethod, j, Value);
        }
    }

  if (logParameterIndex != C_INVALID_INDEX)
    mpActiveMethod->setValue(logParameterIndex, (unsigned C_INT32) mpBoxMethodProtocol->currentIndex());

  CCopasiMethod::SubType Type =
    toEnum(TO_UTF8(mpBoxMethod->currentText()), CCopasiMethod::SubTypeName, CCopasiMethod::unset);

  setActiveMethod(Type);
  loadMethod();

  return;
}

void CQTaskMethodWidget::setTask(CCopasiTask * pTask)
{
  mpTask = pTask;

  if (mpTask != NULL)
    {
      mpMethod = mpTask->getMethod();

      if (mpMethod != NULL)
        {
          setActiveMethod(mpMethod->getSubType());
          *mpActiveMethod = *mpMethod;
        }
    }
  else
    {
      mpMethod = NULL;
      mpActiveMethod = NULL;
    }
}

void CQTaskMethodWidget::setValidMethods(const unsigned int * validMethods)
{
  unsigned C_INT32 i;

  for (i = 0; validMethods[i] != CCopasiMethod::unset; i++)
    mpBoxMethod->insertItem(mpBoxMethod->count(), FROM_UTF8(CCopasiMethod::SubTypeName[validMethods[i]]));

  if (i > 0)
    {
      mShowMethods = true;
      mpLblMethod->show();
      mpBoxMethod->show();
      mpLblMethodProtocol->show();
      mpBoxMethodProtocol->show();

      connect(mpBoxMethod, SIGNAL(activated(int)), this, SLOT(changeMethod(int)));
    }
  else
    {
      mShowMethods = false;
      mpLblMethod->hide();
      mpBoxMethod->hide();
      mpLblMethodProtocol->hide();
      mpBoxMethodProtocol->hide();

      disconnect(mpBoxMethod, SIGNAL(activated(int)), this, SLOT(changeMethod(int)));
    }
}

void CQTaskMethodWidget::showMethodParameters(const bool & show)
{
  mShowMethodParameters = show;

  if (mShowMethodParameters)
    {
      mpLblParameter->show();
      mpTableParameter->show();
    }
  else
    {
      mpLblParameter->hide();
      mpTableParameter->hide();
    }
}

bool CQTaskMethodWidget::loadMethod()
{
  if (!mpTask) return false;

  if (!mpActiveMethod) return false;

  //Index of logging parameter to be ignored when populating list of parameters
  //This is a workaround since the logging level has to be defined as CCopasiParameter as to be assigned to the actual mpMethod
  size_t logParameterIndex = mpActiveMethod->getIndex("#LogDetail");

  if (mShowMethods)
    {
      mpBoxMethod->setCurrentIndex(mpBoxMethod->findText(FROM_UTF8(CCopasiMethod::SubTypeName[mpActiveMethod->getSubType()])));

      mpBoxMethodProtocol->clear();

      //Maybe getMaxLogDetail() should be defined in CCopasiMethod. COptMethod and CSteadyStateMethod have getMethodLog() defined individually now.
      COptMethod * pActiveOptMethod = dynamic_cast< COptMethod * >(mpActiveMethod);

      if (logParameterIndex != C_INVALID_INDEX && pActiveOptMethod != NULL)
        {
          mpLblMethodProtocol->setEnabled(true);
          mpBoxMethodProtocol->setEnabled(true);

          for (unsigned C_INT32 i = 0; i <= pActiveOptMethod->getMaxLogDetail(); i++)
            {
              mpBoxMethodProtocol->addItem(QString::number(i));
            }

          mpBoxMethodProtocol->setCurrentIndex(*mpActiveMethod->getValue(logParameterIndex).pUINT);
        }
      else
        {
          mpLblMethodProtocol->setEnabled(false);
          mpBoxMethodProtocol->setEnabled(false);
        }
    }

  if (mShowMethodParameters)
    {
      QString Value;

      mpTableParameter->setRowCount((int) mpActiveMethod->size());

      unsigned C_INT32 i;
      CCopasiParameter::Type Type;

      for (i = 0; i < mpActiveMethod->size(); i++)
        {
          // create item of the current row and give it a name
          mpTableParameter->setVerticalHeaderItem(i, new QTableWidgetItem());
          mpTableParameter->verticalHeaderItem(i)->setText(FROM_UTF8(mpActiveMethod->getName(i)));

          Value = getParameterValue(mpActiveMethod, i, &Type);

          QTableWidgetItem *pValueItem = new QTableWidgetItem();
          pValueItem->setData(Qt::EditRole, QVariant(Value));
          pValueItem->setTextAlignment(Qt::AlignRight);
          mpTableParameter->setItem(i, 0, pValueItem);
        }

      //Skip adding "#LogDetail"
      if (logParameterIndex != C_INVALID_INDEX)
        mpTableParameter->removeRow(logParameterIndex);
    }

  mpTableParameter->resizeColumnsToContents();

  return true;
}

bool CQTaskMethodWidget::saveMethod()
{
  if (!mpTask) return false;

  const CCopasiMethod * pMethod = mpTask->getMethod();

  if (!pMethod) return false;

  bool changed = false;
  size_t logParameterIndex = mpActiveMethod->getIndex("#LogDetail");

  if (mShowMethods)
    {
      if (pMethod->getSubType() != mpActiveMethod->getSubType())
        {
          mpTask->setMethodType(mpActiveMethod->getSubType());
          mpMethod = mpTask->getMethod();

          changed = true;
        }
    }

  if (mShowMethodParameters)
    {
      unsigned C_INT32 i, j;
      QString Value;
      CCopasiParameter::Type Type;

      for (i = 0; i < mpActiveMethod->size(); i++)
        {
          if (!mpTableParameter->item(i, 0))
            continue;

          Value = mpTableParameter->item(i, 0)->text();
          j = (i < logParameterIndex) ? i : i + 1;
          if (Value != getParameterValue(mpActiveMethod, j, &Type))
            {
              setParameterValue(mpActiveMethod, j, Value);
              changed = true;
            }
        }
    }

  if (logParameterIndex != C_INVALID_INDEX && mpBoxMethodProtocol->currentIndex() != *mpActiveMethod->getValue(logParameterIndex).pUINT)
    {
      mpActiveMethod->setValue(logParameterIndex, (unsigned C_INT32) mpBoxMethodProtocol->currentIndex());
      changed = true;
    }

  if (changed)
    {
      *mpMethod = *mpActiveMethod;
    }

  return changed;
}

void CQTaskMethodWidget::addToHistory(CCopasiMethod * pMethod)
{
  if (pMethod == NULL)
    {
      return;
    }

  std::map< CCopasiMethod::SubType, CCopasiMethod * >::iterator found = mMethodHistory.find(pMethod->getSubType());

  if (found != mMethodHistory.end())
    {
      if (found->second != pMethod)
        {
          delete found->second;
          found->second = pMethod;
        }

      return;
    }

  mMethodHistory[pMethod->getSubType()] = pMethod;
}

void CQTaskMethodWidget::removeFromHistory(CCopasiMethod * pMethod)
{
  if (pMethod == NULL)
    {
      return;
    }

  std::map< CCopasiMethod::SubType, CCopasiMethod * >::iterator found = mMethodHistory.find(pMethod->getSubType());

  if (found != mMethodHistory.end())
    {
      mMethodHistory.erase(found);
    }
}

CCopasiMethod * CQTaskMethodWidget::getFromHistory(const CCopasiMethod::SubType & Type) const
{
  std::map< CCopasiMethod::SubType, CCopasiMethod * >::const_iterator found = mMethodHistory.find(Type);

  if (found != mMethodHistory.end())
    {
      return found->second;
    }

  return NULL;
}

void CQTaskMethodWidget::setActiveMethod(const CCopasiMethod::SubType & Type)
{
  mpActiveMethod = getFromHistory(Type);

  if (mpActiveMethod == NULL)
    {
      mpActiveMethod = mpTask->createMethod(Type);
      addToHistory(mpActiveMethod);
    }

  assert(mpActiveMethod != NULL);

  return;
}

void CQTaskMethodWidget::clearHistory()
{
  std::map< CCopasiMethod::SubType, CCopasiMethod * >::iterator it = mMethodHistory.begin();
  std::map< CCopasiMethod::SubType, CCopasiMethod * >::iterator end = mMethodHistory.end();

  for (; it != end; ++it)
    {
      delete it->second;
    }

  mMethodHistory.clear();
}
