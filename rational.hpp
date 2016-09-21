#ifndef _RATIONAL_H
#define _RATIONAL_H

#include <cmath>
#include <cstddef>
#include <iosfwd>
#include <stdexcept>
#include <type_traits>
#include <limits>

#ifndef RATIONAL_NCONSTEXPR
#define CONSTEXPR constexpr
#else
#define CONSTEXPR /* not supported */
#endif

namespace rational {

    struct NoReduceTag {};

    struct DivideByZeroException : public std::runtime_error {
        DivideByZeroException() noexcept :
            runtime_error("Division by zero is undefined.")
        {  }
    };

    struct OverflowException : public std::runtime_error {
        OverflowException() noexcept :
            runtime_error("Arithmetic operation resulted in an overflow.")
        {  }
    };

    template<class T>
    CONSTEXPR T cpow(const T base, unsigned const exponent) noexcept
    {
        return (exponent == 0) ? 1 : (base * pow(base, exponent-1));
    }
    
    template<class T>
    CONSTEXPR unsigned pow10_cap(T x) noexcept
    {
        unsigned pow10 = 0;
        x = x > T(0) ? x : -x;
        do {
            ++pow10;
            x /= 10;
        } while(x > 1);
        return pow10;
    }

    template<class T>
    T gcd(T a, T b) noexcept {
        if(a < T(0)) a = -a;
        if(b < T(0)) b = -b;
        while(b != T(0)) {
            a %= b;
            if(a == T(0)) {
                return b;
            }
            b %= a;
        }
        return a;
    }

    template<class T>
    class Ratio {
    public:

        static_assert(std::is_integral<T>::value, "Ratio<T>: T must meet the requirements of Integral");

        CONSTEXPR Ratio() noexcept :
            numer_(0),
            denom_(1)
        {  }

        template<class U>
        CONSTEXPR Ratio(const U& u) noexcept :
            Ratio(T(u), T(1), NoReduceTag())
        {
            static_assert(std::is_convertible<U, T>::value,
                "Ratio<T>::Ratio<U>(const U): U must meet the requirements of Convertible to T.");
        }

        Ratio(const T& numer, const T& denom) noexcept :
            numer_(numer),
            denom_(denom)
        {
            if(denom == T(0)) {
                throw DivideByZeroException();
            }
            Ratio<T> ret(numer, denom, NoReduceTag());
            ret.reduce();
            *this = ret;
        }

        CONSTEXPR Ratio(const T& numer, const T& denom, NoReduceTag) noexcept :
            numer_(numer),
            denom_(denom)
        {  }

        CONSTEXPR Ratio(const Ratio<T>& rat) noexcept :
            numer_(rat.numer_),
            denom_(rat.denom_)
        {  }

        CONSTEXPR Ratio& operator=(const Ratio<T>& rat) noexcept
        {
            this->numer_ = rat.numer_;
            this->denom_ = rat.denom_;
            return *this;
        }

        template<class U>
        CONSTEXPR Ratio(const Ratio<U>& rat) noexcept :
            numer_(rat.numer_),
            denom_(rat.denom_)
        {
            static_assert(std::is_convertible<U, T>::value,
                "Ratio<T>::Ratio<U>(const Ratio<U>&): U must meet the requirements of ImplicitlyConvertible to T.");
        }


        template<class U>
        Ratio& operator=(const Ratio<U>& rat) noexcept
        {
            static_assert(std::is_convertible<U, T>::value,
                "Ratio<T>::operator=<U>(const Ratio<U>&): U must meet the requirements of ImplicitlyConvertible to T.");
            this->numer_ = rat.numer_;
            this->denom_ = rat.denom_;
            return *this;
        }

        CONSTEXPR T to_integer() const noexcept
        {
            return (this->numer_ / this->denom_);
        }

        template<class Float = float>
        CONSTEXPR Float to_float() const noexcept
        {
            static_assert(std::is_constructible<Float, T>::value,
                "Ratio<T>::to_floating<Float>(): T must meet the requirements of Convertible to Float.");
            return Float(Float(this->numer_) / Float(this->denom_));
        }

        template<class Float = float>
        Ratio from_float(const Float u&, const unsigned prec)
        {
            Ratio rat (T(u * cpow(10, prec)), T(cpow(10, prec)));
            if((u > 0 && u > rat.numer_)
                || (u < 0 && u < rat.numer_)) {
                throw OverflowException();
            }
            return rat;
        }

