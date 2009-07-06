# Begin CVS Header 
#   $Source: /fs/turing/cvs/copasi_dev/cvs_admin/addHeader,v $ 
#   $Revision: 1.10 $ 
#   $Name:  $ 
#   $Author: shoops $ 
#   $Date: 2008/04/11 15:21:36 $ 
# End CVS Header 
# Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
# and The University of Manchester. 
# All rights reserved. 

LIB = sbmlunit

include(../lib.pri)
include(../common.pri)

# Input
HEADERS +=  CUnitInterfaceSBML.h \
            CUnit.h \
            Expression2PresentationMML.h \
            Expression2PresentationMMLUnits.h

SOURCES +=  CUnitInterfaceSBML.cpp \
            CUnit.cpp \
            Expression2PresentationMML.cpp \
            Expression2PresentationMMLUnits.cpp


#DISTFILES +=ssbmlunit.vcproj
