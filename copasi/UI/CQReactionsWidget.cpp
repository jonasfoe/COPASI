// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include "CQReactionsWidget.h"

#include <QHeaderView>
#include <QClipboard>
#include <QKeyEvent>

#include "copasi.h"

#include "qtUtilities.h"
#include "CQMessageBox.h"

#include "model/CModel.h"
#include "CopasiDataModel/CDataModel.h"
#include "copasi/core/CRootContainer.h"

#include "copasiui3window.h"

/*
 *  Constructs a CQReactionsWidget which is a child of 'parent', with the
 *  name 'name'.'
 */
CQReactionsWidget::CQReactionsWidget(QWidget *parent, const char *name)
  : CopasiWidget(parent, name)
{
  setupUi(this);
  //Create Source Data Model.
  mpReactionDM = new CQReactionDM(this, mpDataModel);
  //Create the Proxy Model for sorting/filtering and set its properties.
  mpProxyModel = new CQSortFilterProxyModel();
  mpProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  mpProxyModel->setFilterKeyColumn(-1);
#if QT_VERSION >= 0x050000
  mpTblReactions->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
  mpTblReactions->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
  mpTblReactions->verticalHeader()->hide();
  mpTblReactions->sortByColumn(COL_ROW_NUMBER, Qt::AscendingOrder);
  setFramework(mFramework);
  // Connect the table widget
  connect(mpReactionDM, SIGNAL(signalNotifyChanges(const CUndoData::ChangeSet &)),
          this, SLOT(slotNotifyChanges(const CUndoData::ChangeSet &)));
  connect(mpReactionDM, SIGNAL(notifyGUI(ListViews::ObjectType, ListViews::Action, const CCommonName &)),
          this, SLOT(protectedNotify(ListViews::ObjectType, ListViews::Action, const CCommonName &)));
  connect(mpReactionDM, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
  connect(mpLEFilter, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotFilterChanged()));
  CopasiUI3Window   *pWindow = dynamic_cast<CopasiUI3Window * >(parent->parent());
  mpReactionDM->setUndoStack(pWindow->getUndoStack());
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQReactionsWidget::~CQReactionsWidget()
{
  pdelete(mpProxyModel);
  pdelete(mpReactionDM);
  // no need to delete child widgets, Qt does it all for us
}

void CQReactionsWidget::slotBtnNewClicked()
{
  mpReactionDM->insertRow(mpReactionDM->rowCount() - 1, QModelIndex());
  updateDeleteBtns();
}

void CQReactionsWidget::slotBtnDeleteClicked()
{
  if (mpTblReactions->hasFocus())
    {deleteSelectedReactions();}

  updateDeleteBtns();
}

void CQReactionsWidget::slotBtnClearClicked()
{
  int ret = CQMessageBox::question(this, tr("Confirm Delete"), "Delete all Reactions?",
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (ret == QMessageBox::Yes)
    {
      mpReactionDM->clear();
    }

  updateDeleteBtns();
}

bool CQReactionsWidget::updateProtected(ListViews::ObjectType objectType, ListViews::Action action, const CCommonName & cn)
{
  if (mIgnoreUpdates || !isVisible())
    {
      return true;
    }

  if (objectType == ListViews::MODEL)
    {
      enterProtected();
    }

  return true;
}

bool CQReactionsWidget::leave()
{
  return true;
}

bool CQReactionsWidget::enterProtected()
{
  if (mpTblReactions->selectionModel() != NULL)
    {
      disconnect(mpTblReactions->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 this, SLOT(slotSelectionChanged(const QItemSelection &, const QItemSelection &)));
    }

  mpProxyModel->setSourceModel(mpReactionDM);
  //Set Model for the TableView
  mpTblReactions->setModel(NULL);
  mpTblReactions->setModel(mpProxyModel);
  connect(mpTblReactions->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this, SLOT(slotSelectionChanged(const QItemSelection &, const QItemSelection &)));
  updateDeleteBtns();
  mpTblReactions->resizeColumnsToContents();
  setFramework(mFramework);
  return true;
}

void CQReactionsWidget::updateDeleteBtns()
{
  bool selected = false;
  QModelIndexList selRows = mpTblReactions->selectionModel()->selectedRows();

  if (selRows.size() == 0)
    selected = false;
  else
    {
      if (selRows.size() == 1)
        {
          if (mpReactionDM->isDefaultRow(mpProxyModel->mapToSource(selRows[0])))
            selected = false;
          else
            selected = true;
        }
      else
        selected = true;
    }

  mpBtnDelete->setEnabled(selected);

  if (mpProxyModel->rowCount() - 1)
    mpBtnClear->setEnabled(true);
  else
    mpBtnClear->setEnabled(false);
}

void CQReactionsWidget::slotSelectionChanged(const QItemSelection &C_UNUSED(selected),
    const QItemSelection &C_UNUSED(deselected))
{
  updateDeleteBtns();
}

void CQReactionsWidget::dataChanged(const QModelIndex &C_UNUSED(topLeft),
                                    const QModelIndex &C_UNUSED(bottomRight))
{
  mpTblReactions->resizeColumnsToContents();
  setFramework(mFramework);
  updateDeleteBtns();
}

void CQReactionsWidget::slotDoubleClicked(const QModelIndex proxyIndex)
{
  QModelIndex index = mpProxyModel->mapToSource(proxyIndex);

  if (index.row() < 0)
    return;

  if (mpReactionDM->isDefaultRow(index))
    {
      slotBtnNewClicked();
    }

  assert(mpDataModel != NULL);
  CModel *pModel = mpDataModel->getModel();

  if (pModel == NULL)
    return;

  std::string key = pModel->getReactions()[index.row()].getKey();

  if (CRootContainer::getKeyFactory()->get(key))
    mpListView->switchToOtherWidget(C_INVALID_INDEX, key);
}

void CQReactionsWidget::keyPressEvent(QKeyEvent *ev)
{
  if (ev->key() == Qt::Key_Delete)
    slotBtnDeleteClicked();
  else if (ev->key() == Qt::Key_C && (ev->modifiers() & Qt::ControlModifier))
    {
      QModelIndexList selRows = mpTblReactions->selectionModel()->selectedRows(0);

      if (selRows.empty())
        {return;}

      QString str;
      QModelIndexList::const_iterator i;

      for (i = selRows.begin(); i != selRows.end(); ++i)
        {
          for (int x = 0; x < mpReactionDM->columnCount(); ++x)
            {
              if (!mpTblReactions->isColumnHidden(x))
                {
                  if (!str.isEmpty())
                    str += "\t";

                  str += mpReactionDM->index(mpProxyModel->mapToSource(*i).row(), x).data().toString();
                }
            }

          str += "\n";
        }

      QApplication::clipboard()->setText(str);
    }
}

void CQReactionsWidget::slotFilterChanged()
{
  QRegExp regExp(mpLEFilter->text() + "|New Reaction", Qt::CaseInsensitive, QRegExp::RegExp);
  mpProxyModel->setFilterRegExp(regExp);
}

void CQReactionsWidget::setFramework(int framework)
{
  CopasiWidget::setFramework(framework);

  switch (mFramework)
    {
      case 0:
        mpTblReactions->showColumn(COL_FLUX);
        mpTblReactions->hideColumn(COL_PARTICLE_FLUX);
        break;

      case 1:
        mpTblReactions->hideColumn(COL_FLUX);
        mpTblReactions->showColumn(COL_PARTICLE_FLUX);
        break;
    }
}

CQBaseDataModel * CQReactionsWidget::getCqDataModel()
{
  return mpReactionDM;
}

//UNDO section
void CQReactionsWidget::deleteSelectedReactions()
{
  const QItemSelectionModel *pSelectionModel = mpTblReactions->selectionModel();
  QModelIndexList mappedSelRows;
  size_t i, imax = mpReactionDM->rowCount();

  for (i = 0; i < imax; i++)
    {
      if (pSelectionModel->isRowSelected((int) i, QModelIndex()))
        {
          mappedSelRows.append(mpProxyModel->mapToSource(mpProxyModel->index((int) i, 0)));
        }
    }

  if (mappedSelRows.empty())
    {return;}

  mpReactionDM->removeRows(mappedSelRows);
}
