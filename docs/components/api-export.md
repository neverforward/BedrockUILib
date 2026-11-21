# API Export

The library is a DLL with an exported C++ API: every public class and function
carries `BUL_API` (`bul/Export.h`), and the build generates an import library
next to the dll. A consumer mod

- adds `src/` and `src-client/` of this repository to its include path,
- links `BedrockUILib.lib`, and
- is built with the same compiler toolchain and LeviLamina SDK version, because
  the C++ ABI crosses the dll boundary.
