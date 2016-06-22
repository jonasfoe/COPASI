// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2002 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  File name: COptProblem.cpp
 *
 *  Programmer: Yongqun He
 *  Contact email: yohe@vt.edu
 *  Purpose: This is the source file of the COptProblem class.
 *           It specifies the optimization problem with its own members and
 *           functions. It's used by COptAlgorithm class and COptimization class
 */
#include <cmath>

#include "copasi.h"
#include "COptTask.h"
#include "COptProblem.h"
#include "COptItem.h"

#include "function/CFunctionDB.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"

#include "steadystate/CSteadyStateTask.h"
#include "trajectory/CTrajectoryTask.h"
#include "steadystate/CSteadyStateProblem.h"
#include "trajectory/CTrajectoryProblem.h"

#include "math/CMathContainer.h"
#include "model/CModel.h"
#include "model/CCompartment.h"

#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"

#include "utilities/CProcessReport.h"
#include "utilities/CCopasiException.h"

// static
const unsigned int COptProblem::ValidSubtasks[] =
{
  CTaskEnum::steadyState,
  CTaskEnum::timeCourse,
  CTaskEnum::scan,
  CTaskEnum::parameterFitting,
  CTaskEnum::mca,
  CTaskEnum::lyap,
  CTaskEnum::tssAnalysis,
  CTaskEnum::sens,
  CTaskEnum::crosssection,
  CTaskEnum::lna,
  CTaskEnum::UnsetTask
};

//  Default constructor
COptProblem::COptProblem(const CTaskEnum::Task & type,
                         const CCopasiContainer * pParent):
  CCopasiProblem(type, pParent),
  mWorstValue(0.0),
  mpParmSubtaskCN(NULL),
  mpParmObjectiveExpression(NULL),
  mpParmMaximize(NULL),
  mpParmRandomizeStartValues(NULL),
  mpParmCalculateStatistics(NULL),
  mpGrpItems(NULL),
  mpGrpConstraints(NULL),
  mpOptItems(NULL),
  mpConstraintItems(NULL),
  mpSubtask(NULL),
  mpObjectiveExpression(NULL),
  mInitialRefreshSequence(),
  mUpdateObjectiveFunction(),
  mUpdateConstraints(),
  mCalculateValue(0),
  mSolutionVariables(),
  mOriginalVariables(),
  mContainerVariables(),
  mSolutionValue(0),
  mCounter(0),
  mFailedCounterException(0),
  mFailedCounterNaN(0),
  mConstraintCounter(0),
  mFailedConstraintCounter(0),
  mCPUTime(CCopasiTimer::PROCESS, this),
  mhSolutionValue(C_INVALID_INDEX),
  mhCounter(C_INVALID_INDEX),
  mStoreResults(false),
  mHaveStatistics(false),
  mGradient(0)
{
  initializeParameter();
  initObjects();
}

// copy constructor
COptProblem::COptProblem(const COptProblem& src,
                         const CCopasiContainer * pParent):
  CCopasiProblem(src, pParent),
  mWorstValue(src.mWorstValue),
  mpParmSubtaskCN(NULL),
  mpParmObjectiveExpression(NULL),
  mpParmMaximize(NULL),
  mpParmRandomizeStartValues(NULL),
  mpParmCalculateStatistics(NULL),
  mpGrpItems(NULL),
  mpGrpConstraints(NULL),
  mpOptItems(NULL),
  mpConstraintItems(NULL),
  mpSubtask(NULL),
  mpObjectiveExpression(NULL),
  mInitialRefreshSequence(),
  mUpdateObjectiveFunction(),
  mUpdateConstraints(),
  mCalculateValue(src.mCalculateValue),
  mSolutionVariables(src.mSolutionVariables),
  mOriginalVariables(src.mOriginalVariables),
  mContainerVariables(src.mContainerVariables),
  mSolutionValue(src.mSolutionValue),
  mCounter(0),
  mFailedCounterException(0),
  mFailedCounterNaN(0),
  mConstraintCounter(0),
  mFailedConstraintCounter(0),
  mCPUTime(CCopasiTimer::PROCESS, this),
  mhSolutionValue(C_INVALID_INDEX),
  mhCounter(C_INVALID_INDEX),
  mStoreResults(src.mStoreResults),
  mHaveStatistics(src.mHaveStatistics),
  mGradient(src.mGradient)
{
  initializeParameter();
  initObjects();
}

