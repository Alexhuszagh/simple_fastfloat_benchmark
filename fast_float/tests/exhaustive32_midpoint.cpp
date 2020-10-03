#include "fast_float/fast_float.h"


#include <cassert>
#include <cmath>

template <typename T> char *to_string(T d, char *buffer) {
  auto written = std::snprintf(buffer, 64, "%.*e",
                               std::numeric_limits<T>::max_digits10 - 1, d);
  return buffer + written;
}

void strtod_from_string(const char * st, float& d) {
    char *pr = (char *)st;
#ifdef _WIN32
    static _locale_t c_locale = _create_locale(LC_ALL, "C");
    d = _strtof_l(st, &pr,  c_locale);
#else
    static locale_t c_locale = newlocale(LC_ALL_MASK, "C", NULL);
    d = strtof_l(st, &pr,  c_locale);
#endif
    if (pr == st) {
      throw std::runtime_error("bug in strtod_from_string");
    }
}

void allvalues() {
  char buffer[64];
  for (uint64_t w = 0; w <= 0xFFFFFFFF; w++) {
    float v;
    if ((w % 1048576) == 0) {
      std::cout << ".";
      std::cout.flush();
    }
    uint32_t word = w;
    memcpy(&v, &word, sizeof(v));
    if(std::isfinite(v)) { 
      float nextf = std::nextafterf(v, INFINITY);
      if(!std::isfinite(nextf)) { continue; }
      double v1{v};
      assert(float(v1) == v);
      double v2{nextf};
      assert(float(v2) == nextf);
      double midv{v1 + (v2 - v1) / 2};
      float expected_midv(midv);

      const char *string_end = to_string(midv, buffer);
      float str_answer;
      strtod_from_string(buffer, str_answer);

      float result_value;
      auto result = fast_float::from_chars(buffer, string_end, result_value);
      if (result.ec != std::errc()) {
        std::cerr << "parsing error ? " << buffer << std::endl;
        abort();
      }
      if (std::isnan(v)) {
        if (!std::isnan(result_value)) {
          std::cerr << "not nan" << buffer << std::endl;
          abort();
        }
      } else if (result_value != str_answer) {
        std::cerr << "no match ? " << buffer << std::endl;
        std::cout << "started with " << std::hexfloat << midv << std::endl;
        std::cout << "round down to " << std::hexfloat << str_answer << std::endl;
        std::cout << "got back " << std::hexfloat << result_value << std::endl; 
        std::cout << std::dec;
        abort();
      }
    }
  }
  std::cout << std::endl;
}

int main() {
  allvalues();
  std::cout << std::endl;
  std::cout << "all ok" << std::endl;
  return EXIT_SUCCESS;
}