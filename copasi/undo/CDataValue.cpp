// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

#include "copasi.h"

#include "CDataValue.h"
#include "CData.h"

CDataValue::CDataValue(const Type & type):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  allocateData(type);
}

CDataValue::CDataValue(const CDataValue & src):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(src);

  assert(mType == src.mType);
}

CDataValue::CDataValue(const C_FLOAT64 & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const C_INT32 & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const unsigned C_INT32 & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const size_t & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const bool & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const std::string & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const char * value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(std::string(value));
}

CDataValue::CDataValue(const CData & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const void * pVoidPointer):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(pVoidPointer);
}

CDataValue::CDataValue(const std::vector< CDataValue > & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::CDataValue(const std::vector< CData > & value):
  mType(CDataValue::INVALID),
  mpData(NULL)
{
  assignData(value);
}

CDataValue::~CDataValue()
{
  deleteData();
}

CDataValue & CDataValue::operator = (const CDataValue & rhs)
{
  if (this != &rhs)
    {
      assignData(rhs);
    }

  return *this;
}

CDataValue & CDataValue::operator = (const C_FLOAT64 & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const C_INT32 & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const unsigned C_INT32 & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const size_t & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const bool & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const std::string & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const CData & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const std::vector< CDataValue > & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const std::vector< CData > & value)
{
  assignData(value);

  return *this;
}

CDataValue & CDataValue::operator = (const void * pVoidPointer)
{
  assignData(pVoidPointer);

  return *this;
}

const C_FLOAT64 & CDataValue::toDouble() const
{
  static const C_FLOAT64 Invalid(std::numeric_limits< C_FLOAT64 >::quiet_NaN());

  if (mType != DOUBLE) return Invalid;

  return *static_cast< const C_FLOAT64 * >(mpData);
}

const C_INT32 & CDataValue::toInt() const
{
  static const C_INT32 Invalid(0);

  if (mType != INT)
    return Invalid;

  return *static_cast< const C_INT32 * >(mpData);
}

const unsigned C_INT32 & CDataValue::toUint() const
{
  static const unsigned C_INT32 Invalid(std::numeric_limits< unsigned C_INT32 >::max());

  if (mType != UINT)
    return Invalid;

  return *static_cast< const unsigned C_INT32 * >(mpData);
}

const size_t & CDataValue::toSizeT() const
{
  static const size_t Invalid(std::numeric_limits< size_t >::max());

  if (mType != SIZE_T)
    return Invalid;

  return *static_cast< const size_t * >(mpData);
}

const bool & CDataValue::toBool() const
{
  static const bool Invalid(false);

  if (mType != BOOL)
    return Invalid;

  return *static_cast< const bool * >(mpData);
}

const std::string & CDataValue::toString() const
{
  static const std::string Invalid("");

  if (mType != STRING)
    return Invalid;

  return *static_cast< const std::string * >(mpData);
}

const CData & CDataValue::toData() const
{
  static const CData Invalid;

  if (mType != DATA)
    return Invalid;

  return *static_cast< const CData * >(mpData);
}

const std::vector< CDataValue > & CDataValue::toDataValues() const
{
  static const std::vector< CDataValue > Invalid(0);

  if (mType != DATA_VALUES)
    return Invalid;

  return *static_cast< const std::vector< CDataValue > * >(mpData);
}

const std::vector< CData > & CDataValue::toDataVector() const
{
  static const std::vector< CData > Invalid(0);

  if (mType != DATA_VECTOR)
    return Invalid;

  return *static_cast< const std::vector< CData > * >(mpData);
}

const void * CDataValue::toVoidPointer() const
{
  return (mType != VOID_POINTER) ? NULL : mpData;
}

const CDataValue::Type & CDataValue::getType() const
{
  return mType;
}

bool CDataValue::operator == (const CDataValue & rhs) const
{
  if (mType != rhs.mType) return false;

  switch (mType)
    {
      case DOUBLE:
        return (toDouble() == rhs.toDouble());
        break;

      case INT:
        return (toInt() == rhs.toInt());
        break;

      case UINT:
        return (toUint() == rhs.toUint());
        break;

      case SIZE_T:
        return (toSizeT() == rhs.toSizeT());
        break;

      case BOOL:
        return (toBool() == rhs.toBool());
        break;

      case STRING:
        return (toString() == rhs.toString());
        break;

      case DATA:
        return (toData() == rhs.toData());
        break;

      case DATA_VALUES:
        return (toDataValues() == rhs.toDataValues());
        break;

      case DATA_VECTOR:
        return (toDataVector() == rhs.toDataVector());
        break;

      case VOID_POINTER:
        return (toVoidPointer() == rhs.toVoidPointer());
        break;

      case INVALID:
        return false;
        break;
    }

  return false;
}

bool CDataValue::operator != (const CDataValue & rhs) const
{
  if (mType != rhs.mType) return true;

  switch (mType)
    {
      case DOUBLE:
        return (toDouble() != rhs.toDouble());
        break;

      case INT:
        return (toInt() != rhs.toInt());
        break;

      case UINT:
        return (toUint() != rhs.toUint());
        break;

      case SIZE_T:
        return (toSizeT() != rhs.toSizeT());
        break;

      case BOOL:
        return (toBool() != rhs.toBool());
        break;

      case STRING:
        return (toString() != rhs.toString());
        break;

      case DATA:
        return (toData() != rhs.toData());
        break;

      case DATA_VALUES:
        return (toDataValues() != rhs.toDataValues());
        break;

      case DATA_VECTOR:
        return (toDataVector() != rhs.toDataVector());
        break;

      case VOID_POINTER:
        return (toVoidPointer() != rhs.toVoidPointer());
        break;

      case INVALID:
        return true;
        break;
    }

  return true;
}

void CDataValue::allocateData(const CDataValue::Type & type)
{
  if (type == mType) return;

  deleteData();

  mType = type;

  switch (mType)
    {
      case DOUBLE:
        mpData = new C_FLOAT64;
        break;

      case INT:
        mpData = new C_INT32;
        break;

      case UINT:
        mpData = new unsigned C_INT32;
        break;

      case SIZE_T:
        mpData = new size_t;
        break;

      case BOOL:
        mpData = new bool;
        break;

      case STRING:
        mpData = new std::string;
        break;

      case DATA:
        mpData = new CData;
        break;

      case DATA_VALUES:
        mpData = new std::vector< CDataValue >;
        break;

      case DATA_VECTOR:
        mpData = new std::vector< CData >;
        break;

      case VOID_POINTER:
        mpData = NULL;
        break;

      case INVALID:
        mpData = NULL;
        break;
    }

  return;
}

void CDataValue::deleteData()
{
  if (mpData == NULL) return;

  switch (mType)
    {
      case DOUBLE:
        delete static_cast< C_FLOAT64 * >(mpData);
        break;

      case INT:
        delete static_cast< C_INT32 * >(mpData);
        break;

      case UINT:
        delete static_cast< unsigned C_INT32 * >(mpData);
        break;

      case SIZE_T:
        delete static_cast< size_t * >(mpData);
        break;

      case BOOL:
        delete static_cast< bool * >(mpData);
        break;

      case STRING:
        delete static_cast< std::string * >(mpData);
        break;

      case DATA:
        delete static_cast< CData * >(mpData);
        break;

      case DATA_VALUES:
        delete static_cast< std::vector< CDataValue > * >(mpData);
        break;

      case DATA_VECTOR:
        delete static_cast< std::vector< CData > * >(mpData);
        break;

      case VOID_POINTER:
      case INVALID:
        break;
    }

  mpData = NULL;
}

void CDataValue::assignData(const CDataValue & rhs)
{
  switch (rhs.mType)
    {
      case DOUBLE:
        assignData(*static_cast< C_FLOAT64 * >(rhs.mpData));
        break;

      case INT:
        assignData(*static_cast< C_INT32 * >(rhs.mpData));
        break;

      case UINT:
        assignData(*static_cast< unsigned C_INT32 * >(rhs.mpData));
        break;

      case SIZE_T:
        assignData(*static_cast< size_t * >(rhs.mpData));
        break;

      case BOOL:
        assignData(*static_cast< bool * >(rhs.mpData));
        break;

      case STRING:
        assignData(*static_cast< std::string * >(rhs.mpData));
        break;

      case DATA:
        assignData(*static_cast< CData * >(rhs.mpData));
        break;

      case DATA_VALUES:
        assignData(*static_cast< std::vector< CDataValue > * >(rhs.mpData));
        break;

      case DATA_VECTOR:
        assignData(*static_cast< std::vector< CData > * >(rhs.mpData));
        break;

      case VOID_POINTER:
        assignData(rhs.mpData);
        break;

      case INVALID:
        break;
    }
}

void CDataValue::assignData(const C_FLOAT64 & value)
{
  allocateData(DOUBLE);

  *static_cast< C_FLOAT64 * >(mpData) = value;
}

void CDataValue::assignData(const C_INT32 & value)
{
  allocateData(INT);

  *static_cast< C_INT32 * >(mpData) = value;
}

void CDataValue::assignData(const unsigned C_INT32 & value)
{
  allocateData(UINT);

  *static_cast< unsigned C_INT32 * >(mpData) = value;
}

void CDataValue::assignData(const size_t & value)
{
  allocateData(SIZE_T);

  *static_cast< size_t * >(mpData) = value;
}

void CDataValue::assignData(const bool & value)
{
  allocateData(BOOL);

  *static_cast< bool * >(mpData) = value;
}

void CDataValue::assignData(const std::string & value)
{
  allocateData(STRING);

  *static_cast< std::string * >(mpData) = value;
}

void CDataValue::assignData(const CData & value)
{
  allocateData(DATA);

  *static_cast< CData * >(mpData) = value;
}

void CDataValue::assignData(const std::vector< CDataValue > & value)
{
  allocateData(DATA_VALUES);

  *static_cast< std::vector< CDataValue > * >(mpData) = value;
}

void CDataValue::assignData(const std::vector< CData > & value)
{
  allocateData(DATA_VECTOR);

  *static_cast< std::vector< CData > * >(mpData) = value;
}

void CDataValue::assignData(const void * pVoidPointer)
{
  allocateData(VOID_POINTER);

  mpData = const_cast< void * >(pVoidPointer);
}

std::ostream & operator << (std::ostream & os, const CDataValue & o)
{
  switch (o.mType)
    {
      case CDataValue::DOUBLE:
        os << o.toDouble();
        break;

      case CDataValue::INT:
        os << o.toInt();
        break;

      case CDataValue::UINT:
        os << o.toUint();
        break;

      case CDataValue::SIZE_T:
        os << o.toSizeT();
        break;

      case CDataValue::BOOL:
        os << o.toBool();
        break;

      case CDataValue::STRING:
        os << o.toString();
        break;

      case CDataValue::DATA:
        os << std::endl << o.toData();
        break;

      case CDataValue::DATA_VALUES:
      {
        std::vector< CDataValue >::const_iterator it = o.toDataValues().begin();
        std::vector< CDataValue >::const_iterator end = o.toDataValues().end();

        for (; it != end; ++it)
          os << std::endl << *it;
      }
      break;

      case CDataValue::DATA_VECTOR:
      {
        std::vector< CData >::const_iterator it = o.toDataVector().begin();
        std::vector< CData >::const_iterator end = o.toDataVector().end();

        for (; it != end; ++it)
          os << std::endl << *it;
      }
      break;

      case CDataValue::VOID_POINTER:
        os << o.toVoidPointer();
        break;

      case CDataValue::INVALID:
        os << "??? Invalid ???";
        break;
    }

  return os;
}
