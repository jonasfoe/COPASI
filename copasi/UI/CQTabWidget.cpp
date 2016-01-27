// Copyright (C) 2012 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include <QtCore/QString>

#include "CQTabWidget.h"

#include "CQMessageBox.h"
#include "qtUtilities.h"
#include "CQNotes.h"

#include "MIRIAMUI/CQMiriamWidget.h"
#include "MIRIAMUI/CQRDFTreeView.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "function/CFunction.h"
#include "UI/CQCompartment.h"
#include "UI/CQSpeciesDetail.h"
#include "UI/ReactionsWidget1.h"

#include <QUndoStack>
#include <copasi/undoFramework/EntityRenameCommand.h>
#include <copasi/UI/copasiui3window.h>

CQTabWidget::CQTabWidget(const ListViews::ObjectType & objectType, CopasiWidget * pCopasiWidget,
                         QWidget * parent, Qt::WindowFlags f) :
  CopasiWidget(parent, NULL, f),
  mPages(),
  mObjectType(objectType),
  mIgnoreLeave(false)
{
  setupUi(this);

  mpLblName->setText("<h3>" + FROM_UTF8(ListViews::ObjectTypeName[mObjectType]) + "</h3>");

  mpTabWidget->addTab(pCopasiWidget, "Details");
  mPages.push_back(pCopasiWidget);

  switch (mObjectType)
    {
      case  ListViews::MODEL:
        mpBtnNew->hide();
        mpBtnCopy->hide();
        mpBtnDelete->hide();
        break;

      case ListViews::MODELPARAMETERSET:
        mpBtnNew->setText("Apply");
        mpBtnNew->setToolTip("Apply the current parameters to the model.");

        // The break statement is intentionally missing

      default:
        CQNotes* pNotes = new CQNotes(mpTabWidget);
        mPages.push_back(pNotes);
        mpTabWidget->addTab(pNotes, "Notes");

        connect(this, SIGNAL(newClicked()), pCopasiWidget, SLOT(slotBtnNew()));
        connect(this, SIGNAL(copyClicked()), pCopasiWidget, SLOT(slotBtnCopy()));
        connect(this, SIGNAL(deleteClicked()), pCopasiWidget, SLOT(slotBtnDelete()));
        connect(this, SIGNAL(copyClicked()), pNotes, SLOT(slotBtnCopy()));
        break;
    }

  CQMiriamWidget* pMIRIAMWidget = new CQMiriamWidget(mpTabWidget);
  mPages.push_back(pMIRIAMWidget);
  mpTabWidget->addTab(pMIRIAMWidget, "Annotation");
  connect(this, SIGNAL(copyClicked()), pMIRIAMWidget, SLOT(slotBtnCopy()));

  CQRDFTreeView* pRDFTreeView = new CQRDFTreeView(mpTabWidget);
  mPages.push_back(pRDFTreeView);
  mpTabWidget->addTab(pRDFTreeView, "RDF Browser");

  CopasiUI3Window *  pWindow = dynamic_cast<CopasiUI3Window * >(parent->parent());
  setUndoStack(pWindow->getUndoStack());
}

CQTabWidget::~CQTabWidget()
{
  // TODO Auto-generated destructor stub
}

bool CQTabWidget::leave()
{
  if (mIgnoreLeave) return true;

  bool success = save();

  std::vector< CopasiWidget * >::iterator it = mPages.begin();
  std::vector< CopasiWidget * >::iterator end = mPages.end();

  for (; it != end; ++it)
    success &= (*it)->leave();

  return true;
}

bool CQTabWidget::enterProtected()
{
  load();

  std::vector< CopasiWidget * >::iterator it = mPages.begin();
  std::vector< CopasiWidget * >::iterator end = mPages.end();

  for (; it != end; ++it)
    (*it)->enter(mKey);

  return true;
}

bool CQTabWidget::update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key)
{
  if (mIgnoreUpdates) return true;

  if (objectType == mObjectType &&
      key == mKey)
    {
      switch (action)
        {
          case ListViews::RENAME:
            load();
            break;

          case ListViews::DELETE:
            mpObject = NULL;
            break;

          default:
            break;
        }
    }

  // We do not need to update the child pages as they are directly called from listviews.
  return true;
}

void CQTabWidget::selectTab(int index) const
{
  mpTabWidget->setCurrentIndex(index);
}

