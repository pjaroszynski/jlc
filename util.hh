#ifndef JLC_UTIL_HH_
#define JLC_UTIL_HH_

#include <memory>

template <typename T>
std::unique_ptr<T> make_unique_ptr(T * t) {
  return std::unique_ptr<T>(t);
}

#endif // JLC_UTIL_HH_
