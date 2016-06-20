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

#include <cmath>

#include "copasi.h"

#include "CFitProblem.h"
#include "CFitItem.h"
#include "CFitTask.h"
#include "CExperimentSet.h"
#include "CExperiment.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "math/CMathContainer.h"
#include "model/CModel.h"
#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"
#include "steadystate/CSteadyStateTask.h"
#include "trajectory/CTrajectoryTask.h"
#include "trajectory/CTrajectoryProblem.h"
#include "utilities/CProcessReport.h"
#include "utilities/CCopasiException.h"
#include "utilities/CAnnotatedMatrix.h"

#include "lapack/blaswrap.h"           //use blas
#include "lapack/lapackwrap.h"        //use CLAPACK

//  Default constructor
CFitProblem::CFitProblem(const CTaskEnum::Task & type,
                         const CCopasiContainer * pParent):
  COptProblem(type, pParent),
  mpParmSteadyStateCN(NULL),
  mpParmTimeCourseCN(NULL),
  mpExperimentSet(NULL),
  mpSteadyState(NULL),
  mpTrajectory(NULL),
  mExperimentValues(0, 0),
  mExperimentConstraints(0, 0),
  mExperimentDependentValues(0),
  mpCrossValidationSet(NULL),
  mCrossValidationValues(0, 0),
  mCrossValidationConstraints(0, 0),
  mCrossValidationDependentValues(0),
  mCrossValidationSolutionValue(mWorstValue),
  mCrossValidationRMS(std::numeric_limits<C_FLOAT64>::quiet_NaN()),
  mCrossValidationSD(std::numeric_limits<C_FLOAT64>::quiet_NaN()),
  mCrossValidationObjective(mWorstValue),
  mThresholdCounter(0),
  mpTrajectoryProblem(NULL),
  mInitialState(),
  mpInitialStateTime(NULL),
  mResiduals(0),
  mRMS(std::numeric_limits<C_FLOAT64>::quiet_NaN()),
  mSD(std::numeric_limits<C_FLOAT64>::quiet_NaN()),
  mParameterSD(0),
  mFisher(0, 0),
  mpFisherMatrixInterface(NULL),
  mpFisherMatrix(NULL),
  mFisherEigenvalues(0, 0),
  mpFisherEigenvaluesMatrixInterface(NULL),
  mpFisherEigenvaluesMatrix(NULL),
  mFisherEigenvectors(0, 0),
  mpFisherEigenvectorsMatrixInterface(NULL),
  mpFisherEigenvectorsMatrix(NULL),
  mFisherScaled(0, 0),
  mpFisherScaledMatrixInterface(NULL),
  mpFisherScaledMatrix(NULL),
  mFisherScaledEigenvalues(0, 0),
  mpFisherScaledEigenvaluesMatrixInterface(NULL),
  mpFisherScaledEigenvaluesMatrix(NULL),
  mFisherScaledEigenvectors(0, 0),
  mpFisherScaledEigenvectorsMatrixInterface(NULL),
  mpFisherScaledEigenvectorsMatrix(NULL),
  mCorrelation(0, 0),
  mpCorrelationMatrixInterface(NULL),
  mpCorrelationMatrix(NULL),
  mpCreateParameterSets(NULL),
  mTrajectoryUpdate(false)

{
  initObjects();
  initializeParameter();
}

// copy constructor
CFitProblem::CFitProblem(const CFitProblem& src,
                         const CCopasiContainer * pParent):
  COptProblem(src, pParent),
  mpParmSteadyStateCN(NULL),
  mpParmTimeCourseCN(NULL),
  mpExperimentSet(NULL),
  mpSteadyState(NULL),
  mpTrajectory(NULL),
  mExperimentValues(0, 0),
  mExperimentConstraints(0, 0),
  mExperimentDependentValues(src.mExperimentDependentValues),
  mpCrossValidationSet(NULL),
  mCrossValidationValues(0, 0),
  mCrossValidationConstraints(0, 0),
  mCrossValidationDependentValues(src.mCrossValidationDependentValues),
  mCrossValidationSolutionValue(mWorstValue),
  mCrossValidationRMS(std::numeric_limits<C_FLOAT64>::quiet_NaN()),
  mCrossValidationSD(std::numeric_limits<C_FLOAT64>::quiet_NaN()),
  mCrossValidationObjective(mWorstValue),
  mThresholdCounter(0),
  mpTrajectoryProblem(NULL),
  mResiduals(src.mResiduals),
  mRMS(src.mRMS),
  mSD(src.mSD),
  mParameterSD(src.mParameterSD),
  mFisher(src.mFisher),
  mpFisherMatrixInterface(NULL),
  mpFisherMatrix(NULL),
  mFisherEigenvalues(src.mFisherEigenvalues),
  mpFisherEigenvaluesMatrixInterface(NULL),
  mpFisherEigenvaluesMatrix(NULL),
  mFisherEigenvectors(src.mFisherEigenvectors),
  mpFisherEigenvectorsMatrixInterface(NULL),
  mpFisherEigenvectorsMatrix(NULL),
  mFisherScaled(src.mFisherScaled),
  mpFisherScaledMatrixInterface(NULL),
  mpFisherScaledMatrix(NULL),
  mFisherScaledEigenvalues(src.mFisherScaledEigenvalues),
  mpFisherScaledEigenvaluesMatrixInterface(NULL),
  mpFisherScaledEigenvaluesMatrix(NULL),
  mFisherScaledEigenvectors(src.mFisherScaledEigenvectors),
  mpFisherScaledEigenvectorsMatrixInterface(NULL),
  mpFisherScaledEigenvectorsMatrix(NULL),
  mCorrelation(src.mCorrelation),
  mpCorrelationMatrixInterface(NULL),
  mpCorrelationMatrix(NULL),
  mpCreateParameterSets(NULL),
  mTrajectoryUpdate(false)
{
  initObjects();
  initializeParameter();
}

// Destructor
CFitProblem::~CFitProblem()
{
  pdelete(mpTrajectoryProblem);
  pdelete(mpFisherMatrixInterface);
  pdelete(mpFisherMatrix);
  pdelete(mpFisherEigenvaluesMatrixInterface);
  pdelete(mpFisherEigenvaluesMatrix);
  pdelete(mpFisherEigenvectorsMatrixInterface);
  pdelete(mpFisherEigenvectorsMatrix);
  pdelete(mpFisherScaledMatrixInterface);
  pdelete(mpFisherScaledMatrix);
  pdelete(mpFisherScaledEigenvaluesMatrixInterface);
  pdelete(mpFisherScaledEigenvaluesMatrix);
  pdelete(mpFisherScaledEigenvectorsMatrixInterface);
  pdelete(mpFisherScaledEigenvectorsMatrix);
  pdelete(mpCorrelationMatrixInterface);
  pdelete(mpCorrelationMatrix);
}

