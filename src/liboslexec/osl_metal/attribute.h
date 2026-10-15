// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// getattribute, written by hand rather than translated: on the cpu it asks
// the ShadingContext's RendererServices, which doesn't exist here. It goes
// to the renderer's rs_get_attribute instead (see renderer.h), the way the
// free function renderer services of rs_free_function.h do.

#pragma once

OSL_NAMESPACE_BEGIN

namespace pvt {

// The TypeDesc the generated code packed into a long long, which msl spells
// long, as the cpu's TYPEDESC macro bit-casts it back. Unpacked member by member, since msl's
// as_type only takes scalars and vectors: the four bytes in the low half,
// in member order, and arraylen in the high half.
inline TypeDesc
typedesc_from_packed(long packed)
{
    const ulong bits = (ulong)packed;

    TypeDesc type     = {};
    type.basetype     = (unsigned char)(bits & 0xff);
    type.aggregate    = (unsigned char)((bits >> 8) & 0xff);
    type.vecsemantics = (unsigned char)((bits >> 16) & 0xff);
    type.reserved     = (unsigned char)((bits >> 24) & 0xff);
    type.arraylen     = (int)(uint)(bits >> 32);
    return type;
}

// osl_get_attribute in shadingsys.cpp. An attribute that isn't an array
// lookup is asked for at index 0, which is what the generated code passes
// for it anyway.
OSL_SHADEOP int
osl_get_attribute(thread void* sg, int dest_derivs, ustringhash_pod obj_name,
                  ustringhash_pod attr_name, int array_lookup, int index,
                  long attr_type, thread void* attr_dest)
{
    return rs_get_attribute((OpaqueExecContextPtr)sg,
                            ustringhash_from(obj_name),
                            ustringhash_from(attr_name),
                            typedesc_from_packed(attr_type), dest_derivs != 0,
                            array_lookup ? index : 0, attr_dest)
               ? 1
               : 0;
}

}  // namespace pvt
OSL_NAMESPACE_END
