// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/elementaryFluxModes/CBitPatternTreeMethod.h,v $
//   $Revision: 1.1 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/09/01 15:58:41 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#ifndef COPASI_CBitPatternTreeMethod
#define COPASI_CBitPatternTreeMethod

#include <vector>

#include "copasi/elementaryFluxModes/CEFMMethod.h"

class CStepMatrix;
class CStepMatrixColumn;
class CReaction;
class CBitPatternTree;
class CBitPatternTreeNode;
class CZeroSet;
template <class CType> class CMatrix;

class CBitPatternTreeMethod: public CEFMMethod
{
  friend CEFMMethod * CEFMMethod::createMethod(CCopasiMethod::SubType subType);

protected:
  /**
   * Default constructor
   * @param const CCopasiContainer * pParent (Default: NULL)
   */
  CBitPatternTreeMethod(const CCopasiContainer * pParent = NULL);

  /**
   * Constructor to be called by derived methods
   * @param const CCopasiMethod::SubType subType
   * @param const CCopasiContainer * pParent (Default: NULL)
   */
  CBitPatternTreeMethod(const CCopasiMethod::SubType subType,
                        const CCopasiContainer * pParent = NULL);

public:
  /**
   * Copy Constructor
   * @param const CBitPatternTreeMethod & src
   */
  CBitPatternTreeMethod(const CBitPatternTreeMethod & src,
                        const CCopasiContainer * pParent = NULL);

  /**
  *  Destructor
  */
  ~CBitPatternTreeMethod();

  /**
   * Execute the optimization algorithm calling simulation routine
   * when needed. It is noted that this procedure can give feedback
   * of its progress by the callback function set with SetCallback.
   * @ return success;
   */
  virtual bool calculate();

  /**
   * Initialize arrays and pointer.
   * @return bool success
   */
  virtual bool initialize();

private:
  /**
   * Initialize the needed CCopasiObjects.
   */
  void initObjects();

  /**
   * Construct the null space matrix
   * @param CMatrix< C_FLOAT64> & matrix
   */
  void buildNullspaceMatrix(CMatrix< C_FLOAT64> & matrix);

  /**
   * Create all possible linear combinations of the bit pattern nodes pPositive
   * and pNegative and all their child nodes.
   * @param const CBitPatternTreeNode * pPositive
   * @param const CBitPatternTreeNode * pNegative
   */
  void combine(const CBitPatternTreeNode * pPositive,
               const CBitPatternTreeNode * pNegative);

  /**
   * Remove the invalid columns from the step matrix
   * @param const std::list< CStepMatrixColumn * > & nullColumns
   */
  void removeInvalidColumns(const std::list< CStepMatrixColumn * > & nullColumns);

  // Attributes
protected:
  /**
   * A pointer to the model which is analyzed.
   */
  CModel * mpModel;

  /**
   * The current step used for process report.
   */
  unsigned C_INT32 mProgressCounter;

  /**
   * The max step used for process report.
   */
  unsigned C_INT32 mProgressCounterMax;

  /**
   * Handle to the process report item "Current Step"
   */
  unsigned C_INT32 mhProgressCounter;

  /**
   * A vector to record the reordering of reactions.
   */
  std::vector< std::pair < const CReaction *, bool > > mReactionPivot;

  /**
   * A vector to record the row (species) rearrangements for the QR factorization.
   */
  std::vector< size_t > mSpeciesPivot;

  /**
   * A pointer to the step matrix for creating the flux modes
   */
  CStepMatrix * mpStepMatrix;

  /**
   * The bit pattern tree for the current step
   */
  CBitPatternTree * mpNullTree;

  /**
   * A list of invalid columns currently in the step matrix
   */
  std::list< CStepMatrixColumn * > mNewColumns;

  /**
   * The minimum set size use to determine whether a linear combination is allowed.
   */
  size_t mMinimumSetSize;

  /**
   * The currently process step
   */
  size_t mStep;
};

#endif // COPASI_CBitPatternTreeMethod