void CFitProblem::initObjects()
{
  addObjectReference("Validation Solution", mCrossValidationSolutionValue, CCopasiObject::ValueDbl);
  addObjectReference("Validation Objective", mCrossValidationObjective, CCopasiObject::ValueDbl);

  mpFisherMatrixInterface = new CCopasiMatrixInterface< CMatrix< C_FLOAT64 > >(&mFisher);
  mpFisherMatrix = new CArrayAnnotation("Fisher Information Matrix", this, mpFisherMatrixInterface, false);
  mpFisherMatrix->setDescription("Fisher Information Matrix");
  mpFisherMatrix->setDimensionDescription(0, "Parameters");
  mpFisherMatrix->setDimensionDescription(1, "Parameters");
  mpFisherMatrix->setMode(CArrayAnnotation::STRINGS);

  mpFisherEigenvaluesMatrixInterface = new CCopasiMatrixInterface< CMatrix< C_FLOAT64 > >(&mFisherEigenvalues);
  mpFisherEigenvaluesMatrix = new CArrayAnnotation("FIM Eigenvalues", this, mpFisherEigenvaluesMatrixInterface, false);
  mpFisherEigenvaluesMatrix->setDescription("FIM Eigenvalues");
  mpFisherEigenvaluesMatrix->setDimensionDescription(0, "Eigenvalues");
  mpFisherEigenvaluesMatrix->setDimensionDescription(1, "Result");
  mpFisherEigenvaluesMatrix->setMode(CArrayAnnotation::NUMBERS);

  mpFisherEigenvectorsMatrixInterface = new CCopasiMatrixInterface< CMatrix< C_FLOAT64 > >(&mFisherEigenvectors);
  mpFisherEigenvectorsMatrix = new CArrayAnnotation("FIM Eigenvectors", this, mpFisherEigenvectorsMatrixInterface, false);
  mpFisherEigenvectorsMatrix->setDescription("FIM Eigenvectors");
  mpFisherEigenvectorsMatrix->setDimensionDescription(0, "Eigenvectors");
  mpFisherEigenvectorsMatrix->setDimensionDescription(1, "Parameters");
  mpFisherEigenvectorsMatrix->setMode(0, CArrayAnnotation::NUMBERS);
  mpFisherEigenvectorsMatrix->setMode(1, CArrayAnnotation::STRINGS);

  mpFisherScaledMatrixInterface = new CCopasiMatrixInterface< CMatrix< C_FLOAT64 > >(&mFisherScaled);
  mpFisherScaledMatrix = new CArrayAnnotation("Fisher Information Matrix (scaled)", this, mpFisherScaledMatrixInterface, false);
  mpFisherScaledMatrix->setDescription("Fisher Information Matrix (scaled)");
  mpFisherScaledMatrix->setDimensionDescription(0, "Parameters");
  mpFisherScaledMatrix->setDimensionDescription(1, "Parameters");
  mpFisherScaledMatrix->setMode(CArrayAnnotation::STRINGS);

  mpFisherScaledEigenvaluesMatrixInterface = new CCopasiMatrixInterface< CMatrix< C_FLOAT64 > >(&mFisherScaledEigenvalues);
  mpFisherScaledEigenvaluesMatrix = new CArrayAnnotation("FIM Eigenvalues (scaled)", this, mpFisherScaledEigenvaluesMatrixInterface, false);
  mpFisherScaledEigenvaluesMatrix->setDescription("FIM Eigenvalues (scaled)");
  mpFisherScaledEigenvaluesMatrix->setDimensionDescription(0, "Eigenvalues");
  mpFisherScaledEigenvaluesMatrix->setDimensionDescription(1, "Result");
  mpFisherScaledEigenvaluesMatrix->setMode(CArrayAnnotation::NUMBERS);

  mpFisherScaledEigenvectorsMatrixInterface = new CCopasiMatrixInterface< CMatrix< C_FLOAT64 > >(&mFisherScaledEigenvectors);
  mpFisherScaledEigenvectorsMatrix = new CArrayAnnotation("FIM Eigenvectors (scaled)", this, mpFisherScaledEigenvectorsMatrixInterface, false);
  mpFisherScaledEigenvectorsMatrix->setDescription("FIM Eigenvectors (scaled)");
  mpFisherScaledEigenvectorsMatrix->setDimensionDescription(0, "Eigenvectors");
  mpFisherScaledEigenvectorsMatrix->setDimensionDescription(1, "Parameters");
  mpFisherScaledEigenvectorsMatrix->setMode(0, CArrayAnnotation::NUMBERS);
  mpFisherScaledEigenvectorsMatrix->setMode(1, CArrayAnnotation::STRINGS);

  mpCorrelationMatrixInterface = new CCopasiMatrixInterface< CMatrix< C_FLOAT64 > >(&mCorrelation);
  mpCorrelationMatrix = new CArrayAnnotation("Correlation Matrix", this, mpCorrelationMatrixInterface, false);
  mpCorrelationMatrix->setDescription("Correlation Matrix");
  mpCorrelationMatrix->setDimensionDescription(0, "Parameters");
  mpCorrelationMatrix->setDimensionDescription(1, "Parameters");
  mpCorrelationMatrix->setMode(CArrayAnnotation::STRINGS);
}

void CFitProblem::initializeParameter()
{
  removeParameter("Subtask");
  mpParmSubtaskCN = NULL;
  removeParameter("ObjectiveExpression");
  mpParmObjectiveExpression = NULL;
  *mpParmMaximize = false;

  mpParmSteadyStateCN = assertParameter("Steady-State", CCopasiParameter::CN, CCopasiObjectName(""));
  mpParmTimeCourseCN = assertParameter("Time-Course", CCopasiParameter::CN, CCopasiObjectName(""));
  mpCreateParameterSets = assertParameter("Create Parameter Sets", CCopasiParameter::BOOL, false);

  assertGroup("Experiment Set");

  assertGroup("Validation Set");

  elevateChildren();
}

void CFitProblem::setCreateParameterSets(const bool & create)
{*mpCreateParameterSets = create;}

const bool & CFitProblem::getCreateParameterSets() const
{return *mpCreateParameterSets;}

bool CFitProblem::elevateChildren()
{
  // This call is necessary since CFitProblem is derived from COptProblem.
  if (!COptProblem::elevateChildren()) return false;

  // Due to a naming conflict the following parameters may have been overwritten during
  // the load of a CopasiML file we replace them with default values if that was the case.
  mpParmSteadyStateCN = assertParameter("Steady-State", CCopasiParameter::CN, CCopasiObjectName(""));
  mpParmTimeCourseCN = assertParameter("Time-Course", CCopasiParameter::CN, CCopasiObjectName(""));

  CCopasiVectorN< CCopasiTask > * pTasks = NULL;
  CCopasiDataModel* pDataModel = getObjectDataModel();

  if (pDataModel)
    pTasks = pDataModel->getTaskList();

  if (pTasks == NULL)
    pTasks = dynamic_cast<CCopasiVectorN< CCopasiTask > *>(getObjectAncestor("Vector"));

  if (pTasks)
    {
      size_t i, imax = pTasks->size();

      if (!mpParmSteadyStateCN->compare(0, 5 , "Task_") ||
          *mpParmSteadyStateCN == "")
        for (i = 0; i < imax; i++)
          if (pTasks->operator[](i).getType() == CTaskEnum::steadyState)
            {
              *mpParmSteadyStateCN = pTasks->operator[](i).getCN();
              break;
            }

      if (!mpParmTimeCourseCN->compare(0, 5 , "Task_") ||
          *mpParmTimeCourseCN == "")
        for (i = 0; i < imax; i++)
          if (pTasks->operator[](i).getType() == CTaskEnum::timeCourse)
            {
              *mpParmTimeCourseCN = pTasks->operator[](i).getCN();
              break;
            }
    }

  std::map<std::string, std::string> ExperimentMap;
  CCopasiParameterGroup * pGroup;
  CExperiment * pExperiment;

  std::vector<CCopasiParameter *> * pExperiments =
    &getGroup("Experiment Set")->CCopasiParameter::getValue< CCopasiParameterGroup::elements >();
  std::vector<CCopasiParameter *>::iterator itExp;
  std::vector<CCopasiParameter *>::iterator endExp;

  for (itExp = pExperiments->begin(), endExp = pExperiments->end(); itExp != endExp; ++itExp)
    if ((pGroup = dynamic_cast< CCopasiParameterGroup * >(*itExp)) != NULL &&
        pGroup->getParameter("Key") != NULL)
      ExperimentMap[pGroup->getValue< std::string >("Key")] = (*itExp)->getObjectName();

  mpExperimentSet =
    elevate<CExperimentSet, CCopasiParameterGroup>(getGroup("Experiment Set"));

  if (!mpExperimentSet) return false;

  std::map<std::string, std::string>::iterator itMap;
  std::map<std::string, std::string>::iterator endMap;

  for (itMap = ExperimentMap.begin(), endMap = ExperimentMap.end(); itMap != endMap; ++itMap)
    {
      pExperiment = mpExperimentSet->getExperiment(itMap->second);
      itMap->second = pExperiment->CCopasiParameter::getKey();
      pExperiment->setValue("Key", itMap->second);
    }

  std::map<std::string, std::string> CrossValidationMap;

  // We have old CopasiML files (only snapshots and test releases), which use
  // "Cross Validation Set"
  pGroup = getGroup("Cross Validation Set");

  if (pGroup != NULL)
    {
      removeParameter("Validation Set");
      pGroup->setObjectName("Validation Set");
    }

  pExperiments =
    &getGroup("Validation Set")->CCopasiParameter::getValue< CCopasiParameterGroup::elements >();

  for (itExp = pExperiments->begin(), endExp = pExperiments->end(); itExp != endExp; ++itExp)
    if ((pGroup = dynamic_cast< CCopasiParameterGroup * >(*itExp)) != NULL &&
        pGroup->getParameter("Key") != NULL)
      CrossValidationMap[pGroup->getValue< std::string >("Key")] = (*itExp)->getObjectName();

  // This intermediate elevation step should be not needed but Viusal C fails when directly
  // going to the CCrossValidationSet.
  elevate< CExperimentSet, CCopasiParameterGroup >(getGroup("Validation Set"));
  mpCrossValidationSet =
    elevate< CCrossValidationSet, CCopasiParameterGroup >(getGroup("Validation Set"));

  if (!mpCrossValidationSet) return false;

  for (itMap = CrossValidationMap.begin(), endMap = CrossValidationMap.end(); itMap != endMap; ++itMap)
    {
      pExperiment = mpCrossValidationSet->getExperiment(itMap->second);
      itMap->second = pExperiment->CCopasiParameter::getKey();
      pExperiment->setValue("Key", itMap->second);
    }

  std::vector<COptItem * >::iterator it = mpOptItems->begin();
  std::vector<COptItem * >::iterator end = mpOptItems->end();

  for (; it != end; ++it)
    {
      if (!((*it) = elevate<CFitItem, COptItem>(*it)))
        return false;

      pExperiments =
        &(*it)->getParameter("Affected Experiments")->getValue< CCopasiParameterGroup::elements >();

      for (itExp = pExperiments->begin(), endExp = pExperiments->end(); itExp != endExp; ++itExp)
        (*itExp)->setValue(ExperimentMap[(*itExp)->getValue< std::string >()]);

      pExperiments =
        &(*it)->getParameter("Affected Cross Validation Experiments")->getValue< CCopasiParameterGroup::elements >();

      for (itExp = pExperiments->begin(), endExp = pExperiments->end(); itExp != endExp; ++itExp)
        (*itExp)->setValue(CrossValidationMap[(*itExp)->getValue< std::string >()]);
    }

  it = mpConstraintItems->begin();
  end = mpConstraintItems->end();

  for (; it != end; ++it)
    {
      if (!((*it) = elevate<CFitConstraint, COptItem>(*it)))
        return false;

      pExperiments =
        &(*it)->getParameter("Affected Experiments")->getValue< CCopasiParameterGroup::elements >();

      for (itExp = pExperiments->begin(), endExp = pExperiments->end(); itExp != endExp; ++itExp)
        (*itExp)->setValue(ExperimentMap[(*itExp)->getValue< std::string >()]);

      pExperiments =
        &(*it)->getParameter("Affected Cross Validation Experiments")->getValue< CCopasiParameterGroup::elements >();

      for (itExp = pExperiments->begin(), endExp = pExperiments->end(); itExp != endExp; ++itExp)
        (*itExp)->setValue(CrossValidationMap[(*itExp)->getValue< std::string >()]);
    }

  return true;
}

