#pragma once

// Symbols shared with consumer mods.
//
// The library is a DLL, so every class or function a consumer links against has
// to be exported explicitly. `BUL_BUILD` is defined only while building the
// library itself (and the test binary, which compiles the same sources); a
// consumer sees `dllimport` instead.
//
// The C++ ABI is the compiler's: consumers must be built with the same SDK and
// toolchain pair as the library. `bul::abiVersion` (bul/Version.h) marks the
// generation that may change layout.
#if defined(BUL_BUILD)
#    define BUL_API __declspec(dllexport)
#else
#    define BUL_API __declspec(dllimport)
#endif
