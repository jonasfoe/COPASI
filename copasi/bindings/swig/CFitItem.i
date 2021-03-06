// Copyright (C) 2019 by Pedro Mendes, Rector and Visitors of the 
// University of Virginia, University of Heidelberg, and University 
// of Connecticut School of Medicine. 
// All rights reserved. 

// Copyright (C) 2017 - 2018 by Pedro Mendes, Virginia Tech Intellectual 
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

%{

#include "copasi/parameterFitting/CFitItem.h"

%}

%ignore CFitItem::updateBounds;
%ignore CFitItem::getObjectValue;
%ignore CFitItem::getUpdateMethod;

%include "copasi/parameterFitting/CFitItem.h"

// unignore getObjectValue
%rename(getObjectValue) CFitItem::getObjectValue;

%extend CFitItem
{
    double getObjectValue() const
    {
        double v=*($self->getObjectValue());
        return v;
    }
}
