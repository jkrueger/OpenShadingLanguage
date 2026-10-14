// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The renderer services the metal runtime calls, which the renderer's own
// msl implements, as rs_free_function.h describes them for other targets.
// A renderer only has to implement the ones its shaders reach.

#pragma once

/// Allocates memory for a closure, from memory the renderer owns for the
/// shading point, such as a pool in thread memory. May return null if
/// there's no room left.
thread void* rs_allocate_closure(OSL::OpaqueExecContextPtr oec, size_t size,
                                 size_t alignment);


// Texture lookups. These follow rs_free_function.h, without the texture
// system's per-thread info, which doesn't exist here. `texture_handle` is
// whatever the generated code passes, which for now is always null, so a
// renderer finds the texture by the hash of its name.
//
// Each lookup asks for `nchannels` channels, starting at the options'
// firstchannel, and writes them to `result`; the runtime adds one for alpha
// when the shader asked for it. The derivatives are with respect to the
// texture coordinates, and only asked for, by non-null pointers, when the
// shader needs them. All of these hold room for four channels, and arrive
// zeroed. Returns false if the lookup failed, for example because the
// texture doesn't exist, which the renderer may describe in `errormessage`
// when that is non-null.

namespace OIIO { struct TextureOpt; }

bool rs_texture(OSL::OpaqueExecContextPtr oec, OSL::ustringhash filename,
                thread void* texture_handle, thread OIIO::TextureOpt& options,
                float s, float t, float dsdx, float dtdx, float dsdy,
                float dtdy, int nchannels, thread float* result,
                thread float* dresultds, thread float* dresultdt,
                thread OSL::ustringhash* errormessage);

bool rs_texture3d(OSL::OpaqueExecContextPtr oec, OSL::ustringhash filename,
                  thread void* texture_handle,
                  thread OIIO::TextureOpt& options, OSL::Vec3 P,
                  OSL::Vec3 dPdx, OSL::Vec3 dPdy, OSL::Vec3 dPdz,
                  int nchannels, thread float* result,
                  thread float* dresultds, thread float* dresultdt,
                  thread float* dresultdr,
                  thread OSL::ustringhash* errormessage);

bool rs_environment(OSL::OpaqueExecContextPtr oec, OSL::ustringhash filename,
                    thread void* texture_handle,
                    thread OIIO::TextureOpt& options, OSL::Vec3 R,
                    OSL::Vec3 dRdx, OSL::Vec3 dRdy, int nchannels,
                    thread float* result, thread float* dresultds,
                    thread float* dresultdt,
                    thread OSL::ustringhash* errormessage);

// Answers gettextureinfo(): writes `dataname`'s value, of type `datatype`,
// to `data`, and returns whether the texture has it.
bool rs_get_texture_info(OSL::OpaqueExecContextPtr oec,
                         OSL::ustringhash filename,
                         thread void* texture_handle, int subimage,
                         OSL::ustringhash dataname, OSL::TypeDesc datatype,
                         thread void* data,
                         thread OSL::ustringhash* errormessage);

bool rs_get_texture_info_st(OSL::OpaqueExecContextPtr oec,
                            OSL::ustringhash filename,
                            thread void* texture_handle, float s, float t,
                            int subimage, OSL::ustringhash dataname,
                            OSL::TypeDesc datatype, thread void* data,
                            thread OSL::ustringhash* errormessage);
