// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2002 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  COptMethod class
 *  This class describes the interface to all optimization methods.
 *  The various method like RandomSearch or GA have to be derived from
 *  this class.
 *
 *  Created for COPASI by Stefan Hoops 2002
 */

#include <limits.h>

#include "copasi.h"

#include "COptTask.h"
#include "COptMethod.h"
#include "COptProblem.h"

COptMethod::COptMethod(const CCopasiContainer * pParent,
                       const CTaskEnum::Method & methodType,
                       const CTaskEnum::Task & taskType):
  CCopasiMethod(pParent, methodType, taskType),
  mpOptProblem(NULL),
  mpParentTask(NULL),
  mContainerVariables(),
  mpOptItem(NULL),
  mpOptContraints(NULL)
{}

COptMethod::COptMethod(const COptMethod & src,
                       const CCopasiContainer * pParent):
  CCopasiMethod(src, pParent),
  mpOptProblem(src.mpOptProblem),
  mpParentTask(src.mpParentTask),
  mContainerVariables(),
  mpOptItem(src.mpOptItem),
  mpOptContraints(src.mpOptContraints)
{
  mContainerVariables.initialize(src.mContainerVariables);
}

//YOHE: seems "virtual" cannot be outside of class declaration
COptMethod::~COptMethod()
{}

void COptMethod::setProblem(COptProblem * problem)
{
  assert(problem);
  mpOptProblem = problem;
}

//virtual C_INT32 COptMethod::Optimise(C_FLOAT64 (*func) (void))
bool COptMethod::optimise(void)
{
  return false;
}

bool COptMethod::initialize()
{
  if (!mpOptProblem)
    return false;

  if (!(mpOptItem = &mpOptProblem->getOptItemList()))
    return false;

  if (!(mpOptContraints = &mpOptProblem->getConstraintList()))
    return false;

  mContainerVariables.initialize(mpOptProblem->getContainerVariables());

  mpParentTask = dynamic_cast<COptTask *>(getObjectParent());

  if (!mpParentTask) return false;

  /*if (pTask &&
      (mpReport = &pTask->getReport()) &&
      !mpReport->getStream())
    mpReport = NULL;*/

  //clear log
  mMethodLog.str("");

  return true;
}

bool COptMethod::cleanup()
{return true;}

//virtual
bool COptMethod::isValidProblem(const CCopasiProblem * pProblem)
{
  if (!CCopasiMethod::isValidProblem(pProblem)) return false;

  const COptProblem * pTP = dynamic_cast<const COptProblem *>(pProblem);

  if (!pTP)
    {
      CCopasiMessage(CCopasiMessage::EXCEPTION, "Problem is not an optimization problem.");
      return false;
    }

  return true;
}

unsigned C_INT32 COptMethod::getMaxLogDetail() const
{
  return 0;
}

std::string COptMethod::getMethodLog() const
{
  return mMethodLog.str();
}
