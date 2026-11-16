# C++ conventions

## Non-negotiable

- No comments. Not `//`, not `/* */`, not `///`, not Doxygen blocks, not TODO/FIXME. If a line needs explaining, rename or restructure it.
- No documentation written for code: no API markdown, no design notes, no usage examples in headers.

## Language

- C++20, no compiler extensions (`CXX_EXTENSIONS OFF`).
- `#pragma once` at the top of every header.
- `namespace spy`, then one namespace per module: `spy::rhi`, `spy::ecs`, `spy::audio`.
- No `using namespace` in headers. Never `using namespace std`.

## Formatting

- 4 spaces, no tabs.
- Allman braces: opening brace on its own line, including one-line bodies.
- 100 column limit.
- Pointer and reference bind to the type: `Buffer& buffer`, `const char* text`.
- `#include` order: standard library, blank line, project headers, blank line, third-party.

## Naming

| Kind | Style | Example |
|------|-------|---------|
| Class, struct, enum class | PascalCase | `CommandBuffer`, `BlendMode` |
| Function, method | camelCase | `bindVertexBuffer` |
| Local, parameter | camelCase | `vertexCount` |
| Private/protected member | `m_` + camelCase | `m_swapchain` |
| Static/constexpr constant | `k` + PascalCase | `kMaxFramesInFlight` |
| Enum enumerator | PascalCase | `TriangleList` |
| File | snake_case | `render_context.hpp` |
| Macro / CMake option | `SPY_` + UPPER_SNAKE | `SPY_PLATFORM_WINDOWS` |

## Headers and includes

- Headers carry declarations only.
- Forward-declare classes in headers and `#include` them in the `.cpp`.
- A header must compile on its own; include everything it uses.

## Types and ownership

- Fixed-width integers from `<cstdint>`: `std::uint32_t`, `std::size_t`.
- `std::unique_ptr` for owned resources; raw pointers and references are non-owning.
- Pass strings as `std::string_view`, buffers as `std::span`.
- Constructors taking one argument are `explicit`.

## Interfaces

- Abstract interfaces: public virtual destructor, deleted copy constructor and assignment, protected default constructor.
- Pure virtual methods return `bool` for fallible lifecycle operations and carry `[[nodiscard]]` on queries.
- No exceptions across module or RHI boundaries; return `bool` and handle failure at the call site.

## Rule of zero

Declare none of the special members unless the resource demands it. Never implement a destructor without the copy/move pair that goes with it.
