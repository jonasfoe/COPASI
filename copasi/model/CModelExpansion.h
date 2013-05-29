// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/model/CModelExpansion.h,v $
//   $Revision: 1.8 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2011/09/30 16:36:30 $
// End CVS Header

// Copyright (C) 2011 - 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#ifndef CMODELEXPANSION_H
#define CMODELEXPANSION_H

#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include "copasi.h"
#include <vector>

class CModel;
class CMetab;
class CCompartment;
class CReaction;
class CModelEntity;
class CExpression;
class CEvent;
class CCopasiObject;

class CModelExpansion
{
public:
  
  
  /**
   * This class describes a set of model elements that are supposed to be duplicated.
   * The second element in each of the pairs is used temporarily to hold the copy.
   */
  class SetOfModelElements
  {
  public:
  
  void addCompartment(const CCompartment* x);
  void addMetab(const CMetab* x);
  void addReaction(const CReaction* x);
  void addGlobalQuantity(const CModelEntity* x);
  void addEvent(const CEvent* x);

  /**
   * Checks whether an object is present in this set
   */
  bool contains(const CCopasiObject* x) const;
  bool contains(const std::string & key) const;
  
  /**
   * Determines which other object have to be publicated based on those already present.
   */
  void fillDependencies(const CModel* pModel);
  
  /**
   * enters all the objects from pModel
   */
  void fillComplete(const CModel* pModel);
  
  //protected:
  std::set<const CCompartment*> mCompartments;
  std::set<const CMetab*> mMetabs;
  std::set<const CReaction*> mReactions;
  std::set<const CModelEntity*> mGlobalQuantities;
  std::set<const CEvent*> mEvents;
  };

  /**
   * This class contains a map from original to duplicated sets of model elements
   */
  class ElementsMap
  {
  public:
  
  ///does a copy of the source object exist?
  bool exists(const CCopasiObject* source) const;
  bool exists(const std::string & sourceKey) const;
  
  ///add a source->duplicate mapping
  void add(const CCopasiObject* source, CCopasiObject* copy);
  
  ///find the pointer of the duplicated object from the pointer to the source object (if it exists)
  CCopasiObject* getDuplicatePtr(const CCopasiObject* source) const;

  ///find the key of the duplicated object from the source object (if it exists)
  std::string getDuplicateKey(const std::string & sourceKey) const;
  
  protected:
  
  std::map<const CCopasiObject*, CCopasiObject*> mMap;
  };
  
    
  CModelExpansion(CModel* pModel);

  void setModel(CModel* pModel);

  //just a simple method to call during development (as an example for how to use the other methods)
  void simpleCall(const CCompartment* source, std::vector< std::string  > listOfMetabolites, int mult, bool diff);

  /**
   * creates n copies of the objects in souce. Diffusion reactions are created for each of 
   * the metabs in listOfMetabolites, so that a linear chain is formed.
   */
  void createLinearArray(const SetOfModelElements & source, size_t n, const std::set< std::string  > & setOfMetabolites);
  
  /**
   * creates nx*ny copies of the objects in souce. Diffusion reactions are created for each of
   * the metabs in listOfMetabolites, so that a rectangular array is formed.
   */
  void createRectangularArray(const SetOfModelElements & source, size_t nx, size_t ny, const std::set< std::string  > & setOfMetabolites);

  /**
   * adds the contents of the source model to the current model
   */
  void copyCompleteModel(const CModel* pSourceModel);

  
  /**
   * creates one duplicate of the set of elements specified by source.
   * the string index is added to the object names
   */
  bool duplicate(const SetOfModelElements & source, const std::string & index, ElementsMap & emap);
  
  void duplicateCompartment(const CCompartment* source, const std::string & index, const SetOfModelElements & sourceSet, ElementsMap & emap);
  void duplicateMetab(const CMetab* source, const std::string & index, const SetOfModelElements & sourceSet, ElementsMap & emap);
  void duplicateReaction(const CReaction* source, const std::string & index, const SetOfModelElements & sourceSet, ElementsMap & emap);
  void duplicateGlobalQuantity(const CModelEntity* source, const std::string & index, const SetOfModelElements & sourceSet, ElementsMap & emap);
  void duplicateEvent(CEvent* source, const std::string & index, const SetOfModelElements & sourceSet, ElementsMap & emap);
 
  /**
   * steps through an expression and replaces references to objects with references to their duplicate.
   * (In case an object that should be duplicated according to sourceSet is not yet duplicated
   *  according to emap, the duplication is performed also)
   */
  void updateExpression(CExpression* exp, const std::string & index, const SetOfModelElements & sourceSet, ElementsMap & emap);

  /**
   * Check if a given expression contains any of the objects in the sourceSet
   */
  bool expressionContainsObject(const CExpression* exp, const SetOfModelElements & sourceSet);
 
  /**
   * Create a first order reversible mass action reaction. 
   * This is intended for diffusion between copies of model parts.
   * The method takes a reaction name, and keys of two metabolites and 
   * a global quantity that is used as a rate constant.
   */
  void createDiffusionReaction(const std::string & name,
                               const std::string & metabkey1, const std::string & metabkey2,
                               const std::string & parameterkey);

protected:


  //CompartmentInfo ci;

  //void initializeNameSets();

  /**
   * determine whether the one of the substrate, products, or modifiers of the reaction
   * is located in the given compartmen
   */
  //static bool reactionInvolvesCompartment(const CReaction * reac, const CCompartment* comp);

  /**
   *   handle non constant model entities
   **/

  //bool  copyCompartmentsExpressions(const CCompartment *source);
  //bool  copyMetabolitesExpressions(const CCompartment *source);

  //bool  copyModelValuesExpressions(std::string copyname);
  //bool  copyEvents(std::string copyname);

  //std::string copyExpression(const CExpression *pExpression);
  //std::string testName(const std::string & mname, std::set<std::string>& nameSet);
  //void  nameInSet(const std::string & mname, std::set<std::string>& nameSet);

  CModel * mpModel;

  // we use several sets for the entities that need to have unique names
  // This will speed up processing for large models
  //std::set<std::string> mCompartmentNameSet;
  //std::set<std::string> mReactionNameSet;
  //std::set<std::string> mModelValueNameSet;
  //std::set<std::string> mEventNameSet;

  //std::string name;
};

#endif // CMODELEXPANSION_H
