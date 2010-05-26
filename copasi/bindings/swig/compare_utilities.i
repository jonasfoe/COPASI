// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/compare_utilities.i,v $ 
//   $Revision: 1.1.22.2 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2010/05/26 17:45:12 $ 
// End CVS Header 

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., University of Heidelberg, and The University 
// of Manchester. 
// All rights reserved. 

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc. and EML Research, gGmbH. 
// All rights reserved. 

%{

#include "compareExpressions/compare_utilities.h"
#include "sbml/math/ASTNode.h"  
#include "sbml/Model.h"  
%}

// we ignore all functions that need objects from libsbml
%ignore create_expression(const ASTNode* , const Model* );
%ignore create_simplified_normalform(const ASTNode *);
%ignore create_normalform(const ASTNode* );
%ignore are_equal(const CNormalFraction* , const CNormalFraction* );
%ignore normalize_variable_names(CNormalBase*, std::map<std::string, std::string>&);
%ignore replace_variable_names;
%ignore expand_function_call(const ASTNode* , const Model* );
%ignore expand_function_calls(const ASTNode* , const Model* );
%ignore create_expression(const ASTNode* , const Model* );
%ignore replace_SEC(const ASTNode* );
%ignore replace_CSC(const ASTNode* );
%ignore replace_COT(const ASTNode* );
%ignore replace_SINH(const ASTNode* );
%ignore replace_COSH(const ASTNode* );
%ignore replace_TANH(const ASTNode* );
%ignore replace_SECH(const ASTNode* );
%ignore replace_CSCH(const ASTNode* );
%ignore replace_COTH(const ASTNode* );
%ignore replace_ARCSINH(const ASTNode* );
%ignore replace_ARCCOSH(const ASTNode* );
%ignore replace_ARCTANH(const ASTNode* );
%ignore replace_ARCSECH(const ASTNode* );
%ignore replace_ARCCSCH(const ASTNode* );

%include "compareExpressions/compare_utilities.h"



