// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CEvaluationNodeObject.cpp,v $
//   $Revision: 1.37 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/04/29 21:24:40 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "copasi.h"
#include "CEvaluationNode.h"
#include "CEvaluationTree.h"
#include "CExpression.h"
#include "report/CCopasiObjectName.h"
#include "report/CCopasiObject.h"
#include "report/CCopasiContainer.h"
#include "model/CModel.h"
#include "CopasiDataModel/CCopasiDataModel.h"

#include "sbml/math/ASTNode.h"
#include "sbml/SBase.h"
#include "sbml/SBMLTypeCodes.h"
#include "sbml/Compartment.h"
#include "sbml/Species.h"
#include "sbml/Parameter.h"
#include "sbml/Reaction.h"

CEvaluationNodeObject::CEvaluationNodeObject():
    CEvaluationNode(CEvaluationNode::INVALID, ""),
    mpValue(NULL),
    mRegisteredObjectCN("")
{mPrecedence = PRECEDENCE_NUMBER;}

CEvaluationNodeObject::CEvaluationNodeObject(const SubType & subType,
    const Data & data):
    CEvaluationNode((Type)(CEvaluationNode::OBJECT | subType), data),
    mpValue(NULL),
    mRegisteredObjectCN(data.substr(1, data.length() - 2))
{mPrecedence = PRECEDENCE_NUMBER;}

CEvaluationNodeObject::CEvaluationNodeObject(const C_FLOAT64 * pValue):
    CEvaluationNode((Type)(CEvaluationNode::OBJECT | POINTER), ""),
    mpValue(pValue),
    mRegisteredObjectCN("")
{
  mPrecedence = PRECEDENCE_NUMBER;

  std::ostringstream Pointer;
  Pointer << pValue;
  mData = Pointer.str();
}

CEvaluationNodeObject::CEvaluationNodeObject(const CEvaluationNodeObject & src):
    CEvaluationNode(src),
    mpValue(src.mpValue),
    mRegisteredObjectCN(src.mRegisteredObjectCN)
{}

CEvaluationNodeObject::~CEvaluationNodeObject() {}

bool CEvaluationNodeObject::compile(const CEvaluationTree * pTree)
{
  switch ((int) subType(mType))
    {
      case CN:
      {
        const CExpression * pExpression = dynamic_cast< const CExpression * >(pTree);

        if (!pExpression) return false;

        const CCopasiObject * pObject =
          pExpression->getNodeObject(mRegisteredObjectCN);

        if (pObject)
          mpValue = (C_FLOAT64 *) pObject->getValuePointer();
        else
          mpValue = NULL;

        if (mpValue == NULL)
          {
            mValue = std::numeric_limits<C_FLOAT64>::quiet_NaN();
            mpValue = &mValue;
            return false;
          }

        if (!pObject->isValueDbl()) return false;

        mData = "<" + mRegisteredObjectCN + ">";
      }
      break;

      case POINTER:
        break;

      case INVALID:
        break;
    }

  return (getChild() == NULL); // We must not have any children.
}

CEvaluationNode::Data CEvaluationNodeObject::getData() const
{return "<" + mRegisteredObjectCN + ">";}

bool CEvaluationNodeObject::setData(const Data & data)
{
  mData = data;

  if ((int) subType(mType) == (int) CN)
    mRegisteredObjectCN = data.substr(1, data.length() - 2);

  return true;
}

std::string CEvaluationNodeObject::getInfix() const
{return "<" + mRegisteredObjectCN + ">";}

#if 0
std::string CEvaluationNodeObject::getDisplayString(const CEvaluationTree * pTree) const
{
  const CExpression * pExpression = dynamic_cast< const CExpression * >(pTree);

  if (!pExpression) return false;

  const CCopasiObject * pObject =
    CCopasiContainer::ObjectFromName(mRegisteredObjectCN);

  if (pObject == NULL) return "<" + mRegisteredObjectCN + ">";

  return "<" + pObject->getObjectDisplayName() + ">";
}
#endif

std::string CEvaluationNodeObject::getDisplayString(const CEvaluationTree * /* pTree */) const
{
  return "<" + mRegisteredObjectCN + ">";
}

std::string CEvaluationNodeObject::getDisplay_C_String(const CEvaluationTree * /* pTree */) const
{
  return mData;
}

std::string CEvaluationNodeObject::getDisplay_MMD_String(const CEvaluationTree * /* pTree */) const
{
  return mData;
}

std::string CEvaluationNodeObject::getDisplay_XPP_String(const CEvaluationTree * /* pTree */) const
{
  return mData;
}

CEvaluationNode* CEvaluationNodeObject::createNodeFromASTTree(const ASTNode& node)
{
  CEvaluationNodeObject* pNode = NULL;
  ASTNodeType_t type = node.getType();

  switch (type)
    {
      case AST_NAME_TIME:
      case AST_NAME:
        pNode = new CEvaluationNodeObject(CN, CCopasiObjectName(std::string("<") + node.getName() + std::string(">")));
        break;
      default:
        break;
    }

  return pNode;
}

ASTNode* CEvaluationNodeObject::toAST(const CCopasiDataModel* pDataModel) const
{
  ASTNode* node = new ASTNode();
  node->setType(AST_NAME);
  // since I can not get the model in which this node is located, I just
  // assume that it will always be the current global model.
  const CCopasiObject* object = pDataModel->getObject(mRegisteredObjectCN);
  assert(object);

  // if it is a reference, we get the parent of the reference
  if (object->isReference())
    {
      object = object->getObjectParent();
    }

  const CModelEntity* pME = dynamic_cast<const CModelEntity*>(object);

  if (pME != NULL)
    {
      const CModel* pModel = dynamic_cast<const CModel*>(pME);

      if (pModel != NULL)
        {
          node->setType(AST_NAME_TIME);
          node->setName("time");

          if (pModel->getInitialTime() != 0.0)
            {
              CCopasiMessage(CCopasiMessage::WARNING, MCSBML + 1);
            }
        }
      else
        {
          node->setName(pME->getSBMLId().c_str());
        }
    }
  else
    {
      const CCopasiParameter* pPara = dynamic_cast<const CCopasiParameter*>(object);

      if (pPara != NULL)
        {
          // now we have to use the common name as the name for the
          // node since we need to be able to identify local parameters
          // in arbitrary expressions for the export
          node->setName(pPara->getCN().c_str());
        }
      else
        {
          fatalError();
        }
    }

  return node;
}

const CRegisteredObjectName & CEvaluationNodeObject::getObjectCN() const
{return mRegisteredObjectCN;}

#include "utilities/copasimathml.h"

void CEvaluationNodeObject::writeMathML(std::ostream & out,
                                        const std::vector<std::vector<std::string> > & /* env */,
                                        const CCopasiDataModel* pDataModel,
                                        bool /* expand */,
                                        unsigned C_INT32 l) const
{
  const CCopasiObject* obj = pDataModel->getObject(mRegisteredObjectCN);
  out << SPC(l) << CMathMl::getMMLName(obj) << std::endl;
  //or use mValue instead?
}
