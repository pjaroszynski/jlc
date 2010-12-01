#ifndef JLC_PARSER_HH_
#define JLC_PARSER_HH_ 1
//#define BOOST_SPIRIT_DEBUG

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_utree.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "types.hh"
#include "operators.hh"
#include "ast.hh"
#include "tags.hh"

#include "keyword.hh"
#include "save_line_pos.hh"

namespace parser {

using namespace boost::spirit;

struct Uprooter {
  template <typename=void, typename=void, typename=void>
  struct result { typedef void type; };

  void operator()(utree & root) const {
    utree child;
    root.swap(child);
    root.push_back(child);
  }

  template <typename T>
  void operator()(utree & root, const T & t) const {
    (*this)(root);
    root.push_back(t);
  }

  template <typename T1, typename T2>
  void operator()(utree & root, const T1 & t1, const T2 & t2) const {
    (*this)(root, t1);
    root.push_back(t2);
  }
};

struct CopyTag {
  template <class, class>
  struct result { typedef void type; };

  void operator()(utree & from, utree & to) const {
    //std::cerr << "Copying tag " << from.tag() << " to tag " << to.tag() << "\n";
    to.tag(from.tag());
  }
};

struct PushBack {
  template <class, class=void, class=void, class=void>
  struct result { typedef void type; };

  template <class T1>
  void operator()(utree & u, const T1 & t1) const {
    u.push_back(t1);
  }

  template <class T1, class T2>
  void operator()(utree & u, const T1 & t1, const T2 & t2) const {
    u.push_back(t1);
    u.push_back(t2);
  }
};

struct DropInvalid {
  template <class, class=void, class=void, class=void>
  struct result { typedef void type; };

  void operator()(utree & u) const {
    if (u.back().which() == utree_type::invalid_type)
      u.pop_back();
  }
};

template <typename Iterator>
class JavaletteSkipper : public qi::grammar<Iterator> {
 public:
  JavaletteSkipper() : JavaletteSkipper::base_type(start)
  {
    using namespace boost::spirit::qi;
    using boost::phoenix::val;

    start =
      space
      | (lit("//")|lit("#")) >> *(char_ - eol) >> -eol
      | ("/*" >> *(char_ - "*/") > "*/")
      ;
    on_error<rethrow> (
        start,
        std::cout
          << val("Error! Unterminated /* comment\n")
          << std::endl
    );
  }

  qi::rule<Iterator, unused_type> start;
};


template <class Iterator, class Tags>
class JavaletteParser : public qi::grammar<Iterator, utree(), JavaletteSkipper<Iterator>> {
 public:
  JavaletteParser(Tags & t);

 private:
  const boost::phoenix::function<Uprooter> up;
  const boost::phoenix::function<CopyTag> copy_tag;
  const boost::phoenix::function<PushBack> pb;
  const boost::phoenix::function<DropInvalid> di;

  SaveLinePos<Iterator, Tags> pos;
  const boost::phoenix::function<CopyLinePos<Tags>> copy_pos;
  const boost::phoenix::function<CodeTagger<Tags>> code_tag;

  typedef JavaletteSkipper<Iterator> Skipper;
  TypeSymbols types;

  qi::rule<Iterator, utf8_symbol_type()> id;
  qi::rule<Iterator, utree()> type;
  qi::rule<Iterator, utree()> literal;
  qi::rule<Iterator, utf8_string_type()> literal_string;

  OpAndSymbols op_and;
  OpOrSymbols op_or;
  OpEqualitySymbols op_equality;
  OpRelationalSymbols op_relational;
  OpAdditiveSymbols op_additive;
  OpMultiplicativeSymbols op_multiplicative;
  OpUnarySymbols op_unary;
  OpIncDecSymbols op_incdec;

  typedef qi::rule<Iterator, utree::list_type(), Skipper> ListRule;

  qi::rule<Iterator, utf8_symbol_type(std::string)> symbol;

  qi::rule<Iterator, utree(), Skipper> start;
  qi::rule<Iterator, utree(), Skipper> fundecl;
  ListRule arglist;

  qi::rule<Iterator, utree::list_type(), Skipper> inst;
  qi::rule<Iterator, utree::list_type(), Skipper> inst_block, instlist, inst_decl, inst_if,
    inst_for, inst_while, inst_ret, inst_exp, assign;
  qi::rule<Iterator, utree::list_type(), Skipper> decl;
  qi::rule<Iterator, utree(), Skipper> inst_assign;

