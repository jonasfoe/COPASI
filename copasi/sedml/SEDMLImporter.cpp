// Copyright (C) 2019 - 2021 by Pedro Mendes, Rector and Visitors of the
// University of Virginia, University of Heidelberg, and University
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2017 - 2018 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2013 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/**
 * SEDMLImporter.cpp
 * $Rev:               $:  Revision of last commit
 * $Author:            $:  Author of last commit
 * $Date:              $:  Date of last commit
 * $HeadURL:       $
 * $Id:        $
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <limits>
#include <cmath>
#include <algorithm>

#include <sedml/SedTypes.h>
#include <sbml/math/FormulaFormatter.h>

#include "copasi/copasi.h"

#include "copasi/report/CKeyFactory.h"
#include "copasi/model/CModel.h"
#include "copasi/model/CCompartment.h"
#include "copasi/model/CMetab.h"
#include "copasi/model/CReaction.h"
#include "copasi/model/CModelValue.h"
#include "copasi/model/CEvent.h"
#include "copasi/function/CNodeK.h"
#include "copasi/function/CFunctionDB.h"
#include "copasi/function/CEvaluationTree.h"
#include "copasi/function/CExpression.h"
#include "copasi/function/CFunctionParameters.h"
#include "copasi/core/CDataObjectReference.h"
#include "copasi/utilities/CCopasiTree.h"
#include "copasi/utilities/CNodeIterator.h"
#include "copasi/CopasiDataModel/CDataModel.h"
#include "copasi/core/CRootContainer.h"
#include "copasi/MIRIAM/CRDFGraphConverter.h"
#include "copasi/compareExpressions/CEvaluationNodeNormalizer.h"
#include "copasi/commandline/CLocaleString.h"
#include "copasi/commandline/COptions.h"

#include "copasi/utilities/CProcessReport.h"
#include "copasi/commandline/CConfigurationFile.h"

#include "copasi/utilities/CCopasiMessage.h"

//TODO SEDML
#include "copasi/trajectory/CTrajectoryTask.h"
#include "copasi/trajectory/CTrajectoryProblem.h"
#include "copasi/sbml/SBMLImporter.h"
#include "copasi/utilities/CDirEntry.h"
#include "copasi/utilities/CCopasiException.h"
#include "copasi/utilities/CCopasiTask.h"
#include "copasi/plot/COutputDefinitionVector.h"
#include "copasi/plot/CPlotSpecification.h"

#include "copasi/report/CReportDefinitionVector.h"

#include <copasi/steadystate/CSteadyStateTask.h>
#include <copasi/scan/CScanTask.h>

#include "SEDMLImporter.h"
#include "SEDMLUtils.h"

// static
C_FLOAT64 SEDMLImporter::round(const C_FLOAT64 & x)
{
  return
    x < 0.0 ? -floor(-x + 0.5) : floor(x + 0.5);
}

void SEDMLImporter::setImportHandler(CProcessReport* pHandler)
{
  mpImportHandler = pHandler;
}

CProcessReport* SEDMLImporter::getImportHandlerAddr() const
{
  return mpImportHandler;
}

void SEDMLImporter::clearCallBack()
{
  setImportHandler(NULL);
}

const std::string SEDMLImporter::getArchiveFileName()
{return mArchiveFileName;}

/**
 * Creates and returns a COPASI CTrajectoryTask from the SEDML simulation
 * given as argument.
 */
