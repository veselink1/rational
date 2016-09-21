# rational
Simple Rational Numbers Library for Modern C++ in a Single File

Examples

```c++
using namespace rational;

Rational x = 1; // equals 1 / 1
Rational y (1, 2); // equals 0.5
Rational z (10, 3); // equals 0.(3) or 0.3333..

Rational xPlusY = x + y; // equals 3 / 2 or 1.5

Rational result = xPlusY * z; // equals 8 / 3 or 2.(6) or 2.6666..

std::cout << result; // prints "5/1"
```
