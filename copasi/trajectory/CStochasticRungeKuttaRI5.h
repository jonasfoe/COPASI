// Copyright (C) 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef COPASI_CStochasticRungeKuttaRI5
#define COPASI_CStochasticRungeKuttaRI5

#include <sstream>

#include "copasi/utilities/CVector.h"
#include "copasi/trajectory/CTrajectoryMethod.h"
#include "copasi/trajectory/CRootFinder.h"
#include "copasi/odepack++/CLSODA.h"
#include "copasi/odepack++/CLSODAR.h"
#include "copasi/model/CState.h"
#include "copasi/utilities/CMatrix.h"

class CModel;
class CRandom;

class CStochasticRungeKuttaRI5 : public CTrajectoryMethod
{
public:

  // Operations
private:
  enum RootMasking
  {
    NONE = 0,
    ALL,
    DISCRETE
  };

  /**
   * Constructor.
   */
  CStochasticRungeKuttaRI5();

public:
  /**
   * Specific constructor
   * @param const CCopasiContainer * pParent
   * @param const CTaskEnum::Method & methodType (default: deterministic)
   * @param const CTaskEnum::Task & taskType (default: timeCourse)
   */
  CStochasticRungeKuttaRI5(const CCopasiContainer * pParent,
                           const CTaskEnum::Method & methodType = CTaskEnum::stochasticRunkeKuttaRI5,
                           const CTaskEnum::Task & taskType = CTaskEnum::timeCourse);

  /**
   * Copy constructor.
   * @param "const CStochasticRungeKuttaRI5 &" src
   * @param const CCopasiContainer * pParent (default: NULL)
   */
  CStochasticRungeKuttaRI5(const CStochasticRungeKuttaRI5 & src,
                           const CCopasiContainer * pParent);

  /**
   *  Destructor.
   */
  ~CStochasticRungeKuttaRI5();

  /**
   * This methods must be called to elevate subgroups to
   * derived objects. The default implementation does nothing.
   * @return bool success
   */
  virtual bool elevateChildren();

  /**
   * Inform the trajectory method that the state has changed outside
   * its control
   * @param const CMath::StateChange & change
   */
  virtual void stateChange(const CMath::StateChange & change);

  /**
   *  This instructs the method to calculate a time step of deltaT
   *  starting with the current state, i.e., the result of the previous
   *  step.
   *  The new state (after deltaT) is expected in the current state.
   *  The return value is the actual time step taken.
   *  @param "const double &" deltaT
   *  @return Status status
   */
  virtual CTrajectoryMethod::Status step(const double & deltaT);

  /**
   *  This instructs the method to prepare for integration
   */
  virtual void start();

  /**
   *  This evaluates the roots
   */
  void evalRoot(const double & time, CVectorCore< C_FLOAT64 > & rootValues);

private:
  /**
   *  This evaluates the derivatives
   */
  void evalRate(C_FLOAT64 * rates);

  /**
   *  This evaluates the noise
   */
  void evalNoise(C_FLOAT64 * pNoise, const size_t & noiseDimension);

  /**
   * Initialize the method parameter
   */
  void initializeParameter();

  void generateRandomNumbers();
  C_FLOAT64 randomIHat();
  C_FLOAT64 randomITilde();

  void buildStage1();
  void buildStage2();
  void buildStage3();
  void calculateStateVariables(const double & time);
  CTrajectoryMethod::Status internalStep();

  C_FLOAT64 calculateSmallestPhysicalValue() const;

  CVectorCore< C_FLOAT64 > mContainerVariables;
  CVectorCore< C_FLOAT64 > mContainerRates;
  CVectorCore< C_FLOAT64 > mContainerNoise;
  CVectorCore< C_FLOAT64 > mContainerRoots;

  size_t mNumVariables;
  size_t mNumNoise;
  size_t mNumRoots;

  /**
   * A pointer to the value of "Internal Steps Size"
   */
  C_FLOAT64 * mpInternalStepSize;

  /**
   * A pointer to the value of "Max Internal Steps"
   */
  unsigned C_INT32 * mpMaxInternalSteps;

  /**
   * A pointer to the value "Force Physical Correctness"
   */
  bool * mpForcePhysicalCorrectness;

  /**
   * A pointer to the value of "Tolerance for Root Finder"
   */
  C_FLOAT64 * mpRootRelativeTolerance;

  /**
   * A pointer to the random number generator
   */
  CRandom * mpRandom;

  C_FLOAT64 mStepSize;
  C_FLOAT64 mSqrtStepSize;

  CVector< C_FLOAT64 > mRandomIHat;
  CVector< C_FLOAT64 > mRandomITilde;
  CMatrix< C_FLOAT64 > mRandomIMatrix;

  C_FLOAT64 mTime;
  C_FLOAT64 mCurrentTime;
  C_FLOAT64 mTargetTime;
  size_t mInternalSteps;

  CVector< C_FLOAT64 > mH10;
  CVector< C_FLOAT64 > mSumAll1;
  CMatrix< C_FLOAT64 > mSumPartial1;

  CVector< C_FLOAT64 > mH20;
  CMatrix< C_FLOAT64 > mH2k;
  CMatrix< C_FLOAT64 > mHH2k;
  CVector< C_FLOAT64 > mSumAll2;
  CMatrix< C_FLOAT64 > mSumPartial2;

  CVector< C_FLOAT64 > mH30;
  CMatrix< C_FLOAT64 > mH3k;
  CMatrix< C_FLOAT64 > mHH3k;

  CMatrix< C_FLOAT64 > mA;
  CVector< CMatrix< C_FLOAT64 > > mB;
  CVector< CMatrix< C_FLOAT64 > > mBB;

  CVector< C_FLOAT64 * > mNoiseInputValues;
  CVector< UpdateSequence > mNoiseUpdateSequences;
  CVector< bool > mPhysicalValues;

  CRootFinder mRootFinder;
  CRootFinder::Eval * mpRootValueCalculator;
  CVectorCore< C_FLOAT64 > mRoots;
  CVectorCore< C_INT > mRootsFound;
  size_t mRootCounter;

  RootMasking mRootMasking;
  CVector< bool > mRootMask;
  CVectorCore< bool > mDiscreteRoots;
};

#endif // COPASI_CStochasticRungeKuttaRI5
