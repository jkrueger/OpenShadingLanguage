// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The oiio helpers the translated sources use from outside the safe_* and
// fast_* section of fmath.h, which is translated whole. Each one is the
// scalar version from fmath.h or hash.h.

#pragma once

#include "std.h"

namespace OIIO {

template <typename OUT_TYPE, typename IN_TYPE>
OIIO_FORCEINLINE OUT_TYPE bitcast(const thread IN_TYPE& in)
{
    static_assert(sizeof(IN_TYPE) == sizeof(OUT_TYPE), "bitcast needs equal sizes");
    return as_type<OUT_TYPE>(in);
}

OIIO_FORCEINLINE int bitcast_to_int(float x) { return as_type<int>(x); }
OIIO_FORCEINLINE float bitcast_to_float(int x) { return as_type<float>(x); }

template <class T>
OIIO_FORCEINLINE T clamp(const thread T& a, const thread T& low, const thread T& high)
{
    return (a >= low) ? ((a <= high) ? a : high) : low;
}

OIIO_FORCEINLINE float madd(float a, float b, float c) { return a * b + c; }

OIIO_FORCEINLINE float nmadd(float a, float b, float c) { return c - a * b; }

// By value, unlike oiio, so values in the constant address space can be
// passed too.
template <class T, class Q>
OIIO_FORCEINLINE T lerp(T v0, T v1, Q x)
{
    return v0 * (Q(1) - x) + v1 * x;
}

template <class T, class Q>
OIIO_FORCEINLINE T bilerp(T v0, T v1, T v2, T v3, Q s, Q t)
{
    Q s1 = Q(1) - s;
    return T((Q(1) - t) * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s));
}

template <class T, class Q>
OIIO_FORCEINLINE T trilerp(T v0, T v1, T v2, T v3, T v4, T v5, T v6, T v7, Q s, Q t, Q r)
{
    Q s1 = Q(1) - s;
    Q t1 = Q(1) - t;
    Q r1 = Q(1) - r;
    return T(r1 * (t1 * (v0 * s1 + v1 * s) + t * (v2 * s1 + v3 * s))
             + r * (t1 * (v4 * s1 + v5 * s) + t * (v6 * s1 + v7 * s)));
}

template <typename T> OIIO_FORCEINLINE T fast_neg(const thread T& x)
{
    // Like OIIO, ignores the difference between +0 and -0.
    return T(0) - x;
}

OIIO_FORCEINLINE int ifloor(float x) { return (int)floorf(x); }

OIIO_FORCEINLINE float floorfrac(float x, thread int* xint)
{
    int i = ifloor(x);
    *xint = i;
    return x - static_cast<float>(i);
}

template <typename T> OIIO_FORCEINLINE T sign(T x)
{
    return x < T(0) ? T(-1) : (x == T(0) ? T(0) : T(1));
}

OIIO_FORCEINLINE void sincos(float x, thread float* sine, thread float* cosine)
{
    *sine   = metal::sin(x);
    *cosine = metal::cos(x);
}

template <typename T> OIIO_FORCEINLINE T radians(T deg) { return deg * T(M_PI_F / 180.0f); }
template <typename T> OIIO_FORCEINLINE T degrees(T rad) { return rad * T(180.0f / M_PI_F); }

namespace bjhash {

OIIO_FORCEINLINE uint32_t rotl32(uint32_t x, int k) { return (x << k) | (x >> (32 - k)); }

inline void bjmix(thread uint32_t& a, thread uint32_t& b, thread uint32_t& c)
{
    a -= c;  a ^= rotl32(c, 4);  c += b;
    b -= a;  b ^= rotl32(a, 6);  a += c;
    c -= b;  c ^= rotl32(b, 8);  b += a;
    a -= c;  a ^= rotl32(c, 16); c += b;
    b -= a;  b ^= rotl32(a, 19); a += c;
    c -= b;  c ^= rotl32(b, 4);  b += a;
}

inline uint32_t bjfinal(uint32_t a, uint32_t b, uint32_t c = 0xdeadbeef)
{
    c ^= b; c -= rotl32(b, 14);
    a ^= c; a -= rotl32(c, 11);
    b ^= a; b -= rotl32(a, 25);
    c ^= b; c -= rotl32(b, 16);
    a ^= c; a -= rotl32(c, 4);
    b ^= a; b -= rotl32(a, 14);
    c ^= b; c -= rotl32(b, 24);
    return c;
}

}  // namespace bjhash
}  // namespace OIIO