  ListRule exp, explist, exp_or, exp_and, exp_equality, exp_relational,
    exp_additive, exp_multiplicative, exp_unary;
  qi::rule<Iterator, utree::list_type(), Skipper> exp_primary;
  qi::rule<Iterator, utree(), Skipper> funcall;
};

template <class T>
utree::list_type & operator+=(utree::list_type & u, const T & a) {
  u.push_back(a);
  return u;
}

template <class Iterator, class Tags>
JavaletteParser<Iterator, Tags>::JavaletteParser(Tags & t) :
  JavaletteParser::base_type(start, "javalette"),
  pos(t),
  copy_pos(CopyLinePos<Tags>(t)),
  code_tag(CodeTagger<Tags>(t))
{
  using namespace boost::spirit;
  using namespace boost::spirit::qi;

  namespace phoenix = boost::phoenix;
  using phoenix::function;
  using phoenix::ref;
  using phoenix::size;
  using phoenix::val;
  using phoenix::construct;
  using phoenix::push_back;
  using distinct::keyword;

  typedef as<utf8_symbol_type> as_symbol_type;
  as_symbol_type const as_symbol = as_symbol_type();

  start %= +(fundecl[code_tag(_1, CodeTag::fun_decl)]);

  real_parser<double, strict_real_policies<double>> strict_double;

  id = as_symbol[raw[alpha >> *(alnum | '_')]];
  literal = strict_double | int_ | bool_ | literal_string;
  literal_string = raw['"' > *(char_ - '"') > '"'];
  type = keyword[val(types)];
  symbol = keyword[string(_r1)];

  fundecl = type > id > '(' > arglist > ')' > inst_block;
  arglist = -((type > id) % ',');

  inst =
    pos(_val) >>
    ( inst_block   [code_tag(_val, CodeTag::inst_block)]
    | inst_if      [code_tag(_val, CodeTag::inst_if)]
    | inst_for     [code_tag(_val, CodeTag::inst_for)]
    | inst_while   [code_tag(_val, CodeTag::inst_while)]
    | inst_ret     [code_tag(_val, CodeTag::inst_ret)]
    | inst_assign  [code_tag(_val, CodeTag::inst_assign)]
    | inst_decl    [code_tag(_val, CodeTag::inst_decl)]
    | inst_exp     [code_tag(_val, CodeTag::exp)]
    )[copy_tag(_val, _1), _val = _1]
    ;

  inst_block %= '{' > *inst > '}';

  inst_decl %= type > decl % ',' > ';';
  decl %= as_symbol[id] > -(as_symbol["="] > exp)[pb(_val, _1, _2)] > eps[di(_val)];
  inst_assign = assign[_val = _1] > ';';

  assign =
    pos(_val) >> id[pb(_val, _1)] >>
      ( (as_symbol[val("=")][pb(_val, _1)] > &(char_ - '=') > exp[pb(_val, _1)])
        | op_incdec[pb(_val, _1)]
      )
    ;

  inst_if %= symbol(val("if")) > '(' > exp > ')' > inst
    > -(symbol(val("else")) > inst)[pb(_val, _1, _2)] > eps[di(_val)];
  inst_while %= symbol(val("while")) > '(' > exp > ')' > inst;
  inst_for %= symbol(val("for")) > '(' > assign > ';' > exp > ';' > assign > ')' > inst;
  inst_ret %= symbol(val("return")) > -exp > ';' > eps[di(_val)];
  inst_exp = exp[_val = _1] > ';';

  auto exp_binary = [&up, &copy_pos](const ListRule & subrule, qi::symbols<char, Op> & op) {
    return boost::proto::deep_copy(
        subrule[_val = _1] >> *(op > subrule)[up(_val, _1, _2)][copy_pos(_val[0], _val)]
    );
  };

  exp = exp_or.alias();

#if 1
  exp_or = exp_binary(exp_and, op_or);
  exp_and = exp_binary(exp_equality, op_and);
  exp_equality = exp_binary(exp_relational, op_equality);
  exp_relational = exp_binary(exp_additive, op_relational);
  exp_additive = exp_binary(exp_multiplicative, op_additive);
  exp_multiplicative = exp_binary(exp_unary, op_multiplicative);
#else
  exp_or = exp_and[_val = _1] >> *(op_or > exp_and)[up(_val, _1, _2)][copy_pos(_val[0], _val)];
  exp_or = exp_equality[_val = _1] >> *(op_or > exp_equality)[up(_val, _1, _2)][copy_pos(_val[0], _val)];
  exp_equality = exp_relational[_val = _1] >> *(op_equality > exp_relational)[up(_val, _1, _2)][copy_pos(_val[0], _val)];
  exp_relational = exp_additive[_val = _1] >> *(op_relational > exp_additive)[up(_val, _1, _2)][copy_pos(_val[0], _val)];
  exp_additive = exp_multiplicative[_val = _1] >> *(op_additive > exp_multiplicative)[up(_val, _1, _2)][copy_pos(_val[0], _val)];
  exp_multiplicative = exp_unary[_val = _1] >> *(op_multiplicative > exp_unary)[up(_val, _1, _2)][copy_pos(_val[0], _val)];
#endif

  exp_unary =
    ( -op_unary[pb(_val, _1)]
      >> exp_primary
    )[if_(_1)[
      pb(_val, _2), copy_pos(_val[1], _val)
     ].else_[
      _val = _2
     ]]
    ;

  exp_primary =
    pos(_val) >>
    ( ( funcall    [copy_tag(_val, _1), _val = _1]
      | ('(' > exp [copy_tag(_val, _1), _val = _1] > ')')
      )
    | literal  [pb(_val, _1)]
    | id       [pb(_val, _1)]
    )
    ;

  funcall %= id >> explist;

  explist %= '(' > -(exp % ',') > ')';

  id.name("id");
  type.name("type");
  fundecl.name("function-decl");
  arglist.name("argument list");

  inst.name("instruction");
  inst_block.name("instruction block");

  exp.name("expression");
  exp_or.name("or-expression");
  exp_and.name("and-expression");
  exp_equality.name("equliaty-expression");
  exp_relational.name("relational-expression");
  exp_additive.name("additive-expression");
  exp_multiplicative.name("multiplicative-expression");
  exp_unary.name("unary-expression");
  exp_primary.name("primary-expression");

  on_error<fail> (
      start,
      std::cout
      << val("Error! Expecting ")
      << _4
      << val(" here: \"")
      << construct<std::string>(_3, _2)   // iterators to error-pos, end
      << val("\"")
      << std::endl
  );
}
}

#endif // JLC_PARSER_HH_
