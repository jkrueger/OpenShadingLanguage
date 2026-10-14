// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The texture ops of optexture.cpp, written by hand rather than translated:
// on the cpu they ask the ShadingContext for its texture thread info, and
// gather into simd floats, neither of which exists here. These follow the
// __CUDA_ARCH__ path instead. Each lookup goes to the renderer's rs_texture,
// rs_texture3d or rs_environment (see renderer.h), which asks for exactly
// the channels the shader wants, plus one for alpha if it asked for that.

#pragma once

namespace OIIO {

// OIIO::TextureOpt (TextureOpt_v2 in texture.h), with the same members in
// the same order and with the same defaults. The options only ever live in
// the shading thread's memory, between the generated code that sets them
// and the renderer that reads them, so the layout doesn't have to match
// the host's; keeping it the same just keeps the two easy to compare.
struct TextureOpt {
    // Tex::Wrap, Tex::MipMode and Tex::InterpMode, which are uint8_t enums
    // with these values.
    enum Wrap : uint8_t {
        WrapDefault,
        WrapBlack,
        WrapClamp,
        WrapPeriodic,
        WrapMirror,
        WrapPeriodicPow2,
        WrapPeriodicSharedBorder,
        WrapLast
    };
    enum MipMode : uint8_t {
        MipModeDefault,
        MipModeNoMIP,
        MipModeOneLevel,
        MipModeTrilinear,
        MipModeAniso
    };
    enum InterpMode : uint8_t {
        InterpClosest,
        InterpBilinear,
        InterpBicubic,
        InterpSmartBicubic
    };

    int firstchannel = 0;
    int subimage     = 0;
    // A ustring on the host. The gpu has only the hash, which is also what
    // the cuda path stores, in the ustring's place.
    OSL::ustringhash subimagename = { 0 };
    Wrap swrap                    = WrapDefault;
    Wrap twrap                    = WrapDefault;
    Wrap rwrap                    = WrapDefault;
    MipMode mipmode               = MipModeDefault;
    InterpMode interpmode         = InterpSmartBicubic;
    bool conservative_filter      = true;
    uint16_t anisotropic          = 32;
    float sblur = 0, tblur = 0, rblur = 0;
    float swidth = 1, twidth = 1;
    float rwidth = 1;
    float fill   = 0;
    // The generated code's own `_missing` array, when the shader passes
    // "missingcolor" or "missingalpha". Null means a failed lookup is
    // left to the renderer's fill.
    thread const float* missingcolor = nullptr;
    float rnd                        = -1;
    int colortransformid             = 0;
    int envlayout                    = 0;
};

}  // namespace OIIO

OSL_NAMESPACE_BEGIN

using OIIO::TextureOpt;

