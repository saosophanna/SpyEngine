# SpyEngine

C++20 game engine: Vulkan renderer, ECS world, editor, standalone runtime, and the `fruit_spin` game.

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
- Reuse the modules already in `engine/` before adding a replacement: memory, containers, math, time, logging, events, ECS, assets, renderer, platform. Add a new abstraction only when no existing module covers the need.
- Dependencies point downward only. Foundation and utility code never includes platform, renderer, ECS, or game headers.

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

## ECS

Include `ecs/ecs.hpp`. Components are plain structs; entities are generation-safe
handles.

```cpp
struct Position { float x = 0.0f; float y = 0.0f; };
struct Velocity { float x = 0.0f; float y = 0.0f; };

spy::ecs::Registry registry;
spy::ecs::Entity player = registry.createEntity();

registry.add<Position>(player, Position{100.0f, 200.0f});
registry.add<Velocity>(player, Velocity{10.0f, 0.0f});

registry.each<Position, const Velocity>(
    [](spy::ecs::Entity entity, Position& position, const Velocity& velocity)
    {
        position.x += velocity.x;
        position.y += velocity.y;
    });
```

| API | Use |
|-----|-----|
| `Entity`, `kInvalidEntity` | `spy::Handle` with index and generation; invalidated by destroy and `clear()` |
| `EntityManager` | Standalone create/destroy when no component storage is needed |
| `Registry` | `createEntity`, `destroyEntity`, `add`, `replace`, `remove`, `tryGet`, `has`, `reserve`, `clear` |
| `Registry::each<Ts...>` | Iterate matching entities; prefix a type with `const` for read-only access |
| `Registry::view<Ts...>` | `size()` and `each()` over an intersection; drives from the smallest pool |
| `SparseSet<T>`, `ComponentStorage<T>` | Direct O(1) typed storage when the registry is not used |
| `ISystem`, `SystemScheduler` | Ordered phases: `PreUpdate`, `FixedUpdate`, `Update`, `PostUpdate`, `Render` |

- Fallible calls return `nullptr` or `false`; no exceptions cross the ECS API.
- `destroyEntity` removes every component attached to the entity.
- Components must be move-constructible and move-assignable.
- Pointers returned by `add` and `tryGet` are invalidated by later insertions or
  removals in the same storage.
- Structural changes during `each` are unsupported; collect them and apply after
  iteration.
- Back storage with an engine allocator through `Registry(allocator)`, or pass a
  `std::pmr::memory_resource*`. `spy::memory::AllocatorResource` adapts any
  `IAllocator`.

## Module Dependency Guidelines

SpyEngine modules must follow a **one-way dependency architecture** to maintain modularity, reusability, and testability.

### Core Rules

1. **Foundation modules must remain independent.** They must not depend on Platform, Renderer, ECS, or game-specific modules.
2. **Dependencies flow downward.** Higher-level modules may depend on lower-level modules, but lower-level modules must never depend on higher-level modules.
3. **Avoid circular dependencies.** If two modules need to communicate, use interfaces, events, or dependency injection.
4. **Prefer interfaces over implementations.** Depend on abstractions rather than concrete backend classes.
5. **Keep modules independently testable.** Each module should compile and run its unit tests without requiring unrelated engine modules.
6. **Declare dependencies explicitly.** Every module must list its required CMake targets and public dependencies.

### Foundation Dependencies

- Utilities: No engine module dependencies.
- Math: Depends on Utilities.
- Time: Depends on Utilities.
- Memory: Depends on Utilities.
- Events: Depends on Utilities.
- Lifecycle: Depends on Utilities and may use Time for update scheduling.

### Architecture Rule

```text
Higher-Level Modules
        |
        v
Engine Systems
        |
        v
Foundation
        |
        v
C++ Standard Library
```

### Important

- Never introduce circular dependencies.
- Do not use global access to bypass module dependencies.
- Do not expose platform-specific or renderer-specific types in Foundation APIs.
- Use Events or interfaces when modules need indirect communication.
- Keep public headers free from unnecessary implementation dependencies.

**Goal:** Every module should have a clear responsibility, explicit dependencies, and the ability to evolve independently without breaking unrelated modules.