// Destructor
COptProblem::~COptProblem()
{}

void COptProblem::initializeParameter()
{
  mpParmSubtaskCN = assertParameter("Subtask", CCopasiParameter::CN, CCopasiObjectName(""));
  mpParmObjectiveExpression = assertParameter("ObjectiveExpression", CCopasiParameter::EXPRESSION, std::string(""));
  mpParmMaximize = assertParameter("Maximize", CCopasiParameter::BOOL, false);
  mpParmRandomizeStartValues = assertParameter("Randomize Start Values", CCopasiParameter::BOOL, false);
  mpParmCalculateStatistics = assertParameter("Calculate Statistics", CCopasiParameter::BOOL, true);

  mpGrpItems = assertGroup("OptimizationItemList");
  mpGrpConstraints = assertGroup("OptimizationConstraintList");

  elevateChildren();
}

bool COptProblem::elevateChildren()
{
  // We need to handle the old file format which had two different task keys
  if (mpParmSubtaskCN != NULL)
    {
      CCopasiParameter * pParameter;

      if ((pParameter = getParameter("Steady-State")) != NULL)
        {
          if (pParameter->getValue< std::string >() != "")
            {
              setSubtaskType(CTaskEnum::steadyState);
            }

          removeParameter("Steady-State");
        }

      if ((pParameter = getParameter("Time-Course")) != NULL)
        {
          if (pParameter->getValue< std::string >() != "")
            {
              setSubtaskType(CTaskEnum::timeCourse);
            }

          removeParameter("Time-Course");
        }

      // If no subtask is defined we default to steady-state
      if (*mpParmSubtaskCN == "")
        setSubtaskType(CTaskEnum::steadyState);
    }

  // Handle old file format in which the objective expression was stored in the function DB
  if (mpParmObjectiveExpression != NULL)
    {
      CCopasiParameter * pParameter = getParameter("ObjectiveFunction");
      CExpression * pObjectiveFunction = NULL;

      // We do not use the key to find the objective function because keys are not re-mapped
      // for unknown parameters, instead we rely on the uniqueness of the name and the fact that
      // this is the only expression in the list.
      size_t Index = CCopasiRootContainer::getFunctionList()->loadedFunctions().getIndex("Objective Function");

      if (Index != C_INVALID_INDEX)
        {
          pObjectiveFunction =
            dynamic_cast<CExpression *>(&CCopasiRootContainer::getFunctionList()->loadedFunctions()[Index]);
        }

      if (pObjectiveFunction != NULL &&
          pParameter != NULL)
        {
          *mpParmObjectiveExpression = pObjectiveFunction->getInfix();

          removeParameter("ObjectiveFunction");
        }

      setObjectiveFunction(*mpParmObjectiveExpression);
    }

  mpGrpItems =
    elevate<CCopasiParameterGroup, CCopasiParameterGroup>(mpGrpItems);

  if (!mpGrpItems) return false;

  std::vector< CCopasiParameter * > * pValue =
    &mpGrpItems->CCopasiParameter::getValue< CCopasiParameterGroup::elements >();

  index_iterator it = pValue->begin();
  index_iterator end = pValue->end();

  for (; it != end; ++it)
    if (!elevate<COptItem, CCopasiParameterGroup>(*it)) return false;

  mpOptItems = &mpGrpItems->CCopasiParameter::getValue< std::vector< COptItem * > >();

  mpGrpConstraints =
    elevate<CCopasiParameterGroup, CCopasiParameterGroup>(mpGrpConstraints);

  if (!mpGrpConstraints) return false;

  pValue = &mpGrpConstraints->CCopasiParameter::getValue< CCopasiParameterGroup::elements >();

  it = pValue->begin();
  end = pValue->end();

  for (; it != end; ++it)
    if (!elevate<COptItem, CCopasiParameterGroup>(*it)) return false;

  mpConstraintItems =
    &mpGrpConstraints->CCopasiParameter::getValue< std::vector< COptItem * > >();

  return true;
}