void SEDMLImporter::updateCopasiTaskForSimulation(SedSimulation* sedmlsim,
    std::map<CDataObject*, SedBase*>& copasi2sedmlmap)
{

  switch (sedmlsim->getTypeCode())
    {
      case SEDML_SIMULATION_UNIFORMTIMECOURSE:
      {
        CTrajectoryTask *tTask = static_cast<CTrajectoryTask*>(&mpDataModel->getTaskList()->operator[]("Time-Course"));
        tTask->setScheduled(true);

        CTrajectoryProblem* tProblem = static_cast<CTrajectoryProblem*>(tTask->getProblem());
        SedUniformTimeCourse* tc = static_cast<SedUniformTimeCourse*>(sedmlsim);
        double outputStartTime = tc->getOutputStartTime();
        double outputEndTime = tc->getOutputEndTime();
        int numberOfPoints = tc->getNumberOfPoints();
        tProblem->setOutputStartTime(outputStartTime);

        if (tc->getInitialTime() != outputStartTime)
          {
            // calculate number of steps between (timeStart, timeEnd)
            //
            double stepSize = (outputEndTime - outputStartTime) / numberOfPoints;
            int additionalSteps = (int)ceil((outputStartTime - tc->getInitialTime()) / stepSize);
            tProblem->setStepNumber(numberOfPoints + additionalSteps);
            tProblem->setDuration(outputEndTime);
          }
        else
          {
            tProblem->setDuration(outputEndTime - outputStartTime);
            tProblem->setStepNumber(numberOfPoints);
          }

        // TODO read kisao terms
        if (tc->isSetAlgorithm())
          {
            const SedAlgorithm* alg = tc->getAlgorithm();

            if (alg->isSetKisaoID())
              {
                if (alg->getKisaoID() == SEDML_KISAO_STOCHASTIC)
                  {
                    tTask->setMethodType(CTaskEnum::Method::stochastic);
                  }
              }
          }

        break;
      }

      case SEDML_SIMULATION_ONESTEP:
      {

        CTrajectoryTask *tTask = static_cast<CTrajectoryTask*>(&mpDataModel->getTaskList()->operator[]("Time-Course"));
        tTask->setScheduled(true);

        CTrajectoryProblem* tProblem = static_cast<CTrajectoryProblem*>(tTask->getProblem());
        SedOneStep* step = static_cast<SedOneStep*>(sedmlsim);
        tProblem->setOutputStartTime(0);
        tProblem->setDuration(step->getStep());
        tProblem->setStepNumber(1);

        // TODO read kisao terms

        break;
      }

      case SEDML_SIMULATION_STEADYSTATE:
      {
        // nothing to be done for this one
        CSteadyStateTask *tTask = static_cast<CSteadyStateTask*>(&mpDataModel->getTaskList()->operator[]("Steady-State"));
        tTask->setScheduled(true);

        // TODO read kisao terms
        //CCopasiProblem* tProblem = static_cast<CCopasiProblem*>(tTask->getProblem());
        //SedSteadyState* tc = static_cast<SedSteadyState*>(sedmlsim);

        break;
      }

      default:
        CCopasiMessage(CCopasiMessage::EXCEPTION, "SEDMLImporter Error: encountered unknown simulation.");
        break;
    }
}

bool isTC(const SedTask* task)
{
  if (task == NULL || task->getSedDocument() == NULL) return false;

  const SedDocument* doc = task->getSedDocument();

  if (task->isSetSimulationReference())
    {
      const SedSimulation* sim = doc->getSimulation(task->getSimulationReference());

      if (sim != NULL && (
            sim->getTypeCode() == SEDML_SIMULATION_UNIFORMTIMECOURSE))
        return true;
    }

  return false;
}

bool isScan(const SedTask* task)
{

  if (task == NULL || task->getSedDocument() == NULL) return false;

  const SedDocument* doc = task->getSedDocument();

  if (task->isSetSimulationReference())
    {
      const SedSimulation* sim = doc->getSimulation(task->getSimulationReference());

      if (sim != NULL && (
            sim->getTypeCode() == SEDML_SIMULATION_STEADYSTATE ||
            sim->getTypeCode() == SEDML_SIMULATION_ONESTEP ||
            sim->getTypeCode() == SEDML_SIMULATION_UNIFORMTIMECOURSE))
        return true;
    }

  return false;
}

bool isScan(const SedRepeatedTask* task)
{
  if (task == NULL || task->getSedDocument() == NULL) return false;

  const SedDocument* doc = task->getSedDocument();

  for (size_t i = 0; i < task->getNumSubTasks(); ++i)
    {
      const SedSubTask* subTask = task->getSubTask(i);
      const SedTask * t = dynamic_cast< const SedTask * >(doc->getTask(subTask->getTask()));

      if (isScan(t)) return true;
    }

  return false;
}

