# SpyEngine

C++20 game engine: Vulkan renderer, ECS world, editor, standalone runtime, and the `khmer_survivors` game.

## Build

```sh
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

Presets: `windows-msvc`, `windows-ninja`, `linux-ninja`. Binaries land in `build/<preset>/bin/`.

## Rules

- No comments anywhere. No `//`, no `/* */`, no `///`, no Doxygen, no TODO or FIXME markers. Rename the code until it explains itself.
- No documentation files for code. No API markdown, no design notes, no per-module READMEs.
- Keep `README.md` to build instructions only.
- Every change must leave `cmake --build` and `ctest` green.

## Conventions

@.claude/rules/cpp.md
@.claude/rules/cmake.md

## Layout

| Directory | Responsibility |
|-----------|----------------|
| `cmake/` | Reusable CMake modules: options, platform, warnings, dependencies |
| `engine/core/` | Application, lifecycle, logging, time, events, memory, threading, math, utilities |
| `engine/platform/` | Window, input, filesystem, controller, OS backends |
| `engine/renderer/` | RHI interfaces, Vulkan/OpenGL backends, resources, materials, sprites, camera, lighting, batching, postprocess |
| `engine/world/` | Scenes, entities, transforms, world management |
| `engine/ecs/` | Registry, entities, components, systems |
| `engine/physics/` | Collision, broadphase, rigid bodies, spatial queries |
| `engine/animation/` | Sprite, skeletal, timeline |
| `engine/audio/` | Device, sources, mixer, effects |
| `engine/ui/` | Widgets, layout, rendering, input |
| `engine/assets/` | Manager, importer, cache, serialization, formats |
| `engine/scripting/` | Runtime, bindings, public API |
| `editor/` | Editor application, panels, gizmos, scene editing, project manager |
| `runtime/` | Game application, loop, launcher |
| `games/` | Game projects |
| `shaders/` | GLSL sources (`vulkan/`) and compiled SPIR-V (`compiled/`) |
| `third_party/` | Vendored libraries, resolved local-first |
| `tools/` | Shader compiler, asset compiler, asset packer |
| `tests/` | Unit tests, one `*_test.cpp` per suite |
