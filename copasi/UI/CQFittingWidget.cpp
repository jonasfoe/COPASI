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

#include "CQFittingWidget.h"

#include <QtGui/QLabel>
#include <QtGui/QToolButton>

#include "UI/CQTaskBtnWidget.h"
#include "UI/CQTaskHeaderWidget.h"
#include "UI/CQTaskMethodWidget.h"
#include "UI/CQFittingItemWidget.h"
#include "UI/CProgressBar.h"
#include "UI/CQExperimentData.h"
#include "UI/CQOptPopulation.h"
#include "UI/CQCopasiApplication.h"

#include <copasi/utilities/CCopasiMessage.h>
#include <copasi/UI/CQMessageBox.h>

#include "report/CKeyFactory.h"
#include "parameterFitting/CFitTask.h"
#include "parameterFitting/CFitItem.h"
#include "optimization/COptMethod.h"
#include "parameterFitting/CFitProblem.h"
#include "parameterFitting/CExperimentSet.h"
#include "parameterFitting/CExperiment.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "utilities/CCopasiException.h"
#include "report/CCopasiRootContainer.h"

#include <copasi/parameterFitting/CFitTask.h>
#include <copasi/model/CModel.h>

/*
 *  Constructs a CQFittingWidget which is a child of 'parent', with the
 *  name 'name'.'
 */
CQFittingWidget::CQFittingWidget(QWidget* parent, const char* name)
  : TaskWidget(parent, name)
{
  setupUi(this);

  init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQFittingWidget::~CQFittingWidget()
{
  destroy();
  // no need to delete child widgets, Qt does it all for us
}

bool CQFittingWidget::saveTask()
{
  CFitTask * pTask =
    dynamic_cast< CFitTask * >(mpTask);

  if (!pTask) return false;

  saveCommon();
  saveMethod();

  CFitProblem * pProblem =
    dynamic_cast<CFitProblem *>(mpTask->getProblem());

  if (!pProblem) return false;

  std::map<std::string, std::string> ExperimentMap;
  std::map<std::string, std::string> CrossValidationMap;

  // Save experiment set
  CExperimentSet * pExperimentSet =
    dynamic_cast<CExperimentSet *>(pProblem->getGroup("Experiment Set"));

  std::string Name;
  CExperiment * pExperiment;
  size_t i, imax;

  std::set<CExperiment *> DealtWith;

  for (i = pExperimentSet->getExperimentCount() - 1; i != C_INVALID_INDEX; i--)
    {
      pExperiment =
        dynamic_cast<CExperiment *>(CCopasiRootContainer::getKeyFactory()->get(mExperimentKeyMap[pExperimentSet->getExperiment(i)->CCopasiParameter::getKey()]));

      if (pExperiment && pExperiment == mpExperimentSet->getExperiment(pExperiment->getObjectName()))
        {
          if (!(*pExperimentSet->getExperiment(i) == *pExperiment))
            {
              *pExperimentSet->getExperiment(i) = *pExperiment;
              mChanged = true;
            }

          DealtWith.insert(pExperiment);
        }
      else
        {
          mExperimentKeyMap.erase(pExperimentSet->getExperiment(i)->CCopasiParameter::getKey());
          pExperimentSet->removeExperiment(i);
          mChanged = true;
        }
    }

  for (i = 0, imax = mpExperimentSet->getExperimentCount(); i < imax; i++)
    {
      pExperiment = mpExperimentSet->getExperiment(i);

      if (DealtWith.count(pExperiment)) continue;

      pExperiment = pExperimentSet->addExperiment(*pExperiment);
      mExperimentKeyMap[pExperiment->CCopasiParameter::getKey()] =
        mpExperimentSet->getExperiment(i)->CCopasiParameter::getKey();
      mChanged = true;
    }

  // We need to invert the key map for saving!
  std::map<std::string, std::string>::iterator it = mExperimentKeyMap.begin();
  std::map<std::string, std::string>::iterator end = mExperimentKeyMap.end();

  for (; it != end; ++it)
    ExperimentMap[it->second] = it->first;

  // Save cross validation experiment set
  CCrossValidationSet * pCrossValidationSet =
    dynamic_cast<CCrossValidationSet *>(pProblem->getGroup("Validation Set"));

  if (pCrossValidationSet->getWeight() != mpCrossValidationSet->getWeight())
    {
      pCrossValidationSet->setWeight(mpCrossValidationSet->getWeight());
      mChanged = true;
    }

  if (pCrossValidationSet->getThreshold() != mpCrossValidationSet->getThreshold())
    {
      pCrossValidationSet->setThreshold(mpCrossValidationSet->getThreshold());
      mChanged = true;
    }

  DealtWith.clear();

  for (i = pCrossValidationSet->getExperimentCount() - 1; i != C_INVALID_INDEX; i--)
    {
      pExperiment =
        dynamic_cast<CExperiment *>(CCopasiRootContainer::getKeyFactory()->get(mCrossValidationKeyMap[pCrossValidationSet->getExperiment(i)->CCopasiParameter::getKey()]));

      if (pExperiment && pExperiment == mpCrossValidationSet->getExperiment(pExperiment->getObjectName()))
        {
          if (!(*pCrossValidationSet->getExperiment(i) == *pExperiment))
            {
              *pCrossValidationSet->getExperiment(i) = *pExperiment;
              mChanged = true;
            }

          DealtWith.insert(pExperiment);
        }
      else
        {
          mCrossValidationKeyMap.erase(pCrossValidationSet->getExperiment(i)->CCopasiParameter::getKey());
          pCrossValidationSet->removeExperiment(i);
          mChanged = true;
        }
    }

  for (i = 0, imax = mpCrossValidationSet->getExperimentCount(); i < imax; i++)
    {
      pExperiment = mpCrossValidationSet->getExperiment(i);

      if (DealtWith.count(pExperiment)) continue;

      pExperiment = pCrossValidationSet->addExperiment(*pExperiment);
      mCrossValidationKeyMap[pExperiment->CCopasiParameter::getKey()] =
        mpCrossValidationSet->getExperiment(i)->CCopasiParameter::getKey();
      mChanged = true;
    }

  // We need to invert the key map for saving!
  it = mCrossValidationKeyMap.begin();
  end = mCrossValidationKeyMap.end();

  for (; it != end; ++it)
    CrossValidationMap[it->second] = it->first;

  if (mpCheckRandomize->isChecked() != pProblem->getRandomizeStartValues())
    {
      mChanged = true;
      pProblem->setRandomizeStartValues(mpCheckRandomize->isChecked());
    }

  if (mpCreateParameterSets->isChecked() != pProblem->getCreateParameterSets())
    {
      mChanged = true;
      pProblem->setCreateParameterSets(mpCreateParameterSets->isChecked());
    }

  if (mpCheckStatistics->isChecked() != pProblem->getCalculateStatistics())
    {
      mChanged = true;
      pProblem->setCalculateStatistics(mpCheckStatistics->isChecked());
    }

  mChanged |= mpParameters->save(&ExperimentMap, &CrossValidationMap);
  mChanged |= mpConstraints->save(&ExperimentMap, &CrossValidationMap);

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);

  if (mChanged) CCopasiRootContainer::getDatamodelList()->operator[](0).changed();

  mChanged = false;
  return true;
}

