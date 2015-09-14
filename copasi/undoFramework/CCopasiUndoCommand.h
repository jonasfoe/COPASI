// Copyright (C) 2014 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/*
 * CCopasiUndoCommand.h
 *
 *  Created on: 3 Jul 2014
 *      Author: dada
 */
#ifndef CCOPASIUNDOCOMMAND_H_
#define CCOPASIUNDOCOMMAND_H_

#include <QUndoCommand>
#include <QModelIndex>


#include <set>

typedef QPair<int, int> PathItem;
typedef QList<PathItem> Path;

class CCopasiObject;
class UndoData;
class UndoSpecieData;
class UndoReactionData;
class UndoGlobalQuantityData;
class UndoEventData;

class CCopasiUndoCommand : public QUndoCommand
{
public:
  /**
   *  The valid command types for the undo history
   */
  enum Type
  {
    COMPARTMENTCREATE = 0 , //creation of single compartment
    EVENTCREATE, //creation of single event
    GLOBALQUANTITYCREATE, //creation of single global quantity
    REACTIONCREATE, //creation of single reaction
    SPECIECREATE, //creation of single species
    COMPARTMENTDELETE, //deletion of single compartment
    EVENTDELETE, //deletion of single event
    GLOBALQUANTITYDELETE, //deletion of single global quantity
    REACTIONDELETE, //deletion of single reaction
    SPECIEDELETE, //deletion of single species
    COMPARTMENTINSERT, //insert compartment
    EVENTINSERT, //insert event
    GLOBALQUANTITYINSERT, //insert global quantity
    REACTIONINSERT, //insert reaction
    SPECIEINSERT, //insert species
    COMPARTMENTREMOVE, //remove compartment
    EVENTREMOVE, //remove event
    GLOBALQUANTITYREMOVE, //remove global quantity
    REACTIONREMOVE, //remove reaction
    SPECIEREMOVE, //remove species
    COMPARTMENTREMOVEALL, //remove all compartment
    EVENTREMOVEALL, //remove all event
    GLOBALQUANTITYREMOVEALL, //remove all global quantity
    REACTIONREMOVEALL, //remove all reaction
    SPECIEREMOVEALL, //remove all species
    COMPARTMENTDATACHANGE, //change compartment data
    EVENTDATACHANGE, //change event data
    GLOBALQUANTITYDATACHANGE, //change global quantity data
    REACTIONDATACHANGE, //change reaction data
    SPECIEDATACHANGE, //change species data
    SPECIESTYPECHANGE, //change of species type
    MODEL_INITIAL_TIME_CHANGE, // change of model initial time
    MODEL_TIME_UNIT_CHANGE, // change of model time unit
    MODEL_QUANTITY_UNIT_CHANGE, // change of model quantity unit
    MODEL_VOLUME_UNIT_CHANGE, // change of model volume unit
    MODEL_AREA_UNIT_CHANGE, // change of model area unit
    MODEL_LENGTH_UNIT_CHANGE, // change of model length unit
    MODEL_STOCHASTIC_CORRECTION_CHANGE, // change of the stochastic correct on model
    COMPARTMENT_SIMULATION_TYPE_CHANGE, //change of compartment simulation type
    COMPARTMENT_INITIAL_VOLUME_CHANGE, // change of compartment initial volume
    COMPARTMENT_INITIAL_EXPRESSION_CHANGE, // change of compartment initial expression
    COMPARTMENT_EXPRESSION_CHANGE, // change of assignment / ode expression
    COMPARTMENT_SPATIAL_DIMENSION_CHANGE, // change of compartment spatial dimensions
    GLOBALQUANTITY_INITAL_VALUE_CHANGE, // change of parameter initial value
    GLOBALQUANTITY_INITIAL_EXPRESSION_CHANGE, // change of parameter initial expression
    GLOBALQUANTITY_SIMULATION_TYPE_CHANGE, // change of parameter simulation type
    GLOBALQUANTITY_EXPRESSION_CHANGE, // change of parameter assignment / ode expression
    EVENT_TRIGGER_EXPRESSION_CHANGE, // change of event trigger expression
    EVENT_DELAY_TYPE_CHANGE, //change of event delay type
    EVENT_DELAY_EXPRESSION_CHANGE, //change of event delay expression
    EVENT_ASSIGNMENT_TARGET_CHANGE, //change of event assignment target
    EVENT_ASSIGNMENT_EXPRESSION_CHANGE, //change of event assignment value
    EVENT_ASSIGNMENT_ADDED, // add event assignment to event
    EVENT_ASSIGNMENT_REMOVED, // remove event assignment from event
    REACTION_SCHEME_CHANGE, //change reaction scheme
    REACTION_REVERSIBLE_CHANGE, // change of reaction reversible flag
    REACTION_FAST_CHANGE, // change of reaction fast flag
    REACTION_FUNCTION_CHANGE, // change of reaction kinetics
    REACTION_LOCAL_PARAMETER_VALUE_CHANGE, // change of a local parameter value
    REACTION_MAPPING_VOLUME_CHANGE, // mapping of volume in kinetic law changed
    REACTION_MAPPING_SPECIES_CHANGE, // mapping of metabolite changed
    REACTION_MAPPING_PARAMETER_CHANGE, // mapping of local / global parametr changed
    INVALID_TYPE
  };