        const T& numer() const noexcept
        {
            return this->numer_;
        }

        const T& denom() const noexcept
        {
            return this->denom_;
        }

        bool is_integer() const noexcept
        {
            return this->denom_ == T(1);
        }

        void reduce() noexcept
        {
            T g = gcd(this->numer_, this->denom_);

            this->numer_ = this->numer_ / g;
            this->denom_ = this->denom_ / g;

            if(this->denom_ < T(0)) {
                this->numer_ = T(0) - this->numer_;
                this->denom_ = T(0) - this->denom_;
            }
        }

        Ratio<T> reduced() const noexcept
        {
            Ratio<T> ret(*this);
            ret.reduce();
            return ret;
        }

        Ratio<T> floor() const noexcept
        {
            if(*this < Ratio<T>(0)) {
                T one (1);
                return Ratio<T>((this->numer_ - this->denom_ + one) / this->denom_);
            } else {
                return Ratio<T>(this->numer_ / this->denom_);
            }
        }

        Ratio<T> ceil() const noexcept
        {
            if(*this < Ratio<T>(0)) {
                return Ratio<T>(this->numer_ / this->denom_);
            } else {
                T one (1);
                return Ratio<T>((this->numer_ + this->denom_ - one) / this->denom_);
            }
        }

        Ratio<T> round() const noexcept
        {
            const Ratio<T> zero(0);
            const T one(1);
            const T two(one + one);

            Ratio<T> fractional = this->fract();
            if(fractional < zero) {
                fractional = zero - fractional;
            }

            const bool half_or_larger;
            if(fractional.denom() % 2 == 0) {
                half_or_larger = fractional.numer_ >= fractional.denom_ / two;
            } else {
                half_or_larger = fractional.numer_ >= (fractional.denom_ / two) + one;
            }

            if(half_or_larger) {
                Ratio<T> one(1);
                if(*this > Ratio<T>(0)) {
                    return this->trunc() + one;
                } else {
                    return this->trunc() - one;
                }
            } else {
                return this->trunc();
            }
        }

        Ratio<T> trunc() const noexcept
        {
            return Ratio<T>(this->numer_ / this->denom_);
        }

        Ratio<T> fract() const noexcept
        {
            return Ratio<T>(this->numer_ % this->denom_, this->denom_, NoReduceTag());
        }

        Ratio<T> pow(const Ratio<T>& r, int expon) const noexcept
        {
            if(expon == 0) {
                return Ratio<T>(1);
            } else if(expon < 0) {
                return std::pow(recip(r), -expon);
            } else {
                return Ratio<T>(std::pow(r.numer(), expon), std::pow(r.numer(), expon));
            }
        }

        bool is_zero() const noexcept
        {
            return (this->numer_ == 0);
        }

        bool is_positive() const noexcept
        {
            return (this->numer_ > 0);
        }

        bool is_negative() const noexcept
        {
            return (this->numer_ < 0);
        }

        static CONSTEXPR Ratio<T> zero() noexcept
        {
            return Ratio<T>(0, 1, NoReduceTag());
        }

        static CONSTEXPR Ratio<T> one() noexcept
        {
            return Ratio<T>(1, 1, NoReduceTag());
        }

        static CONSTEXPR Ratio<T> pi() noexcept
        {
            return Ratio<T>(6283, 2000, NoReduceTag());
        }

        Ratio<T> abs() const noexcept
        {
            return (this->is_positive() || this->is_zero() ? *this : -*this);
        }

        Ratio<T> abs_sub(const Ratio<T>& rhs) const noexcept
        {
            return abs(*this - rhs);
        }

        T signum() const noexcept
        {
            if(this->is_zero()) {
                return T(0);
            } else if(this->is_positive()) {
                return T(1);
            } else {
                return T(-1);
            }
        }

        template<class U>
        explicit CONSTEXPR operator Ratio<U>() const noexcept
        {
            static_assert(std::is_convertible<U, T>::value,
                "Ratio<T>::operator Ratio<U>(): T must meet the requirements of ImplicitlyConvertible to U.");
            return Ratio<U>(U(this->numer_), U(this->denom_), NoReduceTag());
        }
        
        template<class U>
        friend class Ratio;
        
    private:

        T numer_;
        T denom_;

    };

