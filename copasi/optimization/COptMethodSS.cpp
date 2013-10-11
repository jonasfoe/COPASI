// Copyright (C) 2013 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#define DEBUG_OPT 1

#include <limits>
#include <string>
#include <cmath>
#ifdef DEBUG_OPT
# include <iostream>
# include <fstream>
#endif

#include "copasi.h"
#include "COptMethodSS.h"
#include "COptMethodPraxis.h" //used for local minimisation
#include "COptProblem.h"
#include "parameterFitting/CFitProblem.h"
#include "COptItem.h"
#include "COptTask.h"

#include "randomGenerator/CRandom.h"
#include "utilities/CProcessReport.h"
#include "utilities/CSort.h"
#include "report/CCopasiObjectReference.h"

COptMethodSS::COptMethodSS(const CCopasiContainer * pParent):
  COptMethod(CCopasiTask::optimization, CCopasiMethod::ScatterSearch, pParent),
  mIterations(0),
  mPopulationSize(0),
  mVariableSize(0),
  mRefSet(0),
  mRefSetVal(0),
  mPool(0),
  mPoolVal(0),
  mPoolSize(0),
  mEvaluationValue(std::numeric_limits< C_FLOAT64 >::max()),
  mIteration(0),
  mBestValue(std::numeric_limits< C_FLOAT64 >::max()),
  mBestIndex(C_INVALID_INDEX),
  mpRandom(NULL),
  mpOptProblemLocal(NULL),
  mpLocalMinimizer(NULL)
{
  addParameter("Number of Iterations", CCopasiParameter::UINT, (unsigned C_INT32) 200);
  addParameter("Population Size", CCopasiParameter::UINT, (unsigned C_INT32) 10);
// we no longer give the user choice of rng, we use the mersenne twister!
// but in DEBUG versions we should still have access to it
#ifdef COPASI_DEBUG
  addParameter("Random Number Generator", CCopasiParameter::UINT, (unsigned C_INT32) CRandom::mt19937);
  addParameter("Seed", CCopasiParameter::UINT, (unsigned C_INT32) 0);
#endif
  initObjects();
}

COptMethodSS::COptMethodSS(const COptMethodSS & src,
                           const CCopasiContainer * pParent):
  COptMethod(src, pParent),
  mIterations(0),
  mPopulationSize(0),
  mVariableSize(0),
  mRefSet(0),
  mRefSetVal(0),
  mPool(0),
  mPoolVal(0),
  mPoolSize(0),
  mEvaluationValue(std::numeric_limits< C_FLOAT64 >::max()),
  mIteration(0),
  mBestValue(std::numeric_limits< C_FLOAT64 >::max()),
  mBestIndex(C_INVALID_INDEX),
  mpRandom(NULL),
  mpOptProblemLocal(NULL),
  mpLocalMinimizer(NULL)
{initObjects();}

COptMethodSS::~COptMethodSS()
{cleanup();}

void COptMethodSS::initObjects()
{
  addObjectReference("Current Iteration", mIteration, CCopasiObject::ValueInt);
}

