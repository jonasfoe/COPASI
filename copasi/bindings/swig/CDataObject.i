// Copyright (C) 2019 - 2021 by Pedro Mendes, Rector and Visitors of the 
// University of Virginia, University of Heidelberg, and University 
// of Connecticut School of Medicine. 
// All rights reserved. 

// Copyright (C) 2017 - 2018 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., University of Heidelberg, and University of 
// of Connecticut School of Medicine. 
// All rights reserved. 










%{
#include "copasi/core/CRegisteredCommonName.h"
#include "copasi/core/CDataObject.h"
%}

%ignore CObjectInterface::getValidity() const;
%ignore CObjectInterface::getValuePointer;
%ignore UpdateMethod;
%ignore SpecificUpdateMethod;
%ignore Refresh;
%ignore operator<<;
%ignore CDataObject::setRenameHandler;
%ignore CDataObject::setRenameHandler;
%ignore CDataObject::getValuePointer;
%ignore CDataObject::print;
%ignore CDataObject::setUpdateMethod;
%ignore CDataObject::getUpdateMethod;
%ignore CDataObject::setRefresh;
%ignore CDataObject::getRefresh;
%ignore CDataObject::buildUpdateSequence;
%ignore CDataObject::getDirectDependencies;
%ignore CDataObject::setDirectDependencies;
%ignore CDataObject::getAllDependencies;
%ignore CDataObject::destruct;
%ignore CDataObject::createUndoData;

%include "copasi/core/CRegisteredCommonName.h"
%include "copasi/core/CDataObject.h"

%extend CObjectInterface
{
public:
	CDataObject* toObject()
	{
		return dynamic_cast<CDataObject*>($self);
	}
  
  std::string printToString()
  {
    CDataObject* obj = dynamic_cast<CDataObject*>($self);
    if (obj == NULL)
      return "";
    std::stringstream str; 
    obj->print(&str);
    return str.str();
  }
  
  bool getBoolValue()
  {
    CDataObject* obj = dynamic_cast<CDataObject*>($self);
    if (obj == NULL || obj->getValuePointer() == NULL || obj->hasFlag(CDataObject::ValueBool))
      return false;
    return *static_cast<bool*>(obj->getValuePointer());
  }
  
  void setBoolValue(bool value)
  {
    CDataObject* obj = dynamic_cast<CDataObject*>($self);
    if (obj == NULL || obj->getValuePointer() == NULL || obj->hasFlag(CDataObject::ValueBool))
      return;

    *(bool*)obj->getValuePointer() = value;
  }
  
  int getIntValue()
  {
    CDataObject* obj = dynamic_cast<CDataObject*>($self);
    if (obj == NULL || obj->getValuePointer() == NULL || obj->hasFlag(CDataObject::ValueInt))
      return 0;
    return *static_cast<C_INT32*>(obj->getValuePointer());
  }
  
  void setIntValue(int value)
  {
    CDataObject* obj = dynamic_cast<CDataObject*>($self);
    if (obj == NULL || obj->getValuePointer() == NULL || obj->hasFlag(CDataObject::ValueInt))
      return;

    *(C_INT32*)obj->getValuePointer() = value;
  }
  
  double getDblValue()
  {
    CDataObject* obj = dynamic_cast<CDataObject*>($self);
    if (obj == NULL || obj->getValuePointer() == NULL || obj->hasFlag(CDataObject::ValueDbl))
      return std::numeric_limits<double>::quiet_NaN();
    return *static_cast<double*>(obj->getValuePointer());
  }
  
  void setDblValue(double value)
  {
    CDataObject* obj = dynamic_cast<CDataObject*>($self);
    if (obj == NULL || obj->getValuePointer() == NULL || obj->hasFlag(CDataObject::ValueDbl))
      return;

    *(C_FLOAT64*)obj->getValuePointer() = value;
  }

}

namespace std {

#if !defined(SWIGR) && !defined(SWIGPERL) && !defined(SWIGOCTAVE)
%template(DataObjectSet) set< const CDataObject * >;
%template(DataObjectMap) map< const CDataObject *, const CDataObject * >;
%template(DataObjectVector) vector< const CDataObject * >;
%template(VectorOfDataObjectVector) vector< vector< const CDataObject * > >;
#endif // !defined(SWIGR) && !defined(SWIGPERL) && !defined(SWIGOCTAVE)
}
