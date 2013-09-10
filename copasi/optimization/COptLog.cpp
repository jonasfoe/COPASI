/**
 *  COptLog class
 *  Container for a log of events registed by the copasi optimization methods.
 *  Stores a list of COptLogItems and has methods for generation of a complete log string.
 *
 *  Created for COPASI by Jonas Förster 2013
 */

#include "COptLog.h"

// Default constructor
COptLog::COptLog()
{}

COptLog::~COptLog()
{}

void COptLog::enterLogItem(COptLogItem item)
{
  mLogItems.push_back(item);
}

std::string COptLog::getPlainLog() const
{
  std::string log;

  for (std::vector<COptLogItem>::const_iterator item = mLogItems.begin(); item != mLogItems.end(); ++item)
    {
      log.append(item->getPlainMessage());
    }

  return log;
}

std::string COptLog::getRichLog() const
{
  std::string log;

  for (std::vector<COptLogItem>::const_iterator item = mLogItems.begin(); item != mLogItems.end(); ++item)
    {
      log.append(item->getRichMessage());
    }

  return log;
}