bool COptMethodSS::initialize()
{
  cleanup();

  size_t i;

  if (!COptMethod::initialize())
    {
      if (mpCallBack)
        mpCallBack->finishItem(mhIterations);

      return false;
    }

  // get total number of iterations
  mIterations = * getValue("Number of Iterations").pUINT;
  // set current iteration to zero
  mIteration = 0;

  if (mpCallBack)
    mhIterations =
      mpCallBack->addItem("Current Iteration",
                          mIteration,
                          & mIterations);

  mIteration++;

  // this is hardcoded as per Jose Egea,
  // but we could change it or even make it a parameter
  mLocalFreq = 20;

#ifdef COPASI_DEBUG
  mpRandom =
    CRandom::createGenerator(* (CRandom::Type *) getValue("Random Number Generator").pUINT,
                             * getValue("Seed").pUINT);
#else
  CRandom::createGenerator(* (CRandom::Type *) CRandom::mt19937, 0);
#endif

  // get number of individuals in population
  mPopulationSize = * getValue("Population Size").pUINT;
  // get number of variables in the problem
  mVariableSize = mpOptItem->size();

  CFitProblem * pFitProblem = dynamic_cast< CFitProblem * >(mpOptProblem);

  if (pFitProblem != NULL)
    {
      // this is a least squares problem (param estimation)
      mpLocalMinimizer = COptMethod::createMethod(CCopasiMethod::LevenbergMarquardt);
      mpLocalMinimizer->setValue("Tolerance", (C_FLOAT64) 1.e-004);
      mpLocalMinimizer->setValue("Iteration Limit", (C_INT32) 2000);
    }
  else
    {
      // this is a generic optimisation problem
      //  mpLocalMinimizer = COptMethod::createMethod(CCopasiMethod::Praxis);
      //mpLocalMinimizer = COptMethod::createMethod(CCopasiMethod::NelderMead);
      mpLocalMinimizer = COptMethod::createMethod(CCopasiMethod::HookeJeeves);
      // with a rather relaxed tolerance (1e-3)
      mpLocalMinimizer->setValue("Tolerance", (C_FLOAT64) 1.e-005);
      mpLocalMinimizer->setValue("Iteration Limit", (C_INT32) 50);
      mpLocalMinimizer->setValue("Rho", (C_FLOAT64) 0.2);
      // mpLocalMinimizer->setValue("Scale", (C_FLOAT64) 10.0);
    }

  // local minimization problem (starts as a copy of the current problem)
  if (pFitProblem != NULL)
    {
      // this is a least squares problem (param estimation)
      mpOptProblemLocal = new CFitProblem(*pFitProblem, getObjectParent());
    }
  else
    {
      // this is a generic optimisation problem
      mpOptProblemLocal = new COptProblem(*mpOptProblem, getObjectParent());
    }

  // the local method should not have a callback
  mpOptProblemLocal->setCallBack(NULL);

  // set object parent (this is needed or else initialize() will fail)
  mpLocalMinimizer->setObjectParent(getObjectParent());
  // we also have to initialize the subtask
  mpOptProblemLocal->initializeSubtaskBeforeOutput();
  // initialize it
  mpOptProblemLocal->initialize();
  // no statistics to be calculated
  mpOptProblemLocal->setCalculateStatistics(false);
  // do not randomize the initial values
  mpOptProblemLocal->setRandomizeStartValues(false);
  mpLocalMinimizer->setProblem(mpOptProblemLocal);

  // create matrix for the RefSet (population)
  mRefSet.resize(mPopulationSize);

  for (i = 0; i < mPopulationSize; i++)
    mRefSet[i] = new CVector< C_FLOAT64 >(mVariableSize);

  // create vector for function values (of RefSet members)
  mRefSetVal.resize(mPopulationSize);
  mRefSetVal = std::numeric_limits<C_FLOAT64>::infinity();
  // create vector for counting stuck iterations
  mStuck.resize(mPopulationSize);
  mStuck = 0;
  // create matrix for the RefSet children
  mChild.resize(mPopulationSize);

  for (i = 0; i < mPopulationSize; i++)
    mChild[i] = new CVector< C_FLOAT64 >(mVariableSize);

  // create vector for function values (of child members)
  mChildVal.resize(mPopulationSize);
  mChildVal = std::numeric_limits<C_FLOAT64>::infinity();
  // we have not generated any children yet
  mChildrenGenerated = false;

  // create matrix for the pool of diverse solutions
  // this will also be used to store the initial and
  // final positions of local optimizations
  if (10 * mVariableSize > 2 * mIterations / mLocalFreq)
    {
      mPoolSize = 10 * mVariableSize;
    }
  else
    {
      mPoolSize = 2 * mIterations / mLocalFreq;
    }

  mPool.resize(mPoolSize);

  for (i = 0; i < mPoolSize; i++)
    mPool[i] = new CVector< C_FLOAT64 >(mVariableSize);

  mPoolVal.resize(mPoolSize);
  mPoolVal = std::numeric_limits<C_FLOAT64>::infinity();

  mBestValue = std::numeric_limits<C_FLOAT64>::infinity();

  // array for frequencies
  // initialized at 1
  mFreq.resize(mVariableSize);

  for (i = 0; i < mVariableSize; i++)
    {
      mFreq[i] = new CVector< C_INT32 >(4);
      *mFreq[i] = 1;
    }

  // vector for probabilities
  // initialized at 0
  mProb.resize(4);
  mProb = 0.0;

  return true;
}

bool COptMethodSS::cleanup()
{
  size_t i;

  pdelete(mpRandom);

  pdelete(mpOptProblemLocal);

  pdelete(mpLocalMinimizer);

  for (i = 0; i < mRefSet.size(); i++)
    pdelete(mRefSet[i]);

  for (i = 0; i < mChild.size(); i++)
    pdelete(mChild[i]);

  for (i = 0; i < mPool.size(); i++)
    pdelete(mPool[i]);

  for (i = 0; i < mFreq.size(); i++)
    pdelete(mFreq[i]);

  return true;
}

// Find a local minimum
// solution has initial guess on entry, and solution on exit
// fval has value of objective function on exit
bool COptMethodSS::localmin(CVector< C_FLOAT64 > & solution, C_FLOAT64 & fval)
{
  bool Running = true;
  unsigned C_INT32 i;

  mpOptProblemLocal->reset();

  // first we set up the problem
  // (optmethod and optproblem already setup in initialization)
  // let's get the list of parameters
  std::vector<COptItem *> optitem = mpOptProblemLocal->getOptItemList();

  // and set them to the values passed in solution
  for (i = 0; i < mVariableSize; i++)
    {
      optitem[i]->setStartValue(solution[i]);
    }

  // run it
  Running &= mpLocalMinimizer->optimise();
  // pass the results on to the calling parameters
  fval = mpOptProblemLocal->getSolutionValue();

  for (i = 0; i < mVariableSize; i++)
    {
      solution[i] = mpOptProblemLocal->getSolutionVariables()[i];
    }

  return Running;
}

// evaluate the fitness of one individual
bool COptMethodSS::evaluate(const CVector< C_FLOAT64 > & /* individual */)
{
  bool Running = true;

  // We do not need to check whether the parametric constraints are fulfilled
  // since the parameters are created within the bounds.

  // evaluate the fitness
  Running &= mpOptProblem->calculate();

  // check whether the functional constraints are fulfilled
  if (!mpOptProblem->checkFunctionalConstraints())
    mEvaluationValue = std::numeric_limits<C_FLOAT64>::infinity();
  else
    mEvaluationValue = mpOptProblem->getCalculateValue();

  return Running;
}

// check the best individual in the RefSet
C_INT32 COptMethodSS::fittest()
{
// the best is always at the top of the RefSet
  return 0;
}