bool CFitProblem::setCallBack(CProcessReport * pCallBack)
{return COptProblem::setCallBack(pCallBack);}

bool CFitProblem::initialize()
{
  bool success = true;

  mHaveStatistics = false;
  mStoreResults = false;

  if (!COptProblem::initialize())
    {
      while (CCopasiMessage::peekLastMessage().getNumber() == MCOptimization + 5 ||
             CCopasiMessage::peekLastMessage().getNumber() == MCOptimization + 7)
        CCopasiMessage::getLastMessage();

      if (CCopasiMessage::getHighestSeverity() > CCopasiMessage::WARNING &&
          CCopasiMessage::peekLastMessage().getNumber() != MCCopasiMessage + 1)

        success = false;
    }

  CCopasiDataModel * pDataModel = getObjectDataModel();
  assert(pDataModel != NULL);

  // We only need to initialize the steady-state task if steady-state data is present.
  if (mpExperimentSet->hasDataForTaskType(CTaskEnum::steadyState))
    {
      mpSteadyState =
        dynamic_cast< CSteadyStateTask * >(const_cast< CCopasiObject * >(CObjectInterface::DataObject(getObjectFromCN(*mpParmSteadyStateCN))));

      if (mpSteadyState == NULL)
        {
          mpSteadyState =
            static_cast<CSteadyStateTask *>(&pDataModel->getTaskList()->operator[]("Steady-State"));
        }

      if (mpSteadyState == NULL) fatalError();

      *mpParmSteadyStateCN = mpSteadyState->getCN();
      mpSteadyState->initialize(CCopasiTask::NO_OUTPUT, NULL, NULL);
    }
  else
    {
      mpSteadyState = NULL;
    }

  pdelete(mpTrajectoryProblem);

  // We only need to initialize the trajectory task if time course data is present.
  if (mpExperimentSet->hasDataForTaskType(CTaskEnum::timeCourse))
    {
      mpTrajectory =
        dynamic_cast< CTrajectoryTask * >(const_cast< CCopasiObject * >(CObjectInterface::DataObject(getObjectFromCN(*mpParmTimeCourseCN))));

      if (mpTrajectory == NULL)
        {
          mpTrajectory =
            static_cast<CTrajectoryTask *>(&pDataModel->getTaskList()->operator[]("Time-Course"));
        }

      if (mpTrajectory == NULL) fatalError();

      *mpParmTimeCourseCN = mpTrajectory->getCN();

      // do not update initial values when running fit
      mTrajectoryUpdate = mpTrajectory->isUpdateModel();
      mpTrajectory->setUpdateModel(false);

      mpTrajectory->initialize(CCopasiTask::NO_OUTPUT, NULL, NULL);

      mpTrajectoryProblem =
        new CTrajectoryProblem(*static_cast<CTrajectoryProblem *>(mpTrajectory->getProblem()), NO_PARENT);
    }
  else
    {
      mpTrajectory = NULL;
    }

  mInitialState = mpContainer->getInitialState();
  mpInitialStateTime = mpContainer->getInitialState().array() + mpContainer->getCountFixedEventTargets();

  success &= mpExperimentSet->compile(mpContainer);

  // Initialize the object set which the experiment independent objects.
  std::vector< CObjectInterface::ObjectSet > ObjectSet;
  ObjectSet.resize(mpExperimentSet->getExperimentCount());
  size_t i, imax;

  for (i = 0, imax = mpExperimentSet->getExperimentCount(); i < imax; i++)
    {
      ObjectSet[i] = mpExperimentSet->getExperiment(i)->getIndependentObjects();
    }

  // Build a matrix of experiment and experiment local items.
  mExperimentValues.resize(mpExperimentSet->getExperimentCount(), mpOptItems->size());
  mExperimentValues = NULL;

  mExperimentInitialUpdates.resize(mpExperimentSet->getExperimentCount());

  std::vector<COptItem * >::iterator it = mpOptItems->begin();
  std::vector<COptItem * >::iterator end = mpOptItems->end();

  std::vector<COptItem * >::iterator itTmp;

  CFitItem * pItem;
  size_t j;
  size_t Index;

  imax = mSolutionVariables.size();

  mFisher.resize(imax, imax);
  mpFisherMatrix->resize();
  mFisherEigenvalues.resize(imax, 1);
  mpFisherEigenvaluesMatrix->resize();
  mFisherEigenvectors.resize(imax, imax);
  mpFisherEigenvectorsMatrix->resize();
  mFisherScaled.resize(imax, imax);
  mpFisherScaledMatrix->resize();
  mFisherScaledEigenvalues.resize(imax, 1);
  mpFisherScaledEigenvaluesMatrix->resize();
  mFisherScaledEigenvectors.resize(imax, imax);
  mpFisherScaledEigenvectorsMatrix->resize();
  mCorrelation.resize(imax, imax);
  mpCorrelationMatrix->resize();

  for (j = 0; it != end; ++it, j++)
    {
      pItem = static_cast<CFitItem *>(*it);
      pItem->updateBounds(mpOptItems->begin());

      // We cannot directly change the container values as multiple parameters
      // may point to the same value.
      mContainerVariables[j] = const_cast< C_FLOAT64 * >(&pItem->getLocalValue());

      std::string Annotation = pItem->getObjectDisplayName();

      imax = pItem->getExperimentCount();

      if (imax == 0)
        {
          for (i = 0, imax = mpExperimentSet->getExperimentCount(); i < imax; i++)
            {
              const CObjectInterface * object = pItem->getObject();

              if (object != NULL)
                {
                  mExperimentValues(i, j) = (C_FLOAT64 *)object->getValuePointer();
                  ObjectSet[i].insert(object);
                }
            }
        }
      else
        {
          Annotation += "; {" + pItem->getExperiments() + "}";

          for (i = 0; i < imax; i++)
            {
              if ((Index = mpExperimentSet->keyToIndex(pItem->getExperiment(i))) == C_INVALID_INDEX)
                return false;

              mExperimentValues(Index, j) = (C_FLOAT64 *) pItem->getObject()->getValuePointer();
              ObjectSet[Index].insert(pItem->getObject());
            };
        }

      mpFisherMatrix->setAnnotationString(0, j, Annotation);
      mpFisherMatrix->setAnnotationString(1, j, Annotation);
      mpFisherEigenvectorsMatrix->setAnnotationString(1, j, Annotation);
      mpFisherScaledMatrix->setAnnotationString(0, j, Annotation);
      mpFisherScaledMatrix->setAnnotationString(1, j, Annotation);
      mpFisherScaledEigenvectorsMatrix->setAnnotationString(1, j, Annotation);
      mpCorrelationMatrix->setAnnotationString(0, j, Annotation);
      mpCorrelationMatrix->setAnnotationString(1, j, Annotation);
    }

  // Create a joined sequence of update methods for parameters and independent values.
  for (i = 0, imax = mpExperimentSet->getExperimentCount(); i < imax; i++)
    {
      mpContainer->getInitialDependencies().getUpdateSequence(mExperimentInitialUpdates[i], CMath::UpdateMoieties, ObjectSet[i], mpContainer->getInitialStateObjects());
    }

  // Build a matrix of experiment and constraint items;
  mExperimentConstraints.resize(mpExperimentSet->getExperimentCount(),
                                mpConstraintItems->size());
  mExperimentConstraints = NULL;
  mExperimentConstraintUpdates.resize(mpExperimentSet->getExperimentCount());
  ObjectSet.clear();
  ObjectSet.resize(mpExperimentSet->getExperimentCount());

  it = mpConstraintItems->begin();
  end = mpConstraintItems->end();

  CFitConstraint * pConstraint;
  std::set< const CCopasiObject * >::const_iterator itDepend;
  std::set< const CCopasiObject * >::const_iterator endDepend;

  for (j = 0; it != end; ++it, j++)
    {
      pConstraint = static_cast<CFitConstraint *>(*it);
      itDepend = pConstraint->getDirectDependencies().begin();
      endDepend = pConstraint->getDirectDependencies().end();
      imax = pConstraint->getExperimentCount();

      if (imax == 0)
        {
          for (i = 0, imax = mpExperimentSet->getExperimentCount(); i < imax; i++)
            {
              mExperimentConstraints(i, j) = pConstraint;
              ObjectSet[i].insert(itDepend, endDepend);
            }
        }
      else
        {
          for (i = 0; i < imax; i++)
            {
              if ((Index = mpExperimentSet->keyToIndex(pConstraint->getExperiment(i))) == C_INVALID_INDEX)
                return false;

              mExperimentConstraints(Index, j) = pConstraint;
              ObjectSet[Index].insert(itDepend, endDepend);
            };
        }
    }

  for (i = 0, imax = mpExperimentSet->getExperimentCount(); i < imax; i++)
    {
      mpContainer->getTransientDependencies().getUpdateSequence(mExperimentConstraintUpdates[i], CMath::Default,
          mpContainer->getStateObjects(false), ObjectSet[i],
          mpContainer->getSimulationUpToDateObjects());
    }

  mExperimentDependentValues.resize(mpExperimentSet->getDataPointCount());

  if (!mpCrossValidationSet->compile(mpContainer)) return false;

  // Initialize the object set which the experiment independent objects.
  ObjectSet.clear();
  ObjectSet.resize(mpCrossValidationSet->getExperimentCount());

  for (i = 0, imax = mpCrossValidationSet->getExperimentCount(); i < imax; i++)
    {
      ObjectSet[i] = mpCrossValidationSet->getExperiment(i)->getIndependentObjects();
    }

  // Build a matrix of cross validation experiments  and local items.
  mCrossValidationValues.resize(mpCrossValidationSet->getExperimentCount(),
                                mpOptItems->size());
  mCrossValidationValues = NULL;

  mCrossValidationInitialUpdates.resize(mpCrossValidationSet->getExperimentCount());

  it = mpOptItems->begin();
  end = mpOptItems->end();

  for (j = 0; it != end; ++it, j++)
    {
      pItem = static_cast<CFitItem *>(*it);
      pItem->updateBounds(mpOptItems->begin());

      imax = pItem->getCrossValidationCount();

      if (imax == 0)
        {
          for (i = 0, imax = mpCrossValidationSet->getExperimentCount(); i < imax; i++)
            {
              mCrossValidationValues(i, j) = (C_FLOAT64 *) pItem->getObject()->getValuePointer();
              ObjectSet[i].insert(pItem->getObject());
            }
        }
      else
        {
          for (i = 0; i < imax; i++)
            {
              if ((Index = mpCrossValidationSet->keyToIndex(pItem->getCrossValidation(i))) == C_INVALID_INDEX)
                return false;

              mCrossValidationValues(Index, j) = (C_FLOAT64 *) pItem->getObject()->getValuePointer();
              ObjectSet[Index].insert(pItem->getObject());
            };
        }
    }

  // Create a joined sequence of update methods for parameters and independent values.
  for (i = 0, imax = mpCrossValidationSet->getExperimentCount(); i < imax; i++)
    {
      mpContainer->getInitialDependencies().getUpdateSequence(mCrossValidationInitialUpdates[i], CMath::UpdateMoieties, ObjectSet[i], mpContainer->getInitialStateObjects());
    }

  // Build a matrix of cross validation experiments and constraint items;
  mCrossValidationConstraints.resize(mpCrossValidationSet->getExperimentCount(),
                                     mpConstraintItems->size());
  mCrossValidationConstraints = NULL;
  mCrossValidationConstraintUpdates.resize(mpCrossValidationSet->getExperimentCount());
  ObjectSet.clear();
  ObjectSet.resize(mpCrossValidationSet->getExperimentCount());

  it = mpConstraintItems->begin();
  end = mpConstraintItems->end();

  for (j = 0; it != end; ++it, j++)
    {
      pConstraint = static_cast<CFitConstraint *>(*it);

      imax = pConstraint->getCrossValidationCount();

      if (imax == 0)
        {
          for (i = 0, imax = mpCrossValidationSet->getExperimentCount(); i < imax; i++)
            {
              mCrossValidationConstraints(i, j) = pConstraint;
              ObjectSet[i].insert(pConstraint->getObject());
            }
        }
      else
        {
          for (i = 0; i < imax; i++)
            {
              if ((Index = mpCrossValidationSet->keyToIndex(pConstraint->getCrossValidation(i))) == C_INVALID_INDEX)
                return false;

              mCrossValidationConstraints(Index, j) = pConstraint;
              ObjectSet[Index].insert(pConstraint->getObject());
            };
        }
    }

  for (i = 0, imax = mpCrossValidationSet->getExperimentCount(); i < imax; i++)
    {
      mpContainer->getTransientDependencies().getUpdateSequence(mCrossValidationConstraintUpdates[i], CMath::Default,
          mpContainer->getStateObjects(false), ObjectSet[i],
          mpContainer->getSimulationUpToDateObjects());
    }

  mCrossValidationDependentValues.resize(mpCrossValidationSet->getDataPointCount());

  mCrossValidationObjective = mWorstValue;
  mThresholdCounter = 0;

  setResidualsRequired(false);

  return success;
}