bool CQFittingWidget::loadTask()
{
  CFitTask * pTask =
    dynamic_cast< CFitTask * >(mpTask);

  if (!pTask) return false;

  loadCommon();
  loadMethod();

  CFitProblem * pProblem =
    dynamic_cast<CFitProblem *>(mpTask->getProblem());

  if (!pProblem) return false;

  pdelete(mpExperimentSet)
  CExperimentSet * pExperimentSet =
    dynamic_cast<CExperimentSet *>(pProblem->getGroup("Experiment Set"));
  mpExperimentSet = new CExperimentSet(*pExperimentSet, NO_PARENT);

  mExperimentKeyMap.clear();
  size_t i, imax = mpExperimentSet->getExperimentCount();

  for (i = 0; i < imax; i++)
    mExperimentKeyMap[pExperimentSet->getExperiment(i)->CCopasiParameter::getKey()] =
      mpExperimentSet->getExperiment(i)->CCopasiParameter::getKey();

  pdelete(mpCrossValidationSet)
  CCrossValidationSet * pCrossValidationSet =
    dynamic_cast<CCrossValidationSet *>(pProblem->getGroup("Validation Set"));
  mpCrossValidationSet = new CCrossValidationSet(*pCrossValidationSet, NO_PARENT);

  mCrossValidationKeyMap.clear();
  imax = mpCrossValidationSet->getExperimentCount();

  for (i = 0; i < imax; i++)
    mCrossValidationKeyMap[pCrossValidationSet->getExperiment(i)->CCopasiParameter::getKey()] =
      mpCrossValidationSet->getExperiment(i)->CCopasiParameter::getKey();

  mpCheckRandomize->setChecked(pProblem->getRandomizeStartValues());
  mpCreateParameterSets->setChecked(pProblem->getCreateParameterSets());
  mpCheckStatistics->setChecked(pProblem->getCalculateStatistics());

  mpParameters->load(mpDataModel, pProblem->getGroup("OptimizationItemList"), &mExperimentKeyMap, &mCrossValidationKeyMap);

  if (CCopasiMessage::peekLastMessage().getType() == CCopasiMessage::ERROR)
    {
      CQMessageBox::critical(this, "Error loading Parameter estimation task",
                             CCopasiMessage::getAllMessageText().c_str(),
                             QMessageBox::Ok | QMessageBox::Default,
                             QMessageBox::NoButton);
    }

  mpParameters->setExperimentSet(const_cast<const CExperimentSet *&>(mpExperimentSet));
  mpParameters->setCrossValidationSet(const_cast<const CCrossValidationSet *&>(mpCrossValidationSet));

  mpConstraints->load(mpDataModel, pProblem->getGroup("OptimizationConstraintList"), &mExperimentKeyMap, &mCrossValidationKeyMap);
  mpConstraints->setExperimentSet(const_cast<const CExperimentSet *&>(mpExperimentSet));
  mpConstraints->setCrossValidationSet(const_cast<const CCrossValidationSet *&>(mpCrossValidationSet));

  mChanged = false;

  return true;
}

