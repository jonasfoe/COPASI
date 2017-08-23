// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

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

#ifndef COPASI_CILDMMethod
#define COPASI_CILDMMethod

#include <sstream>
#include "utilities/CVector.h"
#include "utilities/CMatrix.h"

#include "CTSSAMethod.h"

class CModel;
class CState;

/**
 * @brief The CILDMMethod class implements the ILDM method
 *
 * More information under: http://copasi.org/Support/User_Manual/Methods/Time_Scale_Separation_Methods/ILDM_Deuflhard/
 */
class CILDMMethod : public CTSSAMethod
{
  // Operations
private:
  /**
   * Default constructor.
   */
  CILDMMethod();

public:
  /**
   * Specific constructor
   * @param const CCopasiContainer * pParent
   * @param const CTaskEnum::Method & methodType (default: tssILDM)
   * @param const CTaskEnum::Task & taskType (default: tssAnalysis)
   */
  CILDMMethod(const CCopasiContainer * pParent,
              const CTaskEnum::Method & methodType = CTaskEnum::tssILDM,
              const CTaskEnum::Task & taskType = CTaskEnum::tssAnalysis);

  /**
   * Copy constructor.
   * @param "const CILDMMethod &" src
   * @param const CCopasiContainer * pParent (default: NULL)
   */
  CILDMMethod(const CILDMMethod & src,
              const CCopasiContainer * pParent);

  /**
   *  Destructor.
   */
  virtual ~CILDMMethod();

  /**
   *  Initialize the method parameters
   */
  virtual void initializeParameter();

  /**
   *  This instructs the method to calculate a time step of deltaT
   *  starting with the current state, i.e., the result of the previous
   *  step.
   *  The new state (after deltaT) is expected in the current state.
   *  The return value is the actual timestep taken.
   *  @param "const double &" deltaT
   */
  virtual void step(const double & deltaT);

  /**
   *  This instructs the method to prepare for integration
   *  starting with the initialState given.
   */
  virtual void start();

  /**
   * @return CArrayAnnotation for visualization in ILDM-tab
   * in the CQTSSAResultSubWidget
   **/
  const CArrayAnnotation* getVslowPrintAnn() const;
  const CArrayAnnotation* getVslowSpacePrintAnn() const;
  const CArrayAnnotation* getVfastSpacePrintAnn() const;
  const CArrayAnnotation* getVslowMetabPrintAnn() const;
  const CArrayAnnotation* getReacSlowSpacePrintAnn() const;

  /* temporary tabs */
  const CArrayAnnotation* getTMP1PrintAnn() const;
  const CArrayAnnotation* getTMP2PrintAnn() const;
  const CArrayAnnotation* getTMP3PrintAnn() const;

  /* temporary tabs */

  /**
   * upgrade all vectors with values from actually calculalion for current step
   **/
  void setVectors(int slowMode);

  /**
   * empty every vector to be able to fill them with new values for a
   * new calculation also nullify the step counter
   **/
  void emptyVectors();

  /**
   * create the CArraAnnotations for every ILDM-tab in the CQTSSAResultSubWidget
   * input for each CArraAnnotations is a seperate CMatrix
   **/
  virtual void createAnnotationsM();

  /**
   * initialize output for the result elements, this method
   * initializes the output elements so that an output handler
   * can be used afterwards
   **/
  virtual void initializeOutput();

  /**
   * set the every CArrayAnnotation for the requested step
   * set the description of CArayAnnotation for both dimensions
   **/
  virtual bool setAnnotationM(size_t step);

  /**
   *  print of the standard report sequence for ILDM Method
   *  @param std::ostream * ostream
   **/
  virtual void printResult(std::ostream * ostream) const;

protected:
  /**
   *
   **/
  void newton(C_FLOAT64 *ys, C_INT & slow, C_INT & info);

  /**
   *
   **/
  void transformation_norm(C_INT & slow, C_INT & info);

  void deuflhard(C_INT & slow, C_INT & info);

  /**
   * vectors contain whole data for all calculation steps
   **/
  CVector<C_FLOAT64> mReacSlowSpace; // NEW TAB

  /* temporary tabs */

  CMatrix<C_FLOAT64> mTMP1;
  CMatrix<C_FLOAT64> mTMP2;
  CMatrix<C_FLOAT64> mTMP3;

  std::vector< CMatrix<C_FLOAT64> > mVec_mVslow;
  std::vector< CMatrix<C_FLOAT64> > mVec_mVslowMetab;
  std::vector< CVector<C_FLOAT64> > mVec_mVslowSpace;
  std::vector< CVector<C_FLOAT64> > mVec_mVfastSpace;
  std::vector< CVector<C_FLOAT64> > mVec_mReacSlowSpace;

  /* temporary tabs */
  std::vector< CMatrix<C_FLOAT64> > mVec_mTMP1;
  std::vector< CMatrix<C_FLOAT64> > mVec_mTMP2;
  std::vector< CMatrix<C_FLOAT64> > mVec_mTMP3;

  /**
   * CArrayAnnotation for every ILDM-tab in the CQTSSAResultSubWidget
   **/
  CArrayAnnotation* pVslowPrintAnn;
  CArrayAnnotation* pVslowMetabPrintAnn;
  CArrayAnnotation* pVslowSpacePrintAnn;
  CArrayAnnotation* pVfastSpacePrintAnn;
  CArrayAnnotation* pReacSlowSpacePrintAnn;

  /* temporary tabs */

  CArrayAnnotation* pTMP1PrintAnn;
  CArrayAnnotation* pTMP2PrintAnn;
  CArrayAnnotation* pTMP3PrintAnn;

  /**
  *required for creation of above listed CArrayAnnotation
  **/
  CArrayAnnotation* pTmp1;
  CArrayAnnotation* pTmp2;
  CArrayAnnotation* pTmp3;
  CArrayAnnotation* pTmp4;
  CArrayAnnotation* pTmp5;

  /* temporary tabs */

  CArrayAnnotation* pTMP1;
  CArrayAnnotation* pTMP2;
  CArrayAnnotation* pTMP3;

  /**
  *input for every CArraAnnotations
  *contain data for single stepcalculation
  **/
  CMatrix<C_FLOAT64> mVslowPrint;
  CMatrix<C_FLOAT64> mVslowSpacePrint;
  CMatrix<C_FLOAT64> mVfastSpacePrint;
  CMatrix<C_FLOAT64> mVslowMetabPrint;
  CMatrix<C_FLOAT64> mReacSlowSpacePrint;

  /* temporary tabs */

  CMatrix<C_FLOAT64> mTMP1Print;
  CMatrix<C_FLOAT64> mTMP2Print;
  CMatrix<C_FLOAT64> mTMP3Print;
};
#endif // COPASI_CILDMMethod