void CQTabWidget::load()
{
  // mpObject can not be trusted
  mpObject = CCopasiRootContainer::getKeyFactory()->get(mKey);

  if (mpObject != NULL)
    {
      mpEditName->setText(FROM_UTF8(mpObject->getObjectName()));

      if (mObjectType == ListViews::FUNCTION)
        {
          bool readOnly = static_cast< const CFunction * >(mpObject)->isReadOnly();

          mpEditName->setReadOnly(readOnly);
          mpBtnCommit->setEnabled(!readOnly);
          mpBtnRevert->setEnabled(!readOnly);
          mpBtnDelete->setEnabled(!readOnly);
        }
      else if (mObjectType == ListViews::UNIT)
        {
          bool readOnly = static_cast< const CUnitDefinition * >(mpObject)->isReadOnly();

          mpEditName->setReadOnly(readOnly);
          mpBtnCommit->setEnabled(!readOnly);
          mpBtnRevert->setEnabled(!readOnly);
          mpBtnDelete->setEnabled(!readOnly);
        }
    }
  else
    {
      mpEditName->setText("");
    }
}

bool CQTabWidget::save()
{
  // mpObject can not be trusted
  mpObject = CCopasiRootContainer::getKeyFactory()->get(mKey);

  if (mpObject == NULL) return false;

  // We need to tell the sub-widgets to ignore all notifications
  std::vector< CopasiWidget * >::iterator it = mPages.begin();
  std::vector< CopasiWidget * >::iterator end = mPages.end();

  for (; it != end; ++it)
    (*it)->setIgnoreUpdates(true);

  if (mpObject->getObjectName() != TO_UTF8(mpEditName->text()))
    {
      mpUndoStack->push(new EntityRenameCommand(mpObject,
                        mpObject->getObjectName(),
                        TO_UTF8(mpEditName->text()),
                        this));
    }

  // We need to tell the sub-widgets to accept notifications again
  it = mPages.begin();
  end = mPages.end();

  for (; it != end; ++it)
    (*it)->setIgnoreUpdates(false);

  return true;
}

void CQTabWidget::slotBtnCommit()
{
  mpBtnCommit->setFocus();

  leave();
  enterProtected();
}

void CQTabWidget::slotBtnRevert()
{
  enterProtected();
}

void CQTabWidget::slotBtnDelete()
{
  mIgnoreLeave = true;
  emit deleteClicked();
  mIgnoreLeave = false;
}

void CQTabWidget::slotBtnNew()
{
  mpBtnNew->setFocus();

  leave();

  mIgnoreLeave = true;
  emit newClicked();
  mIgnoreLeave = false;
}

void CQTabWidget::slotBtnCopy()
{
  mpBtnCopy->setFocus();

  leave();

  mIgnoreLeave = true;

  // CQCompartments and CQSpecies have copy options, use CModelExpansion, and do their own switching.
  if (QString(mPages[0]->metaObject()->className()) == "CQCompartment")
    {
      CQCompartment * pQCompartment = dynamic_cast< CQCompartment * >(mPages[0]);
      pQCompartment->copy();
    }
  else if (QString(mPages[0]->metaObject()->className()) == "CQSpeciesDetail")
    {
      CQSpeciesDetail * pQSpeciesDetail = dynamic_cast< CQSpeciesDetail * >(mPages[0]);
      pQSpeciesDetail->copy();
    }
  else if (QString(mPages[0]->metaObject()->className()) == "ReactionsWidget1")
    {
      ReactionsWidget1 * pReactionsWidget1 = dynamic_cast< ReactionsWidget1 * >(mPages[0]);
      pReactionsWidget1->copy();
    }
  else
    {
      emit copyClicked();
      emit newClicked();
    }

  mIgnoreLeave = false;
}

bool CQTabWidget::renameEntity(const std::string& key, const std::string& newName)
{
  mKey = key;
  load();
  mpListView->switchToOtherWidget(C_INVALID_INDEX, mKey);
  qApp->processEvents();

  if (!mpObject->setObjectName(newName))
    {
      QString msg;
      msg = "Unable to rename " + FROM_UTF8(ListViews::ObjectTypeName[mObjectType]).toLower() + " '" + FROM_UTF8(mpObject->getObjectName()) + "'\n"
            + "to '" + FROM_UTF8(newName) + "' since a " + FROM_UTF8(ListViews::ObjectTypeName[mObjectType]).toLower() + " with that name already exists.\n";

      CQMessageBox::information(this,
                                "Unable to rename " + FROM_UTF8(ListViews::ObjectTypeName[mObjectType]),
                                msg,
                                QMessageBox::Ok, QMessageBox::Ok);

      mpEditName->setText(FROM_UTF8(mpObject->getObjectName()));
      return false;
    }

  mpEditName->setText(FROM_UTF8(newName));

  protectedNotify(mObjectType, ListViews::RENAME, mKey);

  if (mpDataModel != NULL)
    {
      mpDataModel->changed();
    }

  return true;
}
