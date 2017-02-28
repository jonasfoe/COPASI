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

#ifndef CQMERGINGDATA_H
#define CQMERGINGDATA_H

#include <string>
#include <vector>

#include <QPixmap>
#include <QDialog>

#include "model/CModel.h"

#include "ui_CQMergingData.h"

class CQMergingData : public QDialog, public Ui::CQMergingData
{
  Q_OBJECT

public:
  CQMergingData(QWidget* parent = 0, CModel * pModel = 0, Qt::WindowFlags fl = 0);
  ~CQMergingData();

  std::map< QTreeWidgetItem *, const CCopasiObject * > mItemMap1;
  std::map< QTreeWidgetItem *, const CCopasiObject * > mItemMap2;

protected:
  CModel* mpModel;

protected slots:

  void treeSelectionChanged();

private:

  /**
   * fills a tree widget with the items of the model. All tree items are mapped to the model items
   * in itemMap.
   * const std::set<CCopasiObject*> & added : contains the model elements that were added by the latest
   * model adding operation. They will be highlighted.
   * If highlightInvolved is true, items that are actually referred to in other parts of the model are
   * highlighted (these are the items that it is useful to replace)
   */
  void fillTree(QTreeWidget* pW, const CModel* pModel, std::map<QTreeWidgetItem*, const CCopasiObject*>& itemMap,
                bool flagGlobalQuantities, bool flagReactions,
                const std::set<const CCopasiObject*> & added,
                bool highlightInvolved);

  void load();

private slots:
  void slotBtnMerge();
  void slotBtnCancel();
};

#endif // CQMERGINGDATA_H
