/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/optimization/CRandomSearch.h,v $
   $Revision: 1.13 $
   $Name:  $
   $Author: shoops $
   $Date: 2011/03/07 19:31:26 $
   End CVS Header */

// Copyright (C) 2011 - 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 * CRandomSearch class
 */

#ifndef COPASI_CRandomSearch
#define COPASI_CRandomSearch

//class CRandom;
//*** need to be added for definition
class COptProblem;
class COptMethod;

#include "randomGenerator/CRandom.h"
#include "utilities/CVector.h"

// YOHE: this is an abstract class that contains many virtual functions
// without definitions
//
/** @dia:pos 48.05,34.05 */
/** @dia:route COptMethod; v,46.9608,16.35,33,59.1652,34.05 */
class CRandomSearch : public COptMethod
{
  friend COptMethod * COptMethod::createMethod(CCopasiMethod::SubType subType);

  // Operations
private:
  /**
   * Default Constructor
   */
  CRandomSearch();

  /**
      * Initialize arrays and pointer.
      * @return bool success
      */
  virtual bool initialize();

  /**
   * Initialize contained objects.
   */
  void initObjects();

  /**
   * Evaluate the fitness of one individual
   * @param const CVector< C_FLOAT64 > & individual
   * @return bool continue
   */
  bool evaluate(const CVector< C_FLOAT64 > & individual);

  /**
   * Find the best individual at this generation
   * @return size_t fittest
   */
  size_t fittest();

  /**
   * number of iterations
   */
  unsigned C_INT32 mIterations;

  /**
   * The current iteration
   */
  unsigned C_INT32 mCurrentIteration;

  /**
   * a vector of the number of individuals.
   */
  CVector<C_FLOAT64> mIndividual;

  /**
   * array of values of objective function f/ individuals
   */
  C_FLOAT64 mValue;

  /**
   * a pointer to the randomnumber generator.
   */
  CRandom * mpRandom;

  /**
   * *** Perhaps this is actually not needed ****number of parameters
   */
  size_t mVariableSize;

  /**
   * The best value found so far.
   */
  C_FLOAT64 mBestValue;

public:
  /**
   * Copy Constructor
   * @param const CRandomSearch & src
   */
  CRandomSearch(const CRandomSearch & src);

  /**
   * Destructor
   */
  virtual ~CRandomSearch();

  /**
   * Execute the optimization algorithm calling simulation routine
   * when needed. It is noted that this procedure can give feedback
   * of its progress by the callback function set with SetCallback.
   */
  virtual bool optimise();

  /**
   * Returns the maximum verbosity at which the method can log.
   */
  virtual unsigned C_INT32 getMaxLogVerbosity() const;
};

#endif  // COPASI_CRandomSearch