void SEDMLImporter::readListOfPlotsFromSedMLOutput(
  COutputDefinitionVector *pLotList, CModel* pModel,
  SedDocument *pSEDMLDocument,
  std::map<CDataObject*, SedBase*>& copasi2sedmlmap)
{
  size_t i, numOutput = pSEDMLDocument->getNumOutputs();

  std::map<const CDataObject*, SBase*>& copasiMap = pModel->getObjectDataModel()->getCopasi2SBMLMap();

  CReportDefinitionVector* pReports = mpDataModel->getReportDefinitionList();

  for (i = 0; i < numOutput; ++i)
    {
      SedOutput* current = pSEDMLDocument->getOutput(i);

      switch (current->getTypeCode())
        {
          case SEDML_OUTPUT_REPORT:
          {
            SedReport* r = static_cast<SedReport*>(current);
            std::string name = current->isSetName() ? current->getName() : current->getId();
            CReportDefinition* def = new CReportDefinition(name);
            int count = 0;

            // creation fails on duplicated name!
            while (def == NULL)
              {
                def = new CReportDefinition(SEDMLUtils::getNextId(name + " ", ++count));
              }

            def->setComment("Import from SED-ML");
            def->setIsTable(false);
            def->setSeparator(", ");

            std::vector<CRegisteredCommonName>* pHeader = def->getHeaderAddr();
            std::vector<CRegisteredCommonName>* pBody = def->getBodyAddr();

            bool isTimeCourse = false;
            bool isScanTask = false;

            for (size_t i = 0; i < r->getNumDataSets(); ++i)
              {
                SedDataSet* ds = r->getDataSet(i);
                const SedDataGenerator* generator = pSEDMLDocument->getDataGenerator(ds->getDataReference());
                const CDataObject * tmp = SEDMLUtils::resolveDatagenerator(pModel, generator);

                if (generator == NULL || tmp == NULL) continue;

                std::string title = ds->isSetLabel() ? ds->getLabel() : generator->isSetName() ? generator->getName() : ds->getId();

                pHeader->push_back(CDataString(title).getCN());
                pHeader->push_back(def->getSeparator().getCN());

                pBody->push_back(tmp->getCN());
                pBody->push_back(def->getSeparator().getCN());

                if (!isTimeCourse && !isScanTask)
                  for (size_t j = 0; j < generator->getNumVariables(); ++j)
                    {
                      const auto* t = mpSEDMLDocument->getTask(generator->getVariable(j)->getTaskReference());

                      if (t == NULL) continue;

                      isScanTask = t->getTypeCode() == SEDML_TASK_REPEATEDTASK && isScan((const SedRepeatedTask*)t);
                      isTimeCourse = isTC(dynamic_cast<const SedTask*>(t));
                    }
              }

            // assign report to scan task
            if (isScanTask)
              {
                mReportMap[def] = "Scan";
              }

            // assign report to Timecourse task
            if (isTimeCourse)
              {
                mReportMap[def] = "Time-Course";
              }

            break;
          }

          case SEDML_OUTPUT_PLOT2D: //get the curves data
          {
            SedPlot2D* p = static_cast<SedPlot2D*>(current);
            std::string name = current->isSetName() ? current->getName() :
                               current->getId();
            CPlotSpecification* pPl = pLotList->createPlotSpec(
                                        name, CPlotItem::plot2d);

            int count = 0;

            while (pPl == NULL)
              {
                // creation fails on duplicated name!
                pPl = pLotList->createPlotSpec(
                        SEDMLUtils::getNextId(name + " ", ++count), CPlotItem::plot2d);
              }

            bool logX = false;
            bool logY = false;

            for (unsigned int ic = 0; ic < p->getNumCurves(); ++ic)
              {
                SedCurve * curve = dynamic_cast < SedCurve*>(p->getCurve(ic));

                if (!curve)
                  continue;

                std::string xDataReference = curve->getXDataReference();
                std::string yDataReference = curve->getYDataReference();

                const SedDataGenerator* xGenerator = pSEDMLDocument->getDataGenerator(xDataReference);
                const SedDataGenerator* yGenerator = pSEDMLDocument->getDataGenerator(yDataReference);

                //create the curves
                const CDataObject * tmpX = SEDMLUtils::resolveDatagenerator(pModel, xGenerator);
                const CDataObject * tmpY = SEDMLUtils::resolveDatagenerator(pModel, yGenerator);

                if (tmpX != NULL && tmpY != NULL)
                  {
                    std::string  itemTitle;

                    if (curve->isSetName())
                      itemTitle = curve->getName();
                    else if (yGenerator != NULL && yGenerator->isSetName())
                      itemTitle = yGenerator->getName();
                    else
                      itemTitle = tmpY->getObjectDisplayName();

                    CPlotItem * plItem = pPl->createItem(itemTitle, CPlotItem::curve2d);
                    plItem->setValue("Line width", 2.0);
                    plItem->addChannel(tmpX->getCN());
                    plItem->addChannel(tmpY->getCN());
                  }

                logX = logX || (curve->isSetLogX() && curve->getLogX());
                logY = logY || (curve->isSetLogY() && curve->getLogY());
              }

            pPl->setLogX(logX);
            pPl->setLogY(logY);
            break;
          }

          default:
            CCopasiMessage(CCopasiMessage::EXCEPTION, "SEDMLImporter Error: No support for this plot: typecode = %d", current->getTypeCode());
            break;
        }
    }
}

