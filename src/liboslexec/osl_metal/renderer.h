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
