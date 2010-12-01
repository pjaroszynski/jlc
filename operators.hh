#ifndef JLC_OPERATORS_HH_
#define JLC_OPERATORS_HH_

#include <boost/spirit/include/qi_symbols.hpp>

typedef int Op;

namespace op {

const Op mul_ = 1;
const Op div_ = 2;
const Op mod_ = 3;

const Op plus_ = 4;
const Op minus_ = 5;

const Op gt_ = 6;
const Op gte_ = 7;
const Op lt_ = 8;
const Op lte_ = 9;

const Op eq_ = 10;
const Op neq_ = 11;

const Op and_ = 12;
const Op or_ = 13;

const Op not_ = 14;

const Op inc_ = 15;
const Op dec_ = 16;

bool NumericArgs(Op op) {
  return op <= neq_;
}

bool BooleanArgs(Op op) {
  return op >= eq_ && op <= not_;
}

bool BooleanResult(Op op) {
  return op >= gt_;
}

bool NumericResult(Op op) {
  return op <= minus_;
}

}

struct OpAndSymbols : boost::spirit::qi::symbols<char, Op> {
  OpAndSymbols() {
    add
      ("&&", op::and_)
    ;
  }
};

struct OpOrSymbols : boost::spirit::qi::symbols<char, Op> {
  OpOrSymbols() {
    add
      ("||", op::or_)
    ;
  }
};

struct OpEqualitySymbols : boost::spirit::qi::symbols<char, Op> {
  OpEqualitySymbols() {
    add
      ("==", op::eq_)
      ("!=", op::neq_)
    ;
  }
};

struct OpRelationalSymbols : boost::spirit::qi::symbols<char, Op> {
  OpRelationalSymbols() {
    add
      ("<=", op::lte_)
      (">=", op::gte_)
      ("<", op::lt_)
      (">", op::gt_)
    ;
  }
};

struct OpAdditiveSymbols : boost::spirit::qi::symbols<char, Op> {
  OpAdditiveSymbols() {
    add
      ("+", op::plus_)
      ("-", op::minus_)
    ;
  }
};

struct OpMultiplicativeSymbols : boost::spirit::qi::symbols<char, Op> {
  OpMultiplicativeSymbols() {
    add
      ("*", op::mul_)
      ("/", op::div_)
      ("%", op::mod_)
    ;
  }
};

struct OpUnarySymbols : boost::spirit::qi::symbols<char, Op> {
  OpUnarySymbols() {
    add
      ("!", op::not_)
      ("+", op::plus_)
      ("-", op::minus_)
    ;
  }
};

struct OpIncDecSymbols : boost::spirit::qi::symbols<char, Op> {
  OpIncDecSymbols() {
    add
      ("++", op::inc_)
      ("--", op::dec_)
    ;
  }
};

#endif // JLC_OPERATORS_HH_
