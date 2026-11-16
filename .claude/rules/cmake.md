# CMake conventions

## Scope

- CMake 3.24 or newer. Configure and build through presets in `CMakePresets.json` only.
- Never edit anything under `build/`, `CMakeCache.txt`, or generated project files.

## Where logic belongs

| Concern | File |
|---------|------|
| User-facing switches | `cmake/BuildOptions.cmake`, named `SPY_*` |
| Platform detection, `SPY_PLATFORM_*` defines | `cmake/Platform.cmake` |
| Warnings, standards, target defaults | `cmake/CompilerOptions.cmake` |
| Third-party resolution | `cmake/Dependencies.cmake` |
| Target definitions | the nearest `CMakeLists.txt` |

## Dependencies

Resolution order in `cmake/Dependencies.cmake`, per library:

1. `third_party/<Name>/CMakeLists.txt` if populated.
2. `find_package()` from the system, vcpkg, or conan.
3. Skipped, with a `message(STATUS)` line. The target is simply not linked.

Nothing is downloaded automatically. Do not add `FetchContent` or `ExternalProject`.

## Targets

- Create libraries and executables through `spy_add_library` / `spy_target_defaults` so warnings, the C++20 standard, and platform defines are applied consistently.
- `spy_add_library` degrades to an `INTERFACE` library when a module has no sources yet.
- Public targets get namespaced aliases: `SpyEngine::Engine`, `SpyEngine::Renderer`.
- Sources are globbed per module with `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`. Never list source files by hand.
- Output directories are fixed in `BuildOptions.cmake`: `bin/` for executables, `lib/` for libraries.

## Style

- No comments in CMake files.
- Lowercase commands, quoted paths, aligned arguments within a block.
- Guard optional subtrees with the matching `SPY_*` option.
