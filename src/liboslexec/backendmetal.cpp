// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

#include <regex>

#include "oslexec_pvt.h"
#include "backendmetal.h"


OSL_NAMESPACE_BEGIN

namespace pvt {  // OSL::pvt


BackendMetal::BackendMetal(ShadingSystemImpl& shadingsys, ShaderGroup& group,
                           ShadingContext* ctx)
    : BackendCpp(shadingsys, group, ctx)
{
}



std::string
BackendMetal::metal_source() const
{
    // msl has no generic address space, so every pointer type BackendCpp
    // spells needs one. Everything the generated code points at (the shader
    // globals, the group data, its own temporaries) lives in the calling
    // thread, so they all become thread pointers.
    //
    // This rewrites the finished text, because BackendCpp spells most of its
    // pointers out literally, as `(void*)&...` casts in the op generators,
    // rather than through lang_ptr_syntax(). Routing those through the hook
    // would let this override it instead.
    static const std::regex pointer(
        R"(\b((?:const\s+)?(?:void|char|GroupData|OSL::\w+))\s*\*)");
    return std::regex_replace(str(), pointer, "thread $1*");
}



std::string
BackendMetal::lang_preamble()
{
    return "#include \"osl_metal_runtime.h\"";
}



std::string
BackendMetal::lang_linkage_prefix()
{
    // The group entry is included into the renderer's own msl, so it needs
    // no c linkage to be found.
    return "";
}



std::string
BackendMetal::lang_file_extension()
{
    return ".metal";
}



std::string
BackendMetal::cpp_string_literal_rep(string_view s) const
{
    // The hash is known now, so emit it, instead of asking ustring at
    // runtime: the gpu has no ustring table.
    return fmtformat("0x{:016x}ull", ustring(s).hash());
}



std::string
BackendMetal::lang_string_const_decl() const
{
    // msl forbids static locals. A literal hash costs nothing to initialize.
    return "constexpr uint64_t";
}


};  // namespace pvt
OSL_NAMESPACE_END
