/**
 * CKinFunction
 *
 * Created for Copasi by Stefan Hoops
 * (C) Stefan Hoops 2002
 */

#define COPASI_TRACE_CONSTRUCTION

#undef yyFlexLexer
#define yyFlexLexer CKinFunctionFlexLexer

#include <sstream>

#include "copasi.h"
#include "FlexLexer.h"
#include "CKinFunction.h"
#include "utilities/CCopasiException.h"

CKinFunction::CKinFunction(const std::string & name,
                           const CCopasiContainer * pParent):
    CFunction(name, pParent)
{
  CONSTRUCTOR_TRACE;
  setType(CFunction::UserDefined);
}

CKinFunction::CKinFunction(const CFunction & src,
                           CReadConfig * configBuffer,
                           const CCopasiContainer * pParent):
    CFunction(src, pParent)
{
  CONSTRUCTOR_TRACE;

  if (configBuffer)
    if (configBuffer->getVersion() < "4")
      {
        unsigned C_INT32 i, Size;
        configBuffer->getVariable("Nodes", "C_INT32", &Size);
        mNodes.resize(Size);

        for (i = 0; i < Size; i++)
          {
            mNodes[i] = new CNodeK;
            mNodes[i]->load(*configBuffer);
          }

        createParameters();

        cleanupNodes();
      }

  compile();
}

CKinFunction::CKinFunction(const CKinFunction & src,
                           const CCopasiContainer * pParent):
    CFunction(src, pParent)
{
  CONSTRUCTOR_TRACE;
  compile();
}

CKinFunction::~CKinFunction()
{
  cleanup();
  DESTRUCTOR_TRACE;
}

void CKinFunction::cleanup()
{
  cleanupNodes();
  CFunction::cleanup();
}

#ifdef XXXX
void CKinFunction::load(CReadConfig & configBuffer,
                        CReadConfig::Mode mode)
{
  CFunction::load(configBuffer, mode);

  // For older file version the parameters have to be build from information
  // dependend on the function type. Luckilly, only user defined functions are
  // the only ones occuring in those files.

  if (configBuffer.getVersion() < "4")
    {
      unsigned C_INT32 Size;
      configBuffer.getVariable("Nodes", "C_INT32", &Size);

      mNodes.load(configBuffer, Size);
      createParameters();
      mNodes.cleanup();
    }

  compile();
}
#endif // XXXX

void CKinFunction::saveOld(CWriteConfig & configBuffer)
{
  C_INT32 i, size;
  CFunction::saveOld(configBuffer);
  size = mNodes.size();
  configBuffer.setVariable("Nodes", "C_INT32", &size);
  for (i = 0; i < size; i++)
    mNodes[i]->saveOld(configBuffer);
}

std::string CKinFunction::getSBMLString(const CCallParameters & callParameterNames, const std::string &r) const
  {
    return mNodes[0]->getExplicitFunctionString(callParameterNames, r);
  }

void CKinFunction::compile()
{
  cleanupNodes();
  parse();
  connectNodes();
  initIdentifierNodes();
}

C_INT32 CKinFunction::parse()
{
  int i = 1;
  std::istringstream buffer(getDescription());
  CKinFunctionFlexLexer Scanner((std::istream *) &buffer);

  // add the root node
  CNodeK * pNode = new CNodeK(N_ROOT, N_NOP);
  mNodes.push_back(pNode);

  // call the lexical analyser successively until done

  while (i)
    {
      i = Scanner.yylex();

      switch (i)
        {
        case N_IDENTIFIER:
        case N_OBJECT:
          pNode = new CNodeK(Scanner.YYText());
          pNode->setType(i);
          mNodes.push_back(pNode);
          break;

        case N_NUMBER:
          pNode = new CNodeK(atof(Scanner.YYText()));
          mNodes.push_back(pNode);
          break;

        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '(':
        case ')':
          pNode = new CNodeK(N_OPERATOR, i);
          mNodes.push_back(pNode);
          break;

        case N_LOG:
        case N_LOG10:
        case N_EXP:
        case N_SIN:
        case N_COS:
          pNode = new CNodeK(N_FUNCTION, i);
          mNodes.push_back(pNode);
          break;

        case N_NOP:                  // this is an error
          cleanupNodes();
          /* :TODO: create a valid error message returning the eroneous node */
          fatalError();
          return 0;

        default:
          break;
        }
    }

  return 0;
}