void COptProblem::reset()
{
  mSolutionValue = (*mpParmMaximize ? - std::numeric_limits<C_FLOAT64>::infinity() : std::numeric_limits<C_FLOAT64>::infinity());
  mCounter = 0;
}

bool COptProblem::setCallBack(CProcessReport * pCallBack)
{
  CCopasiProblem::setCallBack(pCallBack);

  if (pCallBack)
    {
      reset();

      // We need to reset mSolutionValue to correctly initialize the progress item.
      mhSolutionValue =
        mpCallBack->addItem("Best Value",
                            mSolutionValue);
      // We need to reset mCounter to correctly initialize the progress item.
      mhCounter =
        mpCallBack->addItem("Function Evaluations",
                            mCounter);
    }

  return true;
}

void COptProblem::initObjects()
{
  addObjectReference("Function Evaluations", mCounter, CCopasiObject::ValueInt);
  addObjectReference("Best Value", mSolutionValue, CCopasiObject::ValueDbl);
  addVectorReference("Best Parameters", mSolutionVariables, CCopasiObject::ValueDbl);
}

bool COptProblem::initializeSubtaskBeforeOutput()
{
  if (mpParmSubtaskCN != NULL)
    {
      CObjectInterface::ContainerList ListOfContainer;
      ListOfContainer.push_back(getObjectAncestor("Vector"));
      mpSubtask = dynamic_cast< CCopasiTask * >(CObjectInterface::GetObjectFromCN(ListOfContainer, *mpParmSubtaskCN));

      try
        {
          if (mpSubtask != NULL)
            return mpSubtask->initialize(CCopasiTask::NO_OUTPUT, NULL, NULL);
        }

      catch (...) {}

      return false;
    }

  // We have a CFitProblem for which it is OK not to have a subtask.
  mpSubtask = NULL;
  return true;
}

bool COptProblem::initialize()
{
  mWorstValue = std::numeric_limits<C_FLOAT64>::infinity();

  if (*mpParmMaximize)
    {
      mWorstValue *= -1.0;
    }

  if (mpContainer == NULL) return false;

  bool success = true;

  mpReport = NULL;
  mCounter = 0;
  mFailedCounterException = 0;
  mFailedCounterNaN = 0;
  mConstraintCounter = 0;
  mFailedConstraintCounter = 0;

  mSolutionValue = mWorstValue;

  CObjectInterface::ContainerList ContainerList;
  ContainerList.push_back(mpContainer);

  COptTask * pTask = dynamic_cast< COptTask * >(getObjectParent());

  if (pTask)
    {
      ContainerList.push_back(pTask);
      mpReport = &pTask->getReport();

      if (!mpReport->getStream()) mpReport = NULL;
    }

  if (mpSubtask != NULL)
    ContainerList.push_back(mpSubtask);

  size_t i;
  size_t Size = mpOptItems->size();

  mSolutionVariables.resize(Size);
  mOriginalVariables.resize(Size);
  mContainerVariables.resize(Size);

  mSolutionVariables = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mOriginalVariables = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  std::vector< COptItem * >::iterator it = mpOptItems->begin();
  std::vector< COptItem * >::iterator end = mpOptItems->end();

  if (it == end)
    {
      CCopasiMessage(CCopasiMessage::ERROR, MCOptimization + 6);
      return false;
    }

  CObjectInterface::ObjectSet changedObjects;

  for (i = 0; it != end; ++it, i++)
    {
      success &= (*it)->compile(ContainerList);

      if ((*it)->getObject() != NULL)
        {
          changedObjects.insert((*it)->getObject());
          mContainerVariables[i] = (C_FLOAT64 *)(*it)->getObject()->getValuePointer();
          mOriginalVariables[i] = *mContainerVariables[i];
        }
      else
        {
          mContainerVariables[i] = &DummyValue;
          mOriginalVariables[i] = std::numeric_limits< C_FLOAT64 >::quiet_NaN();
        }
    }

  changedObjects.erase(NULL);
  mpContainer->getInitialDependencies().getUpdateSequence(mInitialRefreshSequence, CMath::UpdateMoieties, changedObjects, mpContainer->getInitialStateObjects());

  it = mpConstraintItems->begin();
  end = mpConstraintItems->end();

  // We need to build a refresh sequence so the constraint values are updated
  CObjectInterface::ObjectSet Objects;

  for (i = 0; it != end; ++it, i++)
    {
      success &= (*it)->compile(ContainerList);

      if ((*it)->getObject() != NULL)
        {
          Objects.insert((*it)->getObject());
        }
    }

  mpContainer->getTransientDependencies().getUpdateSequence(mUpdateConstraints, CMath::Default, mpContainer->getStateObjects(false), Objects, mpContainer->getSimulationUpToDateObjects());

  mCPUTime.start();

  // TODO CRITICAL PARRELIZATION Add the objective expression to the math container
  if (mpObjectiveExpression == NULL ||
      mpObjectiveExpression->getInfix() == "" ||
      !mpObjectiveExpression->compile(ContainerList))
    {
      mUpdateObjectiveFunction.clear();
      CCopasiMessage(CCopasiMessage::ERROR, MCOptimization + 5);
      return false;
    }

  mpContainer->getTransientDependencies().getUpdateSequence(mUpdateObjectiveFunction, CMath::Default, mpContainer->getStateObjects(false), Objects, mpContainer->getSimulationUpToDateObjects());

  return success;
}

