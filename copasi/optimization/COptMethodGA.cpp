// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2004 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <cmath>
#include <limits>
#include <string>
#ifdef DEBUG_OPT
# include <iostream>
# include <fstream>
#endif

#include "copasi.h"

#include "COptMethodGA.h"
#include "COptProblem.h"
#include "COptItem.h"
#include "COptTask.h"

#include "randomGenerator/CRandom.h"
#include "randomGenerator/CPermutation.h"
#include "utilities/CProcessReport.h"
#include "utilities/CSort.h"
#include "report/CCopasiObjectReference.h"

COptMethodGA::COptMethodGA(const CCopasiContainer * pParent,
                           const CTaskEnum::Method & methodType,
                           const CTaskEnum::Task & taskType):
  COptMethod(pParent, methodType, taskType),
  mGenerations(0),
  mPopulationSize(0),
  mpRandom(NULL),
  mVariableSize(0),
  mIndividual(0),
  mCrossOverFalse(0),
  mCrossOver(0),
  mEvaluationValue(std::numeric_limits< C_FLOAT64 >::max()),
  mValue(0),
  mpPermutation(NULL),
  mLosses(0),
  mPivot(0),
  mMutationVarians(0.1),
  mBestValue(std::numeric_limits< C_FLOAT64 >::max()),
  mBestIndex(C_INVALID_INDEX),
  mGeneration(0)

{
  addParameter("Number of Generations", CCopasiParameter::UINT, (unsigned C_INT32) 200);
  addParameter("Population Size", CCopasiParameter::UINT, (unsigned C_INT32) 20);
  addParameter("Random Number Generator", CCopasiParameter::UINT, (unsigned C_INT32) CRandom::mt19937);
  addParameter("Seed", CCopasiParameter::UINT, (unsigned C_INT32) 0);
  addParameter("#LogVerbosity", CCopasiParameter::UINT, (unsigned C_INT32) 0);

  initObjects();
}

COptMethodGA::COptMethodGA(const COptMethodGA & src,
                           const CCopasiContainer * pParent):
  COptMethod(src, pParent),
  mGenerations(0),
  mPopulationSize(0),
  mpRandom(NULL),
  mVariableSize(0),
  mIndividual(0),
  mCrossOverFalse(0),
  mCrossOver(0),
  mEvaluationValue(std::numeric_limits< C_FLOAT64 >::max()),
  mValue(0),
  mpPermutation(NULL),
  mLosses(0),
  mPivot(0),
  mMutationVarians(0.1),
  mBestValue(std::numeric_limits< C_FLOAT64 >::max()),
  mBestIndex(C_INVALID_INDEX),
  mGeneration(0)
{initObjects();}

COptMethodGA::~COptMethodGA()
{cleanup();}

// evaluate the fitness of one individual
bool COptMethodGA::evaluate(const CVector< C_FLOAT64 > & /* individual */)
{
  bool Continue = true;

  // We do not need to check whether the parametric constraints are fulfilled
  // since the parameters are created within the bounds.

  // evaluate the fitness
  Continue &= mpOptProblem->calculate();

  // check whether the functional constraints are fulfilled
  if (!mpOptProblem->checkFunctionalConstraints())
    mEvaluationValue = std::numeric_limits<C_FLOAT64>::infinity();
  else
    mEvaluationValue = mpOptProblem->getCalculateValue();

  return Continue;
}

bool COptMethodGA::swap(size_t from, size_t to)
{
  CVector< C_FLOAT64 > * pTmp = mIndividual[to];
  mIndividual[to] = mIndividual[from];
  mIndividual[from] = pTmp;

  C_FLOAT64 dTmp = mValue[to];
  mValue[to] = mValue[from];
  mValue[from] = dTmp;

  size_t iTmp = mLosses[to];
  mLosses[to] = mLosses[from];
  mLosses[from] = iTmp;

  return true;
}

//mutate one individual
bool COptMethodGA::mutate(CVector< C_FLOAT64 > & individual)
{
  size_t j;

  // mutate the parameters
  for (j = 0; j < mVariableSize; j++)
    {
      COptItem & OptItem = *(*mpOptItem)[j];
      C_FLOAT64 & mut = individual[j];

      // calculate the mutatated parameter
      mut *= mpRandom->getRandomNormal(1, mMutationVarians);

      // force it to be within the bounds
      switch (OptItem.checkConstraint(mut))
        {
          case - 1:
            mut = *OptItem.getLowerBoundValue();
            break;

          case 1:
            mut = *OptItem.getUpperBoundValue();
            break;
        }

      // We need to set the value here so that further checks take
      // account of the value.
      *mContainerVariables[j] = mut;
    }

  return true;
}

