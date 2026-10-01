#!/usr/bin/env python3
# Copyright Contributors to the Open Shading Language project.
# SPDX-License-Identifier: BSD-3-Clause
# https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

"""Translate osl (and oiio) c++ sources into msl, for the metal runtime.

msl is c++17 without a generic address space: every pointer and reference
has to name one. The runtime only ever points at the calling thread's own
memory, so this adds `thread` to each pointer and reference declarator,
and leaves the rest of the code as it is:

    const Vec3& a     ->  const Vec3 thread& a
    (Dual2<float>*)p  ->  (Dual2<float> thread*)p

Conditional sections for simd and cuda are removed first with unifdef, so
only the scalar cpu path is translated, which is what the cpu results come
from. Anything the mechanical pass can't handle is fixed with explicit
textual patches, listed per file in the job file.

    metalize.py <job.json> <source root> <output dir>
"""

import json
import os
import re
import subprocess
import sys

BUILTIN_TYPES = {
    "void", "bool", "char", "short", "int", "long", "float", "double",
    "unsigned", "signed", "size_t", "auto", "int8_t", "uint8_t", "int16_t",
    "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t", "uint",
}

TOKEN = re.compile(r"""
    (?P<comment>//[^\n]*|/\*.*?\*/)
  | (?P<string>"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')
  | (?P<pp>^[ \t]*\#(?:\\\n|[^\n])*)
  | (?P<ident>[A-Za-z_]\w*)
  | (?P<number>\.?\d(?:[\w.]|[eEpP][+-])*)
  | (?P<punct>::|->|\.\.\.|&&|\|\||<<=|>>=|<=|>=|==|!=|\+\+|--|\+=|-=|\*=|/=|%=|&=|\|=|\^=|<<|[^\s\w])
  | (?P<space>\s+)
""", re.S | re.M | re.X)


def tokenize(text):
    pos = 0
    out = []
    while pos < len(text):
        m = TOKEN.match(text, pos)
        if not m:
            raise ValueError(f"can't tokenize at: {text[pos:pos + 40]!r}")
        out.append((m.lastgroup, m.group()))
        pos = m.end()
    return out


def collect_types(text):
    """Names declared as types anywhere in the text."""
    names = set()
    # The name follows any all-caps export macros, `class IMATH_EXPORT Vec3`.
    for m in re.finditer(r"\b(?:struct|class|union|enum(?:\s+class)?)\s+(?:[A-Z][A-Z0-9_]*\s+)*([A-Za-z_]\w*)", text):
        names.add(m.group(1))
    for m in re.finditer(r"\busing\s+([A-Za-z_]\w*)\s*=", text):
        names.add(m.group(1))
    for m in re.finditer(r"\btypedef\b[^;{]*?\b([A-Za-z_]\w*)\s*;", text):
        names.add(m.group(1))
    # Template parameters, including those after a nested `template<int>`.
    for m in re.finditer(r"\b(?:typename|class)\s*(?:\.\.\.)?\s*([A-Za-z_]\w*)\b(?!\s*::)", text):
        names.add(m.group(1))
    return names


def annotate(text, types):
    """Insert `thread` before every pointer or reference declarator."""
    toks = tokenize(text)
    out = []
    sig = []  # indices into out of significant tokens
    for kind, value in toks:
        if kind == "pp":
            out.append(annotate_pp(value, types))
            continue
        if kind in ("comment", "space"):
            out.append(value)
            continue
        if value in ("*", "&", "&&") and is_declarator(out, sig, types):
            # Keep the source's spacing: `Vec3 &a` becomes `Vec3 thread&a`.
            out.append(" thread")
        sig.append(len(out))
        out.append(value)
    return "".join(out)


def annotate_pp(line, types):
    """Annotate the body of a #define. Other directives pass through."""
    m = re.match(r"([ \t]*#[ \t]*define[ \t]+\w+(?:\([^)]*\))?)(.*)", line, re.S)
    if not m:
        return line
    return m.group(1) + annotate(m.group(2), types)


