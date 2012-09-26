// Begin git Header 
//   Commit: 28d5663ff3fc99993d3b249dec626841cb5247ab 
//   Author: Frank T. Bergmann fbergman@caltech.edu 
//   Date: 2012-08-29 10:43:00 +0200 
// End git Header 


// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.



%{

#include <string>
#include "report/CCopasiObjectName.h"

%}


%ignore CCopasiObjectName::findEx;
%ignore CRegisteredObjectName::getSet;

%include "std_string.i"
%include "report/CCopasiObjectName.h"

%extend CCopasiObjectName
{
  std::string getString()
  {
   return *self;
  }
}



