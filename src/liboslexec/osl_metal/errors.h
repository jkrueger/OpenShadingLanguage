// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The runtime functions that report an error on the cpu, without the
// report: the gpu has nowhere to send it. Each keeps the cpu's fallback
// result.

#pragma once

// As in shadingsys.cpp: an out-of-range index clamps to the nearest end.
inline int
osl_range_check_err(int indexvalue, int length, OSL::ustringhash_pod /*symname*/,
                    OSL::OpaqueExecContextPtr /*ec*/, OSL::ustringhash_pod /*sourcefile*/,
                    int /*sourceline*/, OSL::ustringhash_pod /*groupname*/, int /*layer*/,
                    OSL::ustringhash_pod /*layername*/, OSL::ustringhash_pod /*shadername*/)
{
    if (indexvalue >= length)
        return length - 1;
    if (indexvalue < 0)
        return 0;
    return indexvalue;
}
