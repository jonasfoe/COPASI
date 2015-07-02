// Copyright (C) 2010 - 2013 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2006 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 * COptMethodPS class
 */

#ifndef COPASI_COptMethodPS
#define COPASI_COptMethodPS

#include "optimization/COptMethod.h"
#include "utilities/CVector.h"
#include "utilities/CMatrix.h"

class CRandom;
class CPermutation;

class COptMethodPS : public COptMethod
{
  friend COptMethod * COptMethod::createMethod(CCopasiMethod::SubType subType);

  // Operations
public:
  /**
   * Copy Constructor
   * @param const COptMethodPS & src
   * @param const CCopasiContainer * pParent (default: NULL)
   */
  COptMethodPS(const COptMethodPS & src,
               const CCopasiContainer * pParent = NULL);

  /**
   * Destructor
   */
  virtual ~COptMethodPS();

  /**
   * Execute the optimization algorithm calling simulation routine
   * when needed. It is noted that this procedure can give feedback
   * of its progress by the callback function set with SetCallback.
   * @ return success;
   */
  virtual bool optimise();

  /**
   * Returns the maximum verbosity at which the method can log.
   */
  virtual unsigned C_INT32 getMaxLogVerbosity() const;

private:
  /**
   * Default Constructor
   * @param const CCopasiContainer * pParent (default: NULL)
   */
  COptMethodPS(const CCopasiContainer * pParent = NULL);

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
   * Evaluate the fitness of one individual
   * @return const C_FLOAT64 value
   */
  const C_FLOAT64 & evaluate();

  /**
   * Move the indexed individual in the swarm
   * @param const size_t & index
   * @return bool continue
   */
  bool move(const size_t & index);

  /**
   * Create the indexed individual in the swarm
   * @param const size_t & index
   * @return bool continue
   */
  bool create(const size_t & index);

  /**
   * create the informant for each individual
   */
  void buildInformants();

  bool reachedStdDeviation();

  /**
   * Calculate the swarm variance of the function value
   * @return C_FLOAT64 variance
   */
  C_FLOAT64 calcFValVariance() const;

  /**
   * Calculate the swarm variance of a given parameter
   * @param const size_t & variable
   * @return C_FLOAT64 variance
   */
  C_FLOAT64 calcVariableVariance(const size_t & variable) const;

  /**
   * Create a status dump string containing html tables of parameter and variance information
   * @return std::string statusDump
   */
  std::string dumpStatus() const;

  // Attributes
private:
  /**
   * maximal number of iterations
   */
  unsigned C_INT32 mIterationLimit;

  /**
   * size of the population
   */
  unsigned C_INT32 mSwarmSize;

  /**
   * The variance acceptable for the solution
   */
  C_FLOAT64 mVariance;

  /**
   * a pointer to the random number generator.
   */
  CRandom * mpRandom;

  /**
   * current iterations
   */
  unsigned C_INT32 mIteration;

  /**
   * Handle to the process report item "Current Iteration"
   */
  size_t mhIteration;

  /**
   * number of parameters
   */
  size_t mVariableSize;

  /**
   * Matrix of individuals with candidate values for the parameters
   */
  CVector< CVector< C_FLOAT64 > > mIndividuals;

  /**
   * Vector of values of objective function of each individual
   */
  CVector< C_FLOAT64 > mValues;

  /**
   * Matrix of individual velocities
   */
  CMatrix< C_FLOAT64 > mVelocities;

  /**
   * Vector of individual best values.
   */
  CVector< C_FLOAT64 > mBestValues;

  /**
   * Matrix of best positions for each individual
   */
  CMatrix< C_FLOAT64 > mBestPositions;

  /**
   * A permutation of integers used to create the informants;
   */
  CPermutation * mpPermutation;

  /**
   * Vector containing the set of informants for each individual.
   */
  std::vector< std::set< size_t > > mInformants;

  /**
   * The minimal number of individuals informed by each individual
   */
  size_t mNumInformedMin;

  /**
   * The number of individual informed by each individual
   */
  size_t mNumInformed;

  /**
   * Index of the best solution found so far.
   */
  size_t mBestIndex;

  /**
   * The value of the last evaluation.
   */
  C_FLOAT64 mEvaluationValue;

  /**
   * Indicates whether calculation shall continue
   */
  bool mContinue;
};

#endif  // COPASI_COptMethodPS
