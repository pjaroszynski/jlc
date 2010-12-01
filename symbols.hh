#ifndef JLC_SYMBOLS_HH_
#define JLC_SYMBOLS_HH_

#include <ostream>
#include <vector>
#include <map>
#include <set>
#include <list>

#include "ast.hh"

struct Symbol {
  std::vector<Type> sig;
  std::string name;
  int args;

  Symbol()
  {
  }

  Symbol(Type t, const std::string & n) :
    sig(1, t), name(n), args(-1)
  {
  }

  Symbol(Type t, const std::string & n, Type argt) :
    sig({t, argt}), name(n), args(1)
  {
  }

  Symbol & function() {
    args = 0;
    return *this;
  }
};

std::ostream & operator<<(std::ostream & os, const Symbol & sym) {
  os << sym.sig[0] << " " << sym.name;
  if (sym.args >= 0) {
    os << "(";
    if (sym.args >= 1)
      os << sym.sig[1];
    for (int i = 1 ; i < sym.args ; ++i)
      os << ", " << sym.sig[i + 1];
    os << ")";
  }
  return os;
}

class Symbols {
 public:
  Symbols();

  void Add(const Symbol & s);

  void BeginContext();
  void EndContext();
  bool InContext(const std::string & s) const;
  bool Defined(const std::string & s) const;

  Symbol operator[](const std::string & s) const;

 private:
  std::map<std::string, std::list<Symbol>> symbols;
  std::vector<std::set<std::string>> contexts;
  std::set<std::string> * current_context;
};

Symbols::Symbols() :
  contexts(1)
{
  current_context = &contexts[0];
};

void Symbols::BeginContext() {
  contexts.resize(contexts.size() + 1);
  current_context = &contexts.back();
}

void Symbols::EndContext() {
  for (auto i = current_context->begin() ; i != current_context->end() ; ++i)
    symbols[*i].pop_back();
  contexts.pop_back();
  current_context = &contexts.back();
}

bool Symbols::InContext(const std::string & s) const {
  return current_context->count(s) > 0;
}

bool Symbols::Defined(const std::string & s) const {
  return symbols.count(s) > 0;
}

Symbol Symbols::operator[](const std::string & s) const {
  return symbols.find(s)->second.back();
}

void Symbols::Add(const Symbol & s) {
  current_context->insert(s.name);
  symbols[s.name].push_back(s);
}

#endif // JLC_SYMBOLS_HH_
