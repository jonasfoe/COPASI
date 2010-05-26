// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CFunction.i,v $ 
//   $Revision: 1.4.2.2 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2010/05/26 17:45:12 $ 
// End CVS Header 

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

#include "function/CFunction.h"

%}

%ignore CFunction::load;
%ignore CFunction::dependsOn;
%ignore CFunction::splitFunction;

#ifdef SWIGJAVA
// remove some const methods to get rid of warnings
%ignore CFunction::getVariables() const;

#endif // SWIGJAVA

%include "function/CFunction.h" 