bool COptMethodGA::crossover(const CVector< C_FLOAT64 > & parent1,
                             const CVector< C_FLOAT64 > & parent2,
                             CVector< C_FLOAT64 > & child1,
                             CVector< C_FLOAT64 > & child2)
{
  size_t i, crp;
  size_t nCross = 0;

  mCrossOver = mCrossOverFalse;

  if (mVariableSize > 1)
    nCross = mpRandom->getRandomU((unsigned C_INT32)(mVariableSize / 2));

  if (nCross == 0)
    {
      // if less than 0 just copy parent to child
      child1 = parent1;
      child2 = parent2;

      return true;
    }

  // choose cross over points;
  // We do not mind if a crossover point gets drawn twice
  for (i = 0; i < nCross; i++)
    {
      crp = mpRandom->getRandomU((unsigned C_INT32)(mVariableSize - 1));
      mCrossOver[crp] = true;
    }

  const CVector< C_FLOAT64 > * pParent1 = & parent1;

  const CVector< C_FLOAT64 > * pParent2 = & parent2;

  const CVector< C_FLOAT64 > * pTmp;

  for (i = 0; i < mVariableSize; i++)
    {
      if (mCrossOver[i])
        {
          pTmp = pParent1;
          pParent1 = pParent2;
          pParent2 = pTmp;
        }

      child1[i] = (*pParent1)[i];
      child2[i] = (*pParent2)[i];
    }

  return true;
}

bool COptMethodGA::replicate()
{
  size_t i;
  bool Continue = true;

  // generate a random order for the parents
  mpPermutation->shuffle();

  // reproduce in consecutive pairs
  for (i = 0; i < mPopulationSize / 2; i++)
    crossover(*mIndividual[mpPermutation->next()],
              *mIndividual[mpPermutation->next()],
              *mIndividual[mPopulationSize + i * 2],
              *mIndividual[mPopulationSize + i * 2 + 1]);

  // check if there is one left over and just copy it
  if (mPopulationSize % 2 > 0)
    *mIndividual[2 * mPopulationSize - 1] = *mIndividual[mPopulationSize - 1];

  // mutate the offspring
  for (i = mPopulationSize; i < 2 * mPopulationSize && Continue; i++)
    {
      mutate(*mIndividual[i]);
      Continue &= evaluate(*mIndividual[i]);
      mValue[i] = mEvaluationValue;
    }

  return Continue;
}

// select mPopulationSize individuals
bool COptMethodGA::select()
{
  size_t i, j, nopp, opp;
  size_t TotalPopulation = 2 * mPopulationSize;

  // tournament competition
  mLosses = 0; // Set all wins to 0.

  // compete with ~ 20% of the TotalPopulation
  nopp = std::max<size_t>(1, mPopulationSize / 5);

  // parents and offspring are all in competition
  for (i = 0; i < TotalPopulation; i++)
    for (j = 0; j < nopp; j++)
      {
        // get random opponent
        do
          {
            opp = mpRandom->getRandomU((unsigned C_INT32)(TotalPopulation - 1));
          }
        while (i == opp);

        if (mValue[i] < mValue[opp])
          mLosses[opp]++;
        else
          mLosses[i]++;
      }

  // selection of top mPopulationSize winners
  partialSortWithPivot(mLosses.array(),
                       mLosses.array() + mPopulationSize,
                       mLosses.array() + TotalPopulation,
                       mPivot);

  FSwapClass<COptMethodGA, size_t, bool> Swap(this, &COptMethodGA::swap);
  applyPartialPivot(mPivot, mPopulationSize, Swap);

  return true;
}

// check the best individual at this generation
size_t COptMethodGA::fittest()
{
  size_t i, BestIndex = C_INVALID_INDEX;
  C_FLOAT64 BestValue = std::numeric_limits< C_FLOAT64 >::max();

  for (i = 0; i < mPopulationSize && !mLosses[i]; i++)
    if (mValue[i] < BestValue)
      {
        BestIndex = i;
        BestValue = mValue[i];
      }

  return BestIndex;
}

#ifdef DEBUG_OPT
// serialize the population to a file for debugging purposes
bool COptMethodGA::serializepop(size_t first, size_t last)
{
  size_t Last = std::min(last, (size_t) mPopulationSize);

  size_t i;
  size_t j;

  std::ofstream ofile;
//std::ifstream test("gapop.txt");
//if(!test.good() ) return false;

  // open the file for output, in append mode
  ofile.open("gapop.txt", std::ios::out | std::ios::app);

  if (! ofile.is_open())
    {
      std::cerr << "error opening file \'gapop.txt\'" << std::endl;
      return false;
    }

  for (i = first; i < Last; i++)
    {
      for (j = 0; j < mVariableSize; j++)
        {
          C_FLOAT64 & mut = (*mIndividual[i])[j];
          // print this parameter
          ofile << mut << "\t";
        }

      // print the fitness of the individual
      ofile << mValue[i] << std::endl;
    }

  ofile << std::endl;
  ofile.close();
  return true;
}
#endif