C_FLOAT64 CKinFunction::calcValue(const CCallParameters & callParameters) const
  {
    return mNodes[0]->getLeft().value(callParameters);
  }

bool CKinFunction::dependsOn(const void * parameter,
                             const CCallParameters & callParameters) const
  {
    std::vector< const void * >::const_iterator it = callParameters.begin();
    std::vector< const void * >::const_iterator end = callParameters.end();

    for (; it != end; it++) if (parameter == *it) return true;

    return false;
  }

C_INT32 CKinFunction::connectNodes()
{
  C_INT32 errfl = 0;     // !!! do we need this?
  C_INT32 errnode = -1;  // !!! do we need this?
  unsigned C_INT32 i;

  // initialise the control variables
  mNidx = 1;

  // point all Left & Right to the root node

  for (i = 1; i < mNodes.size(); i++)
    {
      mNodes[i]->setLeft(mNodes[0]);
      mNodes[i]->setRight(mNodes[0]);
    }

  // update pointers and references in the tree
  mNodes[0]->setLeft(parseExpression(0));

  // further checking for errors
  if (mNodes[0]->isLeftValid() &&
      mNodes[0]->getLeft().isOperator() &&
      mNodes[0]->getLeft().getSubtype() == '(')
    {
      //  sprintf(errstr, "ERROR - missing operand");
      //  errnode should index the node in error
      //  but we don't know its index (pointer only)
      CCopasiMessage(CCopasiMessage::ERROR, MCKinFunction + 2,
                     getName().c_str());
      errnode = -1;
      errfl++;
    }

  for (i = 1; i < mNodes.size() && !errfl; i++)
    {
      switch (mNodes[i]->getType())
        {
        case N_OPERATOR:

          if (!mNodes[i]->isLeftValid() ||
              !mNodes[i]->isRightValid() ||
              &mNodes[i]->getLeft() == mNodes[0] ||
              &mNodes[i]->getRight() == mNodes[0])
            if (mNodes[i]->getSubtype() != '(' &&
                mNodes[i]->getSubtype() != ')')
              {
                if (!errfl)
                  {
                    // sprintf(errstr, "ERROR - incorrect number of operands");
                    fatalError();
                    errnode = i;
                  }

                errfl++;
              }

          if (!errfl)
            {
              if (mNodes[i]->isLeftValid() &&
                  mNodes[i]->getLeft().isOperator() &&
                  mNodes[i]->getLeft().getSubtype() == '(')
                {
                  //           sprintf(errstr, "ERROR - missing operand");
                  fatalError();
                  errnode = -1;
                  errfl++;
                }

              if (mNodes[i]->isRightValid() &&
                  mNodes[i]->getRight().isOperator() &&
                  mNodes[i]->getRight().getSubtype() == ')')
                {
                  //           sprintf(errstr, "ERROR - missing operand");
                  fatalError();
                  errnode = -1;
                  errfl++;
                }
            }

          break;

        case N_IDENTIFIER:

          if (mNodes[i]->isLeftValid() ||
              mNodes[i]->isRightValid())
            {
              if (!errfl)
                {
                  //           sprintf(errstr, "ERROR - unexpected identifier");
                  fatalError();
                  errnode = -1;
                }

              ++errfl;
            }

          break;

        case N_NUMBER:

          if (mNodes[i]->isLeftValid() ||
              mNodes[i]->isRightValid())
            {
              if (!errfl)
                {
                  //           sprintf(errstr, "ERROR - unexpected constant");
                  fatalError();
                  errnode = -1;
                }

              ++errfl;
            }

          break;
        }
    }

  // return
  return errfl;
}

