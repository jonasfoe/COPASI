// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2006 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/*
 *  CQFittingResult.cpp
 *  Created by Paul on 4/2/10.
 */

#include "CQFittingResult.h"

#include "copasi.h"

#include "CopasiFileDialog.h"
#include "CQMessageBox.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "parameterFitting/CFitTask.h"
#include "parameterFitting/CFitProblem.h"
#include "parameterFitting/CFitItem.h"
#include "parameterFitting/CExperimentSet.h"
#include "parameterFitting/CExperiment.h"
#include "optimization/COptMethod.h"
#include "optimization/COptLog.h"
#include "report/CCopasiRootContainer.h"
#include "commandline/CLocaleString.h"
#include "model/CModel.h"
#include "math/CMathContainer.h"

#include "UI/qtUtilities.h"
#include <QWebFrame>

/*
 *  Constructs a CQFittingResult which is a child of 'parent', with the
 *  name 'name'.'
 */
CQFittingResult::CQFittingResult(QWidget* parent, const char* name):
  CopasiWidget(parent, name)
{
  setupUi(this);

  init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQFittingResult::~CQFittingResult()
{
  // no need to delete child widgets, Qt does it all for us
}

void CQFittingResult::init()
{
  mpCorrelations->setLegendEnabled(false);
  mpFisherInformationMatrix->setLegendEnabled(false);
  mpFisherInformationEigenvalues->setLegendEnabled(false);
  mpFisherInformationEigenvectors->setLegendEnabled(false);
  mpFisherInformationScaledMatrix->setLegendEnabled(false);
  mpFisherInformationScaledEigenvalues->setLegendEnabled(false);
  mpFisherInformationScaledEigenvectors->setLegendEnabled(false);
}

bool CQFittingResult::update(ListViews::ObjectType /* objectType */,
                             ListViews::Action /* action */,
                             const std::string & /* key */)
{
  // :TODO:
  return true;
}

bool CQFittingResult::leave()
{
  // :TODO:
  return true;
}

bool CQFittingResult::enterProtected()
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = &CCopasiRootContainer::getDatamodelList()->operator[](0);
  assert(pDataModel != NULL);

  mpTask =
    dynamic_cast<CFitTask *>(&pDataModel->getTaskList()->operator[]("Parameter Estimation"));

  if (!mpTask) return false;

  mpProblem = dynamic_cast<const CFitProblem *>(mpTask->getProblem());

  if (!mpProblem) return false;

  mpMain->load(mpProblem);

  if (mpProblem->getCalculateStatistics())
    {
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpExperiments), true);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpValues), true);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCorrelations), true);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpFisherInformationPage), true);

      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCrossValidations), true);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCrossValidationValues), true);
    }
  else
    {
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpExperiments), false);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpValues), false);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCorrelations), false);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpFisherInformationPage), false);

      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCrossValidations), false);
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCrossValidationValues), false);
    }

  size_t i, imax;
  QTableWidgetItem * pItem;

  // Loop over the optimization items
  const std::vector< COptItem * > & Items = mpProblem->getOptItemList();
  const CVector< C_FLOAT64 > & Solutions = mpProblem->getSolutionVariables();
  const CVector< C_FLOAT64 > & StdDeviations = mpProblem->getVariableStdDeviations();
  const CVector< C_FLOAT64 > & Gradients = mpProblem->getVariableGradients();

  imax = Items.size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  //the parameters table
  mpParameters->setRowCount((int) imax);

  QColor BackgroundColor = mpParameters->palette().brush(QPalette::Active, QPalette::Base).color();

  int h, s, v;
  BackgroundColor.getHsv(&h, &s, &v);

  if (s < 20)
    {
      s = 20;
    }

  BackgroundColor.setHsv(0, s, v);

  for (i = 0; i != imax; i++)
    {
      //1st column: parameter name
      const CCopasiObject *pObject =
        CObjectInterface::DataObject(pDataModel->getObjectFromCN(Items[i]->getObjectCN()));

      if (pObject)
        {
          std::string Experiments =
            static_cast<CFitItem *>(Items[i])->getExperiments();

          if (Experiments != "")
            Experiments = "; {" + Experiments + "}";

          pItem = new QTableWidgetItem(FROM_UTF8(pObject->getObjectDisplayName() + Experiments));
        }
      else
        pItem = new QTableWidgetItem("Not Found");

      mpParameters->setItem((int) i, 0, pItem);

      const C_FLOAT64 & Solution = i < Solutions.size() ? Solutions[i] : std::numeric_limits<double>::quiet_NaN();

      //2nd column: lower bound
      const COptItem *current = Items[i];
      pItem = new QTableWidgetItem(FROM_UTF8(current->getLowerBound()));
      mpParameters->setItem((int) i, 1, pItem);

      if (current->getLowerBoundValue() != NULL && 1.01 * *current->getLowerBoundValue() > Solution)
        {
          pItem->setBackgroundColor(BackgroundColor);
        }

      //3rd column: start value
      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, current->getLastStartValue());
      pItem->setForeground(QColor(120, 120, 140));
      mpParameters->setItem((int) i, 2, pItem);

      //4th column: solution value
      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Solution);
      mpParameters->setItem((int) i, 3, pItem);

      //5th column: upper bound
      pItem = new QTableWidgetItem(FROM_UTF8(current->getUpperBound()));
      mpParameters->setItem((int) i, 4, pItem);

      if (current->getUpperBoundValue() != NULL && 0.99 * *current->getUpperBoundValue() < Solution)
        {
          pItem->setBackgroundColor(BackgroundColor);
        }

      const C_FLOAT64 & StdDeviation = i < StdDeviations.size() ?  StdDeviations[i] : std::numeric_limits<double>::quiet_NaN();

      pItem = new QTableWidgetItem(QVariant::Double);

      pItem->setData(Qt::DisplayRole, StdDeviation);

      mpParameters->setItem((int) i, 5, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);

      pItem->setData(Qt::DisplayRole, fabs(100.0 * StdDeviation / Solution));

      mpParameters->setItem((int) i, 6, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);

      pItem->setData(Qt::DisplayRole, i < Gradients.size() ? Gradients[i] : std::numeric_limits<double>::quiet_NaN());

      mpParameters->setItem((int) i, 7, pItem);
    }

  mpParameters->resizeColumnsToContents();
  mpParameters->resizeRowsToContents();

  // Loop over the experiments
  const CExperimentSet & Experiments = mpProblem->getExperiementSet();

  imax = Experiments.getExperimentCount();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpExperiments->setRowCount((int) imax);

  for (i = 0; i != imax; i++)
    {
      const CExperiment & Experiment = * Experiments.getExperiment(i);

      pItem = new QTableWidgetItem(FROM_UTF8(Experiment.getObjectName()));
      mpExperiments->setItem((int) i, 0, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiment.getObjectiveValue());
      mpExperiments->setItem((int) i, 1, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayPropertyRole, Experiment.getRMS());
      mpExperiments->setItem((int) i, 2, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiment.getErrorMean());
      mpExperiments->setItem((int) i, 3, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiment.getErrorMeanSD());
      mpExperiments->setItem((int) i, 4, pItem);
    }

  mpExperiments->resizeColumnsToContents();
  mpExperiments->resizeRowsToContents();

  // Loop over the dependent objects
  imax = Experiments.getDependentObjects().size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpValues->setRowCount((int) imax);

  for (i = 0; i != imax; i++)
    {
      const CObjectInterface * pObject = Experiments.getDependentObjects()[i];

      if (pObject)
        pItem = new QTableWidgetItem(FROM_UTF8(pObject->getObjectDisplayName()));
      else
        pItem = new QTableWidgetItem("Not Found");

      mpValues->setItem((int) i, 0, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiments.getDependentObjectiveValues()[i]);
      mpValues->setItem((int) i, 1, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiments.getDependentRMS()[i]);
      mpValues->setItem((int) i, 2, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiments.getDependentErrorMean()[i]);
      mpValues->setItem((int) i, 3, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiments.getDependentErrorMeanSD()[i]);
      mpValues->setItem((int) i, 4, pItem);
    }

  mpValues->resizeColumnsToContents();
  mpValues->resizeRowsToContents();

  // Fill correlation matrix
  imax = Items.size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  CColorScaleBiLog * tcs = new CColorScaleBiLog();
  mpCorrelations->setColorCoding(tcs);
  mpCorrelations->setColorScalingAutomatic(true);
  mpCorrelations->setArrayAnnotation(&mpProblem->getCorrelations());

  tcs = new CColorScaleBiLog();
  mpFisherInformationMatrix->setColorCoding(tcs);
  mpFisherInformationMatrix->setColorScalingAutomatic(true);
  mpFisherInformationMatrix->setArrayAnnotation(&mpProblem->getFisherInformation());

  tcs = new CColorScaleBiLog();
  mpFisherInformationEigenvalues->setColorCoding(tcs);
  mpFisherInformationEigenvalues->setColorScalingAutomatic(true);
  mpFisherInformationEigenvalues->setArrayAnnotation(&mpProblem->getFisherInformationEigenvalues());
  mpFisherInformationEigenvalues->mpComboRows->setCurrentIndex(1);

  tcs = new CColorScaleBiLog();
  mpFisherInformationEigenvectors->setColorCoding(tcs);
  mpFisherInformationEigenvectors->setColorScalingAutomatic(true);
  mpFisherInformationEigenvectors->setArrayAnnotation(&mpProblem->getFisherInformationEigenvectors());
  mpFisherInformationEigenvectors->mpComboRows->setCurrentIndex(1);

  tcs = new CColorScaleBiLog();
  mpFisherInformationScaledMatrix->setColorCoding(tcs);
  mpFisherInformationScaledMatrix->setColorScalingAutomatic(true);
  mpFisherInformationScaledMatrix->setArrayAnnotation(&mpProblem->getScaledFisherInformation());

  tcs = new CColorScaleBiLog();
  mpFisherInformationScaledEigenvalues->setColorCoding(tcs);
  mpFisherInformationScaledEigenvalues->setColorScalingAutomatic(true);
  mpFisherInformationScaledEigenvalues->setArrayAnnotation(&mpProblem->getScaledFisherInformationEigenvalues());
  mpFisherInformationScaledEigenvalues->mpComboRows->setCurrentIndex(1);

  tcs = new CColorScaleBiLog();
  mpFisherInformationScaledEigenvectors->setColorCoding(tcs);
  mpFisherInformationScaledEigenvectors->setColorScalingAutomatic(true);
  mpFisherInformationScaledEigenvectors->setArrayAnnotation(&mpProblem->getScaledFisherInformationEigenvectors());
  mpFisherInformationScaledEigenvectors->mpComboRows->setCurrentIndex(1);

  bool Enable = (mpProblem->getCrossValidationSet().getExperimentCount() > 0);

  mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCrossValidations), Enable);
  mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpCrossValidationValues), Enable);

  // Loop over the cross validation
  const CCrossValidationSet & CrossValidations = mpProblem->getCrossValidationSet();

  imax = CrossValidations.getExperimentCount();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpCrossValidations->setRowCount(imax);

  for (i = 0; i != imax; i++)
    {
      const CExperiment & Experiment = * CrossValidations.getExperiment(i);

      pItem = new QTableWidgetItem(FROM_UTF8(Experiment.getObjectName()));
      mpCrossValidations->setItem(i, 0, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiment.getObjectiveValue());
      mpCrossValidations->setItem(i, 1, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiment.getRMS());
      mpCrossValidations->setItem(i, 2, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiment.getErrorMean());
      mpCrossValidations->setItem(i, 3, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, Experiment.getErrorMeanSD());
      mpCrossValidations->setItem(i, 4, pItem);
    }

  mpCrossValidations->resizeColumnsToContents();
  mpCrossValidations->resizeRowsToContents();

  // Loop over the dependent objects
  imax = CrossValidations.getDependentObjects().size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpCrossValidationValues->setRowCount(imax);

  for (i = 0; i != imax; i++)
    {
      const CObjectInterface * pObject = CrossValidations.getDependentObjects()[i];

      if (pObject)
        pItem = new QTableWidgetItem(FROM_UTF8(pObject->getObjectDisplayName()));
      else
        pItem = new QTableWidgetItem("Not Found");

      mpCrossValidationValues->setItem(i, 0, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, CrossValidations.getDependentObjectiveValues()[i]);
      mpCrossValidationValues->setItem(i, 1, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, CrossValidations.getDependentRMS()[i]);
      mpCrossValidationValues->setItem(i, 2, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, CrossValidations.getDependentErrorMean()[i]);
      mpCrossValidationValues->setItem(i, 3, pItem);

      pItem = new QTableWidgetItem(QVariant::Double);
      pItem->setData(Qt::DisplayRole, CrossValidations.getDependentErrorMeanSD()[i]);
      mpCrossValidationValues->setItem(i, 4, pItem);
    }

  mpCrossValidationValues->resizeColumnsToContents();
  mpCrossValidationValues->resizeRowsToContents();

  // log
  const COptMethod * pMethod = dynamic_cast<const COptMethod *>(mpTask->getMethod());

  if (pMethod)
    {
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpLogPage), true);

      QStringList logHtml;
      QFile logFile("../protocol/protocol.html");
      logFile.open(QIODevice::ReadOnly);
      logHtml = ((QString)logFile.readAll()).split("id=\"accordion\">\n");
      logFile.close();


      if (logHtml.size() > 1)
        {
          QString & logHtmlBuilder = logHtml[0];

          logHtmlBuilder.append("id=\"accordion\">\n");

          logHtmlBuilder.append(pMethod->getMethodLog().getRichLog().c_str());

          QString logQString = logHtml.join(QString());
          mpLogWebView->page()->action(QWebPage::Reload)->setVisible(false);
          mpLogWebView->page()->action(QWebPage::SelectAll)->setEnabled(true);
          mpLogWebView->page()->action(QWebPage::SelectAll)->setVisible(true);
#ifdef COPASI_DEBUG
          mpLogWebView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif // COPASI_DEBUG
          mpLogWebView->setHtml(logQString, QUrl::fromLocalFile(QFileInfo("../protocol/protocol.html").absoluteFilePath()));
        }
    }
  else
    {
      mpTabWidget->setTabEnabled(mpTabWidget->indexOf(mpLogPage), false);
    }

  return true;
}

