/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/optimization/COptMethodLevenbergMarquardt.h,v $
   $Revision: 1.7 $
   $Name:  $
   $Author: mendes $
   $Date: 2012/06/04 14:12:37 $
   End CVS Header */

// Copyright (C) 2012 - 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
  * COptMethodLevenbergMarquardt class
  */

#ifndef COPASI_COptMethodLevenbergMarquardt
#define COPASI_COptMethodLevenbergMarquardt

#include <vector>
#include "utilities/CMatrix.h"
#include "optimization/COptMethod.h"

class CRandom;

class COptMethodLevenbergMarquardt : public COptMethod
{
  friend COptMethod * COptMethod::createMethod(CCopasiMethod::SubType subType);

  // Operations
public:
  /**
   * Copy Constructor
   * @param const COptMethodLevenbergMarquardt & src
   * @param const CCopasiContainer * pParent (default: NULL)
   */
  COptMethodLevenbergMarquardt(const COptMethodLevenbergMarquardt & src,
                               const CCopasiContainer * pParent = NULL);

  /**
   * Destructor
   */
  virtual ~COptMethodLevenbergMarquardt();

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
   * @param const CCopasiContainer * pParent (default: NULL)
   */
  COptMethodLevenbergMarquardt(const CCopasiContainer * pParent = NULL);

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
   * Evaluate the objective function
   * @return bool continue
   */
  const C_FLOAT64 & evaluate();

  /**
   * Calculate the gradient of the objective at the current parameters
   */
  void gradient();

  /**
   * Calculate the Hessian of the objective at the current point
   */
  void hessian();

  // Attributes
private:

  /**
     * The maximum number of iterations
     */
  unsigned C_INT32 mIterationLimit;

  /**
   * The tolerance
   */
  C_FLOAT64 mTolerance;

  /**
   * The modulation factor
   */
  C_FLOAT64 mModulation;

  /**
   * The number of iterations
   */
  unsigned C_INT32 mIteration;

  /**
   * Count of algorithm leaving parameter space
   */
  unsigned C_INT32 mParameterOutOfBounds;

  /**
   * Handle to the process report item "Current Iteration"
   */
  size_t mhIteration;

  /**
   * number of parameters
   */
  size_t mVariableSize;

  /**
   * The current solution guess
   */
  CVector< C_FLOAT64 > mCurrent;

  /**
   * The last individual
   */
  CVector< C_FLOAT64 > mBest;

  /**
   * The gradient
   */
  CVector< C_FLOAT64 > mGradient;

  /**
   * The step taken
   */
  CVector< C_FLOAT64 > mStep;

  /**
   * The Hessian matrix
   */
  CMatrix<C_FLOAT64> mHessian;

  /**
   * The Hessian matrix modified according to Levenberg-Marquardt
   */
  CMatrix<C_FLOAT64> mHessianLM;

  /**
   * Vector for temporary values
   */
  CVector< C_FLOAT64 > mTemp;

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
   * Indicate whether we have access to the residuals, i.e., it is a
   * fitting problem we are working on.
   */
  bool mHaveResiduals;

  /**
   * The transpose jacobian of the residuals.
   */
  CMatrix< C_FLOAT64 > mResidualJacobianT;
};

#endif  // COPASI_COptMethodLevenbergMarquardt