void COptProblem::updateContainer(const bool & update)
{
  C_FLOAT64 **ppContainerVariable = mContainerVariables.array();
  C_FLOAT64 **ppContainerVariableEnd = ppContainerVariable + mContainerVariables.size();
  C_FLOAT64 *pRestore = (update && mSolutionValue != mWorstValue) ? mSolutionVariables.array() : mOriginalVariables.array();

  for (; ppContainerVariable != ppContainerVariableEnd; ++ppContainerVariable, ++pRestore)
    {
      **ppContainerVariable = *pRestore;
    }
}

bool COptProblem::restore(const bool & updateModel)
{
  bool success = true;

  if (mpSubtask != NULL)
    {
      bool update = mpSubtask->isUpdateModel();
      mpSubtask->setUpdateModel(false);
      success &= mpSubtask->restore();
      mpSubtask->setUpdateModel(update);
    }

  updateContainer(updateModel);
  mpContainer->applyUpdateSequence(mInitialRefreshSequence);
  mpContainer->pushInitialState();

  if ((mFailedCounterException + mFailedCounterNaN) * 20 > mCounter) // > 5% failure rate
    CCopasiMessage(CCopasiMessage::WARNING, MCOptimization + 8, mFailedCounterException + mFailedCounterNaN, mCounter);

  if (10 * mFailedConstraintCounter > 8 * mConstraintCounter) // > 80 % failure rate
    CCopasiMessage(CCopasiMessage::WARNING, MCOptimization + 9, mFailedConstraintCounter, mConstraintCounter);

  return success;
}

bool COptProblem::checkParametricConstraints()
{
  std::vector< COptItem * >::const_iterator it = mpOptItems->begin();
  std::vector< COptItem * >::const_iterator end = mpOptItems->end();

  for (; it != end; ++it)
    if ((*it)->checkConstraint()) return false;

  return true;
}

bool COptProblem::checkFunctionalConstraints()
{
  // Make sure the constraint values are up to date.
  mpContainer->applyUpdateSequence(mUpdateConstraints);

  std::vector< COptItem * >::const_iterator it = mpConstraintItems->begin();
  std::vector< COptItem * >::const_iterator end = mpConstraintItems->end();

  mConstraintCounter++;

  for (; it != end; ++it)
    if ((*it)->checkConstraint())
      {
        mFailedConstraintCounter++;
        return false;
      }

  return true;
}

/**
 * calculate() decides whether the problem is a steady state problem or a
 * trajectory problem based on whether the pointer to that type of problem
 * is null or not.  It then calls the process() method for that type of
 * problem.  Currently process takes ofstream& as a parameter but it will
 * change so that process() takes no parameters.
 */
