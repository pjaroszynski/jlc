#ifndef JLC_COMPILER_HH_
#define JLC_COMPILER_HH_

#include <memory>

#include <boost/spirit/include/support_utree.hpp>

#include "util.hh"

#include "ast.hh"
#include "tags.hh"
#include "operators.hh"
#include "types.hh"
#include "symbols.hh"
#include "exception.hh"

const bool kDebug = false;

using boost::spirit::utree;
using boost::spirit::utree_type;

template <class Tags>
struct Compiler {
  Symbols symbols;
  Tags & tags;
  Symbol current_function;
  bool has_return;

  Compiler(Tags & t) : tags(t) {
    symbols.Add(Symbol{basic_type::void_, "printInt", basic_type::int_});
    symbols.Add(Symbol{basic_type::void_, "printString", basic_type::string_});
    symbols.Add(Symbol{basic_type::void_, "printDouble", basic_type::double_});
    symbols.Add(Symbol{basic_type::void_, "error"}.function());
    symbols.Add(Symbol{basic_type::int_, "readInt"}.function());
    symbols.Add(Symbol{basic_type::double_, "readDouble"}.function());
  }


  std::string GetSymbol(utree & u) {
    if (u.which() != utree_type::symbol_type)
      throw ExpectedIdentifier(u, tags);

    using boost::spirit::utf8_symbol_range_type;

    utf8_symbol_range_type symbol = u.get<utf8_symbol_range_type>();
    return std::string(symbol.begin(), symbol.end());
  }

  std::string GetString(utree & u) {
    using boost::spirit::utf8_string_range_type;

    utf8_string_range_type string = u.get<utf8_string_range_type>();
    return std::string(string.begin(), string.end());
  }

  Type GetType(utree & u) {
    if (u.which() != utree_type::int_type)
      throw ExpectedType(u, tags);

    return u.get<int>();
  }

  Op GetOp(utree & u) {
    if (u.which() != utree_type::int_type)
      throw ExpectedOp(u, tags);

    return u.get<int>();
  }

  CodeTag Tag(utree & u) {
    if (tags.Tagged(u))
      return tags[u].code_tag;
    else
      return CodeTag::none;
  }

  template <typename T>
  std::unique_ptr<Literal<T>> Lit(utree & u) {
    return make_unique_ptr(new Literal<T>(u.get<T>()));
  }

  std::unique_ptr<Literal<std::string>> LitString(utree & u) {
    return make_unique_ptr(new Literal<std::string>(GetString(u)));
  }

  std::unique_ptr<Exp> LitDispatch(utree & u) {
    switch (u.which()) {
      case utree_type::bool_type:
        return Lit<bool>(u);
      case utree_type::int_type:
        return Lit<int>(u);
      case utree_type::double_type:
        return Lit<double>(u);
      case utree_type::string_type:
        return LitString(u);
      default:
        throw CompilerError();
    }
  }

  std::unique_ptr<UnaryExp> UnaryExpression(utree & u) {
    auto exp = new UnaryExp;

    Op o = GetOp(u[0]);
    exp->exp = Expression(u[1]);
    exp->type = exp->exp->GetType();

    if (o == op::not_) {
      if (exp->type != basic_type::boolean_)
        throw IncompatibleUnaryExpArgument(u, tags);
    } else if (exp->type != basic_type::double_ && exp->type != basic_type::int_) {
      throw IncompatibleUnaryExpArgument(u, tags);
    }

    return make_unique_ptr(exp);
  }

  std::unique_ptr<BinaryExp> BinaryExpression(utree & u) {
    auto exp = new BinaryExp;

    Op o = GetOp(u[1]);
    exp->lhs = Expression(u[0]);
    exp->rhs = Expression(u[2]);

    Type t = exp->lhs->GetType();

    if (t != exp->rhs->GetType())
      throw IncompatibleBinaryExpArguments(u, tags);

    // == and != accept both
    if (! (op::NumericArgs(o) && op::BooleanArgs(o))) {
      if (op::NumericArgs(o)) {
        if (t != basic_type::double_ && t != basic_type::int_)
          throw IncompatibleBinaryExpArguments(u, tags);
      } else {
        if (t != basic_type::boolean_)
          throw IncompatibleBinaryExpArguments(u, tags);
      }
    }

    if (op::NumericResult(o))
      exp->type = t;
    else
      exp->type = basic_type::boolean_;

    return make_unique_ptr(exp);
  }