// randomize the values of RefSet[i]
bool COptMethodSS::randomize(C_INT32 i)
{
  C_FLOAT64 mn, mx, la; // for boundaries of rnd
  bool Running = true;  // flag for invalid values

  for (C_INT32 j = 0; j < mVariableSize; j++)
    {
      // get pointers to appropriate elements (easier reading of code)
      COptItem & OptItem = *(*mpOptItem)[j];
      C_FLOAT64 & Sol = (*mRefSet[i])[j];
      // calculate lower and upper bounds for this variable
      mn = *OptItem.getLowerBoundValue();
      mx = *OptItem.getUpperBoundValue();

      try
        {
          // calculate orders of magnitude of the interval
          la = log10(mx) - log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min()));

          // determine if linear or log scale
          if ((mn < 0.0) || (mx <= 0.0))
            Sol = mn + mpRandom->getRandomCC() * (mx - mn);
          else
            {
              if (la < 1.8)
                Sol = mn + mpRandom->getRandomCC() * (mx - mn);
              else
                Sol = pow(10.0, log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min()))
                          + la * mpRandom->getRandomCC());
            }
        }
      catch (...)
        {
          Sol = (mx + mn) * 0.5;
        }

      // force it to be within the bounds
      switch (OptItem.checkConstraint(Sol))
        {
          case - 1:
            Sol = *OptItem.getLowerBoundValue();
            break;

          case 1:
            Sol = *OptItem.getUpperBoundValue();
            break;
        }

      // We need to set the value here so that further checks take
      // account of the value.
      (*(*mpSetCalculateVariable)[j])(Sol);
    }

  // calculate its fitness
  Running = evaluate(*mRefSet[i]);
  mRefSetVal[i] = mEvaluationValue;
  // reset the stuck flag
  mStuck[i] = 1;
  return Running;
}