CNodeK * CKinFunction::parseExpression(C_INT16 priority)
{
  C_INT32 errfl = 0;     // !!! do we need this?
  C_INT32 errnode = -1;  // !!! do we need this?

  CNodeK *lhs, *rhs;
  C_INT32 op;

  lhs = parsePrimary();

  if (!lhs)
    return NULL;

  while (mNidx < mNodes.size() &&
         mNodes[mNidx]->isOperator() &&
         priority < mNodes[mNidx]->leftPrecedence())
    {
      op = mNidx;
      rhs = NULL;
      ++mNidx;
      rhs = parseExpression(mNodes[op]->rightPrecedence());

      if (!rhs)
        {
          if (!errfl)
            {
              //    sprintf(errstr, "ERROR - unexpected operator");
              fatalError();
              errnode = op;
            }

          ++errfl;
        }
      else
        {
          mNodes[op]->setLeft(lhs);
          mNodes[op]->setRight(rhs);
          lhs = mNodes[op];
        }
    }

  return lhs;
}

CNodeK * CKinFunction::parsePrimary()
{
  C_INT32 errfl = 0;     // !!! do we need this?
  C_INT32 errnode = -1;  // !!! do we need this?

  CNodeK *npt, *primary;
  char t;
  C_INT32 op;
  npt = NULL;

  //    if (Node[mNidx]==NULL)

  if (mNidx >= mNodes.size())
    {
      //  if (!errfl) // execute only if no previous error
      //   errnode = mNidx-1;
      //  errfl++;
      return NULL;
    }

  t = mNodes[mNidx]->getType();
  if (t == N_IDENTIFIER ||
      t == N_OBJECT ||
      t == N_NUMBER)
    {
      t = 'K';
    }
  else
    {
      t = mNodes[mNidx]->getSubtype();
    }

  switch (t)
    {
    case 'K':
      mNodes[mNidx]->setLeft(NULL);
      mNodes[mNidx]->setRight(NULL);
      npt = mNodes[mNidx];
      ++mNidx;
      return npt;

    case '(':
      ++mNidx;
      npt = parseExpression(0);

      if (mNidx < mNodes.size() &&
          mNodes[mNidx]->isOperator() &&
          mNodes[mNidx]->getSubtype() == ')')
        {
          ++mNidx;
          return npt;
        }
      else
        {
          if (!errfl) // execute only if no previous error
            {
              //             sprintf(errstr, "ERROR - right bracket missing");
              errnode = mNidx;
            }

          errfl++;
        }

    case '+':

    case '-':

    case N_LOG:

    case N_LOG10:

    case N_EXP:

    case N_SIN:

    case N_COS:
      op = mNidx;
      primary = NULL;
      ++mNidx;
      primary = parsePrimary();

      if (primary == NULL)
        {
          if (!errfl)
            {
              //                  sprintf(errstr, "ERROR - missing operator");
              errnode = op;
            }

          ++errfl;
        }
      else
        {
          npt = mNodes[op];
          // unary operators are taken as functions
          mNodes[op]->setType(N_FUNCTION);
          mNodes[op]->setLeft(primary);
          mNodes[op]->setRight(NULL);
          return mNodes[op];
        }

    default:
      return NULL;
    }

  if (mNidx < mNodes.size() &&
      mNodes[mNidx]->isOperator() &&
      mNodes[mNidx]->getSubtype() == '(')
    {
      ++mNidx;

      if (mNidx < mNodes.size() &&
          mNodes[mNidx]->isOperator() &&
          mNodes[mNidx]->getSubtype() == ')')
        {
          mNodes[mNidx]->setLeft(npt);
          mNodes[mNidx]->setRight(NULL);
          return mNodes[mNidx];
        }
      else
        parseExpression(0);
    }
}

