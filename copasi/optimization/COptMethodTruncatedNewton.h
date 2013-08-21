// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
  * COptMethodTruncatedNewton class
  */

#ifndef COPASI_COptMethodTruncatedNewton
#define COPASI_COptMethodTruncatedNewton

#include <vector>
#include "utilities/CMatrix.h"
#include "optimization/COptMethod.h"

#include "CTruncatedNewton.h"

class FTruncatedNewton;
class CTruncatedNewton;

class COptMethodTruncatedNewton: public COptMethod
{
  // Operations
public:
  /**
   * Specific constructor
   * @param const CCopasiContainer * pParent
   * @param const CTaskEnum::Method & methodType (default: TruncatedNewton)
   * @param const CTaskEnum::Task & taskType (default: optimization)
   */
  COptMethodTruncatedNewton(const CCopasiContainer * pParent,
                            const CTaskEnum::Method & methodType = CTaskEnum::TruncatedNewton,
                            const CTaskEnum::Task & taskType = CTaskEnum::optimization);

  /**
   * Copy Constructor
   * @param const COptMethodTruncatedNewton & src
   * @param const CCopasiContainer * pParent (default: NULL)
   */
  COptMethodTruncatedNewton(const COptMethodTruncatedNewton & src,
                            const CCopasiContainer * pParent);

  /**
   * Destructor
   */
  virtual ~COptMethodTruncatedNewton();

  /**
   * Execute the optimization algorithm calling simulation routine
   * when needed. It is noted that this procedure can give feedback
   * of its progress by the callback function set with SetCallback.
   * @ return success;
   */
  virtual bool optimise();

  /**
    * Returns the maximum detail at which the method can log.
    */
  virtual unsigned C_INT32 getMaxLogDetail() const;

private:
  /**
   * Default Constructor
   */
  COptMethodTruncatedNewton();

  /**
   * Initialize contained objects.
   */
  void initObjects();

  /**
   * Initialize arrays and pointer.
   * @return bool success
   */
  virtual bool initialize();

  /**
   * Cleanup arrays and pointers.
   * @return bool success
   */
  virtual bool cleanup();

  /**
   * The tolerance
   */
  //   C_FLOAT64 mTolerance;

  /**
   * The number of iterations
   */
  unsigned C_INT32 mIteration;

  /**
   * Handle to the process report item "Current Iteration"
   */
  unsigned C_INT32 mhIteration;

  /**
   * number of parameters
   */
  C_INT mVariableSize;

  /**
   * The current solution guess
   */
  CVector< C_FLOAT64 > mCurrent;

  /**
   * The gradient
   */
  CVector< C_FLOAT64 > mGradient;

  /**
   * The last individual
   */
  CVector< C_FLOAT64 > mBest;

  /**
   * The best value found so far
   */
  C_FLOAT64 mBestValue;

  /**
   * The result of a function evaluation
   */
  C_FLOAT64 mEvaluationValue;

  /**
   * Flag indicating whether the computation shall continue
   */
  bool mContinue;

  /**
   * Functor pointing to the Truncated Newton method.
   */
  FTruncatedNewton * mpTruncatedNewton;

  /**
   * CTruncatedNewton function.
   */
  CTruncatedNewton * mpCTruncatedNewton;

  /**
   * callback function for evaluation of objective function and its gradient
   */
  C_INT sFun(C_INT *, C_FLOAT64 *, C_FLOAT64 *, C_FLOAT64 *);

  /**
   * objective function evaluation for specified parameters
   */
  bool evaluateFunction(C_INT *n, C_FLOAT64 *x, C_FLOAT64 *f);

  /**
   * Evaluate the objective function
   * @return bool continue
   */
  const C_FLOAT64 & evaluate();
};

#endif  // COPASI_COptMethodTruncatedNewton