bool COptProblem::calculate()
{
  mCounter++;
  bool success = false;
  COutputHandler * pOutputHandler = NULL;

  if (mpSubtask == NULL)
    return false;

  if (mStoreResults &&
      mpSubtask->getType() == CTaskEnum::timeCourse)
    {
      static_cast< CTrajectoryProblem * >(mpSubtask->getProblem())->setTimeSeriesRequested(true);

      pOutputHandler = new COutputHandler();
      mpSubtask->initialize(CCopasiTask::ONLY_TIME_SERIES, pOutputHandler, NULL);
    }

  try
    {
      mpContainer->applyUpdateSequence(mInitialRefreshSequence);
      // Update all initial values which depend on the optimization items.

      success = mpSubtask->process(true);

      mpContainer->applyUpdateSequence(mUpdateObjectiveFunction);

      // TODO CRITICAL PARRELIZATION We need to point to the created container objective function
      mCalculateValue = *mpParmMaximize ? -mpObjectiveExpression->calcValue() : mpObjectiveExpression->calcValue();
    }

  catch (CCopasiException & /*Exception*/)
    {
      // We do not want to clog the message cue.
      CCopasiMessage::getLastMessage();

      success = false;
    }

  catch (...)
    {
      success = false;
    }

  if (mStoreResults &&
      mpSubtask->getType() == CTaskEnum::timeCourse)
    {
      mStoreResults = false;
      mpSubtask->initialize(CCopasiTask::NO_OUTPUT, NULL, NULL);
      pdelete(pOutputHandler);
    }

  if (!success)
    {
      mFailedCounterException++;
      mCalculateValue = std::numeric_limits< C_FLOAT64 >::infinity();
    }

  if (isnan(mCalculateValue))
    {
      mFailedCounterNaN++;
      mCalculateValue = std::numeric_limits< C_FLOAT64 >::infinity();
    }

  if (mpCallBack) return mpCallBack->progressItem(mhCounter);

  return true;
}

bool COptProblem::calculateStatistics(const C_FLOAT64 & factor,
                                      const C_FLOAT64 & resolution)
{
  // Set the current values to the solution values.
  size_t imax = mSolutionVariables.size();

  mGradient.resize(imax);
  mGradient = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  C_FLOAT64 **ppContainerVariable = mContainerVariables.array();
  C_FLOAT64 **ppContainerVariableEnd = ppContainerVariable + mContainerVariables.size();
  C_FLOAT64 *pSolution = mSolutionVariables.array();

  for (; ppContainerVariable != ppContainerVariableEnd; ++ppContainerVariable, ++pSolution)
    {
      **ppContainerVariable = *pSolution;
    }

  mpContainer->applyUpdateSequence(mInitialRefreshSequence);

  // This is necessary so that the result can be displayed.
  mStoreResults = true;
  calculate();
  mStoreResults = false;

  // Make sure the timer is accurate.
  mCPUTime.calculateValue();

  if (mSolutionValue == mWorstValue)
    return false;

  if (*mpParmCalculateStatistics)
    {
      mHaveStatistics = true;

      C_FLOAT64 Current;
      C_FLOAT64 Delta;

      // Calculate the gradient
      ppContainerVariable = mContainerVariables.array();
      pSolution = mSolutionVariables.array();
      C_FLOAT64 * pGradient = mGradient.array();

      for (; ppContainerVariable != ppContainerVariableEnd; ++ppContainerVariable, ++pSolution, ++pGradient)
        {
          Current = * pSolution;

          if (fabs(Current) > resolution)
            {
              **ppContainerVariable = Current * (1.0 + factor);
              Delta = 1.0 / (Current * factor);
            }
          else
            {
              **ppContainerVariable = resolution;
              Delta = 1.0 / resolution;
            }

          mpContainer->applyUpdateSequence(mInitialRefreshSequence);

          calculate();

          *pGradient = ((*mpParmMaximize ? -mCalculateValue : mCalculateValue) - mSolutionValue) * Delta;

          // Restore the value
          **ppContainerVariable = Current;
        }

      mpContainer->applyUpdateSequence(mInitialRefreshSequence);
      calculate();

      // Make sure the timer is accurate.
      mCPUTime.calculateValue();
    }

  return true;
}