    template<class T>
    Ratio<T> make_ratio(const T& num) noexcept
    {
        return Ratio<T>(num);
    }

    template<class T>
    Ratio<T> make_ratio(const T& numer, const T& denom) noexcept
    {
        return Ratio<T>(numer, denom);
    }

    template<class T>
    Ratio<T> operator+(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return Ratio<T>((lhs.numer() * rhs.denom()) + (lhs.denom() * rhs.numer()), (lhs.denom() * rhs.denom()));
    }

    template<class T>
    Ratio<T> operator-(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return Ratio<T>((lhs.numer() * rhs.denom()) - (lhs.denom() * rhs.numer()), (lhs.denom() * rhs.denom()));
    }

    template<class T>
    Ratio<T> operator%(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return Ratio<T>((lhs.numer() * rhs.denom()) % (lhs.denom() * rhs.numer()), (lhs.denom() * rhs.denom()));
    }

    template<class T>
    Ratio<T> operator*(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return Ratio<T>((lhs.numer() * rhs.numer()), (lhs.denom() * rhs.denom()));
    }

    template<class T>
    Ratio<T> operator/(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return make_ratio((lhs.numer() * rhs.denom()), (lhs.denom() * rhs.numer()));
    }

    template<class T>
    Ratio<T> operator+=(Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return lhs = lhs + rhs;
    }

    template<class T>
    Ratio<T> operator-=(Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return lhs = lhs - rhs;
    }

    template<class T>
    Ratio<T> operator*=(Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return lhs = lhs * rhs;
    }

    template<class T>
    Ratio<T> operator/=(Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return lhs = lhs / rhs;
    }

    template<class T>
    Ratio<T> operator%=(Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return lhs = lhs % rhs;
    }

    template<class T>
    Ratio<T> operator+(const Ratio<T>& rat) noexcept
    {
        return Ratio<T>(rat);
    }

    template<class T>
    Ratio<T> operator-(const Ratio<T>& rat) noexcept
    {
        return Ratio<T>(-rat.numer(), rat.denom(), NoReduceTag());
    }

    template<class T>
    bool operator==(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return (lhs.numer() * lhs.denom()) == (rhs.numer() * rhs.denom());
    }

    template<class T>
    bool operator!=(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<class T>
    bool operator>(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return (lhs.numer() * lhs.denom()) > (rhs.numer() * rhs.denom());
    }


    template<class T>
    bool operator<(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return (rhs > lhs);
    }

    template<class T>
    bool operator<=(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return !(lhs > rhs);
    }

    template<class T>
    bool operator>=(const Ratio<T>& lhs, const Ratio<T>& rhs) noexcept
    {
        return !(lhs < rhs);
    }

    template<class T>
    Ratio<T>& operator++(Ratio<T>& rat) noexcept
    {
        return rat = rat + Ratio<T>(1);;
    }

    template<class T>
    Ratio<T>& operator--(Ratio<T>& rat) noexcept
    {
        return rat = rat - Ratio<T>(1);
    }

    template<class T>
    Ratio<T> operator++(Ratio<T>& rat, int) noexcept
    {
        Ratio<T> cpy = rat;
        ++rat;
        return cpy;
    }

    template<class T>
    Ratio<T> operator--(Ratio<T>& rat, int) noexcept
    {
        Ratio<T> cpy = rat;
        --rat;
        return cpy;
    }

    typedef Ratio<std::ptrdiff_t> Rational;
    typedef Ratio<std::int32_t> Rational32;
    typedef Ratio<std::int64_t> Rational64;

    namespace literals {

        CONSTEXPR Rational operator"" _r(const unsigned long long int n) noexcept
        {
            return Rational(n, 1, NoReduceTag());
        }
        
        CONSTEXPR Rational operator"" _R(const unsigned long long int n) noexcept
        {
            return Rational(n, 1, NoReduceTag());
        }

    } // namespace literals

    template<class T>
    std::ostream& operator<<(std::ostream& os, Ratio<T> x)
    {
        return os << x.numer() << '/' << x.denom();
    }
    
    template<class T>
    std::istream& operator>>(std::istream& is, Ratio<T>& x)
    {
        T numer;
        T denom;
        std::scanf("%ld/%ld", &numer, &denom);
        x = Ratio<T>(numer, denom);
        return is;
    }

} // namespace rational

#endif