// initialise the population using the
// Diversification Generation Method
bool COptMethodSS::creation(void)
{
  C_INT32 i, j, k, l;   // counters
  C_FLOAT64 mn, mx, la; // for boundaries of rnd
  C_FLOAT64 sum;        // to calculate a summation
  C_FLOAT64 a;          // to hold a random number
  bool Running = true;  // flag for invalid values

  // initialize all entries of the Pool.
  // first 4 candidates as a latin hypercube
  for (i = 0; (i < 4) && Running; i++)
    {
      for (j = 0; j < mVariableSize; j++)
        {
          // get pointers to appropriate elements (easier reading of code)
          COptItem & OptItem = *(*mpOptItem)[j];
          C_FLOAT64 & Sol = (*mPool[i])[j];
          // calculate lower and upper bounds for this variable
          mn = *OptItem.getLowerBoundValue();
          mx = *OptItem.getUpperBoundValue();

          try
            {
              // calculate orders of magnitude of the interval
              la = log10(mx) - log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min()));

              // determine if linear or log scale
              if ((mn < 0.0) || (mx <= 0.0))
                Sol = mn + (mpRandom->getRandomCC() + (C_FLOAT64) i) * (mx - mn) * 0.25;
              else
                {
                  if (la < 1.8)
                    Sol = mn + (mpRandom->getRandomCC() + (C_FLOAT64) i) * (mx - mn) * 0.25;
                  else
                    Sol = pow(10.0, log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min()))
                              + la * 0.25 * (mpRandom->getRandomCC() + (C_FLOAT64) i));
                }
            }
          catch (...)
            {
              Sol = (mx + mn) * 0.5;
            }

          // force it to be within the bounds
          switch (OptItem.checkConstraint(Sol))
            {
              case - 1:
                Sol = *OptItem.getLowerBoundValue();
                break;

              case 1:
                Sol = *OptItem.getUpperBoundValue();
                break;
            }

          // We need to set the value here so that further checks take
          // account of the value.
          (*(*mpSetCalculateVariable)[j])(Sol);
        }

      // calculate its fitness
      Running &= evaluate(*mPool[i]);
      mPoolVal[i] = mEvaluationValue;
    }

  // next we add the initial guess from the user
  for (j = 0; j < mVariableSize; j++)
    {
      COptItem & OptItem = *(*mpOptItem)[j];
      C_FLOAT64 & Sol = (*mPool[i])[j];

      // get the vector of initial value
      Sol = OptItem.getStartValue();

      // force it to be within the bounds
      switch (OptItem.checkConstraint(Sol))
        {
          case - 1:
            Sol = *OptItem.getLowerBoundValue();
            break;

          case 1:
            Sol = *OptItem.getUpperBoundValue();
            break;
        }

      // We need to set the value here so that further checks take
      // account of the value.
      (*(*mpSetCalculateVariable)[j])(Sol);
    }

  // calculate its fitness
  Running &= evaluate(*mPool[i]);
  mPoolVal[i] = mEvaluationValue;
  i++;

  // the remaining entries depend on probabilities
  for (; (i < mPoolSize) && Running; i++)
    {
      for (j = 0; j < mVariableSize; j++)
        {
          // get pointers to appropriate elements (easier reading of code)
          COptItem & OptItem = *(*mpOptItem)[j];
          C_FLOAT64 & Sol = (*mPool[i])[j];
          // calculate lower and upper bounds for this variable
          mn = *OptItem.getLowerBoundValue();
          mx = *OptItem.getUpperBoundValue();

          for (k = 0; k < 4; k++)
            {
              for (l = 0, sum = 0.0 ; l < 4; l++)
                sum += 1.0 / (*mFreq[j])[l];

              mProb[k] = 1.0 / (*mFreq[j])[k] / sum;

              // we only want the cumulative probabilities
              if (k > 0) mProb[k] += mProb[k - 1];
            }

          a = mpRandom->getRandomCC();

          for (k = 0; k < 4; k++)
            {
              // note that the original is <= but numerically < is essentially the same
              if (a < mProb[k])
                {
                  try
                    {
                      // calculate orders of magnitude of the interval
                      la = log10(mx) - log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min()));

                      // determine if linear or log scale
                      if ((mn < 0.0) || (mx <= 0.0))
                        Sol = mn + (mpRandom->getRandomCC() + (C_FLOAT64) k) * (mx - mn) * 0.25;
                      else
                        {
                          if (la < 1.8)
                            Sol = mn + (mpRandom->getRandomCC() + (C_FLOAT64) k) * (mx - mn) * 0.25;
                          else
                            Sol = pow(10.0, log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min()))
                                      + la * 0.25 * (mpRandom->getRandomCC() + (C_FLOAT64) k));
                        }
                    }
                  catch (...)
                    {
                      Sol = (mx + mn) * 0.5;
                    }

                  // force it to be within the bounds
                  switch (OptItem.checkConstraint(Sol))
                    {
                      case - 1:
                        Sol = *OptItem.getLowerBoundValue();
                        break;

                      case 1:
                        Sol = *OptItem.getUpperBoundValue();
                        break;
                    }

                  // We need to set the value here so that further checks take
                  // account of the value.
                  (*(*mpSetCalculateVariable)[j])(Sol);
                  // increase the frequency
                  (*mFreq[j])[k] += 1;
                  break;
                }
            }
        }

      // calculate its fitness
      Running &= evaluate(*mPool[i]);
      mPoolVal[i] = mEvaluationValue;
    }

  // at this point the pool is formed
  // now we partially sort the pool for the h = b/2 top elements,
  // where b is mPopulationSize, This is done in place with heap sort
  CVector< C_FLOAT64 > *tempvec;
  C_FLOAT64 tempval;
  C_INT32 parent, child;
  C_INT32 h = mPopulationSize / 2;

  // top h are the heap, we first have to sort it
  for (i = 1; i < h; i++)
    {
      // bubble-up
      child = i;

      for (;;)
        {
          if (child == 0) break;

          parent = floor((double)(child - 1) / 2);

          if (mPoolVal[child] < mPoolVal[parent])
            {
              // swap with parent
              tempval = mPoolVal[child];
              mPoolVal[child] = mPoolVal[parent];
              mPoolVal[parent] = tempval;
              tempvec = mPool[child];
              mPool[child] = mPool[parent];
              mPool[parent] = tempvec;
              // make parent the new child
              child = parent;
            }
          else break;
        }
    }

  for (i = h; i < mPoolSize; i++)
    {
      child = 0;

      // check if this element is smaller than any of the leafs
      for (size_t leaf = h / 2; leaf < h; leaf++)
        {
          if (mPoolVal[i] < mPoolVal[leaf])
            {
              // keep if this leaf is worse than previous
              if (mPoolVal[child] < mPoolVal[leaf])
                child = leaf;
            }
        }

      if (child == 0) continue;

      if (mPoolVal[i] < mPoolVal[child])
        {
          // swap i and h+j
          tempval = mPoolVal[child];
          mPoolVal[child] = mPoolVal[i];
          mPoolVal[i] = tempval;
          tempvec = mPool[child];
          mPool[child] = mPool[i];
          mPool[i] = tempvec;

          // now bubble-up
          for (;;)
            {
              if (child == 0) break;

              parent = floor((double)(child - 1) / 2);

              if (mPoolVal[child] < mPoolVal[parent])
                {
                  // swap with parent
                  tempval = mPoolVal[child];
                  mPoolVal[child] = mPoolVal[parent];
                  mPoolVal[parent] = tempval;
                  tempvec = mPool[child];
                  mPool[child] = mPool[parent];
                  mPool[parent] = tempvec;
                  // make parent the new child
                  child = parent;
                }
              else break;
            }
        }
    }

  // since some leafs are not in order in the array, we now do
  // a bubble sort (note: this is best case for bubble sort)
  // we use j because we do not want to change the value of h
  j = h;

  do
    {
      k = 0;

      for (i = 0; i <= j; i++)
        {
          if (mPoolVal[i] > mPoolVal[i + 1])
            {
              tempval = mPoolVal[i];
              mPoolVal[i] = mPoolVal[i + 1];
              mPoolVal[i + 1] = tempval;
              tempvec = mPool[i];
              mPool[i] = mPool[i + 1];
              mPool[i + 1] = tempvec;
              k = i;
            }
        }

      j = k;
    }
  while (j > 1);

  // at this point the pool is formed and partially sorted
  // now we build the RefSet:
  // 1st b/2 are the best ones in the Pool (sorted already)
  // the 2nd b/2 are random (or later can be made diverse by
  // maximising the Euclidean distances)
  for (i = 0; i < mPopulationSize; i++)
    {
      (*mRefSet[i]) = (*mPool[i]); // copy the whole vector
      mRefSetVal[i] = mPoolVal[i]; // keep the value
      mStuck[i] = 1;               // initialize the mStuck counter
    }

  // RefSet needs to be fully sorted. Note that top half is sorted
  // and we are garanteed that it contains the best values
  // so it is only bottom half that needs sorting.
  sortRefSet(h, mPopulationSize);
  // we're done
  return Running;
}

