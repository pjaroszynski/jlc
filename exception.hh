#ifndef JLC_EXCEPTION_HH_
#define JLC_EXCEPTION_HH_

#include <string>
#include <exception>
#include <sstream>

#include <boost/spirit/include/support_utree.hpp>

using boost::spirit::utree;

class Exception : public std::exception {
 private:
  mutable std::string _what_str;

  const Exception & operator=(const Exception &);

 protected:
  Exception(const std::string &) throw ();
  Exception(const Exception &) throw ();
  std::string _msg;

 public:
  virtual ~Exception() throw ();

  const std::string & message() const throw ();
  virtual const char * what() const throw ();
};

struct CompilerError : Exception {
  CompilerError() : Exception(" Internal error") {}
};

struct CompilationError : Exception {
  template <class Tags>
  CompilationError(utree & u, Tags & t);
};

template <class Tags>
CompilationError::CompilationError(utree & u, Tags & t) :
  Exception("")
{
  std::ostringstream ss;
  ss << " at line " << t[u].line_pos << ": " << u << "\n";
  _msg = ss.str();
}

struct AlreadyDeclared : CompilationError {
  template <class Tags>
  AlreadyDeclared(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct ExpectedIdentifier : CompilationError {
  template <class Tags>
  ExpectedIdentifier(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct ExpectedType : CompilationError {
  template <class Tags>
  ExpectedType(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct ExpectedOp : CompilationError {
  template <class Tags>
  ExpectedOp(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct UndefinedVariable : CompilationError {
  template <class Tags>
  UndefinedVariable(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct UndefinedFunction : CompilationError {
  template <class Tags>
  UndefinedFunction(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct NotAFunction : CompilationError {
  template <class Tags>
  NotAFunction(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct BadArgumentCount : CompilationError {
  template <class Tags>
  BadArgumentCount(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct BadArgumentType : CompilationError {
  template <class Tags>
  BadArgumentType(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct BadReturnType : CompilationError {
  template <class Tags>
  BadReturnType(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct BadAssignExpType : CompilationError {
  template <class Tags>
  BadAssignExpType(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct BadAssignIncDecType : CompilationError {
  template <class Tags>
  BadAssignIncDecType(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct IncompatibleBinaryExpArguments : CompilationError {
  template <class Tags>
  IncompatibleBinaryExpArguments(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct IncompatibleUnaryExpArgument : CompilationError {
  template <class Tags>
  IncompatibleUnaryExpArgument(utree & u, Tags & t) : CompilationError(u, t) { }
};

struct NoReturn : CompilationError {
  template <class Tags>
  NoReturn(utree & u, Tags & t) : CompilationError(u, t) { }
};


#endif // JLC_EXCEPTION_HH_