bool CFitProblem::checkFunctionalConstraints()
{
  std::vector< COptItem * >::const_iterator it = mpConstraintItems->begin();
  std::vector< COptItem * >::const_iterator end = mpConstraintItems->end();

  mConstraintCounter++;

  for (; it != end; ++it)
    if (static_cast<CFitConstraint *>(*it)->getConstraintViolation() > 0.0)
      {
        mFailedConstraintCounter++;
        return false;
      }

  return true;
}

/**
 * Utility function creating a parameter set for each experiment
 */
void CFitProblem::createParameterSet(const std::string & Name)
{
  CModel * pModel = const_cast< CModel * >(&mpContainer->getModel())
                    ;
  std::string origname = "PE: "  + UTCTimeStamp() + " Exp: " + Name;
  std::string name = origname;
  int count = 0;

  while (pModel->getModelParameterSets().getIndex(name) != C_INVALID_INDEX)
    {
      std::stringstream str; str << origname << " (" << ++count << ")";
      name = str.str();
    }

  CModelParameterSet* set = new CModelParameterSet(name);
  pModel->getModelParameterSets().add(set, true);
  set->createFromModel();
}

bool CFitProblem::calculate()
{
  mCounter += 1;
  bool Continue = true;

  size_t i, imax = mpExperimentSet->getExperimentCount();
  size_t j;
  size_t kmax;
  mCalculateValue = 0.0;

  CExperiment * pExp = NULL;

  C_FLOAT64 * Residuals = mResiduals.array();
  C_FLOAT64 * DependentValues = mExperimentDependentValues.array();
  C_FLOAT64 ** pUpdate = mExperimentValues.array();
  std::vector<COptItem *>::iterator itItem;
  std::vector<COptItem *>::iterator endItem = mpOptItems->end();
  std::vector<COptItem *>::iterator itConstraint;
  std::vector<COptItem *>::iterator endConstraint = mpConstraintItems->end();

  std::vector< Refresh *>::const_iterator itRefresh;
  std::vector< Refresh *>::const_iterator endRefresh;

  // Reset the constraints memory
  for (itConstraint = mpConstraintItems->begin(); itConstraint != endConstraint; ++itConstraint)
    static_cast<CFitConstraint *>(*itConstraint)->resetConstraintViolation();

  CFitConstraint **ppConstraint = mExperimentConstraints.array();
  CFitConstraint **ppConstraintEnd;

  try
    {
      for (i = 0; i < imax && Continue; i++) // For each experiment
        {
          pExp = mpExperimentSet->getExperiment(i);

          // set the global and experiment local fit item values.
          for (itItem = mpOptItems->begin(); itItem != endItem; itItem++, pUpdate++)
            if (pUpdate != NULL && *pUpdate != NULL)
              {
                **pUpdate = static_cast<CFitItem *>(*itItem)->getLocalValue();
              }

          kmax = pExp->getNumDataRows();

          switch (pExp->getExperimentType())
            {
              case CTaskEnum::steadyState:

                // set independent data
                for (j = 0; j < kmax && Continue; j++) // For each data row;
                  {
                    pExp->updateModelWithIndependentData(j);
                    mpContainer->applyUpdateSequence(mExperimentInitialUpdates[i]);

                    Continue = mpSteadyState->process(true);

                    if (!Continue)
                      {
                        mFailedCounter++;
                        mCalculateValue = mWorstValue;
                        break;
                      }

                    // We check after each simulation whether the constraints are violated.
                    // Make sure the constraint values are up to date.
                    mpContainer->applyUpdateSequence(mExperimentConstraintUpdates[i]);

                    ppConstraint = mExperimentConstraints[i];
                    ppConstraintEnd = ppConstraint + mExperimentConstraints.numCols();

                    for (; ppConstraint != ppConstraintEnd; ++ppConstraint)
                      if (*ppConstraint)(*ppConstraint)->calculateConstraintViolation();

                    if (mStoreResults)
                      mCalculateValue += pExp->sumOfSquaresStore(j, DependentValues);
                    else
                      mCalculateValue += pExp->sumOfSquares(j, Residuals);
                  }

                break;

              case CTaskEnum::timeCourse:

                size_t numIntermediateSteps;

                if (mStoreResults)
                  {
                    //calculate a reasonable number of intermediate points
                    numIntermediateSteps = 4; //TODO
                    //resize the storage for the extended time series
                    pExp->initExtendedTimeSeries(numIntermediateSteps * (kmax > 0 ? kmax - 1 : 0) + 1);
                  }

                for (j = 0; j < kmax && Continue; j++) // For each data row;
                  {
                    if (j)
                      {
                        if (mStoreResults)
                          {
                            //do additional intermediate steps for nicer display
                            C_FLOAT64 ttt;
                            size_t ic;

                            for (ic = 1; ic < numIntermediateSteps; ++ic)
                              {
                                ttt = pExp->getTimeData()[j - 1] + (pExp->getTimeData()[j] - pExp->getTimeData()[j - 1]) * (C_FLOAT64(ic) / numIntermediateSteps);
                                mpTrajectory->processStep(ttt);
                                //save the simulation results in the experiment
                                pExp->storeExtendedTimeSeriesData(ttt);
                              }
                          }

                        //do the regular step
                        mpTrajectory->processStep(pExp->getTimeData()[j]);
                      }
                    else
                      {
                        // Set independent data. A time course only has one set of
                        // independent data.
                        pExp->updateModelWithIndependentData(0);
                        mpContainer->applyUpdateSequence(mExperimentInitialUpdates[i]);

                        static_cast< CTrajectoryProblem * >(mpTrajectory->getProblem())->setStepNumber(1);
                        mpTrajectory->processStart(true);

                        if (pExp->getTimeData()[0] != *mpInitialStateTime)
                          {
                            mpTrajectory->processStep(pExp->getTimeData()[0]);
                          }
                      }

                    // We check after each simulation step whether the constraints are violated.
                    // Make sure the constraint values are up to date.
                    mpContainer->applyUpdateSequence(mExperimentConstraintUpdates[i]);

                    ppConstraint = mExperimentConstraints[i];
                    ppConstraintEnd = ppConstraint + mExperimentConstraints.numCols();

                    for (; ppConstraint != ppConstraintEnd; ++ppConstraint)
                      if (*ppConstraint)(*ppConstraint)->calculateConstraintViolation();

                    if (mStoreResults)
                      mCalculateValue += pExp->sumOfSquaresStore(j, DependentValues);
                    else
                      mCalculateValue += pExp->sumOfSquares(j, Residuals);

                    if (mStoreResults)
                      {
                        //additionally also store the the simulation result for the extended time series
                        pExp->storeExtendedTimeSeriesData(pExp->getTimeData()[j]);
                      }
                  }

                break;

              default:
                break;
            }

          // Restore the containers initial state. This includes all local reaction parameter
          // Additionally this state is synchronized, i.e. nothing to compute.
          mpContainer->setInitialState(mInitialState);
        }
    }

  catch (CCopasiException &)
    {
      // We do not want to clog the message cue.
      CCopasiMessage::getLastMessage();

      mFailedCounter++;
      mCalculateValue = mWorstValue;

      // Restore the containers initial state. This includes all local reaction parameter
      // Additionally this state is synchronized, i.e. nothing to compute.
      mpContainer->setInitialState(mInitialState);
    }

  catch (...)
    {
      mFailedCounter++;
      mCalculateValue = mWorstValue;

      // Restore the containers initial state. This includes all local reaction parameter
      // Additionally this state is synchronized, i.e. nothing to compute.
      mpContainer->setInitialState(mInitialState);
    }

  if (isnan(mCalculateValue))
    {
      mFailedCounter++;
      mCalculateValue = mWorstValue;
    }

  if (mpCallBack) return mpCallBack->progressItem(mhCounter);

  return true;
}

