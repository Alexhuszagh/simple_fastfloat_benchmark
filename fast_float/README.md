## fast_float number parsing library

The fast_float library provides fast header-only implementations for the C++ from_chars
functions for `float` and `double` types.  These functions convert ASCII strings representing
decimal values (e.g., `1.3e10`) into binary types. We provide exact rounding (including
round to even). In our experience, these `fast_float` functions are faster than any other comparable number-parsing functions. They provide a performance similar to that of the [fast_double_parser](https://github.com/lemire/fast_double_parser) but using an novel algorithm reworked from the ground up, and while offering an API more in line with the expectations of C++ programmers.

Specifically, `fast_float` provides the following two functions with a C++17-like syntax:

```C++
from_chars_result from_chars(const char* first, const char* last, float& value, ...);
from_chars_result from_chars(const char* first, const char* last, double& value, ...);
```

The return type (`from_chars_result`) is defined as the struct:
```C++
struct from_chars_result {
    const char* ptr;
    std::errc ec;
};
```

It parses the character sequence [first,last) for a number. It parses floating-point numbers expecting
a locale-indepent format equivalent to what is used by `std::strtod` in the default ("C") locale. 
The resulting floating-point value is the closest floating-point values (using either float or double), 
using the "round to even" convention for values that would otherwise fall right in-between two values.
That is, we provide exact parsing according to the IEEE standard.

Given a successful parse, the pointer (`ptr`) in the returned value is set to point right after the
parsed number, and the `value` referenced is set to the parsed value. In case of error, the returned
`ec` contains a representative error, otherwise the default (`std::errc()`) value is stored.

The implementation does not throw and does not allocate memory (e.g., with `new` or `malloc`).

Example:

``` C++
#include "fast_float/parse_number.h"
#include <iostream>
 
int main() {
    const std::string input =  "3.1416 xyz ";
    double result;
    auto answer = fast_float::from_chars(input.data(), input.data()+input.size(), result);
    if(answer.ec != std::errc()) { std::cerr << "parsing failure\n"; return EXIT_FAILURE; }
    std::cout << "parsed the number " << result << std::endl;
}
```

Like the C++17 standard, the `fast_float::from_chars` functions take an optional last argument of
the type `fast_float::chars_format`. It is a bitset value: we check whether 
`fmt & fast_float::chars_format::fixed` and `fmt & fast_float::chars_format::scientific` are set
to determine whether we allow the fixed point and scientific notation respectively.
The default is  `fast_float::chars_format::general` which allows both `fixed` and `scientific`.

## Using as a CMake dependency

This library is header-only by design. The CMake file provides the `fast_float` target
which is merely a pointer to the `include` directory.

If you drop the `fast_float` repository in your CMake project, you should be able to use
it in this manner:

```
add_subdirectory(fast_float)
target_link_libraries(myprogram PUBLIC fast_float)
```



## Requirements and Limitations

In many cases, this library can be used as a drop-in replacement for the C++17 `from_chars` function, especially when performance is a concerned. Thus we expect C++17 support. Though it might be reasonable to want C++17 features as part of old compilers, support old systems is not an objective of this library.

The `from_chars` is meant to be locale-independent. Thus it is not an objective of this library to support
locale-sensitive parsing.

The performance is optimized for 19 or fewer significant digits. In practice, there should
never be more than 17 digits since it is enough to identify exactly all possible 64-bit numbers (double).
In fact, for many numbers, far fewer than 17 digits are needed.

## Credit

Though this work is inspired by many different people, this work benefited especially from exchanges with 
Michael Eisel, who motivated the original research with his key insights, and with Nigel Tao who provided 
invaluable feedback. 

The library includes code adapted from Google Wuffs (written by Nigel Tao) which was originally published 
under the Apache 2.0 license.