// sort the RefSet and associated RefSetVal & Stuck
// between positions lower and upper.
void COptMethodSS::sortRefSet(C_INT32 lower, C_INT32 upper)
{
  C_INT32 i, j, k;
  C_INT32 parent, child;
  CVector< C_FLOAT64 > *tempvec;
  C_FLOAT64 tempval;

  // Use heap sort
  for (i = lower + 1; i < upper; i++)
    {
      // bubble-up
      child = i;

      for (;;)
        {
          if (child == 0) break;

          parent = floor((double)(child - 1) / 2);

          if (mRefSetVal[child] < mRefSetVal[parent])
            {
              // swap with parent
              tempval = mRefSetVal[child];
              mRefSetVal[child] = mRefSetVal[parent];
              mRefSetVal[parent] = tempval;
              tempval = mStuck[child];
              mStuck[child] = mStuck[parent];
              mStuck[parent] = tempval;
              tempvec = mRefSet[child];
              mRefSet[child] = mRefSet[parent];
              mRefSet[parent] = tempvec;
              // make parent the new child
              child = parent;
            }
          else break;
        }
    }

  // since some leafs are not in order in the array, we now do
  // a bubble sort (note: this is best case for bubble sort)
  j = upper - 1;

  do
    {
      k = lower;

      for (i = lower; i < j; i++)
        {
          if (mRefSetVal[i] > mRefSetVal[i + 1])
            {
              tempval = mRefSetVal[i];
              mRefSetVal[i] = mRefSetVal[i + 1];
              mRefSetVal[i + 1] = tempval;
              tempval = mStuck[i];
              mStuck[i] = mStuck[i + 1];
              mStuck[i + 1] = tempval;
              tempvec = mRefSet[i];
              mRefSet[i] = mRefSet[i + 1];
              mRefSet[i + 1] = tempvec;
              k = i;
            }
        }

      j = k;
    }
  while (j > lower);

#ifdef EXCLUDE
  j = upper - 1; // becasue we look ahead...

  do
    {
      k = lower;

      for (i = lower; i < j; i++)
        {
          if (mRefSetVal[i] > mRefSetVal[i + 1])
            {
              // swap the value
              tempval = mRefSetVal[i];
              mRefSetVal[i] = mRefSetVal[i + 1];
              mRefSetVal[i + 1] = tempval;
              // swap the vector (only the pointers!)
              tempvec = mRefSet[i];
              mRefSet[i] = mRefSet[i + 1];
              mRefSet[i + 1] = tempvec;
              k = i;
            }
        }

      j = k;
    }
  while (j > lower);

#endif
#ifdef DEBUG_OPT
  serializerefset(0, mPopulationSize);
#endif
}

// check if all the indexes of a Child member are closer to
// the indexes of a Pool member than a certain relative distance
bool COptMethodSS::closerChild(C_INT32 i, C_INT32 j, C_FLOAT64 dist)
{
  C_FLOAT64 mx;

  for (C_INT32 k = 0; k < mVariableSize; k++)
    {
      if (fabs((*mChild[i])[k]) > fabs((*mPool[j])[k]))
        mx = fabs((*mChild[i])[k]);
      else
        mx = fabs((*mPool[j])[k]);

      if (fabs((*mChild[i])[k] - (*mPool[j])[k]) / mx > dist) return false;
    }

  return true;
}

// check if all the indexes of two refset members
// are closer than a certain relative distance
bool COptMethodSS::closerRefSet(C_INT32 i, C_INT32 j, C_FLOAT64 dist)
{
  C_FLOAT64 mx;

  for (C_INT32 k = 0; k < mVariableSize; k++)
    {
      if (fabs((*mRefSet[i])[k]) > fabs((*mRefSet[j])[k]))
        mx = fabs((*mRefSet[i])[k]);
      else
        mx = fabs((*mRefSet[j])[k]);

      if (fabs((*mRefSet[i])[k] - (*mRefSet[j])[k]) / mx > dist) return false;
    }

  return true;
}

// measure the distance between two members of the refset
C_FLOAT64 COptMethodSS::distRefSet(C_INT32 i, C_INT32 j)
{
  C_FLOAT64 dist;
  dist = 0.0;

  for (C_INT32 k = 0; k < mVariableSize; k++)
    dist += 2.0 * fabs((*mRefSet[i])[k] - (*mRefSet[j])[k]) / (fabs((*mRefSet[i])[k]) + fabs((*mRefSet[j])[k]));

  dist /= mVariableSize;
  return dist;
}

// measure the distance between two members of the Child set
C_FLOAT64 COptMethodSS::distChild(C_INT32 i, C_INT32 j)
{
  C_FLOAT64 dist;
  dist = 0.0;

  for (C_INT32 k = 0; k < mVariableSize; k++)
    dist += 2.0 * fabs((*mChild[i])[k] - (*mChild[j])[k]) / (fabs((*mChild[i])[k]) + fabs((*mChild[j])[k]));

  dist /= mVariableSize;
  return dist;
}

