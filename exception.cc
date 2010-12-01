#include <cxxabi.h>

#include "exception.hh"

Exception::Exception(const std::string & msg) throw () :
  _msg(msg)
{
}

Exception::Exception(const Exception & other) throw () :
  std::exception(other),
  _msg(other._msg)
{
}

Exception::~Exception() throw () {
}

const char * Exception::what() const throw () {
  if (_what_str.empty()) {
    int status = 0;
    char * const name(abi::__cxa_demangle((std::string("_Z") + typeid(*this).name()).c_str(), 0, 0, &status));

    if (status == 0) {
      _what_str = name;
      std::free(name);
    }
  }

  if (_what_str.empty())
    _what_str = std::string(std::exception::what());

  return _what_str.c_str();
}

const std::string & Exception::message() const throw () {
  return _msg;
}