/**
 * Function reads an SEDML file with libsedml and converts it to a Copasi CModel
 */
CModel* SEDMLImporter::readSEDML(std::string filename,
                                 CProcessReport* pImportHandler,
                                 SBMLDocument *& pSBMLDocument,
                                 SedDocument*& pSedDocument,
                                 std::map<CDataObject*, SedBase*>& copasi2sedmlmap,
                                 std::map<CDataObject*, SBase*>& copasi2sbmlmap,
                                 CListOfLayouts *& prLol,
                                 COutputDefinitionVector * &plotList,
                                 CDataModel* pDataModel)
{
  // convert filename to the locale encoding
  std::ifstream file(CLocaleString::fromUtf8(filename).c_str());

  if (!file)
    {
      CCopasiMessage(CCopasiMessage::EXCEPTION, MCSEDML + 5, filename.c_str());
    }

  std::ostringstream stringStream;
  char c;

  while (file.get(c))
    {
      stringStream << c;
    }

  file.clear();
  file.close();

  //using libzip to read SEDML file
  /*  SEDMLUtils utils;
  std::string SEDMLFileName, fileContent("");
  SEDMLFileName = "sedml.xml";

  int success = utils.processArchive(filename, SEDMLFileName, fileContent);*/

  pDataModel->setSEDMLFileName(filename);

  return this->parseSEDML(stringStream.str(), pImportHandler,
                          pSBMLDocument, pSedDocument, copasi2sedmlmap, copasi2sbmlmap, prLol,  plotList, pDataModel);
}
/**
 * Function parses an SEDML document with libsedml and converts it to a COPASI CModel
 * object which is returned. Deletion of the returned pointer is up to the
 * caller.
 */