const C_FLOAT64 & COptProblem::getCalculateValue() const
{return mCalculateValue;}

const CVector< C_FLOAT64 > & COptProblem::getSolutionVariables() const
{return mSolutionVariables;}

const CVector< C_FLOAT64 > & COptProblem::getVariableGradients() const
{return mGradient;}

bool COptProblem::setSolution(const C_FLOAT64 & value,
                              const CVector< C_FLOAT64 > & variables)
{
  mSolutionValue = *mpParmMaximize ? -value : value;
  mSolutionVariables = variables;

  bool Continue = true;

  if (value == -std::numeric_limits< C_FLOAT64 >::infinity())
    Continue = false;

  if (mpCallBack)
    Continue &= mpCallBack->progressItem(mhSolutionValue);

  return Continue;
}

const C_FLOAT64 & COptProblem::getSolutionValue() const
{return mSolutionValue;}

COptItem & COptProblem::getOptItem(const size_t & index)
{return *(*mpOptItems)[index];}

size_t COptProblem::getOptItemSize() const
{return mpGrpItems->size();}

COptItem & COptProblem::addOptItem(const CCopasiObjectName & objectCN)
{
  CCopasiDataModel* pDataModel = getObjectDataModel();
  assert(pDataModel != NULL);

  COptItem * pItem = new COptItem(pDataModel);
  pItem->setObjectCN(objectCN);

  mpGrpItems->addParameter(pItem);

  return *pItem;
}

bool COptProblem::removeOptItem(const size_t & index)
{return mpGrpItems->removeParameter(index);}

bool COptProblem::swapOptItem(const size_t & iFrom,
                              const size_t & iTo)
{return mpGrpItems->swap(iFrom, iTo);}

const std::vector< COptItem * > & COptProblem::getOptItemList() const
{return *mpOptItems;}

const std::vector< COptItem * > & COptProblem::getConstraintList() const
{return *mpConstraintItems;}

CVectorCore< C_FLOAT64 * > & COptProblem::getContainerVariables() const
{return mContainerVariables;}

bool COptProblem::setObjectiveFunction(const std::string & infix)
{
  *mpParmObjectiveExpression = infix;

  if (mpObjectiveExpression == NULL)
    mpObjectiveExpression = new CExpression("Expression", this);

  return mpObjectiveExpression->setInfix(infix);
}

const std::string COptProblem::getObjectiveFunction()
{
  return *mpParmObjectiveExpression;
}

bool COptProblem::setSubtaskType(const CTaskEnum::Task & subtaskType)
{
  mpSubtask = NULL;
  *mpParmSubtaskCN = "";

  CCopasiVectorN< CCopasiTask > * pTasks =
    dynamic_cast< CCopasiVectorN< CCopasiTask > *>(getObjectAncestor("Vector"));

  CCopasiDataModel* pDataModel = getObjectDataModel();

  if (pTasks == NULL && pDataModel)
    pTasks = pDataModel->getTaskList();

  if (pTasks)
    {
      size_t i, imax = pTasks->size();

      for (i = 0; i < imax; i++)
        if (pTasks->operator[](i).getType() == subtaskType)
          {
            mpSubtask = &pTasks->operator[](i);
            *mpParmSubtaskCN = mpSubtask->getCN();
            return true;
          }
    }

  return false;
}

CTaskEnum::Task COptProblem::getSubtaskType() const
{
  CObjectInterface::ContainerList ListOfContainer;
  ListOfContainer.push_back(getObjectAncestor("Vector"));
  mpSubtask = dynamic_cast< CCopasiTask * >(CObjectInterface::GetObjectFromCN(ListOfContainer, *mpParmSubtaskCN));

  if (mpSubtask == NULL)
    return CTaskEnum::UnsetTask;

  return mpSubtask->getType();
}

void COptProblem::setMaximize(const bool & maximize)
{*mpParmMaximize = maximize;}

const bool & COptProblem::maximize() const
{return *mpParmMaximize;}

void COptProblem::setRandomizeStartValues(const bool & randomize)
{*mpParmRandomizeStartValues = randomize;}