// combine individuals in the RefSet two by two
// this is a sort of (1+1)-ES strategy
bool COptMethodSS::combination(void)
{
  C_INT32 i, j, k, l;    // counters
  C_FLOAT64 mn, mx;     // for bounds on parameters
  C_FLOAT64 beta;       // bias
  C_FLOAT64 la;         // for orders of magnitude
  C_FLOAT64 alpha;      // 1 or -1
  C_FLOAT64 bm2;        // b-2
  C_FLOAT64 omatb;      // (1-alpha*beta)*0.5
  C_FLOAT64 dd;         // (x(i) - x(j) ) / 2 * (1-alpha*beta)
  C_FLOAT64 c1, c2;     // coordinates of the edges of hyperectangle
  CVector< C_FLOAT64 > xnew, xpr;
  C_FLOAT64 xnewval, xprval; // to hold temp value of "parent" in go-beyond strategy
  C_FLOAT64 lambda = 1.0; // damping factor for go-beyond strategy
  C_INT32 improvement; // count iterations with improvement in go-beyond strategy
  bool Running = true; // flag for invalid values

  // make xnew large enough
  xnew.resize(mVariableSize);
  xpr.resize(mVariableSize);
  // calculate a constant
  bm2 = C_FLOAT64(mPopulationSize) - 2.0;
  // signal no children yet
  mChildrenGenerated = false;

  // generate children for each member of the population
  for (i = 0; (i < mPopulationSize) && Running; i++)
    {
      for (j = 0; j < mPopulationSize; j++)
        {
          // no self-reproduction...
          if (i != j)
            {
              if (i < j) alpha = 1.0; else alpha = -1.0;

              beta = (fabs(C_FLOAT64(j) - C_FLOAT64(i)) - 1.0) / bm2;
              omatb = (1.0 + alpha * beta) * 0.5;

              // generate a child
              for (k = 0; k < mVariableSize; k++)
                {
                  // get the bounds of this parameter
                  COptItem & OptItem = *(*mpOptItem)[k];
                  mn = *OptItem.getLowerBoundValue();
                  mx = *OptItem.getUpperBoundValue();

                  try
                    {
                      // calculate orders of magnitude of the interval
                      if (((*mRefSet[i])[k] > 0.0) && ((*mRefSet[j])[k] > 0.0))
                        {
                          la = log10((*mRefSet[i])[k]) - log10(std::max((*mRefSet[j])[k], std::numeric_limits< C_FLOAT64 >::min()));
                        }
                      else
                        la = 1.0;

                      if ((la > -1.8) && (la < 1.8))
//                if(true)
                        {
                          dd = ((*mRefSet[i])[k] - (*mRefSet[j])[k]) * omatb;
                          // one of the box limits
                          c1 = (*mRefSet[i])[k] - dd;

                          // force it to be within the bounds
                          switch (OptItem.checkConstraint(c1))
                            {
                              case -1:
                                c1 = mn;
                                break;

                              case 1:
                                c1 = mx;
                                break;
                            }

                          // the other box limit
                          c2 = (*mRefSet[i])[k] + dd;

                          // force it to be within the bounds
                          switch (OptItem.checkConstraint(c2))
                            {
                              case -1:
                                c2 = mn;
                                break;

                              case 1:
                                c2 = mx;
                                break;
                            }

                          xnew[k] = c1 + (c2 - c1) * mpRandom->getRandomCC();
                        }
                      else
                        {
                          dd = la * omatb;
                          // one of the box limits
                          c1 = pow(10.0, log10(std::max((*mRefSet[i])[k], std::numeric_limits< C_FLOAT64 >::min())) - dd);
#ifdef DEBUG_OPT

                          if (isnan(c1))
                            {
                              inforefset(7, j);
                            }

#endif

                          // force it to be within the bounds
                          switch (OptItem.checkConstraint(c1))
                            {
                              case -1:
                                c1 = mn;
                                break;

                              case 1:
                                c1 = mx;
                                break;
                            }

                          // the other box limit
                          c2 = pow(10.0, log10(std::max((*mRefSet[i])[k], std::numeric_limits< C_FLOAT64 >::min())) + dd);
#ifdef DEBUG_OPT

                          if (isnan(c2))
                            {
                              inforefset(8, j);
                            }

#endif

                          // force it to be within the bounds
                          switch (OptItem.checkConstraint(c2))
                            {
                              case -1:
                                c2 = mn;
                                break;

                              case 1:
                                c2 = mx;
                                break;
                            }

                          if (dd > 0.0)
                            {
                              la = log10(c2) - log10(c1);
                              la *= mpRandom->getRandomCC();
                              xnew[k] = pow(10.0, log10(c1 + la));
                            }
                          else
                            {
                              la = log10(c1) - log10(c2);
                              la *= mpRandom->getRandomCC();
                              xnew[k] = pow(10.0, log10(c2 + la));
                            }

#ifdef DEBUG_OPT

                          if (isnan(xnew[k]))
                            {
                              inforefset(9, j);
                            }

#endif
                        }
                    }
                  catch (...)
                    {
                      // if something failed leave the value intact
                      xnew[k] = (*mRefSet[i])[k];
                    }

                  // We need to set the value here so that further checks take
                  // account of the value.
                  (*(*mpSetCalculateVariable)[k])(xnew[k]);
                }

              // calculate the child's fitness
              Running &= evaluate(xnew);

              // keep it if it is better than the previous one
              if (mEvaluationValue < mChildVal[i])
                {
                  // keep a copy of this vector in mChild
                  (*mChild[i]) = xnew;
                  // and the fitness value
                  mChildVal[i] = mEvaluationValue;
                  // signal that child is better than parent
                  mStuck[i] = 0;
                  // signal we have generated a child (improvement)
                  mChildrenGenerated = true;
#ifdef DEBUG_OPT
                  inforefset(1, i);
#endif
                }
            }
        }

      // now we apply the go-beyond strategy for
      // cases where the child was an improvement
      if (mStuck[i] == 0)
        {
          // copy the parent
          xpr = (*mRefSet[i]);
          xprval = mRefSetVal[i];
          lambda = 1.0; // this is really 1/lambda so we can use mult rather than div
          improvement = 1;

          // while newval < childval
          for (; ;)
            {
              for (k = 0; k < mVariableSize; k++)
                {
                  dd = (xpr[i] - (*mChild[i])[k]) * lambda;
                  xnew[k] = (*mChild[i])[k] + dd * mpRandom->getRandomCC();
                  // get the bounds of this parameter
                  COptItem & OptItem = *(*mpOptItem)[k];

                  // put it on the bounds if it had exceeded them
                  switch (OptItem.checkConstraint(xnew[k]))
                    {
                      case -1:
                        xnew[k] = *OptItem.getLowerBoundValue();
                        break;

                      case 1:
                        xnew[k] = *OptItem.getUpperBoundValue();
                        break;
                    }

                  // We need to set the value here so that further checks take
                  // account of the value.
                  (*(*mpSetCalculateVariable)[k])(xnew[k]);
                }

              // calculate the child's fitness
              Running &= evaluate(xnew);
              xnewval = mEvaluationValue;

              // if there was no improvement we finish here => exit for(;;)
              if (mChildVal[i] <= xnewval) break;

#ifdef DEBUG_OPT
              inforefset(2, i);
#endif
              // old child becomes parent
              xpr = (*mChild[i]);
              xprval = mChildVal[i];
              // new child becomes child
              (*mChild[i]) = xnew;
              mChildVal[i] = xnewval;
              // mark improvement
              improvement++;

              if (improvement == 2)
                {
                  lambda *= 0.5;
                  improvement = 0;
                }
            }
        }
    }

  return Running;
}