CModel*
SEDMLImporter::parseSEDML(const std::string& sedmlDocumentText,
                          CProcessReport* pImportHandler,
                          SBMLDocument *& pSBMLDocument,
                          SedDocument *& pSEDMLDocument,
                          std::map<CDataObject*, SedBase*>& copasi2sedmlmap,
                          std::map<CDataObject*, SBase*>& copasi2sbmlmap,
                          CListOfLayouts *& prLol,
                          COutputDefinitionVector * & pPlotList,
                          CDataModel* pDataModel)
{
  mReportMap.clear();
  this->mUsedSEDMLIdsPopulated = false;

  mpDataModel = pDataModel;
  assert(mpDataModel != NULL);

  this->mpCopasiModel = NULL;

  SedReader reader;

  mImportStep = 0;

  if (mpImportHandler)
    {
      mpImportHandler->setName("Importing SED-ML file...");
      mTotalSteps = 11;
      mhImportStep = mpImportHandler->addItem("Step", mImportStep,
                                              &mTotalSteps);
    }

  unsigned C_INT32 step = 0, totalSteps = 0;
  size_t hStep = C_INVALID_INDEX;

  if (this->mpImportHandler != 0)
    {
      step = 0;
      totalSteps = 1;
      hStep = mpImportHandler->addItem("Reading SED-ML file...", step,
                                       &totalSteps);
    }

  mpSEDMLDocument = reader.readSedMLFromString(sedmlDocumentText);

  assert(mpSEDMLDocument != NULL);

  if (mpImportHandler)
    mpImportHandler->finishItem(hStep);

  if (this->mpImportHandler != 0)
    {
      step = 0;
      totalSteps = 1;
      hStep = mpImportHandler->addItem("Checking consistency...", step,
                                       &totalSteps);
    }

  if (mpImportHandler)
    mpImportHandler->finishItem(hStep);

  int fatal = -1;
  unsigned int i, iMax = mpSEDMLDocument->getNumErrors();

  for (i = 0; (i < iMax) && (fatal == -1); ++i)
    {
      const SedError* pSEDMLError = mpSEDMLDocument->getError(i);

      CCopasiMessage::Type messageType = CCopasiMessage::RAW;

      switch (pSEDMLError->getSeverity())
        {

          case LIBSEDML_SEV_WARNING:

            // if issued as warning, this message is to be disregarded,
            // it was a bug in earlier versions of libSEDML
#if LIBSEDML_VERSION < 1
            if (pSEDMLError->getErrorId() == SedInvalidNamespaceOnSed)
              continue;

#endif

            if (mIgnoredSEDMLMessages.find(pSEDMLError->getErrorId())
                != mIgnoredSEDMLMessages.end())
              {
                messageType = CCopasiMessage::WARNING_FILTERED;
              }
            else
              {
                messageType = CCopasiMessage::WARNING;
              }

            CCopasiMessage(messageType, MCSEDML + 6, "WARNING",
                           pSEDMLError->getErrorId(), pSEDMLError->getLine(),
                           pSEDMLError->getColumn(),
                           pSEDMLError->getMessage().c_str());
            break;

          case LIBSEDML_SEV_ERROR:

            if (mIgnoredSEDMLMessages.find(pSEDMLError->getErrorId())
                != mIgnoredSEDMLMessages.end())
              {
                messageType = CCopasiMessage::ERROR_FILTERED;
              }

            CCopasiMessage(messageType, MCSEDML + 6, "ERROR",
                           pSEDMLError->getErrorId(), pSEDMLError->getLine(),
                           pSEDMLError->getColumn(),
                           pSEDMLError->getMessage().c_str());
            break;

          case LIBSEDML_SEV_FATAL:

            // treat unknown as fatal
          default:

            if (pSEDMLError->getErrorId() == 10804)
              {
                // this error indicates a problem with a notes element
                // although libsedml flags this as fatal, we would still
                // like to read the model
                CCopasiMessage(messageType, MCSEDML + 6, "ERROR",
                               pSEDMLError->getErrorId(), pSEDMLError->getLine(),
                               pSEDMLError->getColumn(),
                               pSEDMLError->getMessage().c_str());
              }
            else
              {
                fatal = i;
              }

            break;
        }
    }

  if (fatal != -1)
    {
      const XMLError* pSEDMLError = mpSEDMLDocument->getError(fatal);
      CCopasiMessage Message(CCopasiMessage::EXCEPTION, MCXML + 2,
                             pSEDMLError->getLine(), pSEDMLError->getColumn(),
                             pSEDMLError->getMessage().c_str());

      if (mpImportHandler)
        mpImportHandler->finishItem(mhImportStep);

      return NULL;
    }

  if (mpSEDMLDocument->getListOfModels() == NULL)
    {
      CCopasiMessage Message(CCopasiMessage::ERROR, MCSEDML + 2);

      if (mpImportHandler)
        mpImportHandler->finishItem(mhImportStep);

      return NULL;
    }

  //delete reader;
  pSEDMLDocument = mpSEDMLDocument;
  this->mLevel = pSEDMLDocument->getLevel();

  this->mOriginalLevel = this->mLevel;
  this->mVersion = pSEDMLDocument->getVersion();

  importFirstSBMLModel(pImportHandler, pSBMLDocument, copasi2sbmlmap, prLol, pDataModel);

  pPlotList = new COutputDefinitionVector("OutputDefinitions", mpDataModel);
  readListOfPlotsFromSedMLOutput(pPlotList, mpCopasiModel, pSEDMLDocument, copasi2sedmlmap);

  importTasks(copasi2sedmlmap);

  if (mpImportHandler)
    mpImportHandler->finishItem(mhImportStep);

  return mpCopasiModel;

  return NULL;
}

