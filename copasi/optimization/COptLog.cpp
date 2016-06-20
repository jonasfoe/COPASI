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

unsigned C_INT32 COptLog::getElementCount() const
{
  return mLogItems.size();
}

std::string COptLog::getPlainLog() const
{
  std::string log;
  std::string currSubtext;
  std::string currStatusDetails;

  for (std::vector<COptLogItem>::const_iterator item = mLogItems.begin(); item != mLogItems.end(); ++item)
    {
      log.append(item->getHeader() + "\n");

      currSubtext = item->getSubtext();
      if (!currSubtext.empty())
        {
          log.append(currSubtext + "\n");
        }

      currStatusDetails = item->getStatusDetails();
      if (!currStatusDetails.empty())
        {
          log.append(currStatusDetails +"\n");
        }

      log.append("\n");
    }

  return log;
}

std::string COptLog::getRichLog() const
{
  std::string log;
  std::string currSubtext;
  std::string currStatusDetails;
  bool currHasContent; //if an item has no content, the div has to be closed without any linebreaks in between. Otherwise Webkit detects the linebreak as content.

  for (std::vector<COptLogItem>::const_iterator item = mLogItems.begin(); item != mLogItems.end(); ++item)
    {
      currHasContent = false;

      log.append("<h4>" + item->getHeader() + "</h4>\n");

      log.append("<div>");

      currSubtext = item->getSubtext();
      if (!currSubtext.empty())
        {
          currHasContent = true;
          log.append("\n<div class=\"content-set\">\n" + currSubtext + "\n</div>");
        }

      currStatusDetails = item->getStatusDetails();
      if (!currStatusDetails.empty())
        {
          currHasContent = true;
          log.append("\n<div class=\"content-set\">\n" + currStatusDetails + "\n</div>");
        }

      if (currHasContent)
        log.append("\n");

      log.append("</div>\n");
    }

  return log;
}
