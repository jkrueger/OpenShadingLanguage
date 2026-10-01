// Copyright Contributors to the Open Shading Language project.
// SPDX-License-Identifier: BSD-3-Clause
// https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

// The closure types of oslclosure.h. There, ClosureComponent, ClosureMul
// and ClosureAdd derive from ClosureColor; msl has no derived classes, so
// each one spells out the id the base would hold, at the same offset, and
// the layouts match the host's.

#pragma once

OSL_NAMESPACE_BEGIN

struct ClosureComponent;
struct ClosureMul;
struct ClosureAdd;

struct ClosureColor {
    enum ClosureID { COMPONENT_BASE_ID = 0, MUL = -1, ADD = -2 };

    int id;

    thread const ClosureComponent* as_comp() const
    {
        return reinterpret_cast<thread const ClosureComponent*>(this);
    }
    thread const ClosureMul* as_mul() const
    {
        return reinterpret_cast<thread const ClosureMul*>(this);
    }
    thread const ClosureAdd* as_add() const
    {
        return reinterpret_cast<thread const ClosureAdd*>(this);
    }
};

// 16 bytes, aligned like the host's, with the parameters right after it.
struct alignas(16) ClosureComponent {
    int id;
    Vec3 w;  ///< Weight of this component

    thread void* data() { return (thread char*)(this + 1); }
    thread const void* data() const { return (thread const char*)(this + 1); }

    template <typename T> thread const T* as() const
    {
        return reinterpret_cast<thread const T*>(data());
    }
    template <typename T> thread T* as()
    {
        return reinterpret_cast<thread T*>(data());
    }
};

struct ClosureMul {
    int id;
    Color3 weight;
    thread const ClosureColor* closure;
};

struct ClosureAdd {
    int id;
    thread const ClosureColor* closureA;
    thread const ClosureColor* closureB;
};

typedef thread ClosureColor* ClosureColorPtr;
typedef thread ClosureComponent* ClosureComponentPtr;

OSL_NAMESPACE_END