def is_declarator(out, sig, types):
    """Whether the * or & about to be appended follows a type, as opposed to
    being multiplication, address-of, dereference or bitwise and."""
    if not sig:
        return False
    prev = out[sig[-1]]
    if prev == "thread" or (len(sig) > 1 and out[sig[-2]] == "operator"):
        return False
    if prev == "operator":
        return False
    return ends_type(out, sig, len(sig) - 1, types)


def ends_type(out, sig, i, types):
    tok = out[sig[i]]
    if tok in ("const", "volatile"):
        return i > 0 and ends_type(out, sig, i - 1, types)
    if tok in ("*", "&"):
        # a chain like `char**`: the earlier one was already a declarator
        return out[sig[i] - 1] == " thread"
    if tok == ">":
        # the end of a template argument list, `Dual2<float>`
        depth = 0
        for j in range(i, -1, -1):
            t = out[sig[j]]
            if t == ">":
                depth += 1
            elif t == "<":
                depth -= 1
                if depth == 0:
                    return j > 0 and is_type_name(out[sig[j - 1]], types)
            elif t in (";", "{", "}"):
                return False
        return False
    return is_type_name(tok, types)


def is_type_name(tok, types):
    return tok in BUILTIN_TYPES or tok in types


# Macros that open or close a namespace, like OSL_NAMESPACE_BEGIN. They're
# declarations of their own, not the start of the next one.
NAMESPACE_MACRO = re.compile(r"\w*NAMESPACE\w*_(BEGIN|END|ENTER|EXIT)$")


def split_decls(toks, start, end):
    """Split tokens[start:end] into top-level declarations. Returns a list of
    (first, last) ranges; namespace and extern "C" bodies are entered rather
    than treated as one declaration."""
    decls = []
    i, first = start, start
    while i < end:
        kind, value = toks[i]
        if kind in ("space", "comment", "pp") and i == first or (
                kind == "ident" and i == first and NAMESPACE_MACRO.match(value)):
            decls.append((i, i + 1))
            i += 1
            first = i
            continue
        if kind == "ident" and value == "namespace" or (kind == "string" and value == '"C"'):
            j = i
            while j < end and toks[j][1] not in ("{", ";"):
                j += 1
            if j < end and toks[j][1] == "{":
                close = match_brace(toks, j)
                decls.append((first, j + 1))
                decls.extend(split_decls(toks, j + 1, close))
                decls.append((close, close + 1))
                i = first = close + 1
                continue
        if value == "{":
            close = match_brace(toks, i)
            i = close + 1
            # a function body ends the declaration; a class body is
            # followed by its `;`
            k = i
            while k < end and toks[k][0] in ("space", "comment"):
                k += 1
            if k < end and toks[k][1] == ";":
                i = k + 1
            elif not is_function_body(toks, first, close):
                continue
            decls.append((first, i))
            first = i
            continue
        if value == ";":
            decls.append((first, i + 1))
            first = i + 1
        elif (value in ("public", "private", "protected") and i + 1 < end
              and toks[i + 1][1] == ":" and all(
                  k in ("space", "comment") for k, _ in toks[first:i])):
            decls.append((first, i + 2))
            first = i + 2
            i += 2
            continue
        i += 1
    if first < end:
        decls.append((first, end))
    return decls


def is_function_body(toks, first, close):
    head = "".join(v for _, v in toks[first:close])
    return not re.match(r"\s*(template\s*<[^{]*>\s*)?(struct|class|union|enum)\b", head)


def match_brace(toks, i):
    depth = 0
    for j in range(i, len(toks)):
        if toks[j][1] == "{":
            depth += 1
        elif toks[j][1] == "}":
            depth -= 1
            if depth == 0:
                return j
    raise ValueError("unbalanced braces")


def drop_decls(text, patterns):
    """Remove the smallest declarations that match one of the patterns. A
    class whose body matches loses only the matching members; it's dropped
    whole only when its head, before the body, matches."""
    if not patterns:
        return text
    toks = tokenize(text)
    return "".join(drop_range(toks, 0, len(toks), patterns))


