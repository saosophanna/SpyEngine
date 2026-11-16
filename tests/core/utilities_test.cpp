#include "core/utilities/utilities.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

int g_failures = 0;

void check(bool condition, const char* expression, int line) {
    if (!condition) {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expression, line);
        ++g_failures;
    }
}

struct EntityTag {};
struct TextureTag {};

using EntityId = spy::Id<EntityTag>;
using TextureId = spy::Id<TextureTag>;

struct MoveOnly {
    explicit MoveOnly(int initial) : value(initial) {}

    MoveOnly(MoveOnly&&) noexcept = default;
    MoveOnly& operator=(MoveOnly&&) noexcept = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    int value = 0;
};

struct DerivedNonCopyable : spy::NonCopyable {};

}

#define SPY_CHECK(expr) check((expr), #expr, __LINE__)

int main() {
    static_assert(sizeof(spy::int8) == 1);
    static_assert(sizeof(spy::int16) == 2);
    static_assert(sizeof(spy::int32) == 4);
    static_assert(sizeof(spy::int64) == 8);
    static_assert(sizeof(spy::uint8) == 1);
    static_assert(sizeof(spy::uint16) == 2);
    static_assert(sizeof(spy::uint32) == 4);
    static_assert(sizeof(spy::uint64) == 8);
    static_assert(sizeof(spy::float32) == 4);
    static_assert(sizeof(spy::float64) == 8);
    static_assert(sizeof(spy::byte) == 1);
    static_assert(std::is_same_v<spy::usize, std::size_t>);
    static_assert(std::is_same_v<spy::isize, std::ptrdiff_t>);

    static_assert(!std::is_convertible_v<EntityId, TextureId>);
    static_assert(!std::is_copy_constructible_v<DerivedNonCopyable>);
    static_assert(std::is_move_constructible_v<DerivedNonCopyable>);

    {
        const EntityId invalid;
        SPY_CHECK(!invalid.isValid());
        SPY_CHECK(!invalid);
        SPY_CHECK(invalid == EntityId());

        spy::IdGenerator<EntityTag> generator;
        const EntityId first = generator.next();
        const EntityId second = generator.next();

        SPY_CHECK(first.isValid());
        SPY_CHECK(second.isValid());
        SPY_CHECK(first != second);
        SPY_CHECK(first.value() == 1);
        SPY_CHECK(second.value() == 2);
        SPY_CHECK(generator.lastIssued() == second.value());
        SPY_CHECK(spy::IdHash<EntityTag>{}(first) == static_cast<std::size_t>(first.value()));
        SPY_CHECK(spy::hashValue(first) == first.value());
        SPY_CHECK(std::hash<EntityId>{}(first) == std::hash<EntityId>{}(first));

        constexpr std::size_t threadCount = 4;
        constexpr std::size_t perThread = 512;

        std::vector<std::uint64_t> generated(threadCount * perThread, 0);
        std::vector<std::thread> workers;
        workers.reserve(threadCount);

        for (std::size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
        {
            workers.emplace_back([&generator, &generated, threadIndex]() {
                const std::size_t offset = threadIndex * perThread;

                for (std::size_t index = 0; index < perThread; ++index)
                {
                    generated[offset + index] = generator.next().value();
                }
            });
        }

        for (std::thread& worker : workers)
        {
            worker.join();
        }

        std::sort(generated.begin(), generated.end());
        SPY_CHECK(std::adjacent_find(generated.begin(), generated.end()) == generated.end());
        SPY_CHECK(generated.front() == 3);
        SPY_CHECK(generated.back() == 2 + threadCount * perThread);
    }

    {
        const spy::Handle<TextureTag> invalid;
        SPY_CHECK(!invalid.isValid());
        SPY_CHECK(invalid.index() == spy::Handle<TextureTag>::kInvalidIndex);
        SPY_CHECK(invalid.generation() == 0);

        const spy::Handle<TextureTag> handle(3, 7);
        SPY_CHECK(handle.isValid());
        SPY_CHECK(handle.index() == 3);
        SPY_CHECK(handle.generation() == 7);
        SPY_CHECK(handle == spy::Handle<TextureTag>(3, 7));
        SPY_CHECK(handle != spy::Handle<TextureTag>(3, 8));
        SPY_CHECK(handle.packed() == ((static_cast<std::uint64_t>(7) << 32) | 3));
        SPY_CHECK(spy::hashValue(handle) == handle.packed());
    }

    {
        const spy::Version version{1, 2, 3};
        SPY_CHECK(version.toString() == "1.2.3");
        SPY_CHECK((version > spy::Version{1, 2, 2}));
        SPY_CHECK((version < spy::Version{1, 3, 0}));
        SPY_CHECK((spy::Version{2, 0, 0} > version));
        SPY_CHECK((version.isCompatibleWith(spy::Version{1, 2, 0})));
        SPY_CHECK((!version.isCompatibleWith(spy::Version{1, 3, 0})));
        SPY_CHECK((!version.isCompatibleWith(spy::Version{2, 0, 0})));
        SPY_CHECK(spy::hashValue(version) == spy::hashValue(spy::Version{1, 2, 3}));
    }

    {
        constexpr std::uint64_t compileTimeHash = spy::fnv1a64("Texture");
        SPY_CHECK(compileTimeHash != 0);
        SPY_CHECK(spy::fnv1a64("") == spy::kFnv1a64OffsetBasis);
        SPY_CHECK(spy::fnv1a64("a") == 0xaf63dc4c8601ec8cull);
        SPY_CHECK(spy::fnv1a64("foobar") == 0x85944171f73967e8ull);
        SPY_CHECK(spy::fnv1a32("") == spy::kFnv1a32OffsetBasis);
        SPY_CHECK(spy::fnv1a32("a") == 0xe40c292cu);
        SPY_CHECK(spy::fnv1a32("foobar") == 0xbf9cf968u);

        SPY_CHECK(spy::hashValue("Texture") == spy::hashValue(std::string_view("Texture")));
        SPY_CHECK(spy::hashValue(std::string_view("Texture")) == spy::fnv1a64("Texture"));
        SPY_CHECK(spy::hashCombine(1, 2) == spy::hashCombine(1, 2));
        SPY_CHECK(spy::hashCombine(1, 2) != spy::hashCombine(2, 1));
        SPY_CHECK(spy::Hash<std::string_view>{}("Texture") ==
                  static_cast<std::size_t>(spy::fnv1a64("Texture")));

        const spy::StringHash first("Texture");
        const spy::StringHash second("Texture");
        const spy::StringHash third("Material");
        SPY_CHECK(first == second);
        SPY_CHECK(first != third);
        SPY_CHECK(first.value() == spy::fnv1a64("Texture"));
        SPY_CHECK(spy::StringHashHasher{}("Texture") ==
                  static_cast<std::size_t>(spy::fnv1a64("Texture")));
    }

    {
        SPY_CHECK(spy::startsWith("Texture.png", "Texture"));
        SPY_CHECK(!spy::startsWith("Texture", "Texture.png"));
        SPY_CHECK(spy::endsWith("Texture.png", ".png"));
        SPY_CHECK(!spy::endsWith("Texture.png", ".jpg"));
        SPY_CHECK(spy::contains("Texture.png", "ure."));
        SPY_CHECK(!spy::contains("Texture.png", "mesh"));

        SPY_CHECK(spy::trimLeft("  text  ") == "text  ");
        SPY_CHECK(spy::trimRight("  text  ") == "  text");
        SPY_CHECK(spy::trim(" \t\r\ntext\f\v ") == "text");
        SPY_CHECK(spy::trim("   ") == "");

        SPY_CHECK(spy::toLowerAscii("TeXture.PNG") == "texture.png");
        SPY_CHECK(spy::toUpperAscii("TeXture.png") == "TEXTURE.PNG");
        SPY_CHECK(spy::equalsIgnoreCaseAscii("Texture", "texture"));
        SPY_CHECK(!spy::equalsIgnoreCaseAscii("Texture", "textures"));

        const std::vector<std::string_view> charParts = spy::split("a,b,,c", ',');
        SPY_CHECK(charParts.size() == 4);
        SPY_CHECK(charParts[0] == "a");
        SPY_CHECK(charParts[1] == "b");
        SPY_CHECK(charParts[2] == "");
        SPY_CHECK(charParts[3] == "c");
        SPY_CHECK(spy::split("", ',').empty());
        const std::vector<std::string_view> stringParts = spy::split("a::b::c", "::");
        SPY_CHECK(stringParts.size() == 3);
        SPY_CHECK(stringParts[0] == "a");
        SPY_CHECK(stringParts[1] == "b");
        SPY_CHECK(stringParts[2] == "c");
        SPY_CHECK(spy::split("a::b", "").size() == 1);

        const std::array<std::string_view, 3> joinParts{"a", "b", "c"};
        SPY_CHECK(spy::join(joinParts, ", ") == "a, b, c");
        SPY_CHECK(spy::join(std::span<const std::string_view>(), ", ") == "");

        std::string text = "one two two";
        SPY_CHECK(spy::replaceAll(text, "two", "3"));
        SPY_CHECK(text == "one 3 3");
        SPY_CHECK(!spy::replaceAll(text, "", "x"));
    }

    {
        int calls = 0;

        {
            auto guard = spy::makeScopeGuard([&calls]() { ++calls; });
            SPY_CHECK(guard.isActive());
        }

        SPY_CHECK(calls == 1);

        {
            auto guard = spy::makeScopeGuard([&calls]() { ++calls; });
            guard.dismiss();
            SPY_CHECK(!guard.isActive());
        }

        SPY_CHECK(calls == 1);

        {
            auto guard = spy::makeScopeGuard([&calls]() { ++calls; });
            auto moved = std::move(guard);
            SPY_CHECK(!guard.isActive());
            SPY_CHECK(moved.isActive());
        }

        SPY_CHECK(calls == 2);

        {
            SPY_DEFER { ++calls; };
            SPY_CHECK(calls == 2);
        }

        SPY_CHECK(calls == 3);

        {
            auto guard = spy::makeScopeGuard([&calls]() { ++calls; });
            auto moved = std::move(guard);
            moved.dismiss();
        }

        SPY_CHECK(calls == 3);
    }

    {
        spy::FixedVector<int, 3> values;
        SPY_CHECK(values.empty());
        SPY_CHECK(values.capacity() == 3);
        SPY_CHECK(values.pushBack(1) != nullptr);
        SPY_CHECK(values.pushBack(2) != nullptr);
        SPY_CHECK(values.pushBack(3) != nullptr);
        SPY_CHECK(values.full());
        SPY_CHECK(values.pushBack(4) == nullptr);
        SPY_CHECK(values.size() == 3);
        SPY_CHECK(values[0] == 1);
        SPY_CHECK(values.back() == 3);

        values.popBack();
        SPY_CHECK(values.size() == 2);
        SPY_CHECK(values.pushBack(5) != nullptr);
        SPY_CHECK(values[2] == 5);

        int sum = 0;
        for (int value : values) {
            sum += value;
        }
        SPY_CHECK(sum == 8);

        spy::FixedVector<MoveOnly, 2> moveOnly;
        SPY_CHECK(moveOnly.emplaceBack(7) != nullptr);
        SPY_CHECK(moveOnly.emplaceBack(9) != nullptr);
        SPY_CHECK(moveOnly.emplaceBack(11) == nullptr);
        SPY_CHECK(moveOnly[0].value == 7);
        SPY_CHECK(moveOnly[1].value == 9);

        spy::FixedVector<int, 3> copied(values);
        SPY_CHECK(copied.size() == 3);
        SPY_CHECK(copied[2] == 5);
    }

    {
        spy::SmallVector<int, 2> values;
        SPY_CHECK(values.empty());
        SPY_CHECK(values.capacity() == 2);

        values.pushBack(1);
        values.pushBack(2);
        SPY_CHECK(values.capacity() == 2);
        SPY_CHECK(values.size() == 2);

        values.pushBack(3);
        values.pushBack(4);
        values.pushBack(5);
        SPY_CHECK(values.size() == 5);
        SPY_CHECK(values.capacity() >= 5);
        SPY_CHECK(values[0] == 1);
        SPY_CHECK(values[2] == 3);
        SPY_CHECK(values.back() == 5);

        spy::SmallVector<int, 2> copied(values);
        SPY_CHECK(copied.size() == 5);
        SPY_CHECK(copied[4] == 5);

        spy::SmallVector<int, 2> assigned;
        assigned = copied;
        SPY_CHECK(assigned.size() == 5);
        SPY_CHECK(assigned[0] == 1);

        spy::SmallVector<int, 2> moved(std::move(values));
        SPY_CHECK(moved.size() == 5);
        SPY_CHECK(values.empty());
        SPY_CHECK(moved[3] == 4);

        moved.popBack();
        SPY_CHECK(moved.size() == 4);
        moved.clear();
        SPY_CHECK(moved.empty());

        spy::SmallVector<MoveOnly, 1> moveOnly;
        moveOnly.pushBack(MoveOnly(1));
        moveOnly.pushBack(MoveOnly(2));
        SPY_CHECK(moveOnly.size() == 2);
        SPY_CHECK(moveOnly[0].value == 1);
        SPY_CHECK(moveOnly[1].value == 2);
    }

    {
        spy::SlotMap<int, TextureTag> objects;
        SPY_CHECK(objects.empty());

        const auto first = objects.insert(10);
        const auto second = objects.insert(20);
        SPY_CHECK(first.isValid());
        SPY_CHECK(second.isValid());
        SPY_CHECK(objects.size() == 2);
        SPY_CHECK(objects.contains(first));
        SPY_CHECK(objects.contains(second));
        SPY_CHECK(objects.tryGet(first) != nullptr);
        SPY_CHECK(*objects.tryGet(second) == 20);
        SPY_CHECK(objects.get(first) == 10);

        SPY_CHECK(objects.erase(first));
        SPY_CHECK(!objects.erase(first));
        SPY_CHECK(!objects.contains(first));
        SPY_CHECK(objects.tryGet(first) == nullptr);
        SPY_CHECK(objects.size() == 1);

        const auto reused = objects.insert(30);
        SPY_CHECK(reused.index() == first.index());
        SPY_CHECK(reused.generation() != first.generation());
        SPY_CHECK(!objects.contains(first));
        SPY_CHECK(objects.contains(reused));
        SPY_CHECK(objects.contains(second));
        SPY_CHECK(*objects.tryGet(reused) == 30);
        SPY_CHECK(*objects.tryGet(second) == 20);

        int sum = 0;
        for (int value : objects) {
            sum += value;
        }
        SPY_CHECK(sum == 50);

        objects.clear();
        SPY_CHECK(objects.empty());
        SPY_CHECK(!objects.contains(reused));
    }

    if (g_failures == 0) {
        std::puts("all tests passed");
        return 0;
    }

    std::fprintf(stderr, "%d check(s) failed\n", g_failures);
    return 1;
}