bool COptMethodSS::childLocalMin(void)
{
  C_INT32 i, best;
  C_FLOAT64 bestVal, fvalmin;
  bool Running = true;

  // signal nothing found yet
  best = -1;
  bestVal = std::numeric_limits<C_FLOAT64>::infinity();

  // find the best child
  for (i = 0; i < mPopulationSize; i++)
    {
      if ((mStuck[i] == 0) && (mChildVal[i] < bestVal))
        {
          bestVal = mChildVal[i];
          best = i;
        }
    }

  // no child in this interation? exit now
  if (best == -1) return true;

  // check if this child is not close to previous ones
  for (i = 0; i < mLocalStored; i++)
    {
      // is the other one like me?
      if (closerChild(best, i, 1e-3))
        //if (distChild(best, i) < 1e-3)
        {
#ifdef DEBUG_OPT
          inforefset(6, best);
#endif
          // it is too close, exit now
          return true;
        }
    }

  // store the initial position
  *(mPool[mLocalStored]) = *(mChild[best]);
  mPoolVal[mLocalStored] = mChildVal[best]; //bestVal;
  mLocalStored++;

  // do local minimization on it
  Running &= localmin(*(mChild[best]), mChildVal[best]);

  // store the result
  *(mPool[mLocalStored]) = *(mChild[best]);
  mPoolVal[mLocalStored] = mChildVal[best];
  mLocalStored++;

#ifdef DEBUG_OPT
  serializepool(0, mLocalStored);
#endif

  return Running;
}

bool COptMethodSS::optimise()
{
  bool Running = true;
  bool needsort;
  size_t i, j, nlocal;
  C_FLOAT64 mx, mn, la;

  mIteration = 0;

  if (!initialize())
    {
      // initialisation failed, we exit
      if (mpCallBack)
        mpCallBack->finishItem(mhIterations);

      return false;
    }

  // create the Pool of diverse candidate solutions
  Running &= creation();

  // best value is (always) at position zero
  // store that value
  mBestValue = mRefSetVal[0];
  // set it upstream
  Running &= mpOptProblem->setSolution(mBestValue, *mRefSet[0]);
  // We found a new best value let's report it.
  mpParentTask->output(COutputInterface::DURING);

  // test if the user wants to stop, and do so if needed
  if (!Running)
    {
      if (mpCallBack)
        mpCallBack->finishItem(mhIterations);

      cleanup();
      return true;
    }

  // mPool is now going to be used to keep track of initial and final
  // points of local minimizations (to avoid running them more than once)
  mPoolSize = 2 * mIterations / mLocalFreq;
  // reset the number of stored minimizations
  mLocalStored = 0;
  // reset the counter for local minimisation
  nlocal = 1;

  // run the mIterations (and count the creation as being the first)
  for (mIteration = 1; mIteration < mIterations && Running; mIteration++)
    {
      // check for stagnation or similarity
      needsort = false;

      for (i = 0; i < mPopulationSize; i++)
        {
          // are we stuck? (20 iterations)
          if (mStuck[i] == 19)
            {
              // substitute this one by a random guess
              Running &= randomize(i);
              needsort = true;
              mStuck[i] = 1;
#ifdef DEBUG_OPT
              inforefset(5, i);
#endif
            }
          else
            {
              // check if another RefSet member is similar to us (relative dist 0.1%)
              for (j = i + 1; j < mPopulationSize; j++)
                {
                  // is the other one like me?
                  if (closerRefSet(i, j, 1e-3))
                    //if (distRefSet(i, j) < 0.01)
                    {
                      // randomize the other one (I am more important)
                      Running &= randomize(j);
                      needsort = true;
                      mStuck[j] = 1;
#ifdef DEBUG_OPT
                      inforefset(4, j);
#endif
                    }
                }
            }
        }

      // sort the RefSet if needed
      if (needsort) sortRefSet(0, mPopulationSize);

      // create children by combination
      Running &= combination();

      // check if we have to run a local search
      if (nlocal >= mLocalFreq && mChildrenGenerated)
        {
          // reset the local counter
          nlocal = 1;
          // carry out a local search
          Running &= childLocalMin();
        }
      else
        {
          // count this
          nlocal++;
        }

      // substitute the parents for children or increment stuck counter
      needsort = false;

      for (i = 0; i < mPopulationSize; i++)
        {
          // check if child was better than parent
          if (mStuck[i] == 0)
            {
              // copy the child into the population
              (*mRefSet[i]) = (*mChild[i]);
              // keep its obj funct value
              mRefSetVal[i] = mChildVal[i];
              // and reset the stuck counter
              mStuck[i] = 1;
              needsort = true;
            }
          else
            {
              // nothing to do, increment parent's stuck counters
              mStuck[i]++;
            }
        }

      // sort the RefSet if needed
      if (needsort) sortRefSet(0, mPopulationSize);

#ifdef DEBUG_OPT

      if (!mChildrenGenerated)
        {
          // signal that nothing happened in this iteration
          inforefset(3, mIteration);
        }

#endif

      // have we made any progress?
      if (mRefSetVal[0] < mBestValue)
        {
          // and store that value
          mBestValue = mRefSetVal[0];
          Running &= mpOptProblem->setSolution(mBestValue, *mRefSet[0]);
          // We found a new best value lets report it.
          mpParentTask->output(COutputInterface::DURING);
        }

      if (mpCallBack)
        Running &= mpCallBack->progressItem(mhIterations);
    }

  // end of loop for iterations

  // the best ever might not be what is on position 0, so bring it back
  *mRefSet[0] = mpOptProblem->getSolutionVariables();
  // now let's do a final local minimisation
  Running &= localmin(*(mRefSet[0]), mRefSetVal[0]);

  // has it improved?
  if (mRefSetVal[0] < mBestValue)
    {
      // and store that value
      mBestValue = mRefSetVal[0];
      Running &= mpOptProblem->setSolution(mBestValue, *mRefSet[0]);
      // We found a new best value lets report it.
      mpParentTask->output(COutputInterface::DURING);
    }

  if (mpCallBack)
    mpCallBack->finishItem(mhIterations);

  cleanup();
  return true;
}