// initialise the population
bool COptMethodGA::creation(size_t first,
                            size_t last)
{
  size_t Last = std::min(last, (size_t) mPopulationSize);

  size_t i;
  size_t j;

  C_FLOAT64 mn;
  C_FLOAT64 mx;
  C_FLOAT64 la;

  bool Continue = true;

  for (i = first; i < Last && Continue; i++)
    {
      for (j = 0; j < mVariableSize; j++)
        {
          // calculate lower and upper bounds
          COptItem & OptItem = *(*mpOptItem)[j];
          mn = *OptItem.getLowerBoundValue();
          mx = *OptItem.getUpperBoundValue();

          C_FLOAT64 & mut = (*mIndividual[i])[j];

          try
            {
              // determine if linear or log scale
              if ((mn < 0.0) || (mx <= 0.0))
                mut = mn + mpRandom->getRandomCC() * (mx - mn);
              else
                {
                  la = log10(mx) - log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min()));

                  if (la < 1.8)
                    mut = mn + mpRandom->getRandomCC() * (mx - mn);
                  else
                    mut = pow(10.0, log10(std::max(mn, std::numeric_limits< C_FLOAT64 >::min())) + la * mpRandom->getRandomCC());
                }
            }

          catch (...)
            {
              mut = (mx + mn) * 0.5;
            }

          // force it to be within the bounds
          switch (OptItem.checkConstraint(mut))
            {
              case - 1:
                mut = *OptItem.getLowerBoundValue();
                break;

              case 1:
                mut = *OptItem.getUpperBoundValue();
                break;
            }

          // We need to set the value here so that further checks take
          // account of the value.
          *mContainerVariables[j] = mut;
        }

      // calculate its fitness
      Continue &= evaluate(*mIndividual[i]);
      mValue[i] = mEvaluationValue;
    }

  return Continue;
}

void COptMethodGA::initObjects()
{
  addObjectReference("Current Generation", mGeneration, CCopasiObject::ValueInt);
}

bool COptMethodGA::initialize()
{
  cleanup();

  size_t i;

  if (!COptMethod::initialize())
    {
      if (mpCallBack)
        mpCallBack->finishItem(mhGenerations);

      return false;
    }

  mLogVerbosity = * getValue("#LogVerbosity").pUINT;

  mGenerations = (unsigned C_INT32)getValue< C_FLOAT64 >("Number of Generations");
  mGeneration = 0;

  if (mpCallBack)
    mhGenerations =
      mpCallBack->addItem("Current Generation",
                          mGeneration,
                          & mGenerations);

  mGeneration++;

  mPopulationSize = getValue< unsigned C_INT32 >("Population Size");
  mpRandom =
    CRandom::createGenerator((CRandom::Type) getValue< unsigned C_INT32 >("Random Number Generator"),
                             getValue< unsigned C_INT32 >("Seed"));

  mVariableSize = mpOptItem->size();

  mIndividual.resize(2 * mPopulationSize);

  for (i = 0; i < 2 * mPopulationSize; i++)
    mIndividual[i] = new CVector< C_FLOAT64 >(mVariableSize);

  mCrossOverFalse.resize(mVariableSize);
  mCrossOverFalse = false;
  mCrossOver.resize(mVariableSize);

  mValue.resize(2 * mPopulationSize);
  mValue = std::numeric_limits<C_FLOAT64>::infinity();
  mBestValue = std::numeric_limits<C_FLOAT64>::infinity();

  mpPermutation = new CPermutation(mpRandom, mPopulationSize);

  mLosses.resize(2 * mPopulationSize);

  // Initialize the variance for mutations
  mMutationVarians = 0.1;

  return true;
}

bool COptMethodGA::cleanup()
{
  size_t i;

  pdelete(mpRandom);
  pdelete(mpPermutation);

  for (i = 0; i < mIndividual.size(); i++)
    pdelete(mIndividual[i]);

  return true;
}

