#ifndef JLC_TAGS_HH_
#define JLC_TAGS_HH_

#include <vector>
#include <boost/spirit/include/support_utree.hpp>

enum class CodeTag {
  none = 0,
  fun_decl,
  exp,
  inst_block,
  inst_for,
  inst_if,
  inst_while,
  inst_ret,
  inst_assign,
  inst_decl,
};

struct Tag {
  CodeTag code_tag;
  int line_pos;
};

template <class T>
struct Tags {
  bool Tagged(const boost::spirit::utree & u) {
    return u.tag() != 0;
  }

  T & operator[](boost::spirit::utree & u) {
    auto i = u.tag();
    if (i == 0) {
      i = tags.size() + 1;
      u.tag(i);
      tags.push_back(T());
    }
    return tags[i - 1];
  }

  std::vector<T> tags;
};

template <class Tags>
struct CodeTagger {
  Tags & tags;

  CodeTagger(Tags & t) : tags(t) { }

  template <class, class>
  struct result { typedef void type; };

  void operator()(boost::spirit::utree & u, CodeTag t) const {
    tags[u].code_tag = t;
  }
};

template <class Tags>
struct TagLinePos {
  Tags & tags;

  template <class, class>
  struct result {
    typedef void type;
  };

  TagLinePos(Tags & t) : tags(t) { }

  template <typename Range>
  void operator()(boost::spirit::utree & u, const Range & rng) const {
    int n = get_line(rng.begin());
    tags[u].line_pos = n;
  }
};

template <class Tags>
struct CopyLinePos {
  Tags & tags;

  template <class, class>
  struct result {
    typedef void type;
  };

  CopyLinePos(Tags & t) : tags(t) { }

  void operator()(boost::spirit::utree & from, boost::spirit::utree & to) const {
    tags[to].line_pos = tags[from].line_pos;
  }
};

#endif // JLC_TAGS_HH_
