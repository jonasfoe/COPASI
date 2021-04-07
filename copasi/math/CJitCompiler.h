// Copyright (C) 2020 - 2021 by Pedro Mendes, Rector and Visitors of the
// University of Virginia, University of Heidelberg, and University
// of Connecticut School of Medicine.
// All rights reserved.

#ifndef COPASI_CJitCompiler
#define COPASI_CJitCompiler

#include <set>

#include <NativeJIT/CodeGen/ExecutionBuffer.h>
#include <NativeJIT/Function.h>

#ifdef ERROR
#  undef ERROR
#endif

#ifdef max
#  undef max
#endif
#include "copasi/core/CCore.h"

class CJitExpression;
class CMathExpression;
class CEvaluationNodeNumber;
class CEvaluationNodeConstant;
class CEvaluationNodeOperator;
class CEvaluationNodeObject;
class CEvaluationNodeFunction;
class CEvaluationNodeChoice;
class CEvaluationNodeLogical;

class CJitCompiler
{
public:
  typedef NativeJIT::Function< C_FLOAT64 >::FunctionType Function;
  typedef NativeJIT::NodeBase Node;

  static bool JitEnabled();

  CJitCompiler();

  CJitCompiler(const CJitCompiler & src);

  virtual ~CJitCompiler();

  bool compile();

  Function compile(const CMathExpression & mathExpression);

  bool allocateExecutionBuffer(const size_t & size);

  bool allocateFunctionBuffer(const size_t & size);

  void release();

  void registerExpression(CJitExpression * pExpression);

  void deregisterExpression(CJitExpression * pExpression);

private:
  typedef C_FLOAT64(*F0)();
  typedef C_FLOAT64(*F1)(C_FLOAT64);
  typedef C_FLOAT64(*F2)(C_FLOAT64, C_FLOAT64);
  typedef C_FLOAT64(*Fv)(C_FLOAT64, C_FLOAT64, C_FLOAT64, C_FLOAT64);

  typedef bool (*B2B)(bool, bool);
  typedef bool (*B2F)(C_FLOAT64, C_FLOAT64);

  /**
   * A pointer to a bool indicating whether the CPU supports the sse4.2 instruction set
   * required for JIT compilation
   */
  static bool * pSSE4support;

  static std::string where(std::runtime_error & err);

  static inline C_FLOAT64 __jit_divide(C_FLOAT64 x, C_FLOAT64 y) {return x / y;}
  static inline C_FLOAT64 __jit_modulus(C_FLOAT64 x, C_FLOAT64 y)
  {
    if ((C_INT32) y == 0)
      return std::numeric_limits< C_FLOAT64 >::quiet_NaN();

    return (C_FLOAT64)(((C_INT32) x) % ((C_INT32) y));
  }
  static inline bool __jit_xor(bool x, bool y) {return (x || y) && (x != y);}
  static inline bool __jit_eq(bool x, bool y) {return x == y;}
  static inline bool __jit_eq(C_FLOAT64 x, C_FLOAT64 y) {return x == y;}
  static inline bool __jit_neq(bool x, bool y) {return x != y;}
  static inline bool __jit_neq(C_FLOAT64 x, C_FLOAT64 y) {return x != y;}
  static inline bool __jit_lt(C_FLOAT64 x, C_FLOAT64 y) {return x < y;}
  static inline bool __jit_le(C_FLOAT64 x, C_FLOAT64 y) {return x <= y;}
  static inline bool __jit_gt(C_FLOAT64 x, C_FLOAT64 y) {return x > y;}
  static inline bool __jit_ge(C_FLOAT64 x, C_FLOAT64 y) {return x >= y;}

  Node * compile(const CEvaluationNodeNumber * pNode);
  Node * compile(const CEvaluationNodeConstant * pNode);
  Node * compile(const CEvaluationNodeOperator * pNode, const std::vector< Node * > & context);
  Node * compile(const CEvaluationNodeObject * pNode);
  Node * compile(const CEvaluationNodeFunction * pNode, const std::vector< Node * > & context);
  Node * compile(const CEvaluationNodeChoice * pNode, const std::vector< Node * > & context);
  Node * compile(const CEvaluationNodeLogical * pNode, const std::vector< Node * > & context);

  inline NativeJIT::Node< C_FLOAT64 > & fromBool(NativeJIT::Node< bool > & boolean)
  {
    return mpExpression->If(boolean, mpExpression->Immediate(1.0), mpExpression->Immediate(0.0));
  }

  /**
   * Allocator for all the JIT compiled expressions
   */
  NativeJIT::Allocator * mpAllocator;

  /**
   * Execution buffer for all the JIT compiled expressions
   */
  NativeJIT::ExecutionBuffer * mpExecutionBuffer;

  /**
   * The currently compiled JIT expressions
   */
  NativeJIT::Function< C_FLOAT64 > * mpExpression;

  /**
   * A set of a compilet expression
   */
  std::set< CJitExpression * > mExpressions;

  /**
   * Execution buffer size
   */
  size_t mExecutionBufferSize;

  /**
   * Execution buffer size
   */
  size_t mFunctionBufferSize;
};

#endif // COPASI_CJitCompiler
