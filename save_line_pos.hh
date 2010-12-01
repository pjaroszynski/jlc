#ifndef JLC_SAVE_LINE_POS_
#define JLC_SAVE_LINE_POS_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <boost/spirit/include/support_utree.hpp>

#include "tags.hh"

template <class Iterator, class Tags>
struct SaveLinePos : boost::spirit::qi::grammar<Iterator, void(boost::spirit::utree &)> {
  boost::spirit::qi::rule<Iterator, void(boost::spirit::utree &)> start;

  boost::phoenix::function<TagLinePos<Tags>> tag_line_pos;

  SaveLinePos(Tags & t) :
    SaveLinePos::base_type(start),
    tag_line_pos(TagLinePos<Tags>(t))
  {
    using boost::spirit::qi::omit;
    using boost::spirit::qi::raw;
    using boost::spirit::qi::eps;
    using boost::spirit::qi::_r1;
    using boost::spirit::qi::_1;

    start = omit[raw[eps][tag_line_pos(_r1, _1)]];
  }
};

#endif // JLC_SAVE_LINE_POS_HH_
