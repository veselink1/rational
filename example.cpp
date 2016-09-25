#include <iostream>
#include <cstdlib>
#include "rational.hpp"

using MyRatio = rational::Ratio<int>; // or use any of the typedef-ed variants

// overload <<
std::ostream& operator<<(std::ostream& os, MyRatio r)
{
    return os << r.numer << '/' << r.denom;
}

void example1()
{
    MyRatio r { 10, 3 }; // initializer_list (since C++11)

    std::cout << r << " = " << MyRatio::to_float<long double>(r) << "..." << std::endl;
}

void example2()
{
    using namespace rational;
    // Rational is defined as Ratio<std::ptrdiff_t>
    Rational x = 7; // same as 7/1 or 7.0
    Rational y (7, 3); // same as 7/3 or 2.(333)
    Rational z = (x / y) + 1; // same as 7.0 / 2.(333) + 1
    // z should be 4/1 now

    std::cout << z << " = " << Rational::to_float<long double>(z) << std::endl;
}

int main()
{
    std::cout.precision(17);
    std::cout << std::fixed;

    example1();
    example2();

    return 0;
}