bool CFitProblem::restore(const bool & updateModel)
{
  bool success = true;

  if (mpTrajectory != NULL)
    {
      success &= mpTrajectory->restore();
      mpTrajectory->setUpdateModel(mTrajectoryUpdate);
    }

  if (mpSteadyState != NULL)
    success &= mpSteadyState->restore();

  if (mpTrajectoryProblem)
    *mpTrajectory->getProblem() = *mpTrajectoryProblem;

  success &= COptProblem::restore(updateModel);

  pdelete(mpTrajectoryProblem);

  return success;
}

// virtual
void CFitProblem::updateContainer(const bool & update)
{
  COptProblem::updateContainer(update);

  size_t i, imax = mpExperimentSet->getExperimentCount();
  std::vector<COptItem *>::iterator itItem;
  std::vector<COptItem *>::iterator endItem = mpOptItems->end();
  C_FLOAT64 ** pUpdate = mExperimentValues.array();

  CExperiment * pExp = NULL;

  for (i = 0; i < imax; i++) // For each experiment
    {
      pExp = mpExperimentSet->getExperiment(i);

      // set the global and experiment local fit item values.
      for (itItem = mpOptItems->begin(); itItem != endItem; itItem++, pUpdate++)
        if (pUpdate != NULL && *pUpdate != NULL)
          {
            **pUpdate = static_cast<CFitItem *>(*itItem)->getLocalValue();
          }
    }
}

void CFitProblem::createParameterSets()
{
  if (!*mpCreateParameterSets) return;

  // Store the current initial state
  CVector< C_FLOAT64 > CurrentInitialState = mpContainer->getInitialState();

  // We create an original parameter set and one for each experiment.
  // Restore the original data and create a parameter set
  updateContainer(false);
  mpContainer->applyUpdateSequence(mInitialRefreshSequence);
  mpContainer->pushInitialState();
  CVector< C_FLOAT64 > OriginalInitialState = mpContainer->getInitialState();

  createParameterSet("Original");

  // Apply the current solution values
  COptProblem::updateContainer(true);

  // Loop through all experiments and create
  size_t i, imax = mpExperimentSet->getExperimentCount();
  std::vector<COptItem *>::iterator itItem;
  std::vector<COptItem *>::iterator endItem = mpOptItems->end();
  C_FLOAT64 ** pUpdate = mExperimentValues.array();

  CExperiment * pExp = NULL;

  for (i = 0; i < imax; i++) // For each experiment
    {
      mpContainer->setInitialState(OriginalInitialState);

      pExp = mpExperimentSet->getExperiment(i);

      // set the global and experiment local fit item values.
      for (itItem = mpOptItems->begin(); itItem != endItem; itItem++, pUpdate++)
        if (*pUpdate)
          {
            **pUpdate = static_cast<CFitItem *>(*itItem)->getLocalValue();
          }

      // Update the independent data.
      pExp->updateModelWithIndependentData(0);

      // Synchronize the initial state.
      mpContainer->applyUpdateSequence(mExperimentInitialUpdates[i]);

      mpContainer->pushInitialState();
      createParameterSet(pExp->getObjectName());
    }

  // Restore the current initial state
  mpContainer->setInitialState(CurrentInitialState);
}
void CFitProblem::print(std::ostream * ostream) const
{*ostream << *this;}

void CFitProblem::printResult(std::ostream * ostream) const
{
  std::ostream & os = *ostream;

  if (mSolutionVariables.size() == 0)
    {
      return;
    }

  os << "Objective Function Value:\t" << mSolutionValue << std::endl;
  os << "Standard Deviation:\t" << mSD << std::endl;

  CCopasiTimeVariable CPUTime = const_cast<CFitProblem *>(this)->mCPUTime.getElapsedTime();

  os << "Function Evaluations:\t" << mCounter << std::endl;
  os << "CPU Time [s]:\t"
     << CCopasiTimeVariable::LL2String(CPUTime.getSeconds(), 1) << "."
     << CCopasiTimeVariable::LL2String(CPUTime.getMilliSeconds(true), 3) << std::endl;
  os << "Evaluations/Second [1/s]:\t" << mCounter / (C_FLOAT64)(CPUTime.getMilliSeconds() / 1e3) << std::endl;
  os << std::endl;

  std::vector< COptItem * >::const_iterator itItem =
    mpOptItems->begin();
  std::vector< COptItem * >::const_iterator endItem =
    mpOptItems->end();

  CFitItem * pFitItem;
  CExperiment * pExperiment;

  size_t i, j;

  os << "\tParameter\tValue\tGradient\tStandard Deviation" << std::endl;

  for (i = 0; itItem != endItem; ++itItem, i++)
    {
      os << "\t" << (*itItem)->getObjectDisplayName();
      pFitItem = static_cast<CFitItem *>(*itItem);

      if (pFitItem->getExperimentCount() != 0)
        {
          os << " (";

          for (j = 0; j < pFitItem->getExperimentCount(); j++)
            {
              if (j) os << ", ";

              pExperiment =
                dynamic_cast< CExperiment * >(CCopasiRootContainer::getKeyFactory()->get(pFitItem->getExperiment(j)));

              if (pExperiment)
                os << pExperiment->getObjectName();
            }

          os << ")";
        }

      if (mHaveStatistics)
        {
          os << ":\t" << mSolutionVariables[i];
          os << "\t" << mGradient[i];
          os << "\t" << mParameterSD[i];
        }
      else
        {
          os << ":\t" << std::numeric_limits<C_FLOAT64>::quiet_NaN();
          os << "\t" << std::numeric_limits<C_FLOAT64>::quiet_NaN();
          os << "\t" << std::numeric_limits<C_FLOAT64>::quiet_NaN();
        }

      os << std::endl;
    }

  os << std::endl;

  size_t k, kmax = mpExperimentSet->getExperimentCount();

  for (k = 0; k < kmax; k++)
    {
      mpExperimentSet->getExperiment(k)->printResult(ostream);
      os << std::endl;
    }

  if (*mpParmCalculateStatistics)
    {
      os << "Fisher Information Matrix:" << std::endl;
      os << "  " << mFisher << std::endl;

      os << "FIM Eigenvalues:" << std::endl;
      os << "  " << mFisherEigenvalues << std::endl;

      os << "FIM Eigenvectors corresponding to Eigenvalues:" << std::endl;
      os << "  " << mFisherEigenvectors << std::endl;

      os << "Fisher Information Matrix (scaled):" << std::endl;
      os << "  " << mFisherScaled << std::endl;

      os << "FIM Eigenvalues (scaled):" << std::endl;
      os << "  " << mFisherScaledEigenvalues << std::endl;

      os << "FIM Eigenvectors (scaled) corresponding to Eigenvalues:" << std::endl;
      os << "  " << mFisherScaledEigenvectors << std::endl;

      os << "Correlation Matrix:" << std::endl;
      os << "  " << mCorrelation << std::endl;
    }
}