namespace pvt {

OSL_SHADEOP void
osl_init_texture_options(OpaqueExecContextPtr /*oec*/, thread void* opt)
{
    *(thread TextureOpt*)opt = TextureOpt();
}

// Tex::decode_wrapmode: the wrap mode a name stands for, or the default
// for any other. The generated code only gets here for a name that isn't
// known until the shader runs; a constant one is decoded at compile time.
//
// OIIO also knows "periodic_pow2" and "periodic_sharedborder", which have
// no Hashes entry, and which a renderer wouldn't treat differently from
// "periodic" on the gpu anyway. They decode as the default here.
inline TextureOpt::Wrap
decode_wrapmode(ustringhash_pod name)
{
    if (name == Hashes::black.hash())
        return TextureOpt::WrapBlack;
    if (name == Hashes::clamp.hash())
        return TextureOpt::WrapClamp;
    if (name == Hashes::periodic.hash())
        return TextureOpt::WrapPeriodic;
    if (name == Hashes::mirror.hash())
        return TextureOpt::WrapMirror;
    return TextureOpt::WrapDefault;
}

// tex_interp_to_code in oslexec_pvt.h: -1 for a name that isn't a mode,
// which leaves the mode as it was.
inline int
tex_interp_to_code(ustringhash_pod name)
{
    if (name == Hashes::smartcubic.hash())
        return (int)TextureOpt::InterpSmartBicubic;
    if (name == Hashes::linear.hash())
        return (int)TextureOpt::InterpBilinear;
    if (name == Hashes::cubic.hash())
        return (int)TextureOpt::InterpBicubic;
    if (name == Hashes::closest.hash())
        return (int)TextureOpt::InterpClosest;
    return -1;
}

OSL_SHADEOP void
osl_texture_set_firstchannel(thread void* opt, int x)
{
    ((thread TextureOpt*)opt)->firstchannel = x;
}

OSL_SHADEOP int
osl_texture_decode_wrapmode(ustringhash_pod name)
{
    return (int)decode_wrapmode(name);
}

OSL_SHADEOP void
osl_texture_set_swrap(thread void* opt, ustringhash_pod x)
{
    ((thread TextureOpt*)opt)->swrap = decode_wrapmode(x);
}

OSL_SHADEOP void
osl_texture_set_twrap(thread void* opt, ustringhash_pod x)
{
    ((thread TextureOpt*)opt)->twrap = decode_wrapmode(x);
}

OSL_SHADEOP void
osl_texture_set_rwrap(thread void* opt, ustringhash_pod x)
{
    ((thread TextureOpt*)opt)->rwrap = decode_wrapmode(x);
}

OSL_SHADEOP void
osl_texture_set_stwrap(thread void* opt, ustringhash_pod x)
{
    TextureOpt::Wrap code            = decode_wrapmode(x);
    ((thread TextureOpt*)opt)->swrap = code;
    ((thread TextureOpt*)opt)->twrap = code;
}

OSL_SHADEOP void
osl_texture_set_swrap_code(thread void* opt, int mode)
{
    ((thread TextureOpt*)opt)->swrap = (TextureOpt::Wrap)mode;
}

OSL_SHADEOP void
osl_texture_set_twrap_code(thread void* opt, int mode)
{
    ((thread TextureOpt*)opt)->twrap = (TextureOpt::Wrap)mode;
}

OSL_SHADEOP void
osl_texture_set_rwrap_code(thread void* opt, int mode)
{
    ((thread TextureOpt*)opt)->rwrap = (TextureOpt::Wrap)mode;
}

OSL_SHADEOP void
osl_texture_set_stwrap_code(thread void* opt, int mode)
{
    ((thread TextureOpt*)opt)->swrap = (TextureOpt::Wrap)mode;
    ((thread TextureOpt*)opt)->twrap = (TextureOpt::Wrap)mode;
}

OSL_SHADEOP void
osl_texture_set_sblur(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->sblur = x;
}

OSL_SHADEOP void
osl_texture_set_tblur(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->tblur = x;
}

OSL_SHADEOP void
osl_texture_set_rblur(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->rblur = x;
}

OSL_SHADEOP void
osl_texture_set_stblur(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->sblur = x;
    ((thread TextureOpt*)opt)->tblur = x;
}

OSL_SHADEOP void
osl_texture_set_swidth(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->swidth = x;
}

OSL_SHADEOP void
osl_texture_set_twidth(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->twidth = x;
}

OSL_SHADEOP void
osl_texture_set_rwidth(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->rwidth = x;
}

OSL_SHADEOP void
osl_texture_set_stwidth(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->swidth = x;
    ((thread TextureOpt*)opt)->twidth = x;
}

OSL_SHADEOP void
osl_texture_set_fill(thread void* opt, float x)
{
    ((thread TextureOpt*)opt)->fill = x;
}

// As on the cpu: TextureOpt has no time any more.
OSL_SHADEOP void
osl_texture_set_time(thread void* /*opt*/, float /*x*/)
{
}

OSL_SHADEOP int
osl_texture_decode_interpmode(ustringhash_pod name)
{
    return tex_interp_to_code(name);
}

OSL_SHADEOP void
osl_texture_set_interp(thread void* opt, ustringhash_pod modename)
{
    int mode = tex_interp_to_code(modename);
    if (mode >= 0)
        ((thread TextureOpt*)opt)->interpmode = (TextureOpt::InterpMode)mode;
}

OSL_SHADEOP void
osl_texture_set_interp_code(thread void* opt, int mode)
{
    ((thread TextureOpt*)opt)->interpmode = (TextureOpt::InterpMode)mode;
}

OSL_SHADEOP void
osl_texture_set_subimage(thread void* opt, int subimage)
{
    ((thread TextureOpt*)opt)->subimage = subimage;
}

OSL_SHADEOP void
osl_texture_set_subimagename(thread void* opt, ustringhash_pod subimagename)
{
    ((thread TextureOpt*)opt)->subimagename = ustringhash_from(subimagename);
}

OSL_SHADEOP void
osl_texture_set_missingcolor_arena(thread void* opt, thread const void* missing)
{
    ((thread TextureOpt*)opt)->missingcolor = (thread const float*)missing;
}

OSL_SHADEOP void
osl_texture_set_missingcolor_alpha(thread void* opt, int alphaindex,
                                   float missingalpha)
{
    thread float* m = (thread float*)((thread TextureOpt*)opt)->missingcolor;
    if (m)
        m[alphaindex] = missingalpha;
}



// A texture result holds at most a color and an alpha.
struct TextureResult {
    float c[4];
};

OSL_SHADEOP int
osl_texture(OpaqueExecContextPtr oec, ustringhash_pod name, thread void* handle,
            thread void* opt, float s, float t, float dsdx, float dtdx,
            float dsdy, float dtdy, int chans, thread void* result_,
            thread void* dresultdx_, thread void* dresultdy_,
            thread void* alpha_, thread void* dalphadx_,
            thread void* dalphady_, thread void* errormessage_)
{
    thread float* result    = (thread float*)result_;
    thread float* dresultdx = (thread float*)dresultdx_;
    thread float* dresultdy = (thread float*)dresultdy_;
    thread float* alpha     = (thread float*)alpha_;
    thread float* dalphadx  = (thread float*)dalphadx_;
    thread float* dalphady  = (thread float*)dalphady_;
    thread ustringhash_pod* errormessage = (thread ustringhash_pod*)errormessage_;
    bool derivs = (dresultdx || dalphadx);

    // Zeroed, so whatever the renderer leaves unwritten reads as 0 rather
    // than as whatever the thread's memory held.
    TextureResult r = {}, drds = {}, drdt = {};
    ustringhash em  = { 0 };
    bool ok = rs_texture(oec, ustringhash_from(name), handle,
                         *(thread TextureOpt*)opt, s, t, dsdx, dtdx, dsdy,
                         dtdy, chans + (alpha ? 1 : 0), r.c,
                         derivs ? drds.c : nullptr, derivs ? drdt.c : nullptr,
                         errormessage ? &em : nullptr);

    for (int i = 0; i < chans; ++i)
        result[i] = r.c[i];
    if (alpha)
        alpha[0] = r.c[chans];

    // Correct the st texture space gradients into xy space ones.
    if (derivs) {
        for (int i = 0; i <= chans; ++i) {
            float dx = drds.c[i] * dsdx + drdt.c[i] * dtdx;
            float dy = drds.c[i] * dsdy + drdt.c[i] * dtdy;
            if (i < chans && dresultdx) {
                dresultdx[i] = dx;
                dresultdy[i] = dy;
            } else if (i == chans && dalphadx) {
                dalphadx[0] = dx;
                dalphady[0] = dy;
            }
        }
    }

    if (errormessage)
        *errormessage = em.hash();
    return ok;
}



OSL_SHADEOP int
osl_texture3d(OpaqueExecContextPtr oec, ustringhash_pod name,
              thread void* handle, thread void* opt, thread void* P_,
              thread void* dPdx_, thread void* dPdy_, thread void* dPdz_,
              int chans, thread void* result_, thread void* dresultdx_,
              thread void* dresultdy_, thread void* alpha_,
              thread void* dalphadx_, thread void* dalphady_,
              thread void* errormessage_)
{
    const Vec3 P    = *(thread Vec3*)P_;
    const Vec3 dPdx = *(thread Vec3*)dPdx_;
    const Vec3 dPdy = *(thread Vec3*)dPdy_;
    Vec3 dPdz(0.0f);
    if (dPdz_ != nullptr)
        dPdz = *(thread Vec3*)dPdz_;
    thread float* result    = (thread float*)result_;
    thread float* dresultdx = (thread float*)dresultdx_;
    thread float* dresultdy = (thread float*)dresultdy_;
    thread float* alpha     = (thread float*)alpha_;
    thread float* dalphadx  = (thread float*)dalphadx_;
    thread float* dalphady  = (thread float*)dalphady_;
    thread ustringhash_pod* errormessage = (thread ustringhash_pod*)errormessage_;
    bool derivs = (dresultdx || dalphadx);

    TextureResult r = {}, drds = {}, drdt = {}, drdr = {};
    ustringhash em  = { 0 };
    bool ok = rs_texture3d(oec, ustringhash_from(name), handle,
                           *(thread TextureOpt*)opt, P, dPdx, dPdy, dPdz,
                           chans + (alpha ? 1 : 0), r.c,
                           derivs ? drds.c : nullptr,
                           derivs ? drdt.c : nullptr,
                           derivs ? drdr.c : nullptr,
                           errormessage ? &em : nullptr);

    for (int i = 0; i < chans; ++i)
        result[i] = r.c[i];
    if (alpha)
        alpha[0] = r.c[chans];

    // Correct the str texture space gradients into xyz space ones.
    if (derivs) {
        for (int i = 0; i <= chans; ++i) {
            float dx = drds.c[i] * dPdx.x + drdt.c[i] * dPdx.y
                       + drdr.c[i] * dPdx.z;
            float dy = drds.c[i] * dPdy.x + drdt.c[i] * dPdy.y
                       + drdr.c[i] * dPdy.z;
            if (i < chans && dresultdx) {
                dresultdx[i] = dx;
                dresultdy[i] = dy;
            } else if (i == chans && dalphadx) {
                dalphadx[0] = dx;
                dalphady[0] = dy;
            }
        }
    }

    if (errormessage)
        *errormessage = em.hash();
    return ok;
}



OSL_SHADEOP int
osl_environment(OpaqueExecContextPtr oec, ustringhash_pod name,
                thread void* handle, thread void* opt, thread void* R_,
                thread void* dRdx_, thread void* dRdy_, int chans,
                thread void* result_, thread void* dresultdx_,
                thread void* dresultdy_, thread void* alpha_,
                thread void* dalphadx_, thread void* dalphady_,
                thread void* errormessage_)
{
    const Vec3 R    = *(thread Vec3*)R_;
    const Vec3 dRdx = *(thread Vec3*)dRdx_;
    const Vec3 dRdy = *(thread Vec3*)dRdy_;
    thread float* result    = (thread float*)result_;
    thread float* dresultdx = (thread float*)dresultdx_;
    thread float* dresultdy = (thread float*)dresultdy_;
    thread float* alpha     = (thread float*)alpha_;
    thread float* dalphadx  = (thread float*)dalphadx_;
    thread float* dalphady  = (thread float*)dalphady_;
    thread ustringhash_pod* errormessage = (thread ustringhash_pod*)errormessage_;

    TextureResult r = {};
    ustringhash em  = { 0 };
    bool ok = rs_environment(oec, ustringhash_from(name), handle,
                             *(thread TextureOpt*)opt, R, dRdx, dRdy,
                             chans + (alpha ? 1 : 0), r.c, nullptr, nullptr,
                             errormessage ? &em : nullptr);

    for (int i = 0; i < chans; ++i)
        result[i] = r.c[i];

    // As on the cpu, the derivatives are zero: the renderer returns st
    // gradients, and turning them into xy ones would need to know how it
    // projects R into st.
    if (dresultdx) {
        for (int i = 0; i < chans; ++i)
            dresultdx[i] = 0.0f;
        for (int i = 0; i < chans; ++i)
            dresultdy[i] = 0.0f;
    }
    if (alpha) {
        alpha[0] = r.c[chans];
        if (dalphadx)
            dalphadx[0] = 0.0f;
        if (dalphady)
            dalphady[0] = 0.0f;
    }

    if (errormessage)
        *errormessage = em.hash();
    return ok;
}



inline TypeDesc
texture_typedesc(int type, int arraylen, int aggregate)
{
    TypeDesc typedesc     = {};
    typedesc.basetype     = (unsigned char)type;
    typedesc.aggregate    = (unsigned char)aggregate;
    typedesc.arraylen     = arraylen;
    return typedesc;
}

OSL_SHADEOP int
osl_get_textureinfo(OpaqueExecContextPtr oec, ustringhash_pod name,
                    thread void* handle, ustringhash_pod dataname, int type,
                    int arraylen, int aggregate, thread void* data,
                    thread void* errormessage_)
{
    thread ustringhash_pod* errormessage = (thread ustringhash_pod*)errormessage_;

    ustringhash em = { 0 };
    bool ok = rs_get_texture_info(oec, ustringhash_from(name), handle,
                                  0 /*FIXME-ptex*/, ustringhash_from(dataname),
                                  texture_typedesc(type, arraylen, aggregate),
                                  data, errormessage ? &em : nullptr);
    if (errormessage)
        *errormessage = ok ? 0 : em.hash();
    return ok;
}

OSL_SHADEOP int
osl_get_textureinfo_st(OpaqueExecContextPtr oec, ustringhash_pod name,
                       thread void* handle, float s, float t,
                       ustringhash_pod dataname, int type, int arraylen,
                       int aggregate, thread void* data,
                       thread void* errormessage_)
{
    thread ustringhash_pod* errormessage = (thread ustringhash_pod*)errormessage_;

    ustringhash em = { 0 };
    bool ok = rs_get_texture_info_st(oec, ustringhash_from(name), handle, s,
                                     t, 0 /*FIXME-ptex*/,
                                     ustringhash_from(dataname),
                                     texture_typedesc(type, arraylen,
                                                      aggregate),
                                     data, errormessage ? &em : nullptr);
    if (errormessage)
        *errormessage = ok ? 0 : em.hash();
    return ok;
}

}  // namespace pvt
OSL_NAMESPACE_END
