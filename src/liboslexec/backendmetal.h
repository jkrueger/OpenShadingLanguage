// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

#pragma once

#include "backendcpp.h"


OSL_NAMESPACE_BEGIN

namespace pvt {  // OSL::pvt


/// Generates Metal Shading Language for a shader group, by emitting the
/// C++ of BackendCpp with the differences msl requires: every pointer needs
/// an address space, and a function can't hold static locals. The result
/// includes osl_metal_runtime.h in place of osl_cpp_runtime.h, and is meant
/// to be included by a renderer's own kernel, which calls the group entry.
class BackendMetal final : public BackendCpp {
public:
    BackendMetal(ShadingSystemImpl& shadingsys, ShaderGroup& group,
                 ShadingContext* context);

    /// The generated msl. Call after run().
    std::string metal_source() const;

    std::string lang_preamble() override;
    std::string lang_linkage_prefix() override;
    std::string lang_file_extension() override;
    std::string cpp_string_literal_rep(string_view s) const override;
    std::string lang_string_const_decl() const override;
    bool lang_has_goto() const override { return false; }
};


};  // namespace pvt
OSL_NAMESPACE_END