bool CQFittingWidget::runTask()
{
  CFitTask * pTask =
    dynamic_cast< CFitTask * >(CCopasiRootContainer::getKeyFactory()->get(mKey));

  if (!pTask) return false;

  mnParamterSetsBeforeRun = pTask->getObjectDataModel()->getModel()->getModelParameterSets().size();

  if (!commonBeforeRunTask()) return false;


//#define POP_DISPLAY
#ifdef POP_DISPLAY
  //Preliminary: Create a Pop-Widget, should be controled somewhere else
  CQOptPopulation* pPopWidget = new CQOptPopulation(&CCopasiRootContainer::getDatamodelList()->operator[](0), dynamic_cast<CQCopasiApplication*>(qApp)->getMainWindow());
  pPopWidget->setMethod(dynamic_cast<COptMethod*>(pTask->getMethod()));
  pPopWidget->show();
  CCopasiRootContainer::getDatamodelList()->operator[](0).addInterface(pPopWidget);
#endif

  bool success = commonRunTask();

  return success;
}

bool CQFittingWidget::taskFinishedEvent()
{
  bool result = TaskWidget::taskFinishedEvent();

  CFitTask * pTask =
    dynamic_cast< CFitTask * >(CCopasiRootContainer::getKeyFactory()->get(mKey));

  if (!pTask) return false;

  size_t finalCount = pTask->getObjectDataModel()->getModel()->getModelParameterSets().size();

  for (size_t i = mnParamterSetsBeforeRun; i < finalCount; ++i)
    {
      protectedNotify(ListViews::MODELPARAMETERSET, ListViews::ADD, pTask->getObjectDataModel()->getModel()->getModelParameterSets()[i].getKey());
    }

  return true;
}

void CQFittingWidget::slotExperimentData()
{
  CQExperimentData * pDialog = new CQExperimentData(this);
  pDialog->load(mpExperimentSet, mpDataModel);

  connect(pDialog, SIGNAL(experimentChanged()), mpParameters, SLOT(slotExperimentChanged()));
  connect(pDialog, SIGNAL(experimentChanged()), mpConstraints, SLOT(slotExperimentChanged()));

  pDialog->exec();

  pdelete(pDialog);
}

void CQFittingWidget::slotPageIndexChange(int currentIndex)
{
  if (mpTabWidget->tabText(currentIndex).contains("Parameters", Qt::CaseSensitive))
    mpCurrentList = mpParameters;
  else
    mpCurrentList = mpConstraints;
}

void CQFittingWidget::slotPageChange(QWidget * currentPage)
{
  slotPageIndexChange(mpTabWidget->indexOf(currentPage));
}

void CQFittingWidget::init()
{
  mpHeaderWidget->setTaskName("Parameter Estimation");

  verticalLayout->insertWidget(0, mpHeaderWidget);
  // verticalLayout->insertSpacing(1, 14);      // space between header and body

  mpMethodWidget->setValidMethods(CFitTask::ValidMethods);
  mpMethodWidget->showMethodParameters(true);
  verticalLayout->addWidget(mpMethodWidget);

  verticalLayout->addWidget(mpBtnWidget);

  mpParameters->setItemType(CQFittingItemWidget::FIT_ITEM);
  connect(mpParameters, SIGNAL(numberChanged(int)), this, SLOT(slotParameterNumberChanged(int)));

  mpConstraints->setItemType(CQFittingItemWidget::FIT_CONSTRAINT);
  connect(mpConstraints, SIGNAL(numberChanged(int)), this, SLOT(slotConstraintNumberChanged(int)));

  mpCurrentList = mpParameters;
  mpExperimentSet = NULL;
  mpCrossValidationSet = NULL;
}

void CQFittingWidget::slotParameterNumberChanged(int number)
{
  QString TabLabel = "Parameters (" + QString::number(number) + ")";
  mpTabWidget->setTabText(mpTabWidget->indexOf(mpParameters), TabLabel);
}

void CQFittingWidget::slotConstraintNumberChanged(int number)
{
  QString TabLabel = "Constraints (" + QString::number(number) + ")";
  mpTabWidget->setTabText(mpTabWidget->indexOf(mpConstraints), TabLabel);
}

void CQFittingWidget::destroy()
{
  pdelete(mpExperimentSet);
  pdelete(mpCrossValidationSet);
}

void CQFittingWidget::slotCrossValidationData()
{
  CQExperimentData * pDialog = new CQExperimentData(this);
  pDialog->load(mpCrossValidationSet, mpDataModel);

  connect(pDialog, SIGNAL(experimentChanged()), mpParameters, SLOT(slotCrossValidationChanged()));
  connect(pDialog, SIGNAL(experimentChanged()), mpConstraints, SLOT(slotCrossValidationChanged()));

  pDialog->exec();

  pdelete(pDialog);
}
