#ifndef JLC_AST_HH_
#define JLC_AST_HH_

#include <iostream>
#include <vector>
#include <memory>
#include <utility>

#include "types.hh"
#include "operators.hh"

class AST {
 public:
  virtual ~AST() {}
};

class Inst : public AST {
};

class Exp : public AST {
 public:
  virtual Type GetType() const = 0;
};

struct InstBlock : Inst {
  bool has_return;
  std::vector<std::unique_ptr<Inst>> instructions;

  void Add(std::unique_ptr<Inst> && i) {
    instructions.push_back(std::move(i));
  }
};

struct InstIf : Inst {
  std::unique_ptr<Exp> test;
  std::unique_ptr<Inst> if_inst, else_inst;
};

struct InstFor : Inst {
  std::unique_ptr<Exp> test;
  std::unique_ptr<Inst> pre_inst, body, post_inst;
};

struct InstWhile : Inst {
  std::unique_ptr<Exp> test;
  std::unique_ptr<Inst> body;
};

struct InstReturn : Inst {
  std::unique_ptr<Exp> exp;
};

struct InstAssign : Inst {
};

struct InstAssignExp : InstAssign {
  std::unique_ptr<Exp> exp;
};

struct InstAssignIncDec : InstAssign {
  Op op;
};

struct InstExp : Inst {
  InstExp(std::unique_ptr<Exp> && e) : exp(std::move(e)) {};
  std::unique_ptr<Exp> exp;
};

template <typename T>
class Literal : public Exp {
 public:
  explicit Literal(const T & v) : value(v)
  {
  }

  Type GetType() const {
    return LiteralToBasicType<T>::type::value;
  }

 private:
  T value;
};

struct UnaryExp : Exp {
  Type GetType() const {
    return type;
  }

  Type type;
  Op op;
  std::unique_ptr<Exp> exp;
};

struct BinaryExp : Exp {
  Type GetType() const {
    return type;
  }

  Type type;
  Op op;
  std::unique_ptr<Exp> lhs, rhs;
};

struct FunDef : Inst {
  std::unique_ptr<InstBlock> body;
};

class FunCall : public Exp {
 public:
  Type GetType() const {
    return type;
  }

  std::vector<std::unique_ptr<Exp>> args;
  Type type;
};

class VarRef : public Exp {
 public:
  Type GetType() const {
    return type;
  }
  Type type;
};

#endif // JLC_AST_HH_
