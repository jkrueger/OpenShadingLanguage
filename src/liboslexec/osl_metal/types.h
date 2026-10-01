// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The osl types generated code and the runtime use, as osl defines them in
// oslconfig.h, shaderglobals.h and ustring, minus what only the host needs.

#pragma once

OSL_NAMESPACE_BEGIN

typedef float Float;
typedef Imath::Vec2<Float> Vec2;
typedef Imath::Vec3<Float> Vec3;
typedef Imath::Color3<Float> Color3;
typedef Imath::Matrix22<Float> Matrix22;
typedef Imath::Matrix33<Float> Matrix33;
typedef Imath::Matrix44<Float> Matrix44;

using ustringhash_pod = uint64_t;

// A string, by the hash ustring gave it on the host. The gpu has no string
// table, so the hash is all there is. An aggregate, because msl can't run a
// constructor for an object in the constant address space, which is where
// the Hashes constants live.
struct ustringhash {
    uint64_t m_hash;
    static ustringhash from_hash(uint64_t h)
    {
        ustringhash s;
        s.m_hash = h;
        return s;
    }
    uint64_t hash() const { return m_hash; }
    uint64_t hash() const constant { return m_hash; }
    bool empty() const { return m_hash == 0; }
    bool operator==(ustringhash o) const { return m_hash == o.m_hash; }
    bool operator!=(ustringhash o) const { return m_hash != o.m_hash; }
};

inline ustringhash ustringhash_from(ustringhash_pod h) { return ustringhash::from_hash(h); }

// OIIO::TypeDesc, for its enums and its layout: getattribute and friends
// pass one packed into a long long.
struct TypeDesc {
    enum BASETYPE {
        UNKNOWN, NONE, UINT8, UCHAR = UINT8, INT8, CHAR = INT8, UINT16,
        USHORT = UINT16, INT16, SHORT = INT16, UINT32, UINT = UINT32, INT32,
        INT = INT32, UINT64, ULONGLONG = UINT64, INT64, LONGLONG = INT64,
        HALF, FLOAT, DOUBLE, STRING, PTR, USTRINGHASH, LASTBASE
    };
    enum AGGREGATE {
        SCALAR = 1, VEC2 = 2, VEC3 = 3, VEC4 = 4, MATRIX33 = 9, MATRIX44 = 16
    };
    enum VECSEMANTICS {
        NOXFORM = 0, NOSEMANTICS = 0, COLOR, POINT, VECTOR, NORMAL, TIMECODE,
        KEYCODE, RATIONAL, BOX
    };
    unsigned char basetype;
    unsigned char aggregate;
    unsigned char vecsemantics;
    unsigned char reserved;
    int arraylen;
};

using OIIO::bitcast;

// Imath declares its own copy constructors and assignments, from thread
// references, so values in the constant address space are copied out by
// component instead.
inline Color3 copy_constant(constant Color3& c) { return Color3(c.x, c.y, c.z); }
inline Matrix33 copy_constant(constant Matrix33& m)
{
    Matrix33 r;
    for (int i = 0; i < 3; ++i)
        for (int k = 0; k < 3; ++k)
            r.x[i][k] = m.x[i][k];
    return r;
}
namespace pvt { using OSL::copy_constant; }

// Errors and warnings have nowhere to go on the gpu: shader ops that would
// report one just carry on with their fallback result.
template <typename... Args> inline void errorfmt(Args...) {}
template <typename... Args> inline void warningfmt(Args...) {}

namespace pvt {
// Same members, in the same order, as OSL::pvt::NoiseParams in
// oslexec_pvt.h, with the same defaults.
struct NoiseParams {
    int anisotropic;
    int do_filter;
    Vec3 direction;
    float bandwidth;
    float impulses;

    NoiseParams()
        : anisotropic(0), do_filter(true), direction(1.0f, 0.0f, 0.0f),
          bandwidth(1.0f), impulses(16.0f)
    {
    }
};
}  // namespace pvt
using pvt::NoiseParams;

struct ClosureColor;
class ShadingContext;
typedef thread void* OpaqueExecContextPtr;
typedef thread const void* TransformationPtr;

// Same members, in the same order, as OSL::ShaderGlobals. The host-only
// pointers keep their place as opaque thread pointers.
struct ShaderGlobals {
    Vec3 P, dPdx, dPdy;
    Vec3 dPdz;
    Vec3 I, dIdx, dIdy;
    Vec3 N;
    Vec3 Ng;
    float u, dudx, dudy;
    float v, dvdx, dvdy;
    Vec3 dPdu, dPdv;
    float time;
    float dtime;
    Vec3 dPdtime;
    Vec3 Ps, dPsdx, dPsdy;
    thread void* renderstate;
    thread void* tracedata;
    thread void* objdata;
    thread ShadingContext* context;
    thread void* shadingStateUniform;
    int thread_index;
    int shade_index;
    thread void* renderer;
    TransformationPtr object2common;
    TransformationPtr shader2common;
    thread ClosureColor* Ci;
    float surfacearea;
    int raytype;
    int flipHandedness;
    int backfacing;
};

// The cpu runtime checks this when it loads a generated dso. There's no
// loader on the gpu, but the generated code still exports it.
constant constexpr int OSL_CPP_ABI_VERSION = 11601;

OSL_NAMESPACE_END

using closure_color_t = thread const void*;