  std::unique_ptr<Exp> Expression(utree & u) {
    if (kDebug)
      std::cerr << "exp:" << tags[u].line_pos << ": " << u << "\n";
    if (u[0].which() == utree_type::symbol_type) {
      // function call or variable reference
      switch (u.size()) {
        case 1:
          return VariableRef(u);
        default:
          return FunctionCall(u);
      }
    } else {
      // literal, unary or binary expression
      switch (u.size()) {
        case 1:
          return LitDispatch(u[0]);
        case 2:
          return UnaryExpression(u);
        case 3:
          return BinaryExpression(u);
        default:
          throw CompilerError();
      }
    }
  }

  std::unique_ptr<VarRef> VariableRef(utree & u) {
    std::string name = GetSymbol(u[0]);
    if (! symbols.Defined(name))
      throw UndefinedVariable(u, tags);
    auto var = new VarRef;
    var->type = symbols[name].sig[0];
    return make_unique_ptr(var);
  }

  std::unique_ptr<FunCall> FunctionCall(utree & u) {
    std::string name = GetSymbol(u[0]);
    if (kDebug)
      std::cerr << "funcall? " << u << "\n";
    if (! symbols.Defined(name))
      throw UndefinedFunction(u, tags);
    Symbol fsymbol = symbols[name];
    if (fsymbol.args == -1)
      throw NotAFunction(u, tags);
    if ((int)u[1].size() != fsymbol.args)
      throw BadArgumentCount(u, tags);
    std::vector<std::unique_ptr<Exp>> args;
    for (auto i = u[1].begin() ; i != u[1].end() ; ++i) {
      args.push_back(Expression(*i));
    }
    for (size_t i = 1 ; i < fsymbol.sig.size() ; ++i)
      if (fsymbol.sig[i] != args[i-1]->GetType())
        throw BadArgumentType(u, tags);
    auto fun = new FunCall;
    fun->type = symbols[name].sig[0];
    return make_unique_ptr(fun);
  }

  void FunctionDeclaration(utree & u) {
    Symbol fun(GetType(u[0]), GetSymbol(u[1]));
    if (symbols.InContext(fun.name))
      throw AlreadyDeclared(u, tags);
    fun.args = u[2].size();

    for (auto i = u[2].begin() ; i != u[2].end() ; ++i) {
      auto arg = *i;
      fun.sig.push_back(GetType(arg[0]));
    }
    symbols.Add(fun);
  }

  std::unique_ptr<FunDef> FunctionDefinition(utree & u) {
    auto fundef = new FunDef;
    symbols.BeginContext();
    for (auto i = u[2].begin() ; i != u[2].end() ; ++i) {
      auto arg = *i;
      symbols.Add(Symbol(GetType(arg[0]), GetSymbol(arg[1])));
    }
    current_function = symbols[GetSymbol(u[1])];
    has_return = false;
    fundef->body = InstructionBlock(u[3]);
    symbols.EndContext();
    if (! has_return)
      throw NoReturn(u, tags);
    return make_unique_ptr(fundef);
  }

  std::unique_ptr<Inst> Instruction(utree & u) {
    switch (Tag(u)) {
      case CodeTag::inst_block:
        return InstructionBlock(u);
      case CodeTag::inst_if:
        return InstructionIf(u);
      case CodeTag::inst_for:
        return InstructionFor(u);
      case CodeTag::inst_while:
        return InstructionWhile(u);
      case CodeTag::inst_ret:
        return InstructionReturn(u);
      case CodeTag::inst_assign:
        return InstructionAssign(u);
      case CodeTag::inst_decl:
        return InstructionDecl(u);
      case CodeTag::fun_decl:
        return FunctionDefinition(u);
      case CodeTag::exp:
        return make_unique_ptr(new InstExp(Expression(u)));
      case CodeTag::none:
      default:
        throw CompilerError();
    }
    return std::unique_ptr<Inst>(0);
  }

