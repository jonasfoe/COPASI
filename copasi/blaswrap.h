/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/blaswrap.h,v $
   $Revision: 1.3 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/02/17 15:52:47 $
   End CVS Header */

#ifndef __BLAS_H
#define __BLAS_H

#ifdef min
# undef min
#endif // min

#ifdef max
# undef max
#endif // max

extern "C"
  {
#ifdef USE_MKL
# include "mkl_lapack.h"
#endif // USE_MKL

#if (defined USE_CLAPACK || defined USE_LAPACK)
# ifdef USE_CLAPACK
#  define daxpy_ f2c_daxpy
#  define ddot_ f2c_ddot
#  define dgemm_ f2c_dgemm
#  define dscal_ f2c_dscal
#  define idamax_ f2c_idamax
# endif // USE_CLAPACK
# include "f2c.h"
# include "blas.h"
#endif // USE_CLAPACK || USE_LAPACK

#ifdef Darwin
# define vector
# include "cblas.h"
# define dgemm_(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC) \
    cblas_dgemm(CblasColMajor, \
                (* TRANSA == 'N' ? CblasNoTrans: CblasTrans), \
                (* TRANSB == 'N' ? CblasNoTrans: CblasTrans), \
                * M, * N, * K, * ALPHA, A, * LDA, B, * LDB, * BETA, C, * LDC)
# define vector vector
#endif // Darwin
  }

#ifdef min
# undef min
#endif // min

#ifdef max
# undef max
#endif // max

#ifdef WIN32
# define min _cpp_min
# define max _cpp_max
#endif // WIN32

#endif // __BLAS_H
