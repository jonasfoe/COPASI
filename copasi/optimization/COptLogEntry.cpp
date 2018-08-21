// Copyright (C) 2018 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

/**
 *  COptLogEntry class
 *  Container for a log message created by the copasi optimization methods.
 *  Stores various variable for later compilation of a message string.
 *
 *  Written by Pedro Mendes, based on earlier code by Jonas F�rster
 */

#include "COptLogEntry.h"
#include "utilities/utility.h"

// Constructor
COptLogEntry::COptLogEntry(std::string &header, std::string &subtext = "", std::string &status = ""):
  MsgSubtext(subtext),
  StatusDump(status)
{
  MsgHeader = LocalTimeStamp();
  MsgHeader.append(": ");
  MsgHeader.append(header);
}

COptLogEntry::~COptLogEntry()
{}

std::string COptLogEntry::getHeader() const
{
  return MsgHeader;
}

std::string COptLogEntry::getSubtext() const
{
  return MsgSubtext;
}

std::string COptLogEntry::getStatusDetails() const
{
  return StatusDump;
}
