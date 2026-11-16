# SpyEngine

A cross-platform 2D/3D game engine written in C++20, with a Vulkan renderer,
an ECS-based world, an editor, and a standalone runtime.

## Requirements

| Tool          | Version |
|---------------|---------|
| CMake         | 3.24+   |
| C++ compiler  | MSVC 19.34+, GCC 13+, Clang 17+ |
| Vulkan SDK    | 1.3+ (optional, enables the renderer) |

## Build

```sh
cmake --preset windows-msvc        # or windows-ninja / linux-ninja
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

Outputs land in `build/<preset>/bin/`.

## Layout

| Directory     | Responsibility                                    |
|---------------|---------------------------------------------------|
| `cmake/`      | Reusable CMake modules (options, platform, deps)  |
| `engine/core/`| Application, lifecycle, logging, time, events, memory, threading, math |
| `engine/platform/` | OS abstraction: window, input, filesystem, controller |
| `engine/renderer/` | RHI interfaces plus Vulkan/OpenGL backends   |
| `engine/world/` | Scene and world management                      |
| `engine/ecs/` | Entity Component System                           |
| `engine/physics/` | Collision and rigid bodies                    |
| `engine/animation/` | Sprite, skeletal, and timeline animation    |
| `engine/audio/` | Audio device, sources, mixing, effects         |
| `engine/ui/`  | Runtime UI: widgets, layout, rendering, input     |
| `engine/assets/` | Asset loading, importing, caching, serialization |
| `engine/scripting/` | Script runtime, bindings, and public API   |
| `editor/`     | Visual development environment                    |
| `runtime/`    | Standalone game execution                         |
| `games/`      | Game projects built on the engine                 |
| `shaders/`    | GLSL sources and compiled SPIR-V                  |
| `assets/`     | Engine-wide assets                                |
| `third_party/`| Vendored dependencies (SDK, VMA, glm, EnTT, box2d, miniaudio) |
| `tools/`      | Shader compiler, asset compiler, asset packer     |
| `tests/`      | Unit tests                                        |
| `build/`      | Generated build tree (git-ignored)                |

## Dependencies

`cmake/Dependencies.cmake` resolves each library from `third_party/<Name>/` first,
then from the system via `find_package`. Nothing is downloaded automatically. To
vendor a library, drop its sources into the matching `third_party/` folder so that
`third_party/<Name>/CMakeLists.txt` exists.

## Build options

| Option | Default | Meaning |
|--------|---------|---------|
| `SPY_BUILD_RUNTIME` | ON | Build `SpyRuntime` |
| `SPY_BUILD_EDITOR` | ON | Build `SpyEditor` |
| `SPY_BUILD_GAMES` | ON | Build bundled games |
| `SPY_BUILD_TESTS` | ON | Build and register tests |
| `SPY_ENABLE_VULKAN` | ON | Vulkan backend |
| `SPY_ENABLE_OPENGL` | OFF | OpenGL backend |
| `SPY_WARNINGS_AS_ERRORS` | OFF | Treat warnings as errors |

## License

MIT - see [LICENSE](LICENSE).