  std::unique_ptr<InstBlock> InstructionBlock(utree & u) {
    symbols.BeginContext();
    for (auto i = u.begin() ; i != u.end() ; ++i)
      if (Tag(*i) == CodeTag::fun_decl)
        FunctionDeclaration(*i);

    auto block = new InstBlock;
    for (auto i = u.begin() ; i != u.end() ; ++i)
      block->Add(Instruction(*i));
    symbols.EndContext();

    return make_unique_ptr(block);
  }

  std::unique_ptr<InstIf> InstructionIf(utree & u) {
    if (kDebug)
      std::cerr << "if:" << tags[u].line_pos << ": " << u << "\n";
    auto inst = new InstIf;
    inst->test = Expression(u[1]);
    inst->if_inst = Instruction(u[2]);
    if (u.size() == 5)
      inst->else_inst = Instruction(u[4]);
    return make_unique_ptr(inst);
  }

  std::unique_ptr<InstFor> InstructionFor(utree & u) {
    if (kDebug)
      std::cerr << "for:" << tags[u].line_pos << ": " << u << "\n";
    auto inst = new InstFor;
    inst->test = Expression(u[2]);
    inst->pre_inst = InstructionAssign(u[1]);
    inst->post_inst = InstructionAssign(u[3]);
    inst->body = Instruction(u[4]);
    return make_unique_ptr(inst);
  }

  std::unique_ptr<InstWhile> InstructionWhile(utree & u) {
    if (kDebug)
      std::cerr << "while:" << tags[u].line_pos << ": " << u << "\n";
    auto inst = new InstWhile;
    inst->test = Expression(u[1]);
    inst->body = Instruction(u[2]);
    return make_unique_ptr(inst);
  }

  std::unique_ptr<InstReturn> InstructionReturn(utree & u) {
    if (kDebug)
      std::cerr << "return:" << tags[u].line_pos << ": " << u << "\n";
    auto inst = new InstReturn;

    Type ret_type = basic_type::void_;
    if (u.size() > 1) {
      inst->exp = Expression(u[1]);
      ret_type = inst->exp->GetType();
    }

    if (current_function.sig[0] != ret_type)
      throw BadReturnType(u, tags);

    has_return = true;

    return make_unique_ptr(inst);
  }

  std::unique_ptr<InstAssign> InstructionAssign(utree & u) {
    if (kDebug)
      std::cerr << "assign:" << tags[u].line_pos << ": " << u << "\n";

    std::string name = GetSymbol(u[0]);
    if (! symbols.Defined(name))
      throw UndefinedVariable(u, tags);

    if (u.size() == 3)
      return InstructionAssignExp(u);
    else
      return InstructionAssignIncDec(u);
  }

  std::unique_ptr<InstAssignExp> InstructionAssignExp(utree & u) {
    auto inst = new InstAssignExp;

    Symbol var = symbols[GetSymbol(u[0])];
    inst->exp = Expression(u[2]);

    if (var.sig[0] != inst->exp->GetType())
      throw BadAssignExpType(u, tags);

    return make_unique_ptr(inst);
  }

  std::unique_ptr<InstAssignIncDec> InstructionAssignIncDec(utree & u) {
    auto inst = new InstAssignIncDec;

    Symbol var = symbols[GetSymbol(u[0])];
    inst->op = GetOp(u[1]);

    if (var.sig[0] != basic_type::int_ && var.sig[0] != basic_type::double_)
      throw BadAssignIncDecType(u, tags);

    return make_unique_ptr(inst);
  }

  std::unique_ptr<Inst> InstructionDecl(utree & u) {
    if (kDebug)
      std::cerr << "decl:" << tags[u].line_pos << ": " << u << "\n";

    Type type = GetType(u[0]);

    for (size_t i = 1 ; i < u.size() ; ++i) {
      Symbol var(type, GetSymbol(u[i][0]));
      if (symbols.InContext(var.name))
        throw AlreadyDeclared(u, tags);
      symbols.Add(var);
      if (u[i].size() == 3)
        InstructionAssignExp(u[i]);
    }

    return std::unique_ptr<Inst>(0);
  }
};

#endif // JLC_COMPILER_HH_
