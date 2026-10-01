// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The configuration macros osl, oiio and imath decorate their code with,
// defined for msl. The sources metalize.py translates expect these instead
// of their own config headers.

#pragma once

#include <metal_stdlib>

#define OSL_HOSTDEVICE
#define OSL_DEVICE
#define OSL_FORCEINLINE inline __attribute__((always_inline))
#define OSL_NOINLINE
#define OSL_LIKELY(x)   __builtin_expect(static_cast<bool>(x), true)
#define OSL_UNLIKELY(x) __builtin_expect(static_cast<bool>(x), false)
#define OSL_NAMESPACE_BEGIN namespace OSL {
#define OSL_NAMESPACE_END   }
#define OSL_CPLUSPLUS_VERSION 17
#define OSL_FAST_MATH 1
#define OSL_DLL_EXPORT
#define OSL_LLVM_EXPORT
// Runtime functions are plain inline functions: the generated code is
// compiled together with them, so there's nothing to link against.
#define OSL_SHADEOP         inline
#define OSLNOISEPUBLIC
#define OSLEXECPUBLIC
#define OSL_CONSTANT_DATA constant
#define OSL_ASSERT(x)  ((void)0)
#define OSL_DASSERT(x) ((void)0)
#ifndef NULL
#    define NULL nullptr
#endif
#define OSL_GNUC_ATTRIBUTE(x)
#define OSL_PRAGMA(x) _Pragma(#x)
#define OSL_CLANG_PRAGMA(x) OSL_PRAGMA(x)
#define OSL_GCC_PRAGMA(x)
#define OSL_PRAGMA_WARNING_PUSH
#define OSL_PRAGMA_WARNING_POP
#define OSL_GCC_ONLY_PRAGMA(x)
#define OSL_CLANG_ONLY_PRAGMA(x)
#define OSL_NONINTEL_CLANG_PRAGMA(x)
#define OSL_MSVS_PRAGMA(x)
#define OSL_INTEL_CLASSIC_PRAGMA(x)
#define OSL_SHADEOP_NOINLINE inline

#define OIIO_HOSTDEVICE
#define OIIO_FORCEINLINE inline __attribute__((always_inline))
#define OIIO_NODISCARD
#define OIIO_CONSTEXPR14 constexpr
#define OIIO_CPLUSPLUS_VERSION 17
#define OIIO_LIKELY(x)   __builtin_expect(static_cast<bool>(x), true)
#define OIIO_UNLIKELY(x) __builtin_expect(static_cast<bool>(x), false)

#define IMATH_HOSTDEVICE
// msl rejects exception specifications.
#define noexcept
#define IMATH_NOEXCEPT
#define IMATH_CONSTEXPR14 constexpr
#define IMATH_CPLUSPLUS_VERSION 17
#define IMATH_INTERNAL_NAMESPACE Imath
#define IMATH_INTERNAL_NAMESPACE_HEADER_ENTER namespace Imath {
#define IMATH_INTERNAL_NAMESPACE_HEADER_EXIT  }
#define IMATH_EXPORT
#define IMATH_EXPORT_TEMPLATE_TYPE
#define IMATH_EXPORT_ENUM
#define IMATH_DEPRECATED(msg)
#define IMATH_LIKELY(x)   __builtin_expect(static_cast<bool>(x), true)
#define IMATH_UNLIKELY(x) __builtin_expect(static_cast<bool>(x), false)
#define IMATH_ENABLE_IF(...) metal::enable_if_t<(__VA_ARGS__), int> = 0

// The <cmath> constants, as metal's float versions.
#define M_PI     M_PI_F
#define M_PI_2   M_PI_2_F
#define M_PI_4   M_PI_4_F
#define M_1_PI   M_1_PI_F
#define M_2_PI   M_2_PI_F
#define M_2_SQRTPI M_2_SQRTPI_F
#define M_E      M_E_F
#define M_LN2    M_LN2_F
#define M_LN10   M_LN10_F
#define M_LOG2E  M_LOG2E_F
#define M_LOG10E M_LOG10E_F
#define M_SQRT2  M_SQRT2_F
#define M_SQRT1_2 M_SQRT1_2_F
#define M_TWO_PI (M_PI_F * 2.0f)