std::ostream &operator<<(std::ostream &os, const CFitProblem & o)
{
  os << "Problem Description:" << std::endl;

  os << "Subtask: " << std::endl;

  if (o.mpSteadyState)
    o.mpSteadyState->getDescription().print(&os);

  if (o.mpTrajectory)
    o.mpTrajectory->getDescription().print(&os);

  if (!o.mpTrajectory && !o.mpSteadyState)
    os << "No Subtask specified.";

  os << std::endl;

  os << "List of Fitting Items:" << std::endl;

  std::vector< COptItem * >::const_iterator itItem =
    o.mpOptItems->begin();
  std::vector< COptItem * >::const_iterator endItem =
    o.mpOptItems->end();

  for (; itItem != endItem; ++itItem)
    os << "    " << *static_cast<CFitItem *>(*itItem) << std::endl;

  itItem = o.mpConstraintItems->begin();
  endItem = o.mpConstraintItems->end();

  for (; itItem != endItem; ++itItem)
    os << "    " << *static_cast<CFitItem *>(*itItem) << std::endl;

  return os;
}

void CFitProblem::updateInitialState()
{
  mInitialState = mpContainer->getInitialState();
}

bool CFitProblem::createObjectiveFunction()
{return true;}

bool CFitProblem::setResidualsRequired(const bool & required)
{
  if (required)
    {
      mResiduals.resize(mpExperimentSet->getDataPointCount(), false);
      size_t ii;

      for (ii = 0; ii < mResiduals.size(); ++ii)
        mResiduals[ii] = 0.0; //std::numeric_limits<C_FLOAT64>::quiet_NaN();

      //it would make sense to initialize it with NaN,
      //but some methods expect 0.0 as the residual for a missing data point
    }
  else
    mResiduals.resize(0);

  return true;
}

const CVector< C_FLOAT64 > & CFitProblem::getResiduals() const
{
  return mResiduals;
}

