/* Begin CVS Header
 $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/odepack++/Cxerrwd.cpp,v $
 $Revision: 1.6 $
 $Name:  $
 $Author: shoops $
 $Date: 2009/01/07 19:01:06 $
 End CVS Header */

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.
//
// This C++ code is based on an f2c conversion of the Fortran
// library ODEPACK available at: http://www.netlib.org/odepack/

#include "copasi.h"

#include "Cxerrwd.h"

Cxerrwd::Cxerrwd(const bool & print):
    mPrint(print),
    mpOstream(NULL)
{}

Cxerrwd::~Cxerrwd() {}

void Cxerrwd::setOstream(std::ostream & os)
{mpOstream = &os;}

void Cxerrwd::enablePrint(const bool & print)
{mPrint = print;}

void Cxerrwd::operator() (const std::string & msg, const C_INT *, const C_INT *, const C_INT
                          *level, const C_INT *ni, const C_INT *i1, const C_INT *i2, const C_INT *nr,
                          const double *r1, const double *r2, C_INT)
{
  /* ***BEGIN PROLOGUE  XERRWD */
  /* ***SUBSIDIARY */
  /* ***PURPOSE  Write error message with values. */
  /* ***CATEGORY  R3C */
  /* ***TYPE      DOUBLE PRECISION (XERRWV-S, XERRWD-D) */
  /* ***AUTHOR  Hindmarsh, Alan C., (LLNL) */
  /* ***DESCRIPTION */

  /*  Subroutines XERRWD, XSETF, XSETUN, and the function routine IXSAV, */
  /*  as given here, constitute a simplified version of the SLATEC error */
  /*  handling package. */

  /*  All arguments are input arguments. */

  /*  MSG    = The message (character array). */
  /*  NMES   = The length of MSG (number of characters). */
  /*  NERR   = The error number (not used). */
  /*  LEVEL  = The error level.. */
  /*           0 or 1 means recoverable (control returns to caller). */
  /*           2 means fatal (run is aborted--see note below). */
  /*  NI     = Number of integers (0, 1, or 2) to be printed with message. */
  /*  I1,I2  = Integers to be printed, depending on NI. */
  /*  NR     = Number of reals (0, 1, or 2) to be printed with message. */
  /*  R1,R2  = Reals to be printed, depending on NR. */

  /*  Note..  this routine is machine-dependent and specialized for use */
  /*  in limited context, in the following ways.. */
  /*  1. The argument MSG is assumed to be of type CHARACTER, and */
  /*     the message is printed with a format of (1X,A). */
  /*  2. The message is assumed to take only one line. */
  /*     Multi-line messages are generated by repeated calls. */
  /*  3. If LEVEL = 2, control passes to the statement   STOP */
  /*     to abort the run.  This statement may be machine-dependent. */
  /*  4. R1 and R2 are assumed to be in double precision and are printed */
  /*     in D21.13 format. */

  /* ***ROUTINES CALLED  IXSAV */
  /* ***REVISION HISTORY  (YYMMDD) */
  /*   920831  DATE WRITTEN */
  /*   921118  Replaced MFLGSV/LUNSAV by IXSAV. (ACH) */
  /*   930329  Modified prologue to SLATEC format. (FNF) */
  /*   930407  Changed MSG from CHARACTER*1 array to variable. (FNF) */
  /*   930922  Minor cosmetic change. (FNF) */
  /* ***END PROLOGUE  XERRWD */

  /* *Internal Notes: */

  /* For a different default logical unit number, IXSAV (or a subsidiary */
  /* routine that it calls) will need to be modified. */
  /* For a different run-abort command, change the statement following */
  /* statement 100 at the end. */
  /* ----------------------------------------------------------------------- */
  /* Subroutines called by XERRWD.. None */
  /* Function routine called by XERRWD.. IXSAV */
  /* ----------------------------------------------------------------------- */
  /* **End */

  /*  Declare arguments. */

  /*  Declare local variables. */

  /*  Get logical unit number and message print flag. */

  /* ***FIRST EXECUTABLE STATEMENT  XERRWD */
  if (!mPrint && !mpOstream)
    {
      goto L100;
    }

  /*  Write the message. */

  *mpOstream << msg << std::endl;

  if (*ni == 1)
    {
      *mpOstream << "\tIn above message, I1 = '" << *i1 << "'\n";
    }
  if (*ni == 2)
    {
      *mpOstream << "\tIn above message, I1 = '" << *i1
      << "', I2 = '" << *i2 << "'\n";
    }
  if (*nr == 1)
    {
      *mpOstream << "\tIn above message, R1 = '" << *r1 << "'\n";
    }
  if (*nr == 2)
    {
      *mpOstream << "\tIn above message, R1 = '" << *r1
      << "', R2 = '" << *r2 << "'\n";
    }

  /*  Abort the run if LEVEL = 2. */
  if (*level == 2)
    {
      // :TODO: We need to abort here
    }

L100:

  /* ----------------------- End of Subroutine XERRWD ---------------------- */
  return;
} /* xerrwd_ */