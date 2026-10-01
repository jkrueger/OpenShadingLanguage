// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The part of the c++ standard library the translated sources use, mapped
// onto the metal standard library where it has an equivalent. msl has no
// std namespace of its own, so this one only holds what's listed here.

#pragma once

#include "config.h"

namespace std {

using metal::conditional;
using metal::conditional_t;
using metal::decay;
using metal::enable_if;
using metal::enable_if_t;
using metal::false_type;
using metal::integral_constant;
using metal::is_floating_point;
using metal::is_integral;
using metal::is_same;
using metal::numeric_limits;
using metal::remove_reference;
using metal::true_type;

// msl has no derived classes, and a static data member would need the
// constant address space, so these traits spell value out as an enum.
template <class T> struct is_reference { enum : bool { value = false }; };
template <class T> struct is_reference<thread T&> { enum : bool { value = true }; };
template <class T> using decay_t = typename decay<T>::type;

template <class T, T... Is> struct integer_sequence {
    static size_t size() { return sizeof...(Is); }
};
namespace detail {
template <size_t... Is> struct index_list {};
template <size_t N, size_t... Is> struct make_index_list {
    using type = typename make_index_list<N - 1, N - 1, Is...>::type;
};
template <size_t... Is> struct make_index_list<0, Is...> {
    using type = index_list<Is...>;
};
template <class T, class L> struct to_integer_sequence;
template <class T, size_t... Is> struct to_integer_sequence<T, index_list<Is...>> {
    using type = integer_sequence<T, T(Is)...>;
};
}  // namespace detail
template <class T, T N>
using make_integer_sequence =
    typename detail::to_integer_sequence<T, typename detail::make_index_list<size_t(N)>::type>::type;

template <class... B> struct conjunction {
    enum : bool { value = (true && ... && bool(B::value)) };
};

using metal::abs;
using metal::acos;
using metal::asin;
using metal::atan;
using metal::atan2;
using metal::ceil;
using metal::copysign;
using metal::cos;
using metal::cosh;
using metal::exp;
using metal::exp2;
using metal::fabs;
using metal::floor;
using metal::fmod;
using metal::isfinite;
using metal::isinf;
using metal::isnan;
using metal::log;
using metal::log10;
using metal::log2;
using metal::max;
using metal::min;
using metal::pow;
using metal::rint;
using metal::round;
using metal::sin;
using metal::sinh;
using metal::sqrt;
using metal::tan;
using metal::tanh;
using metal::trunc;

inline float expm1(float x) { return metal::exp(x) - 1.0f; }
inline float cbrt(float x) { return metal::copysign(metal::pow(metal::abs(x), 1.0f / 3.0f), x); }
inline float logb(float x) { return metal::floor(metal::log2(metal::abs(x))); }
inline float modf(float x, thread float* ip) { return metal::modf(x, *ip); }
inline long lrint(float x) { return long(metal::rint(x)); }
inline float hypot(float x, float y) { return metal::sqrt(x * x + y * y); }

// Abramowitz and Stegun 7.1.26. Only reached by code osl compiles with
// OSL_FAST_MATH off, which the runtime doesn't.
inline float erf(float x)
{
    float t = 1.0f / (1.0f + 0.3275911f * metal::abs(x));
    float y = 1.0f - (((((1.061405429f * t - 1.453152027f) * t) + 1.421413741f) * t
                       - 0.284496736f) * t + 0.254829592f) * t * metal::exp(-x * x);
    return metal::copysign(y, x);
}
inline float erfc(float x) { return 1.0f - erf(x); }

template <typename T> inline void swap(thread T& a, thread T& b)
{
    T t = a;
    a   = b;
    b   = t;
}

inline thread void* memcpy(thread void* dst, thread const void* src, size_t n)
{
    thread char* d       = (thread char*)dst;
    thread const char* s = (thread const char*)src;
    for (size_t i = 0; i < n; ++i)
        d[i] = s[i];
    return dst;
}

inline thread void* memset(thread void* dst, int value, size_t n)
{
    thread char* d = (thread char*)dst;
    for (size_t i = 0; i < n; ++i)
        d[i] = char(value);
    return dst;
}

}  // namespace std

// The c math functions oiio and osl call unqualified, as <cmath> leaves
// them in the global namespace.
using metal::fabs;
using metal::floor;
inline int abs(int x) { return metal::abs(x); }
inline float sqrtf(float x) { return metal::sqrt(x); }
inline float floorf(float x) { return metal::floor(x); }
inline float ceilf(float x) { return metal::ceil(x); }
inline float roundf(float x) { return metal::round(x); }
inline float truncf(float x) { return metal::trunc(x); }
inline float fabsf(float x) { return metal::abs(x); }
inline float fmodf(float x, float y) { return metal::fmod(x, y); }
inline float copysignf(float x, float y) { return metal::copysign(x, y); }
inline float expf(float x) { return metal::exp(x); }
inline float exp2f(float x) { return metal::exp2(x); }
inline float logf(float x) { return metal::log(x); }
inline float log2f(float x) { return metal::log2(x); }
inline float powf(float x, float y) { return metal::pow(x, y); }
inline float sinf(float x) { return metal::sin(x); }
inline float cosf(float x) { return metal::cos(x); }
inline float tanf(float x) { return metal::tan(x); }
inline float asinf(float x) { return metal::asin(x); }
inline float acosf(float x) { return metal::acos(x); }
inline float atanf(float x) { return metal::atan(x); }
inline float atan2f(float y, float x) { return metal::atan2(y, x); }
inline float sinhf(float x) { return metal::sinh(x); }
inline float coshf(float x) { return metal::cosh(x); }
inline float tanhf(float x) { return metal::tanh(x); }
inline float expm1f(float x) { return std::expm1(x); }
inline float cbrtf(float x) { return std::cbrt(x); }
inline float erff(float x) { return std::erf(x); }
inline float erfcf(float x) { return std::erfc(x); }
inline float logbf(float x) { return std::logb(x); }
inline float log10f(float x) { return metal::log10(x); }