void CQFittingResult::slotSave(void)
{
  C_INT32 Answer = QMessageBox::No;
  QString fileName;

  while (Answer == QMessageBox::No)
    {
      fileName =
        CopasiFileDialog::getSaveFileName(this,
                                          "Save File Dialog",
                                          "untitled.txt",
                                          "TEXT Files (*.txt)",
                                          "Save to");

      if (fileName.isEmpty()) return;

      if (!fileName.endsWith(".txt") &&
          !fileName.endsWith(".")) fileName += ".txt";

      fileName = fileName.remove(QRegExp("\\.$"));

      Answer = checkSelection(fileName);

      if (Answer == QMessageBox::Cancel) return;
    }

  std::ofstream file(CLocaleString::fromUtf8(TO_UTF8(fileName)).c_str());

  if (file.fail()) return;

  mpProblem->printResult(&file);

  if (mpValues->isEnabled())
    {
      // Set up the fitted values table
      file << "Fitted Values:" << std::endl;
      file << "Fitted Value\tObjective Value\tRoot Mean Square\tError Mean\tError Mean Std. Deviation" << std::endl;

      // Loop over the fitted values objects
      imax = mpValues->rowCount();

      for (i = 0; i != imax; i++)
        {
          file << TO_UTF8(mpValues->item((int) i, 0)->text()) << "\t";
          file << TO_UTF8(mpValues->item((int) i, 1)->text()) << "\t";
          file << TO_UTF8(mpValues->item((int) i, 2)->text()) << "\t";
          file << TO_UTF8(mpValues->item((int) i, 3)->text()) << "\t";
          file << TO_UTF8(mpValues->item((int) i, 4)->text()) << std::endl;
        }

      file << std::endl;
    }

  // Save the parameter correlations
  file << mpProblem->getCorrelations() << std::endl;

  // Save the Fisher information
  file << mpProblem->getFisherInformation() << std::endl;

  // Save the Fisher information Eigenvalues
  file << mpProblem->getFisherInformationEigenvalues() << std::endl;

  // Save the Fisher information Eigenvectors
  file << mpProblem->getFisherInformationEigenvectors() << std::endl;

  // Save the scaled Fisher information
  file << mpProblem->getScaledFisherInformation() << std::endl;

  // Save the scaled Fisher information Eigenvalues
  file << mpProblem->getScaledFisherInformationEigenvalues() << std::endl;

  // Save the scaled Fisher information Eigenvectors
  file << mpProblem->getScaledFisherInformationEigenvectors() << std::endl << std::endl;

  if (mpValues->isEnabled())
    {
      // Set up the cross validations table
      file << "Validations:" << std::endl;
      file << "Validation Experiment\t Objective Value\tRoot Mean Square\tError Mean\tError Mean Std. Deviation" << std::endl;

      // Loop over the experiments
      imax = mpCrossValidations->rowCount();

      for (i = 0; i != imax; i++)
        {
          file << TO_UTF8(mpCrossValidations->item((int) i, 0)->text()) << "\t";
          file << TO_UTF8(mpCrossValidations->item((int) i, 1)->text()) << "\t";
          file << TO_UTF8(mpCrossValidations->item((int) i, 2)->text()) << "\t";
          file << TO_UTF8(mpCrossValidations->item((int) i, 3)->text()) << "\t";
          file << TO_UTF8(mpCrossValidations->item((int) i, 4)->text()) << std::endl;
        }

      file << std::endl;
    }

  if (mpValues->isEnabled())
    {
      // Set up the fitted values table
      file << "Validation Fitted Values:" << std::endl;
      file << "Validation Fitted Value\tObjective Value\tRoot Mean Square\tError Mean\tError Mean Std. Deviation" << std::endl;

      // Loop over the fitted values objects
      imax = mpCrossValidationValues->rowCount();

      for (i = 0; i != imax; i++)
        {
          file << TO_UTF8(mpCrossValidationValues->item((int) i, 0)->text()) << "\t";
          file << TO_UTF8(mpCrossValidationValues->item((int) i, 1)->text()) << "\t";
          file << TO_UTF8(mpCrossValidationValues->item((int) i, 2)->text()) << "\t";
          file << TO_UTF8(mpCrossValidationValues->item((int) i, 3)->text()) << "\t";
          file << TO_UTF8(mpCrossValidationValues->item((int) i, 4)->text()) << std::endl;
        }

      file << std::endl;
    }

  // log
  const COptMethod * pMethod = dynamic_cast<const COptMethod *>(mpTask->getMethod());

  if (pMethod)
    {
      // Set up log output
      file << "Method Log:" << std::endl;

      file << pMethod->getMethodLog().getPlainLog().c_str();

      file << std::endl;
    }

  file.close();
}

void CQFittingResult::slotUpdateModel()
{
  const_cast< CFitProblem * >(mpProblem)->restore(true);

  // We need to notify the GUI to update all values
  protectedNotify(ListViews::STATE, ListViews::CHANGE, mpTask->getMathContainer()->getModel().getKey());
}
