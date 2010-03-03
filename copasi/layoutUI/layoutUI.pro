# Begin CVS Header
#   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layoutUI/layoutUI.pro,v $
#   $Revision: 1.31.2.2 $
#   $Name:  $
#   $Author: shoops $
#   $Date: 2010/03/03 18:09:54 $
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

LIB = layoutUI

QT +=  opengl 

HEADERS = CQGLNetworkPainter.h \
          CArrow.h \
          CCompartmentGraphNode.h \
          CGraphNode.h \
          CGraphCurve.h \
          CQGLViewport.h \
          CQLayoutMainWindow.h \
          CQPlayerControlWidget.h \
          CSimSummaryInfo.h \
          CDataEntity.h \
          CVisParameters.h \
          CQCurrentValueTable.h \
          BezierCurve.h \
          FontChooser.h \
          ParaPanel.h \
          NodeSizePanel.h \
          RGTextureSpec.h \
          CLabel.h


SOURCES = CQLayoutMainWindow.cpp \
          CArrow.cpp \
          CCompartmentGraphNode.cpp \
          CGraphNode.cpp \
          CGraphCurve.cpp \
          CQGLNetworkPainter.cpp \
          CQPlayerControlWidget.cpp \
          CSimSummaryInfo.cpp \
          CDataEntity.cpp \
          CVisParameters.cpp \
          CQCurrentValueTable.cpp \
          CQGLViewport.cpp \
          BezierCurve.cpp \
          FontChooser.cpp \
          ParaPanel.cpp \
          NodeSizePanel.cpp \
          RGTextureSpec.cpp \
          CLabel.cpp


FORMS = ParaPanel.ui \
        NodeSizePanel.ui \
        FontChooser.ui

include(../lib.pri)

CONFIG += qt

include(../common.pri)