def drop_range(toks, start, end, patterns):
    keep = []
    for first, last in split_decls(toks, start, end):
        chunk = "".join(v for _, v in toks[first:last])
        code = "".join(v for k, v in toks[first:last] if k != "comment")
        if last == first + 1 and (toks[first][0] in ("space", "comment", "pp")
                                  or NAMESPACE_MACRO.match(toks[first][1])):
            keep.append(chunk)
            continue
        if not any(re.search(p, code) for p in patterns):
            keep.append(chunk)
            continue
        body = class_body(toks, first, last)
        if body is None:
            continue  # a matching function or variable: drop it
        open_, close = body
        head = "".join(v for k, v in toks[first:open_] if k != "comment")
        if any(re.search(p, head) for p in patterns):
            continue
        keep.append("".join(v for _, v in toks[first:open_ + 1]))
        keep.extend(drop_range(toks, open_ + 1, close, patterns))
        keep.append("".join(v for _, v in toks[close:last]))
    return keep


def class_body(toks, first, last):
    """The brace positions of a class definition's body, or None."""
    head = "".join(v for _, v in toks[first:last])
    if not re.match(r"\s*(template\s*<[^{]*>\s*)?(struct|class|union)\b", head):
        return None
    for i in range(first, last):
        if toks[i][1] == "{":
            return i, match_brace(toks, i)
    return None


def unifdef(text, defines, undefines):
    args = ["unifdef", "-b", "-x", "2"]
    args += [f"-D{d}" for d in defines]
    args += [f"-U{u}" for u in undefines]
    r = subprocess.run(args, input=text, capture_output=True, text=True)
    if r.returncode not in (0, 1):
        raise RuntimeError(r.stderr)
    return r.stdout


def apply_patches(text, patches, name):
    for p in patches:
        old, new = p["old"], p["new"]
        count = text.count(old)
        if count != p.get("count", 1):
            raise RuntimeError(f"{name}: patch expected {p.get('count', 1)} match(es), "
                               f"found {count}: {old[:60]!r}")
        text = text.replace(old, new)
    return text


def main():
    job_file, src_root, out_dir = sys.argv[1:4]
    with open(job_file) as f:
        job = json.load(f)

    sources = {}
    for unit in job["units"]:
        with open(os.path.join(src_root, os.path.expandvars(unit["source"]))) as f:
            text = f.read()
        if "lines" in unit:
            first, last = unit["lines"]
            text = "".join(text.splitlines(keepends=True)[first - 1:last])
        text = apply_patches(text, unit.get("pre", []), unit["output"])
        # A unit's own defines override the job's undefines.
        defines = job.get("define", []) + unit.get("define", [])
        defined = {d.split("=")[0] for d in unit.get("define", [])}
        undefines = [u for u in job.get("undefine", []) if u not in defined]
        text = unifdef(text, defines, undefines + unit.get("undefine", []))
        text = drop_decls(text, job.get("drop", []) + unit.get("drop", []))
        sources[unit["output"]] = (unit, text)

    # A type declared in any unit counts in all of them.
    types = set(job.get("types", []))
    for _, text in sources.values():
        types |= collect_types(text)
    types -= set(job.get("not_types", []))

    os.makedirs(out_dir, exist_ok=True)
    for name, (unit, text) in sources.items():
        text = re.sub(r'^[ \t]*#[ \t]*include[ \t]*[<"][^>"]*[>"][^\n]*\n', "", text, flags=re.M)
        text = annotate(text, types)
        for old, new in list(job.get("rename", {}).items()) + list(unit.get("rename", {}).items()):
            text = re.sub(old, new, text)
        text = apply_patches(text, unit.get("post", []), name)
        header = (f"// Generated by metalize.py from {unit['source']}. Do not edit.\n"
                  "// clang-format off\n#pragma once\n\n")
        with open(os.path.join(out_dir, name), "w") as f:
            f.write(header + unit.get("prefix", "") + text + unit.get("suffix", ""))


if __name__ == "__main__":
    main()
