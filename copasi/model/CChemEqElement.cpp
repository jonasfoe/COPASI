// CChemEqElement
//
// A class describing an element of a chemical equation
// (C) Stefan Hoops 2001
//

#define  COPASI_TRACE_CONSTRUCTION

#include "copasi.h"
#include "utilities/CCopasiVector.h"
#include "utilities/CCopasiException.h"
#include "utilities/utility.h"
#include "CChemEqElement.h"
#include "CCompartment.h"

CChemEqElement::CChemEqElement(const std::string & name,
                               const CCopasiContainer * pParent):
    CCopasiContainer(name, pParent, "Chemical Equation Element"),
    mMetaboliteName(),
    //mCompartmentName(),
    mMultiplicity(0),
    mpMetabolite(NULL)
{CONSTRUCTOR_TRACE;}

CChemEqElement::CChemEqElement(const CChemEqElement & src,
                               const CCopasiContainer * pParent):
    CCopasiContainer(src, pParent),
    mMetaboliteName(src.mMetaboliteName),
    //mCompartmentName(src.mCompartmentName),
    mMultiplicity(src.mMultiplicity),
    mpMetabolite(src.mpMetabolite)
{CONSTRUCTOR_TRACE;}

CChemEqElement::~CChemEqElement() {DESTRUCTOR_TRACE;}

void CChemEqElement::cleanup() {}

void CChemEqElement::setMultiplicity(const C_FLOAT64 multiplicity)
{mMultiplicity = multiplicity;}

C_FLOAT64 CChemEqElement::getMultiplicity() const
  {return mMultiplicity;}

void CChemEqElement::setMetabolite(CMetab & metabolite)
{
  mpMetabolite = &metabolite;
  mMetaboliteName = mpMetabolite->getName();
  //mCompartmentName = mpMetabolite->getCompartment()->getName();
}

const CMetab & CChemEqElement::getMetabolite() const
  {
    if (!mpMetabolite)
      fatalError();

    return *mpMetabolite;
  }

//CMetab * CChemEqElement::getMetaboliteAddr() const
//{return mpMetabolite;}

void CChemEqElement::setMetaboliteName(const std::string & metaboliteName)
{mMetaboliteName = metaboliteName;}

const std::string & CChemEqElement::getMetaboliteName() const
  {return mMetaboliteName;}

//void CChemEqElement::setCompartmentName(const std::string & compartmentName)
//{mCompartmentName = compartmentName;}

//const std::string & CChemEqElement::getCompartmentName() const
//  {return mCompartmentName;}

void CChemEqElement::addToMultiplicity(const C_FLOAT64 multiplicity)
{mMultiplicity += multiplicity;}

void CChemEqElement::compile(const CCopasiVectorN < CCompartment > & compartments)
{
  std::string CompartmentName = "";
  unsigned C_INT32 i, imax = compartments.size();

  for (i = 0; i < imax; i++)
    {
      try
        {
          compartments[i]->getMetabolites()[mMetaboliteName];
          CompartmentName = compartments[i]->getName();
        }
      catch (CCopasiException Exception)
        {
          if ((MCCopasiVector + 1) == Exception.getMessage().getNumber())
            continue;
          else
            throw Exception;
        }
    }

  mpMetabolite =
    compartments[CompartmentName]->getMetabolites()[mMetaboliteName];
}

std::string CChemEqElement::writeElement() const
  {
    if (mMultiplicity == 1.0)
      return mMetaboliteName;
    else
      return StringPrint("%.0lf * %s", mMultiplicity, mMetaboliteName.c_str());
  }
