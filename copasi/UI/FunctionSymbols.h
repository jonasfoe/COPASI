/****************************************************************************
 **  $ CopasiUI/FunctionSymbols.h               
 **  $ Author  : Mudita Singhal
 **  
 ** This is the header file for the Function Symbols
 *****************************************************************************/

#ifndef FUNCTION_SYMBOLS_H
#define FUNCTION_SYMBOLS_H

#include <qtable.h>
#include <qpushbutton.h>

#include "MyTable.h"
#include "copasi.h"
#include "copasiwidget.h"

class CMathModel;

class FunctionSymbols : public CopasiWidget
  {
    Q_OBJECT

  protected:
    CMathModel * mModel;
    MyTable * table;
    QPushButton *btnOK;
    QPushButton *btnCancel;

  signals:
    void name(const QString &);

  public:
    FunctionSymbols (QWidget *parent, const char * name = 0, WFlags f = 0);
    void loadFunctionSymbols(CMathModel *model);
    void resizeEvent(QResizeEvent * re);

  protected slots:
    virtual void slotBtnOKClicked();
    virtual void slotBtnCancelClicked();
    virtual void slotTableSelectionChanged();

  private:
    void showMessage(QString caption, QString text);
  };

#endif