#ifdef DEBUG_OPT
bool COptMethodSS::serializeprob(void)
{
  std::ofstream ofile;
  // open the file for output, in append mode
  ofile.open("ssprob.txt", std::ios::out);  // | std::ios::app );

  if (! ofile.is_open())
    {
      std::cerr << "error opening file \'ssprob.txt\'" << std::endl;
      return false;
    }

  ofile << mProb << std::endl;
  ofile.close();
  return true;
}
// serialize the pool to a file for debugging purposes
bool COptMethodSS::serializepool(size_t first, size_t last)
{
  size_t Last = std::min(last, (size_t) mPoolSize);

  size_t i;
  size_t j;

  std::ofstream ofile;

  // open the file for output, in append mode
  ofile.open("sspool.txt", std::ios::out | std::ios::app);

  if (! ofile.is_open())
    {
      std::cerr << "error opening file \'sspool.txt\'" << std::endl;
      return false;
    }

  for (i = first; i < Last; i++)
    {
      for (j = 0; j < mVariableSize; j++)
        {
          C_FLOAT64 & mut = (*mPool[i])[j];
          // print this parameter
          ofile << mut << "\t";
        }

      // print the fitness of the individual
      ofile << mPoolVal[i] << std::endl;
    }

  ofile << std::endl;
  ofile.close();
  return true;
}
// write informative messages about the progress of the refset
bool COptMethodSS::inforefset(C_INT32 type, C_INT32 element)
{
  std::ofstream ofile;

  // open the file for output, in append mode
  ofile.open("ssrefset.txt", std::ios::out | std::ios::app);

  if (! ofile.is_open())
    {
      std::cerr << "error opening file \'ssrefset.txt\'" << std::endl;
      return false;
    }

  switch (type)
    {
      case 1: ofile << "element " << element << " improved in combination" << std::endl; break;

      case 2: ofile << "element " << element << " improved in go-beyond" << std::endl; break;

      case 3: ofile << "No element improved in iteration " << element << std::endl ; break;

      case 4: ofile << "element " << element << " randomized, too close to another" << std::endl; break;

      case 5: ofile << "element " << element << " randomized, was stuck" << std::endl; break;

      case 6: ofile << "child" << element << " too close to previous solution, no local min" << std::endl; break;

      case 7: ofile << "c1 is NaN (element" << element << ")" << std::endl; break;

      case 8: ofile << "c2 is NaN (element" << element << ")" << std::endl; break;

      case 9: ofile << "xnew[k] is NaN (element" << element << ")" << std::endl; break;
    }

  ofile.close();
  return true;
}

// serialize the population to a file for debugging purposes
bool COptMethodSS::serializerefset(C_INT32 first, C_INT32 last)
{
  C_INT32 Last = std::min(last, mPopulationSize);
  C_INT32 i, j;
  std::ofstream ofile;

  // open the file for output, in append mode
  ofile.open("ssrefset.txt", std::ios::out | std::ios::app);

  if (! ofile.is_open())
    {
      std::cerr << "error opening file \'ssrefset.txt\'" << std::endl;
      return false;
    }

  for (i = first; i < Last; i++)
    {
      for (j = 0; j < mVariableSize; j++)
        {
          C_FLOAT64 & mut = (*mRefSet[i])[j];
          // print this parameter
          ofile << mut << "\t";
        }

      // print the fitness of the individual
      ofile << mRefSetVal[i] << std::endl;
    }

  ofile << std::endl;
  ofile.close();
  return true;
}
// serialize the population to a file for debugging purposes
bool COptMethodSS::serializechildren(size_t first, size_t last)
{
  size_t Last = std::min(last, (size_t) mPopulationSize);

  size_t i;
  size_t j;

  std::ofstream ofile;

  // open the file for output, in append mode
  ofile.open("sschild.txt", std::ios::out | std::ios::app);

  if (! ofile.is_open())
    {
      std::cerr << "error opening file \'sschild.txt\'" << std::endl;
      return false;
    }

  for (i = first; i < Last; i++)
    {
      for (j = 0; j < mVariableSize; j++)
        {
          C_FLOAT64 & mut = (*mChild[i])[j];
          // print this parameter
          ofile << mut << "\t";
        }

      // print the fitness of the individual
      ofile << mChildVal[i] << std::endl;
    }

  ofile << std::endl;
  ofile.close();
  return true;
}
#endif