void
SEDMLImporter::importTasks(std::map<CDataObject*, SedBase*>& copasi2sedmlmap)
{

  for (unsigned int i = 0; i < mpSEDMLDocument->getNumTasks(); ++i)
    {
      auto * task = mpSEDMLDocument->getTask(i);


      switch (task->getTypeCode())
        {
          case SEDML_TASK:
          {
            auto * current = static_cast< SedTask * >(task);

            // skip tasks for models we did not import
            if (current->isSetModelReference() && current->getModelReference() != this->mImportedModel)
              continue;

            SedSimulation * sedmlsim =
              mpSEDMLDocument->getSimulation(current->getSimulationReference());
            updateCopasiTaskForSimulation(sedmlsim, copasi2sedmlmap);
            break;
          }

          case SEDML_TASK_REPEATEDTASK:
          {
            SedRepeatedTask *repeat = static_cast<SedRepeatedTask*>(task);
            SedRange* range = repeat->getRange(repeat->getRangeId());

            if (range == NULL && range->getTypeCode() != SEDML_RANGE_FUNCTIONALRANGE)
              {
                CCopasiMessage(CCopasiMessage::WARNING, "This version of COPASI only supports uniform ranges and value ranges.");
                continue;
              }

            SedUniformRange* urange = dynamic_cast<SedUniformRange*>(range);
            SedVectorRange* vrange = dynamic_cast<SedVectorRange*>(range);
            CScanTask *tTask = static_cast<CScanTask*>(&mpDataModel->getTaskList()->operator[]("Scan"));
            tTask->setScheduled(true);
            CScanProblem *pProblem = static_cast<CScanProblem*>(tTask->getProblem());

            if (urange != NULL && repeat->getNumTaskChanges() == 0)
              {
                pProblem->addScanItem(CScanProblem::SCAN_REPEAT, urange->getNumberOfPoints());
              }
            else
              {
                for (unsigned int j = 0; j < repeat->getNumTaskChanges(); ++j)
                  {
                    SedSetValue* sv = repeat->getTaskChange(j);

                    if (SBML_formulaToString(sv->getMath()) != sv->getRange())
                      {
                        CCopasiMessage(CCopasiMessage::WARNING,
                                       "This version of COPASI only supports setValue elements that apply range values.");
                      }

                    std::string target = sv->getTarget();
                    const CDataObject * obj = SEDMLUtils::resolveXPath(mpCopasiModel, target, true);

                    if (obj == NULL)
                      {
                        CCopasiMessage(CCopasiMessage::WARNING, "This version of COPASI only supports modifications of initial values.");
                        continue;
                      }

                    int numPoints = 0;

                    if (vrange != NULL)
                      numPoints = vrange->getNumValues();
                    else if (urange != NULL)
                      numPoints = urange->getNumberOfPoints();

                    CCopasiParameterGroup*group = pProblem->addScanItem(CScanProblem::SCAN_LINEAR, numPoints, obj);

                    if (urange != NULL)
                      {
                        group->setValue< C_FLOAT64 >("Minimum", urange->getStart());
                        group->setValue< C_FLOAT64 >("Maximum", urange->getEnd());
                        group->setValue< bool >("Use Values", false);
                        group->setValue< bool >("log", (!urange->isSetType() ||
                                                        urange->getType().empty() ||
                                                        urange->getType() == "linear") ? false : true);

                      }

                    if (vrange != NULL)
                      {
                        group->setValue< bool >("Use Values", true);
                        std::stringstream str;
                        std::vector<double> vals = vrange->getValues();

for (double val : vals)
                          str << val << " ";

                        group->setValue< std::string >("Values", str.str());
                      }
                  }
              }

            if (repeat->getNumSubTasks() != 1)
              {
                CCopasiMessage(CCopasiMessage::WARNING, "This version of COPASI only supports repeatedTasks with one subtask.");
                continue;
              }

            pProblem->setContinueFromCurrentState(!repeat->getResetModel());

            SedSubTask* subTask = repeat->getSubTask(0);

            if (!subTask->isSetTask())
              {
                CCopasiMessage(CCopasiMessage::WARNING, "This version of COPASI only supports repeatedTasks with one subtask that has a valid task reference.");
                continue;
              }

            SedTask* actualSubTask = dynamic_cast<SedTask*>(mpSEDMLDocument->getTask(subTask->getTask()));

            if (actualSubTask == NULL || !actualSubTask->isSetSimulationReference())
              {
                CCopasiMessage(CCopasiMessage::WARNING, "This version of COPASI only supports repeatedTasks with one subtask that itself is a task with simulation reference.");
                continue;
              }

            int code = mpSEDMLDocument->getSimulation(actualSubTask->getSimulationReference())->getTypeCode();

            if (code == SEDML_SIMULATION_STEADYSTATE)
              {
                pProblem->setSubtask(CTaskEnum::Task::steadyState);
                pProblem->setOutputInSubtask(false);
              }
            else if (code == SEDML_SIMULATION_ONESTEP || code == SEDML_SIMULATION_UNIFORMTIMECOURSE)
              {
                pProblem->setSubtask(CTaskEnum::Task::timeCourse);
              }

            break;
          }

          default:
          {
            const char * name = SedTypeCode_toString(task->getTypeCode());
            CCopasiMessage(CCopasiMessage::WARNING,
                           "Encountered unsupported Task type '%s'. This task cannot be imported in COPASI",
                           name != NULL ? name : "unknown");
          }
        }
    }

  std::map<CReportDefinition*, std::string>::const_iterator it = mReportMap.begin();

  for (; it != mReportMap.end(); ++it)
    {
      CReport* report = &mpDataModel->getTaskList()->operator[](it->second).getReport();
      report->setReportDefinition(it->first);
      report->setTarget(it->second + ".txt");
      report->setConfirmOverwrite(false);
      report->setAppend(false);
    }
}

