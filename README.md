# rational
Simple Rational Numbers Library for Modern C++ in a Single File

### Why use this?
Floating point value types such as `float`, `double` and `long double` are usually represented according to the IEEE-754 specification, which requires them to store the exponent and mantissa of the number as binary numbers (instead of say, storing them as integers, like the `decimal` type in .NET). This leads to loss of precision and is sometimes error prone. (For example, `0.1 * 3 == 0.3` is `false`). That is why, this library represents numbers as ratios (with a numerator and a denominator). This leads to a correct representation of the values. The numerator and denominator are both of type ptrdiff_t by default (platform-sized integers), but this can be changed by declaring Ratio<T> to use a different type (for example `long`).

### Examples

```c++
using namespace rational;

Rational x = 1; // equals 1 / 1
Rational y (1, 2); // equals 0.5
Rational z (10, 3); // equals 0.(3) or 0.3333..

Rational xPlusY = x + y; // equals 3 / 2 or 1.5

Rational result = xPlusY * z; // equals 8 / 3 or 2.(6) or 2.6666..

std::cout << result; // prints "5/1"
```
