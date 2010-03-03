# Begin CVS Header
#   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layout/layout.pro,v $
#   $Revision: 1.6.2.2 $
#   $Name:  $
#   $Author: shoops $
#   $Date: 2010/03/03 18:09:55 $
# End CVS Header

# Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc., University of Heidelberg, and The University 
# of Manchester. 
# All rights reserved. 

# Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
# and The University of Manchester. 
# All rights reserved. 

# Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
# Properties, Inc. and EML Research, gGmbH. 
# All rights reserved. 

LIB = layout

# Input
HEADERS += CLayout.h \
           CLayoutInitializer.h \
           CLBase.h \
           CLCurve.h \
           CLGraphicalObject.h \
           CLGlyphs.h \
           CListOfLayouts.h \
           CLReactionGlyph.h \
           SBMLDocumentLoader.h


SOURCES += CLayout.cpp \
           CLayoutInitializer.cpp \
           CLBase.cpp \
           CLCurve.cpp \
           CLGraphicalObject.cpp \
           CLGlyphs.cpp \
           CListOfLayouts.cpp \
           CLReactionGlyph.cpp \
           SBMLDocumentLoader.cpp

include(../lib.pri)
include(../common.pri)

#The following line was inserted by qt3to4
QT +=  qt3support 