void CKinFunction::createParameters()
{
  CCopasiVectorN < CFunctionParameter > Substrates;
  CCopasiVectorN < CFunctionParameter > Products;
  CCopasiVectorN < CFunctionParameter > Modifiers;
  CCopasiVectorN < CFunctionParameter > Parameters;

  unsigned C_INT32 i, imax = mNodes.size();

  CFunctionParameter Parameter;
  Parameter.setType(CFunctionParameter::FLOAT64);

  for (i = 0; i < imax; i++)
    {
      if (mNodes[i]->getType() == N_IDENTIFIER)
        {
          try
            {
              Parameter.setName(mNodes[i]->getName());

              switch (mNodes[i]->getSubtype())
                {
                case N_SUBSTRATE:
                  Parameter.setUsage("SUBSTRATE");
                  Substrates.add(Parameter);
                  break;

                case N_PRODUCT:
                  Parameter.setUsage("PRODUCT");
                  Products.add(Parameter);
                  break;

                case N_MODIFIER:
                  Parameter.setUsage("MODIFIER");
                  Modifiers.add(Parameter);
                  break;

                case N_KCONSTANT:
                  Parameter.setUsage("PARAMETER");
                  Parameters.add(Parameter);
                  break;

                case N_NOP:
                  Parameter.setUsage("UNKNOWN");
                  Parameters.add(Parameter);
                  break;

                default:
                  fatalError();
                }
            }

          catch (CCopasiException Exception)
            {
              if ((MCCopasiVector + 2) != Exception.getMessage().getNumber())
                throw Exception;
            }
        }
    }

  imax = Substrates.size();

  for (i = 0; i < imax; i++)
    getParameters().add(Substrates[i]);

  imax = Products.size();

  for (i = 0; i < imax; i++)
    getParameters().add(Products[i]);

  imax = Modifiers.size();

  for (i = 0; i < imax; i++)
    getParameters().add(Modifiers[i]);

  imax = Parameters.size();

  for (i = 0; i < imax; i++)
    getParameters().add(Parameters[i]);
}

void CKinFunction::initIdentifierNodes()
{
  // CCopasiVectorNS < CFunctionParameter > * BaseParameters;
  // BaseParameters = &getParameters().getParameters();

  std::string IdentifierName, ParameterName, Usage;
  unsigned C_INT32 i, imax = getParameters().size();
  unsigned C_INT32 j, jmax = mNodes.size();
  C_INT32 subidx, prodidx, modfidx, constidx;

  for (j = 0; j < jmax; j++)
    {
      if (mNodes[j]->getType() != N_IDENTIFIER)
        continue;
      IdentifierName = mNodes[j]->getName();
      subidx = prodidx = modfidx = constidx = -1;
      for (i = 0; i < imax; i++)
        {
          Usage = getParameters()[i]->getUsage();
          if (Usage == "SUBSTRATE")
            subidx++;
          else if (Usage == "PRODUCT")
            prodidx++;
          else if (Usage == "MODIFIER")
            modfidx++;
          else if (Usage == "PARAMETER")
            constidx++;

          ParameterName = getParameters()[i]->getName();
          if (IdentifierName != ParameterName)
            continue;
          mNodes[j]->setIndex(i);

          // We really do not need the usage information in the binary
          // tree anymore, but we still put it there to allow saving
          //in the old format
          if (Usage == "SUBSTRATE")
            {
              mNodes[j]->setSubtype(N_SUBSTRATE);
              mNodes[j]->setOldIndex(subidx);
            }
          else if (Usage == "PRODUCT")
            {
              mNodes[j]->setSubtype(N_PRODUCT);
              mNodes[j]->setOldIndex(prodidx);
            }
          else if (Usage == "MODIFIER")
            {
              mNodes[j]->setSubtype(N_MODIFIER);
              mNodes[j]->setOldIndex(modfidx);
            }
          else if (Usage == "PARAMETER")
            {
              mNodes[j]->setSubtype(N_KCONSTANT);
              mNodes[j]->setOldIndex(constidx);
            }
          else if (Usage == "UNKNOWN")
            mNodes[j]->setSubtype(N_NOP);
          else
            fatalError();

          break;
        }

      if (i == imax)
        fatalError();
    }
}

std::vector< CNodeK * > & CKinFunction::getNodes() {return mNodes;}

void CKinFunction::cleanupNodes()
{
  unsigned C_INT32 i, imax = mNodes.size();

  for (i = 0; i < imax; i++)
    if (mNodes[i]) delete mNodes[i];

  mNodes.clear();

  return;
}