const bool & COptProblem::getRandomizeStartValues() const
{return *mpParmRandomizeStartValues;}

void COptProblem::randomizeStartValues()
{
  if (*mpParmRandomizeStartValues)
    {
      std::vector< COptItem * >::iterator it = mpOptItems->begin();
      std::vector< COptItem * >::iterator end = mpOptItems->end();

      for (; it != end; ++it)
        {
          (*it)->setStartValue((*it)->getRandomValue(mpContainer->getRandomGenerator()));
        }
    }

  return;
}

void COptProblem::rememberStartValues()
{
  std::vector< COptItem * >::iterator it = mpOptItems->begin();
  std::vector< COptItem * >::iterator end = mpOptItems->end();

  for (; it != end; ++it)
    {
      (*it)->rememberStartValue();
    }

  return;
}

void COptProblem::setCalculateStatistics(const bool & calculate)
{*mpParmCalculateStatistics = calculate;}

const bool & COptProblem::getCalculateStatistics() const
{return *mpParmCalculateStatistics;}

const unsigned C_INT32 & COptProblem::getFunctionEvaluations() const
{return mCounter;}

void COptProblem::incrementEvaluations(unsigned C_INT32 increment)
{mCounter += increment;}

void COptProblem::resetEvaluations()
{mCounter = 0;}

const unsigned C_INT32 & COptProblem::getFailedEvaluationsExc() const
{return mFailedCounterException;}

const unsigned C_INT32 & COptProblem::getFailedEvaluationsNaN() const
{return mFailedCounterNaN;}

const C_FLOAT64 & COptProblem::getExecutionTime() const
{
  return mCPUTime.getElapsedTimeSeconds();
}

void COptProblem::print(std::ostream * ostream) const
{*ostream << *this;}

void COptProblem::printResult(std::ostream * ostream) const
{
  std::ostream & os = *ostream;

  if (mSolutionVariables.size() == 0)
    {
      return;
    }

  os << "    Objective Function Value:\t" << mSolutionValue << std::endl;

  CCopasiTimeVariable CPUTime = const_cast<COptProblem *>(this)->mCPUTime.getElapsedTime();

  os << "    Function Evaluations:\t" << mCounter << std::endl;
  os << "    CPU Time [s]:\t"
     << CCopasiTimeVariable::LL2String(CPUTime.getSeconds(), 1) << "."
     << CCopasiTimeVariable::LL2String(CPUTime.getMilliSeconds(true), 3) << std::endl;
  os << "    Evaluations/Second [1/s]:\t" << mCounter / (C_FLOAT64)(CPUTime.getMilliSeconds() / 1e3) << std::endl;
  os << std::endl;

  std::vector< COptItem * >::const_iterator itItem =
    mpOptItems->begin();
  std::vector< COptItem * >::const_iterator endItem =
    mpOptItems->end();

  size_t i;

  for (i = 0; itItem != endItem; ++itItem, i++)
    {
      os << "    " << (*itItem)->getObjectDisplayName() << ": "
         << mSolutionVariables[i] << std::endl;
    }
}

std::ostream &operator<<(std::ostream &os, const COptProblem & o)
{
  os << "Problem Description:" << std::endl;

  os << "Subtask: " << std::endl;

  if (o.mpSubtask)
    o.mpSubtask->getDescription().print(&os);
  else
    os << "No Subtask specified.";

  os << std::endl;

  if (o.mpObjectiveExpression)
    {
      os << "Objective Function:" << std::endl;
      os << "    " << o.mpObjectiveExpression->getDisplayString() << std::endl;
      os << std:: endl;
    }

  os << "List of Optimization Items:" << std::endl;

  std::vector< COptItem * >::const_iterator itItem =
    o.mpOptItems->begin();
  std::vector< COptItem * >::const_iterator endItem =
    o.mpOptItems->end();

  for (; itItem != endItem; ++itItem)
    os << "    " << **itItem << std::endl;

  itItem = o.mpConstraintItems->begin();
  endItem = o.mpConstraintItems->end();

  for (; itItem != endItem; ++itItem)
    os << "    " << **itItem << std::endl;

  return os;
}
