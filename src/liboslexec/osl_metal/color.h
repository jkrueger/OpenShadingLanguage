// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// Where the color ops find their ColorSystem on metal.
//
// The renderer uploads the bytes ShadingSystem::getattribute("colorsystem")
// points at, as an optix renderer does, and implements
// osl_metal_colorsystem() to hand them back. Each color op then builds a
// small ColorSystem in thread memory from them, which points at the
// blackbody table where it is rather than copying 3.8 KB per thread.

#pragma once

OSL_NAMESPACE_BEGIN
namespace pvt {

// Same members, in the same order, as pvt::ColorSystem on the host.
struct ColorSystemData {
    Color3 m_Red, m_Green, m_Blue;
    Color3 m_White;
    Matrix33 m_XYZ2RGB;
    Matrix33 m_RGB2XYZ;
    Color3 m_luminance_scale;
    Color3 m_blackbody_table[317];
    ustringhash m_colorspace;
};

}  // namespace pvt

// Implemented by the renderer.
constant pvt::ColorSystemData* osl_metal_colorsystem(OpaqueExecContextPtr oec);


inline pvt::ColorSystem
get_colorsystem(OpaqueExecContextPtr oec)
{
    constant pvt::ColorSystemData& d = *osl_metal_colorsystem(oec);
    pvt::ColorSystem cs;
    cs.m_Red             = copy_constant(d.m_Red);
    cs.m_Green           = copy_constant(d.m_Green);
    cs.m_Blue            = copy_constant(d.m_Blue);
    cs.m_White           = copy_constant(d.m_White);
    cs.m_XYZ2RGB         = copy_constant(d.m_XYZ2RGB);
    cs.m_RGB2XYZ         = copy_constant(d.m_RGB2XYZ);
    cs.m_luminance_scale = copy_constant(d.m_luminance_scale);
    cs.m_blackbody_table = d.m_blackbody_table;
    cs.m_colorspace      = d.m_colorspace;
    return cs;
}

OSL_NAMESPACE_END
