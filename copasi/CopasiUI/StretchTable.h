/********************************************************
    Author: Liang Xu
    Version : 1.xx  <first>
    Description: 
    Date: 04/03 
    Comment : Provide automatic stretch for all tables as base class
    Contact: Please contact lixu1@vt.edu.
 *********************************************************/

#ifndef STRETCH_TABLE_H
#define STRETCH_TABLE_H

#include <qtable.h>

class StretchTable: public QTable
  {
  public:
    StretchTable (QWidget * parent = 0, const char * name = 0);
    StretchTable (int numRows, int numCols, QWidget * parent = 0, const char * name = 0);
    ~StretchTable();
    void columnWidthChanged (int col);
    void setNumCols (int c);
    void setNumRows (int r);
    void setProtected(const bool & pRotected);

    bool isProtected() const;

    int numColumn;
    int* minColWidth;
    void DisableColWidthUpdate();
    void EnableColWidthUpdate();

  private:
    bool systemUpdate;
    bool* binitialized;
    bool mProtected;
  };

#endif