bool applyValueToParameterSet(CModelParameterSet& set, CDataObject *obj, double newValue)
{
  const CModelParameter * pParameter = set.getModelParameter(obj->getCN());

  if (pParameter != NULL)
    {
      const_cast< CModelParameter * >(pParameter)->setValue(newValue, CCore::Framework::Concentration);
      return true;
    }

  return false;
}

bool applyAttributeChange(CModel* pCopasiModel, CModelParameterSet& set, const std::string& target, const std::string&  newValue)
{
  CDataObject *obj = const_cast<CDataObject*>(SEDMLUtils::resolveXPath(pCopasiModel, target, true));

  if (obj == NULL)
    return false;

  // convert the string to double
  std::stringstream str;
  str << newValue;
  double result;
  str >> result;

  // set the value
  applyValueToParameterSet(set, obj->getObjectParent(), result);
  return true;
}

CModel* SEDMLImporter::importFirstSBMLModel(CProcessReport* pImportHandler,
    SBMLDocument *& pSBMLDocument,
    std::map<CDataObject*, SBase*>& copasi2sbmlmap,
    CListOfLayouts *& prLol,
    CDataModel* pDataModel)
{
  std::string SBMLFileName, fileContent;

  unsigned int ii, iiMax = mpSEDMLDocument->getListOfModels()->size();

  if (iiMax < 1)
    {
      CCopasiMessage(CCopasiMessage::EXCEPTION, MCSEDML + 2);
    }

  if (iiMax > 1)
    {
      CCopasiMessage(CCopasiMessage::WARNING, "COPASI currently only supports the import of SED-ML models, that involve one model only. Only the simulations for the first model will be imported");
    }

  std::string modelSource; //must be taken from SEDML document.
  std::string modelId; // to ensure only one model is imported since only one model in SEDML file is supported
  SedModel* sedmlModel = NULL;

  for (ii = 0; ii < iiMax; ++ii)
    {
      sedmlModel = mpSEDMLDocument->getModel(ii);

      // need to also allow for the specific urns like
      // urn:sedml:language:sbml.level-3.version-1
      if (sedmlModel->getLanguage().find("urn:sedml:language:sbml") == std::string::npos)
        CCopasiMessage(CCopasiMessage::EXCEPTION,
                       "Sorry currently, only SBML models are supported.");

      if (sedmlModel->getSource() != modelId)
        {
          modelId = sedmlModel->getId();

          if ((sedmlModel->getListOfChanges()->size()) > 0)
            CCopasiMessage(CCopasiMessage::WARNING, "Currently there is only limited support for "
                           "changing model entities. Only value changes are imported into the model.");

          modelSource = sedmlModel->getSource();
          break;
        }
    }

  assert(modelSource != "");

  //process the archive file and get the SBML model file
  //SEDMLUtils utils;
  //int success = utils.processArchive(pDataModel->getSEDMLFileName(), SBMLFileName, fileContent);

  std::string FileName;

  if (CDirEntry::exist(modelSource))
    FileName = modelSource;
  else
    FileName = CDirEntry::dirName(pDataModel->getSEDMLFileName())
               + CDirEntry::Separator + modelSource;

  std::ifstream file(CLocaleString::fromUtf8(FileName).c_str());

  if (!file)
    {
      CCopasiMessage(CCopasiMessage::EXCEPTION, MCSEDML + 4,
                     FileName.c_str());
    }

  //set the SBML file name for later use
  pDataModel->setSBMLFileName(FileName);
  std::ostringstream sbmlStringStream;
  char c;

  while (file.get(c))
    {
      sbmlStringStream << c;
    }

  file.clear();
  file.close();

  std::ifstream File(CLocaleString::fromUtf8(FileName).c_str());

  SBMLImporter importer;
  // Right now we always import the COPASI MIRIAM annotation if it is there.
  // Later this will be settable by the user in the preferences dialog
  importer.setImportCOPASIMIRIAM(true);
  importer.setImportHandler(pImportHandler);

  mpCopasiModel = NULL;

  std::map<const CDataObject*, SBase*> Copasi2SBMLMap;

  try
    {
      mpCopasiModel = importer.parseSBML(sbmlStringStream.str(),
                                         CRootContainer::getFunctionList(), pSBMLDocument,
                                         Copasi2SBMLMap, prLol, mpDataModel);
    }

  catch (CCopasiException & except)
    {
      importer.restoreFunctionDB();
      importer.deleteCopasiModel();
      //    popData();

      throw except;
    }

  if (mpCopasiModel == NULL)
    {
      importer.restoreFunctionDB();
      importer.deleteCopasiModel();
      //   popData();
      return NULL;
    }

  mImportedModel = modelId;

  // apply possible changes to the model
  if (sedmlModel != NULL && sedmlModel->getNumChanges() > 0)
    {
      CModelParameterSet& set = mpCopasiModel->getActiveModelParameterSet();
      bool valueChanged = false;

      for (unsigned int i = 0; i < sedmlModel->getNumChanges(); ++i)
        {
          SedChangeAttribute* change = dynamic_cast<SedChangeAttribute*>(sedmlModel->getChange(i));

          if (change == NULL) continue;

          const std::string& target = change->getTarget();
          const std::string& newValue = change->getNewValue();

          if (!applyAttributeChange(mpCopasiModel, set, target, newValue))
            {
              CCopasiMessage(CCopasiMessage::WARNING, "Could not apply change for target: '%s'", target.c_str());
            }
          else
            {
              valueChanged = true;
            }
        }

      if (valueChanged)
        {
          set.updateModel();
        }
    }

  return mpCopasiModel;
}

/**
 * Constructor that initializes speciesMap and the FunctionDB object
 */
SEDMLImporter::SEDMLImporter():
  mIgnoredSEDMLMessages(),
  mIncompleteModel(false),
  mLevel(0),
  mOriginalLevel(0),
  mVersion(0),
  mpDataModel(NULL),
  mpCopasiModel(NULL),
  mpSEDMLDocument(NULL),
  mpImportHandler(NULL),
  mImportStep(0),
  mhImportStep(C_INVALID_INDEX),
  mTotalSteps(0),
  mUsedSEDMLIds(),
  mUsedSEDMLIdsPopulated(false),
  mImportedModel(),
  mReportMap()
{

  this->mIgnoredSEDMLMessages.insert(10501);
}

void SEDMLImporter::restoreFunctionDB()
{
}

/**
 * Destructor that does nothing.
 */
SEDMLImporter::~SEDMLImporter()
{
  mReportMap.clear();
}

void SEDMLImporter::deleteCopasiModel()
{
  if (this->mpCopasiModel != NULL)
    {
      delete this->mpCopasiModel;
      this->mpCopasiModel = NULL;
    }
}