bool CFitProblem::calculateStatistics(const C_FLOAT64 & factor,
                                      const C_FLOAT64 & resolution)
{
  // Set the current values to the solution values.
  size_t i, imax = mSolutionVariables.size();
  size_t l;

  mRMS = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mSD = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mCrossValidationRMS = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mCrossValidationSD = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mParameterSD.resize(imax);
  mParameterSD = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mFisher = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mFisherEigenvectors = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mFisherEigenvalues = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mFisherScaled = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mFisherScaledEigenvectors = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mFisherScaledEigenvalues = std::numeric_limits<C_FLOAT64>::quiet_NaN();
  mGradient.resize(imax);
  mGradient = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  // Recalculate the best solution.
  for (i = 0; i < imax; i++)
    {
      *mContainerVariables[i] = mSolutionVariables[i];
    }

  // For Output
  mStoreResults = true;
  calculate();
  mStoreResults = false;

  // The statistics need to be calculated for the result, i.e., now.
  mpExperimentSet->calculateStatistics();
  size_t ValidDataCount = mpExperimentSet->getValidValueCount();

  if (ValidDataCount)
    mRMS = sqrt(mSolutionValue / ValidDataCount);

  if (ValidDataCount > imax)
    mSD = sqrt(mSolutionValue / (ValidDataCount - imax));

  calculateCrossValidation();

  mpCrossValidationSet->calculateStatistics();

  size_t lmax = this->mCrossValidationDependentValues.size();

  if (lmax)
    mCrossValidationRMS = sqrt(mCrossValidationSolutionValue / lmax);

  if (lmax > imax)
    mCrossValidationSD = sqrt(mCrossValidationSolutionValue / (lmax - imax));

  mHaveStatistics = true;

  // Make sure the timer is accurate.
  mCPUTime.calculateValue();

  if (mSolutionValue == mWorstValue)
    return false;

  if (*mpParmCalculateStatistics)
    {
      setResidualsRequired(true);
      size_t j, jmax = mResiduals.size();
      calculate();

      // Keep the results
      CVector< C_FLOAT64 > SolutionResiduals = mResiduals;

      CMatrix< C_FLOAT64 > DeltaResidualDeltaParameter;

      bool CalculateFIM = true;

      try
        {
          DeltaResidualDeltaParameter.resize(imax, jmax);
        }

      catch (CCopasiException & /*Exception*/)
        {
          CalculateFIM = false;
        }

      C_FLOAT64 * pDeltaResidualDeltaParameter = DeltaResidualDeltaParameter.array();

      C_FLOAT64 Current;
      C_FLOAT64 Delta;

      // Calculate the gradient
      for (i = 0; i < imax; i++)
        {
          Current = mSolutionVariables[i];

          if (fabs(Current) > resolution)
            {
              *mContainerVariables[i] = Current * (1.0 + factor);
              Delta = 1.0 / (Current * factor);
            }
          else
            {
              *mContainerVariables[i] = resolution;
              Delta = 1.0 / resolution;
            }

          calculate();

          mGradient[i] = (mCalculateValue - mSolutionValue) * Delta;

          C_FLOAT64 * pSolutionResidual = SolutionResiduals.array();
          C_FLOAT64 * pResidual = mResiduals.array();

          if (CalculateFIM)
            for (j = 0; j < jmax; j++, ++pDeltaResidualDeltaParameter, ++pSolutionResidual, ++pResidual)
              *pDeltaResidualDeltaParameter = (*pResidual - *pSolutionResidual) * Delta;

          // Restore the value
          *mContainerVariables[i] = Current;
        }

      if (!CalculateFIM)
        {
          // Make sure the timer is accurate.
          mCPUTime.calculateValue();

          CCopasiMessage(CCopasiMessage::WARNING, MCFitting + 13);
          return false;
        }

      // Construct the fisher information matrix
      for (i = 0; i < imax; i++)
        for (l = 0; l <= i; l++)
          {
            C_FLOAT64 & tmp = mFisher(i, l);

            tmp = 0.0;

            C_FLOAT64 * pI = DeltaResidualDeltaParameter[i];
            C_FLOAT64 * pL = DeltaResidualDeltaParameter[l];

            for (j = 0; j < jmax; j++, ++pI, ++pL)
              tmp += *pI **pL;

            tmp *= 2.0;

            // The Fisher matrix is symmetric.
            if (l != i)
              mFisher(l, i) = tmp;
          }

      // Construct the FIM Eigenvalue/-vector matrix

      /* int dsyev_(char *jobz, char *uplo, integer *n, doublereal *a,
         integer *lda, doublereal *w, doublereal *work, integer *lwork,
         integer *info) */

      /*  Purpose */
      /*  ======= */

      /*  DSYEV computes all eigenvalues and, optionally, eigenvectors of a */
      /*  real symmetric matrix A. */

      /*  Arguments */
      /*  ========= */

      /*  JOBZ    (input) CHARACTER*1 */
      /*          = 'N':  Compute eigenvalues only; */
      /*          = 'V':  Compute eigenvalues and eigenvectors. */

      /*  UPLO    (input) CHARACTER*1 */
      /*          = 'U':  Upper triangle of A is stored; */
      /*          = 'L':  Lower triangle of A is stored. */

      /*  N       (input) INTEGER */
      /*          The order of the matrix A.  N >= 0. */

      /*  A       (input/output) DOUBLE PRECISION array, dimension (LDA, N) */
      /*          On entry, the symmetric matrix A.  If UPLO = 'U', the */
      /*          leading N-by-N upper triangular part of A contains the */
      /*          upper triangular part of the matrix A.  If UPLO = 'L', */
      /*          the leading N-by-N lower triangular part of A contains */
      /*          the lower triangular part of the matrix A. */
      /*          On exit, if JOBZ = 'V', then if INFO = 0, A contains the */
      /*          orthonormal eigenvectors of the matrix A. */
      /*          If JOBZ = 'N', then on exit the lower triangle (if UPLO='L') */
      /*          or the upper triangle (if UPLO='U') of A, including the */
      /*          diagonal, is destroyed. */

      /*  LDA     (input) INTEGER */
      /*          The leading dimension of the array A.  LDA >= max(1,N). */

      /*  W       (output) DOUBLE PRECISION array, dimension (N) */
      /*          If INFO = 0, the eigenvalues in ascending order. */

      /*  WORK    (workspace/output) DOUBLE PRECISION array, dimension (MAX(1,LWORK)) */
      /*          On exit, if INFO = 0, WORK(1) returns the optimal LWORK. */

      /*  LWORK   (input) INTEGER */
      /*          The length of the array WORK.  LWORK >= max(1,3*N-1). */
      /*          For optimal efficiency, LWORK >= (NB+2)*N, */
      /*          where NB is the blocksize for DSYTRD returned by ILAENV. */

      /*          If LWORK = -1, then a workspace query is assumed; the routine */
      /*          only calculates the optimal size of the WORK array, returns */
      /*          this value as the first entry of the WORK array, and no error */
      /*          message related to LWORK is issued by XERBLA. */

      /*  INFO    (output) INTEGER */
      /*          = 0:  successful exit */
      /*          < 0:  if INFO = -i, the i-th argument had an illegal value */
      /*          > 0:  if INFO = i, the algorithm failed to converge; i */
      /*                off-diagonal elements of an intermediate tridiagonal */
      /*                form did not converge to zero. */

      mFisherEigenvectors = mFisher;

      char JOBZ = 'V'; //also compute eigenvectors
      char UPLO = 'U'; //upper triangle
      C_INT N = (C_INT) imax;
      C_INT LDA = std::max< C_INT >(1, N);
      CVector<C_FLOAT64> WORK; //WORK space
      WORK.resize(1);
      C_INT LWORK = -1; //first query the memory need
      C_INT INFO = 0;

      dsyev_(&JOBZ,
             &UPLO,
             &N,
             mFisherEigenvectors.array(),
             &LDA,
             mFisherEigenvalues.array(),
             WORK.array(),
             &LWORK,
             &INFO);

      LWORK = (C_INT) WORK[0];
      WORK.resize(LWORK);

      //now do the real calculation
      dsyev_(&JOBZ,
             &UPLO,
             &N,
             mFisherEigenvectors.array(),
             &LDA,
             mFisherEigenvalues.array(),
             WORK.array(),
             &LWORK,
             &INFO);

      if (INFO != 0)
        {
          CCopasiMessage(CCopasiMessage::WARNING, MCFitting + 14);

          mFisherEigenvectors = std::numeric_limits<C_FLOAT64>::quiet_NaN();
          mFisherEigenvalues = std::numeric_limits<C_FLOAT64>::quiet_NaN();
        }

      for (i = 0; i < imax; i++)
        {
          for (j = 0; j < imax; j++)
            {
              mFisherScaled[i][j] = mFisher[i][j] * mSolutionVariables[i] * mSolutionVariables[j];
            }
        }

      mFisherScaledEigenvectors = mFisherScaled;

      //now do the real calculation
      dsyev_(&JOBZ,
             &UPLO,
             &N,
             mFisherScaledEigenvectors.array(),
             &LDA,
             mFisherScaledEigenvalues.array(),
             WORK.array(),
             &LWORK,
             &INFO);

      if (INFO != 0)
        {
          CCopasiMessage(CCopasiMessage::WARNING, MCFitting + 15);

          mFisherScaledEigenvectors = std::numeric_limits<C_FLOAT64>::quiet_NaN();
          mFisherScaledEigenvalues = std::numeric_limits<C_FLOAT64>::quiet_NaN();
        }

      mCorrelation = mFisher;

      // The Fisher Information matrix is a symmetric positive semidefinit matrix.

      /* int dpotrf_(char *uplo, integer *n, doublereal *a,
       *             integer *lda, integer *info);
       *
       *
       *  Purpose
       *  =======
       *
       *  DPOTRF computes the Cholesky factorization of a real symmetric
       *  positive definite matrix A.
       *
       *  The factorization has the form
       *     A = U**T * U, if UPLO = 'U', or
       *     A = L  * L**T, if UPLO = 'L',
       *  where U is an upper triangular matrix and L is lower triangular.
       *
       *  This is the block version of the algorithm, calling Level 3 BLAS.
       *
       *  Arguments
       *  =========
       *
       *  UPLO    (input) CHARACTER*1
       *          = 'U':  Upper triangle of A is stored;
       *          = 'L':  Lower triangle of A is stored.
       *
       *  N       (input) INTEGER
       *          The order of the matrix A.  N >= 0.
       *
       *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
       *          On entry, the symmetric matrix A.  If UPLO = 'U', the leading
       *          N-by-N upper triangular part of A contains the upper
       *          triangular part of the matrix A, and the strictly lower
       *          triangular part of A is not referenced.  If UPLO = 'L', the
       *          leading N-by-N lower triangular part of A contains the lower
       *          triangular part of the matrix A, and the strictly upper
       *          triangular part of A is not referenced.
       *
       *          On exit, if INFO = 0, the factor U or L from the Cholesky
       *          factorization A = U**T*U or A = L*L**T.
       *
       *  LDA     (input) INTEGER
       *          The leading dimension of the array A.  LDA >= max(1,N).
       *
       *  INFO    (output) INTEGER
       *          = 0:  successful exit
       *          < 0:  if INFO = -i, the i-th argument had an illegal value
       *          > 0:  if INFO = i, the leading minor of order i is not
       *                positive definite, and the factorization could not be
       *                completed.
       *
       */

      char U = 'U';

      dpotrf_(&U, &N, mCorrelation.array(), &N, &INFO);

      if (INFO)
        {
          mCorrelation = std::numeric_limits<C_FLOAT64>::quiet_NaN();
          mParameterSD = std::numeric_limits<C_FLOAT64>::quiet_NaN();

          CCopasiMessage(CCopasiMessage::WARNING, MCFitting + 12);

          // Make sure the timer is accurate.
          mCPUTime.calculateValue();

          return false;
        }

      /* int dpotri_(char *uplo, integer *n, doublereal *a,
       *             integer *lda, integer *info);
       *
       *
       *  Purpose
       *  =======
       *
       *  DPOTRI computes the inverse of a real symmetric positive definite
       *  matrix A using the Cholesky factorization A = U**T*U or A = L*L**T
       *  computed by DPOTRF.
       *
       *  Arguments
       *  =========
       *
       *  UPLO    (input) CHARACTER*1
       *          = 'U':  Upper triangle of A is stored;
       *          = 'L':  Lower triangle of A is stored.
       *
       *  N       (input) INTEGER
       *          The order of the matrix A.  N >= 0.
       *
       *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
       *          On entry, the triangular factor U or L from the Cholesky
       *          factorization A = U**T*U or A = L*L**T, as computed by
       *          DPOTRF.
       *          On exit, the upper or lower triangle of the (symmetric)
       *          inverse of A, overwriting the input factor U or L.
       *
       *  LDA     (input) INTEGER
       *          The leading dimension of the array A.  LDA >= max(1,N).
       *
       *  INFO    (output) INTEGER
       *          = 0:  successful exit
       *          < 0:  if INFO = -i, the i-th argument had an illegal value
       *          > 0:  if INFO = i, the (i,i) element of the factor U or L is
       *                zero, and the inverse could not be computed.
       *
       */

      dpotri_(&U, &N, mCorrelation.array(), &N, &INFO);

      if (INFO)
        {
          mCorrelation = std::numeric_limits<C_FLOAT64>::quiet_NaN();
          mParameterSD = std::numeric_limits<C_FLOAT64>::quiet_NaN();

          CCopasiMessage(CCopasiMessage::WARNING, MCFitting + 1, INFO);

          // Make sure the timer is accurate.
          mCPUTime.calculateValue();

          return false;
        }

      // Assure that the inverse is completed.

      for (i = 0; i < imax; i++)
        for (l = 0; l < i; l++)
          mCorrelation(l, i) = mCorrelation(i, l);

      CVector< C_FLOAT64 > S(imax);

      // rescale the lower bound of the covariant matrix to have unit diagonal
      for (i = 0; i < imax; i++)
        {
          C_FLOAT64 & tmp = S[i];

          if (mCorrelation(i, i) > 0.0)
            {
              tmp = 1.0 / sqrt(mCorrelation(i, i));
              mParameterSD[i] = mSD / tmp;
            }
          else if (mCorrelation(i, i) < 0.0)
            {
              tmp = 1.0 / sqrt(- mCorrelation(i, i));
              mParameterSD[i] = mSD / tmp;
            }
          else
            {
              mParameterSD[i] = mWorstValue;
              tmp = 1.0;
              mCorrelation(i, i) = 1.0;
            }
        }

      for (i = 0; i < imax; i++)
        for (l = 0; l < imax; l++)
          mCorrelation(i, l) *= S[i] * S[l];

      setResidualsRequired(false);
      mStoreResults = true;
      // This is necessary so that CExperiment::printResult shows the correct data.
      calculate();

      // Make sure the timer is accurate.
      mCPUTime.calculateValue();
    }

  mStoreResults = false;

  return true;
}

const C_FLOAT64 & CFitProblem::getRMS() const
{return mRMS;}

const C_FLOAT64 & CFitProblem::getStdDeviation() const
{return mSD;}

const CVector< C_FLOAT64 > & CFitProblem::getVariableStdDeviations() const
{return mParameterSD;}

CArrayAnnotation & CFitProblem::getFisherInformation() const
{return *mpFisherMatrix;}

CArrayAnnotation & CFitProblem::getFisherInformationEigenvalues() const
{return *mpFisherEigenvaluesMatrix;}