  /**
   * Retrieve the type of the command.
   * @return const CCopasiUndoCommand::Type & type
   */
  const CCopasiUndoCommand::Type & getType() const;

  /**
   * Set the type
   * @param const CCopasiUndoCommand::Type & type
   */
  virtual void setType(const CCopasiUndoCommand::Type & type);

  /**
   * Retrieve the Undo Data associated with this command.
   * @return UndoData *undoData
   */
  virtual UndoData *getUndoData() const;

  /**
   * constructor initializing entity type and type
   * @param entityType the entity type (or empty if not given)
   * @param type the type (or INVALID_TYPE if not given)
   */
  CCopasiUndoCommand(const std::string& entityType = "",
                     CCopasiUndoCommand::Type type = INVALID_TYPE,
                     const std::string& action = "",
                     const std::string& property = "",
                     const std::string& newValue = "",
                     const std::string& oldValue = "",
                     const std::string& name = ""
                    );

  virtual ~CCopasiUndoCommand();

  virtual void undo() = 0;
  virtual void redo() = 0;

  Path pathFromIndex(const QModelIndex & index);

  QModelIndex pathToIndex(const Path& path, const QAbstractItemModel *model);

  void setDependentObjects(const std::set<const CCopasiObject*>& deletedObjects);

  QList<UndoReactionData*> *getReactionData() const;

  QList<UndoSpecieData*> *getSpecieData() const;

  void setReactionData(QList<UndoReactionData*>* reactionData);

  void setSpecieData(QList<UndoSpecieData*>* specieData);

  QList<UndoGlobalQuantityData*>* getGlobalQuantityData() const;

  void setGlobalQuantityData(QList<UndoGlobalQuantityData*>* globalQuantityData);

  QList<UndoEventData*> *getEventData() const;

  void setEventData(QList<UndoEventData*>* eventData);

  bool isUndoState() const;

  void setUndoState(bool undoState);

  std::string getEntityType() const;

  std::string getNewValue() const;

  std::string getOldValue() const;

  std::string getProperty() const;

  void setEntityType(const std::string& entityType);

  void setNewValue(const std::string& newValue);

  void setOldValue(const std::string& oldValue);

  void setProperty(const std::string& property);

  std::string getAction() const;

  void setAction(const std::string& action);

  std::string getName() const;

  void setName(const std::string& name);

protected:
  QList<UndoSpecieData*> *mpSpecieData;
  QList<UndoReactionData*> *mpReactionData;
  QList<UndoGlobalQuantityData*> *mpGlobalQuantityData;
  QList<UndoEventData*> *mpEventData;
  /**
   *  Type of the undo command.
   */
  CCopasiUndoCommand::Type mType;

  bool undoState;
  std::string mNewValue;
  std::string mOldValue;
  std::string mProperty;
  std::string mEntityType;
  std::string mAction;
  std::string mName;
};

#endif /* CCOPASIUNDOCOMMAND_H_ */