bool COptMethodGA::optimise()
{
  bool Continue = true;

  if (!initialize())
    {
      // initialisation failed, we exit
      if (mpCallBack)
        mpCallBack->finishItem(mhGenerations);

      return false;
    }

  // Counters to determine whether the optimization process has stalled
  // They count the number of generations without advances.
  size_t Stalled, Stalled10, Stalled30, Stalled50;
  Stalled = Stalled10 = Stalled30 = Stalled50 = 0;

  size_t i;

  // initialise the population
  // first individual is the initial guess
  bool pointInParameterDomain = true;
  for (i = 0; i < mVariableSize; i++)
    {
      C_FLOAT64 & mut = (*mIndividual[0])[i];
      COptItem & OptItem = *(*mpOptItem)[i];

      mut = OptItem.getStartValue();

      // force it to be within the bounds
      switch (OptItem.checkConstraint(mut))
        {
          case - 1:
            mut = *OptItem.getLowerBoundValue();
            pointInParameterDomain = false;
            break;

          case 1:
            mut = *OptItem.getUpperBoundValue();
            pointInParameterDomain = false;
            break;
        }

      // We need to set the value here so that further checks take
      // account of the value.
      *mContainerVariables[i] = mut;
    }
  if (mLogVerbosity >= 1 && !pointInParameterDomain) mMethodLogOld << "Initial point not within parameter domain.\n";


  Continue &= evaluate(*mIndividual[0]);
  mValue[0] = mEvaluationValue;

  if (!isnan(mEvaluationValue))
    {
      // and store that value
      mBestValue = mValue[0];
      Continue &= mpOptProblem->setSolution(mBestValue, *mIndividual[0]);

      // We found a new best value lets report it.
      mpParentTask->output(COutputInterface::DURING);
    }

  // the others are random
  Continue &= creation(1, mPopulationSize);

#ifdef DEBUG_OPT
  serializepop(0, mPopulationSize);
#endif

  Continue &= select();
  mBestIndex = fittest();

  if (mBestIndex != C_INVALID_INDEX &&
      mValue[mBestIndex] < mBestValue)
    {
      // and store that value
      mBestValue = mValue[mBestIndex];
      Continue = mpOptProblem->setSolution(mBestValue, *mIndividual[mBestIndex]);

      // We found a new best value lets report it.
      mpParentTask->output(COutputInterface::DURING);
    }

  // test if the user wants to stop, and do so if needed
  if (!Continue)
    {
      if (mLogVerbosity >= 1) mMethodLogOld << "Algorithm was terminated preemptively after initial population creation.\n";

      if (mpCallBack)
        mpCallBack->finishItem(mhGenerations);

      cleanup();
      return true;
    }

  // ITERATE FOR gener GENERATIONS
  for (mGeneration = 2;
       mGeneration <= mGenerations && Continue;
       mGeneration++, Stalled++, Stalled10++, Stalled30++, Stalled50++)
    {
      // perturb the population if we have stalled for a while
      if (Stalled > 50 && Stalled50 > 50)
        {
          if (mLogVerbosity >= 1) mMethodLogOld << "Generation " << mGeneration << ": Fittest individual has not changed for the last 50 generations. 50% random individuals created.\n";

          Continue &= creation((size_t)(mPopulationSize / 2),
                               mPopulationSize);
          Stalled10 = Stalled30 = Stalled50 = 0;
        }
      else if (Stalled > 30 && Stalled30 > 30)
        {
          if (mLogVerbosity >= 1) mMethodLogOld << "Generation " << mGeneration << ": Fittest individual has not changed for the last 30 generations. 30% random individuals created.\n";

          Continue &= creation((size_t)(mPopulationSize * 0.7),
                               mPopulationSize);
          Stalled10 = Stalled30 = 0;
        }
      else if (Stalled > 10 && Stalled10 > 10)
        {
          if (mLogVerbosity >= 1) mMethodLogOld << "Generation " << mGeneration << ": Fittest individual has not changed for the last 10 generations. 10% random individuals created.\n";

          Continue &= creation((size_t)(mPopulationSize * 0.9),
                               mPopulationSize);
          Stalled10 = 0;
        }
      // replicate the individuals
      else
        Continue &= replicate();

      // select the most fit
      Continue &= select();

      // get the index of the fittest
      mBestIndex = fittest();

      if (mBestIndex != C_INVALID_INDEX &&
          mValue[mBestIndex] < mBestValue)
        {
          // reset the stalled counters, since we made progress this time
          Stalled = Stalled10 = Stalled30 = Stalled50 = 0;
          // keep best value
          mBestValue = mValue[mBestIndex];
          // pass the current best value upstream
          Continue &= mpOptProblem->setSolution(mBestValue, *mIndividual[mBestIndex]);
          // We found a new best value lets report it.
          mpParentTask->output(COutputInterface::DURING);
        }

      if (mpCallBack)
        Continue &= mpCallBack->progressItem(mhGenerations);

#ifdef DEBUG_OPT
      serializepop(0, mPopulationSize);
#endif
    }

  if (mLogVerbosity >= 1) mMethodLogOld << "Algorithm terminated after " << (mGeneration - 1) << " of " << mGenerations << " generations.\n";

  if (mpCallBack)
    mpCallBack->finishItem(mhGenerations);

  cleanup();
  return true;
}

unsigned C_INT32 COptMethodGA::getMaxLogDetail() const
{
  return 1;
}
