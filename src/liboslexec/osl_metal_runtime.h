// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// Metal counterpart of osl_cpp_runtime.h, included by every .metal file
// BackendMetal generates. It provides the types and the osl_* runtime
// functions generated code calls.
//
// Most of it is osl's own code, and the imath and oiio code under it,
// translated by build-scripts/metalize.py into osl_metal/generated/. msl
// has no generic address space, so the translation adds `thread` to every
// pointer and reference, and drops the simd and cuda paths, keeping the
// scalar cpu one the cpu results come from. osl_metal/compat/ stands in for
// the standard library and the config headers. Experimental.

#pragma once

#include "osl_metal/compat/config.h"
#include "osl_metal/compat/std.h"
#include "osl_metal/compat/oiio.h"

#include "osl_metal/generated/imath_math.h"
#include "osl_metal/generated/imath_fun.h"
#include "osl_metal/generated/imath_vec.h"
#include "osl_metal/generated/imath_color.h"
#include "osl_metal/generated/imath_matrix.h"
#include "osl_metal/generated/oiio_fmath.h"
#include "osl_metal/generated/oiio_fmath_numerics.h"

#include "osl_metal/types.h"
#include "osl_metal/closure.h"
#include "osl_metal/renderer.h"
#include "osl_metal/generated/osl_hashes.h"

#include "osl_metal/generated/osl_config.h"
#include "osl_metal/generated/osl_shaderglobals.h"
#include "osl_metal/generated/osl_imathx.h"
#include "osl_metal/generated/osl_dual.h"
#include "osl_metal/generated/osl_dual_vec.h"
#include "osl_metal/generated/osl_ops.h"
#include "osl_metal/errors.h"
#include "osl_metal/generated/osl_cpp_inline.h"
#include "osl_metal/generated/osl_opmatrix.h"

#include "osl_metal/generated/osl_sfmath.h"
#include "osl_metal/generated/osl_oslnoise.h"
#include "osl_metal/generated/osl_simplexnoise.h"
#include "osl_metal/generated/osl_gabornoise_h.h"
#include "osl_metal/generated/osl_gabornoise.h"
#include "osl_metal/generated/osl_opnoise.h"

#include "osl_metal/generated/osl_opcolor_h.h"
#include "osl_metal/generated/osl_opcolor_impl.h"
#include "osl_metal/generated/osl_opcolor.h"
#include "osl_metal/color.h"
#include "osl_metal/generated/osl_splineimpl.h"
#include "osl_metal/generated/osl_opspline.h"
#include "osl_metal/generated/osl_opclosure.h"
