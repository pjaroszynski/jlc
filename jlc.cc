#include "parser.hh"
#include "ast.hh"
#include "compiler.hh"
#include "tags.hh"

#include <sstream>

using namespace boost::spirit;

void GetSource(std::istream & is, std::string & source) {
  is.unsetf(std::ios::skipws);
  std::copy(std::istream_iterator<char>(is), std::istream_iterator<char>(), std::back_inserter(source));
}

int main(int argc, char * argv[]) {
  std::string source_code;

  const char * filename;

  if (argc > 1) {
    filename = argv[1];
    std::ifstream in(filename, std::ios_base::in);
    if (! in) {
      std::cerr << "Error: Could not open input file: " << filename << std::endl;
      return 1;
    }
    GetSource(in, source_code);
  } else {
    filename = "<stdin>";
    GetSource(std::cin, source_code);
  }

  std::istringstream scs(source_code);

  //typedef std::string::const_iterator stream_iterator_type;
#if 0
  typedef boost::spirit::basic_istream_iterator<char> stream_iterator_type;
  typedef boost::spirit::line_pos_iterator<stream_iterator_type> iterator_type;

  stream_iterator_type sfirst(scs);
  stream_iterator_type slast;
  iterator_type iter(sfirst);
  iterator_type end(slast);
#else
  //typedef boost::spirit::basic_istream_iterator<char> iterator_type;
  typedef std::string::const_iterator string_iterator_type;
  typedef boost::spirit::line_pos_iterator<string_iterator_type> iterator_type;

  iterator_type iter(source_code.begin());
  iterator_type end(source_code.end());
#endif
  typedef parser::JavaletteParser<iterator_type, Tags<Tag>> Parser;

  Tags<Tag> tags;
  Parser p(tags);

  typedef parser::JavaletteSkipper<iterator_type> Skipper;
  Skipper s;

  utree u;

  bool r = boost::spirit::qi::phrase_parse(iter, end, p, s, u);

  //std::cout << u << "\n";

  if (!r || iter != end) {
    std::cerr << "Parsing failed\n";
    return 1;
  }

  try {
    Compiler<Tags<Tag>> compiler(tags);
    compiler.InstructionBlock(u);
  } catch (Exception & e) {
    std::cerr << "Compilation failed: " << e.what() << e.message() << "\n";
    return 1;
  }

  return 0;
}