CArrayAnnotation & CFitProblem::getFisherInformationEigenvectors() const
{return *mpFisherEigenvectorsMatrix;}

CArrayAnnotation & CFitProblem::getScaledFisherInformation() const
{return *mpFisherScaledMatrix;}

CArrayAnnotation & CFitProblem::getScaledFisherInformationEigenvalues() const
{return *mpFisherScaledEigenvaluesMatrix;}

CArrayAnnotation & CFitProblem::getScaledFisherInformationEigenvectors() const
{return *mpFisherScaledEigenvectorsMatrix;}

CArrayAnnotation & CFitProblem::getCorrelations() const
{return *mpCorrelationMatrix;}

const CExperimentSet & CFitProblem::getExperiementSet() const
{return *mpExperimentSet;}

const CCrossValidationSet & CFitProblem::getCrossValidationSet() const
{return *mpCrossValidationSet;}

bool CFitProblem::setSolution(const C_FLOAT64 & value,
                              const CVector< C_FLOAT64 > & variables)
{
  bool Continue = COptProblem::setSolution(value, variables);

  if (Continue && mpCrossValidationSet->getExperimentCount() > 0)
    Continue = calculateCrossValidation();

  return Continue;
}

const C_FLOAT64 & CFitProblem::getCrossValidationSolutionValue() const
{return mCrossValidationSolutionValue;}

const C_FLOAT64 & CFitProblem::getCrossValidationRMS() const
{return mCrossValidationRMS;}

const C_FLOAT64 & CFitProblem::getCrossValidationSD() const
{return mCrossValidationSD;}

bool CFitProblem::calculateCrossValidation()
{
  mCounter += 1;
  bool Continue = true;

  unsigned i, imax = mpCrossValidationSet->getExperimentCount();
  unsigned j;
  unsigned kmax;
  C_FLOAT64 CalculateValue = 0.0;

  CExperiment * pExp = NULL;

  C_FLOAT64 * Residuals = NULL;
  C_FLOAT64 * DependentValues = mCrossValidationDependentValues.array();

  C_FLOAT64 ** pUpdate = mExperimentValues.array();

  C_FLOAT64 * pSolution = mSolutionVariables.array();
  C_FLOAT64 * pSolutionEnd = pSolution + mSolutionVariables.size();

  std::vector<COptItem *>::iterator itConstraint;
  std::vector<COptItem *>::iterator endConstraint = mpConstraintItems->end();

  std::vector< Refresh *>::const_iterator itRefresh;
  std::vector< Refresh *>::const_iterator endRefresh;

  // Reset the constraints memory
  for (itConstraint = mpConstraintItems->begin(); itConstraint != endConstraint; ++itConstraint)
    static_cast<CFitConstraint *>(*itConstraint)->resetConstraintViolation();

  CFitConstraint **ppConstraint = mCrossValidationConstraints.array();
  CFitConstraint **ppConstraintEnd;

  try
    {
      for (i = 0; i < imax && Continue; i++) // For each CrossValidation
        {
          pExp = mpCrossValidationSet->getExperiment(i);

          // set the global and CrossValidation local fit item values.
          for (; pSolution != pSolutionEnd; pSolution++, pUpdate++)
            {
              if (*pUpdate)
                {
                  **pUpdate = *pSolution;
                }
            }

          kmax = pExp->getNumDataRows();

          switch (pExp->getExperimentType())
            {
              case CTaskEnum::steadyState:

                // set independent data
                for (j = 0; j < kmax && Continue; j++) // For each data row;
                  {
                    pExp->updateModelWithIndependentData(j);
                    mpContainer->applyUpdateSequence(mCrossValidationInitialUpdates[i]);

                    Continue &= mpSteadyState->process(true);

                    if (!Continue)
                      {
                        CalculateValue = mWorstValue;
                        break;
                      }

                    // We check after each simulation whether the constraints are violated.
                    // Make sure the constraint values are up to date.
                    mpContainer->applyUpdateSequence(mCrossValidationConstraintUpdates[i]);

                    ppConstraint = mCrossValidationConstraints[i];
                    ppConstraintEnd = ppConstraint + mCrossValidationConstraints.numCols();

                    for (; ppConstraint != ppConstraintEnd; ++ppConstraint)
                      if (*ppConstraint)(*ppConstraint)->checkConstraint();

                    if (mStoreResults)
                      CalculateValue += pExp->sumOfSquaresStore(j, DependentValues);
                    else
                      CalculateValue += pExp->sumOfSquares(j, Residuals);
                  }

                break;

              case CTaskEnum::timeCourse:

                size_t numIntermediateSteps;

                if (mStoreResults)
                  {
                    //calculate a reasonable number of intermediate points
                    numIntermediateSteps = 4; //TODO
                    //resize the storage for the extended time series
                    pExp->initExtendedTimeSeries(numIntermediateSteps * kmax - numIntermediateSteps + 1);
                  }

                for (j = 0; j < kmax && Continue; j++) // For each data row;
                  {
                    if (j)
                      {
                        if (mStoreResults)
                          {
                            //do additional intermediate steps for nicer display
                            C_FLOAT64 ttt;
                            size_t ic;

                            for (ic = 1; ic < numIntermediateSteps; ++ic)
                              {
                                ttt = pExp->getTimeData()[j - 1] + (pExp->getTimeData()[j] - pExp->getTimeData()[j - 1]) * (C_FLOAT64(ic) / numIntermediateSteps);
                                mpTrajectory->processStep(ttt);
                                //save the simulation results in the experiment
                                pExp->storeExtendedTimeSeriesData(ttt);
                              }
                          }

                        //do the regular step
                        mpTrajectory->processStep(pExp->getTimeData()[j]);
                      }
                    else
                      {
                        // Set independent data. A time course only has one set of
                        // independent data.
                        pExp->updateModelWithIndependentData(0);

                        // We need to apply the parameter and independent
                        // value updates as one unit.
                        mpContainer->applyUpdateSequence(mCrossValidationInitialUpdates[i]);

                        static_cast< CTrajectoryProblem * >(mpTrajectory->getProblem())->setStepNumber(1);
                        mpTrajectory->processStart(true);

                        if (pExp->getTimeData()[0] != *mpInitialStateTime)
                          {
                            mpTrajectory->processStep(pExp->getTimeData()[0]);
                          }
                      }

                    // We check after each simulation whether the constraints are violated.
                    // Make sure the constraint values are up to date.
                    mpContainer->applyUpdateSequence(mCrossValidationConstraintUpdates[i]);

                    ppConstraint = mCrossValidationConstraints[i];
                    ppConstraintEnd = ppConstraint + mCrossValidationConstraints.numCols();

                    for (; ppConstraint != ppConstraintEnd; ++ppConstraint)
                      if (*ppConstraint)(*ppConstraint)->checkConstraint();

                    if (mStoreResults)
                      CalculateValue += pExp->sumOfSquaresStore(j, DependentValues);
                    else
                      CalculateValue += pExp->sumOfSquares(j, Residuals);

                    if (mStoreResults)
                      {
                        //additionally also store the the simulation result for the extended time series
                        pExp->storeExtendedTimeSeriesData(pExp->getTimeData()[j]);
                      }
                  }

                break;

              default:
                break;
            }

          // Restore the containers initial state. This includes all local reaction parameter
          // Additionally this state is synchronized, i.e. nothing to compute.
          mpContainer->setInitialState(mInitialState);
        }
    }

  catch (CCopasiException &)
    {
      // We do not want to clog the message cue.
      CCopasiMessage::getLastMessage();

      mFailedCounter++;
      CalculateValue = mWorstValue;

      // Restore the containers initial state. This includes all local reaction parameter
      // Additionally this state is synchronized, i.e. nothing to compute.
      mpContainer->setInitialState(mInitialState);
    }

  catch (...)
    {
      mFailedCounter++;
      CalculateValue = mWorstValue;

      // Restore the containers initial state. This includes all local reaction parameter
      // Additionally this state is synchronized, i.e. nothing to compute.
      mpContainer->setInitialState(mInitialState);
    }

  if (isnan(CalculateValue))
    {
      mFailedCounter++;
      CalculateValue = mWorstValue;
    }

  if (!checkFunctionalConstraints())
    CalculateValue = mWorstValue;

  if (mpCallBack)
    Continue &= mpCallBack->progressItem(mhCounter);

  C_FLOAT64 CurrentObjective =
    (1.0 - mpCrossValidationSet->getWeight()) * mSolutionValue
    + mpCrossValidationSet->getWeight() * CalculateValue * mpCrossValidationSet->getDataPointCount() / mpExperimentSet->getDataPointCount();

  if (CurrentObjective > mCrossValidationObjective)
    mThresholdCounter++;
  else
    {
      mThresholdCounter = 0;
      mCrossValidationObjective = CurrentObjective;
      mCrossValidationSolutionValue = CalculateValue;
    }

  Continue &= (mThresholdCounter < mpCrossValidationSet->getThreshold());

  return Continue;
}

void CFitProblem::fixBuild55()
{
  if (mpExperimentSet != NULL)
    {
      mpExperimentSet->fixBuild55();
    }

  if (mpCrossValidationSet != NULL)
    {
      mpCrossValidationSet->fixBuild55();
    }

  return;
}
