#ifndef FASTFLOAT_PARSE_NUMBER_H
#define FASTFLOAT_PARSE_NUMBER_H
#include "ascii_number.h"
#include "decimal_to_binary.h"
#include "thompson_tao.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <system_error>

namespace fastfloat {

struct from_chars_result {
  const char *ptr;
  std::errc ec;
};

namespace {
/**
 * Special case +inf, -inf, nan, infinity, -infinity.
 * The case comparisons could be made much faster given that we know that the 
 * strings a null-free and fixed.
 **/
template <typename T>
from_chars_result parse_infnan(const char *first, const char *last, T &value)  noexcept  {
  from_chars_result answer;
  answer.ec = std::errc(); // be optimistic
  if (last - first >= 3) {
    if (fastfloat_strncasecmp(first, "nan", 3) == 0) {
      answer.ptr = first + 3;
      value = std::numeric_limits<T>::quiet_NaN();
      return answer;
    }
    if (fastfloat_strncasecmp(first, "inf", 3) == 0) {

      if ((last - first >= 8) && (fastfloat_strncasecmp(first, "infinity", 8) == 0)) {
        answer.ptr = first + 8;
      } else {
        answer.ptr = first + 3;
      }
      value = std::numeric_limits<T>::infinity();
      return answer;
    }
    if (last - first >= 4) {
      if ((fastfloat_strncasecmp(first, "+nan", 4) == 0) || (fastfloat_strncasecmp(first, "-nan", 4) == 0)) {
        answer.ptr = first + 4;
        value = std::numeric_limits<T>::quiet_NaN();
        if (first[0] == '-') {
          value = -value;
        }
        return answer;
      }

      if ((fastfloat_strncasecmp(first, "+inf", 4) == 0) || (fastfloat_strncasecmp(first, "-inf", 4) == 0)) {
        if ((last - first >= 8) && (fastfloat_strncasecmp(first + 1, "infinity", 8) == 0)) {
          answer.ptr = first + 9;
        } else {
          answer.ptr = first + 4;
        }
        value = std::numeric_limits<T>::infinity();
        if (first[0] == '-') {
          value = -value;
        }
        return answer;
      }
    }
  }
  answer.ec = std::errc::invalid_argument;
  return answer;
}
} // namespace



template<typename T>
from_chars_result from_chars(const char *first, const char *last,
                             T &value)  noexcept  {

  from_chars_result answer;
  while ((first != last) && fastfloat::is_space(*first)) {
    first++;
  }
  if (first == last) {
    answer.ec = std::errc::invalid_argument;
    answer.ptr = first;
    return answer;
  }
  parsed_number_string pns = parse_number_string(first, last);
  if (!pns.valid) {
    return parse_infnan(first, last, value);
  }
  answer.ec = std::errc(); // be optimistic
  answer.ptr = pns.lastmatch;
  // Here we could use a fast path for small values of pns.exponent, but it is not very advantageous given how
  // fast compute_float is.
  adjusted_mantissa am = pns.too_many_digits ? 
    parse_long_mantissa<binary_format<T>>(first,last) 
    : compute_float<binary_format<T>>(pns.exponent, pns.mantissa);
  uint64_t word = am.mantissa;
  word |= uint64_t(am.power2) << binary_format<T>::mantissa_explicit_bits();
  word = pns.negative 
  ? word | (uint64_t(1) << binary_format<T>::sign_index()) : word;
  memcpy(&value, &word, sizeof(T));
  return answer;
}

} // namespace fastfloat

#endif