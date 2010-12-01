#ifndef JLC_TYPES_HH_
#define JLC_TYPES_HH_

#include <boost/mpl/map.hpp>
#include <string>
#include <boost/spirit/include/qi_symbols.hpp>

typedef int Type;

namespace basic_type {
  const Type void_ = 1;
  const Type int_ = 2;
  const Type double_ = 3;
  const Type boolean_ = 4;
  const Type string_ = 5;
}

template <typename T>
struct LiteralToBasicType {
  template <Type type>
  struct TypeWrapper {
    static const Type value = type;
  };

  typedef boost::mpl::map<
    boost::mpl::pair<int, TypeWrapper<basic_type::int_>>,
    boost::mpl::pair<double, TypeWrapper<basic_type::double_>>,
    boost::mpl::pair<bool, TypeWrapper<basic_type::boolean_>>,
    boost::mpl::pair<std::string, TypeWrapper<basic_type::string_>>
  > map;
  typedef typename boost::mpl::at<map, T>::type type;
};

struct TypeSymbols : boost::spirit::qi::symbols<char, Type> {
  TypeSymbols() {
    add
      ("void", basic_type::void_)
      ("int", basic_type::int_)
      ("double", basic_type::double_)
      ("boolean", basic_type::boolean_)
      ;
  }
};

#endif // JLC_TYPES_HH_
