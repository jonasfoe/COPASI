// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual 
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

%{

#include "utilities/CTaskEnum.h"  
#include "utilities/CCopasiMethod.h"  

%}



%newobject CCopasiMethod::createMethod(const CCopasiContainer * pParent,
                                      const CTaskEnum::Method & methodType,
                                      const CTaskEnum::Task & taskType);

%ignore operator<<;
%ignore CCopasiMethod::XMLSubType;
%ignore CTaskEnum::MethodName;
%ignore CTaskEnum::MethodXML;
%ignore CCopasiMethod::SubTypeName;
%ignore CCopasiMethod::setCallBack;
%ignore CCopasiMethod::load;
%ignore CCopasiMethod::print;
%ignore CCopasiMethod::printResult;

%include "utilities/CCopasiMethod.h"
%include "utilities/CTaskEnum.h"


%extend CCopasiMethod{
  static std::string getSubTypeName(const int& subType)
  {
    return CTaskEnum::MethodName[subType];
  }

  static int TypeNameToEnum(const std::string& typeName)
  {
     return toEnum(typeName.c_str(), CTaskEnum::MethodName, CTaskEnum::UnsetMethod); 
  }
}