#include "core/memory/memory.hpp"
#include "ecs/ecs.hpp"

#include <cstddef>
#include <cstdio>
#include <memory>
#include <vector>

namespace {

int g_failures = 0;

void check(bool condition, const char* expression, int line)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expression, line);
        ++g_failures;
    }
}

struct Position
{
    float x = 0.0f;
    float y = 0.0f;
};

struct Velocity
{
    float x = 0.0f;
    float y = 0.0f;
};

struct Health
{
    int value = 0;
};

struct Marker
{
};

struct MoveOnly
{
    explicit MoveOnly(int initial) : value(initial) {}

    MoveOnly(MoveOnly&&) noexcept = default;
    MoveOnly& operator=(MoveOnly&&) noexcept = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    int value = 0;
};

class MovementSystem final : public spy::ecs::ISystem
{
public:
    bool update(spy::ecs::Registry& registry, spy::float32 deltaTime) override
    {
        registry.each<Position, const Velocity>(
            [deltaTime](spy::ecs::Entity, Position& position, const Velocity& velocity)
            {
                position.x += velocity.x * deltaTime;
                position.y += velocity.y * deltaTime;
            });

        return true;
    }
};

class RecordingSystem final : public spy::ecs::ISystem
{
public:
    RecordingSystem(std::vector<int>& order, int id, bool succeeds)
        : m_order(&order)
        , m_id(id)
        , m_succeeds(succeeds)
    {
    }

    bool update(spy::ecs::Registry&, spy::float32) override
    {
        m_order->push_back(m_id);

        return m_succeeds;
    }

private:
    std::vector<int>* m_order = nullptr;
    int m_id = 0;
    bool m_succeeds = true;
};

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

int main()
{
    using spy::ecs::Entity;

    {
        spy::ecs::EntityManager manager;

        SPY_CHECK(manager.aliveCount() == 0);
        SPY_CHECK(manager.capacity() == 0);

        const Entity first = manager.create();
        const Entity second = manager.create();

        SPY_CHECK(first.isValid());
        SPY_CHECK(second.isValid());
        SPY_CHECK(first.index() == 0);
        SPY_CHECK(second.index() == 1);
        SPY_CHECK(manager.aliveCount() == 2);
        SPY_CHECK(manager.capacity() == 2);
        SPY_CHECK(manager.isAlive(first));
        SPY_CHECK(manager.isAlive(second));
        SPY_CHECK(manager.destroy(first));
        SPY_CHECK(!manager.destroy(first));
        SPY_CHECK(!manager.isAlive(first));
        SPY_CHECK(manager.aliveCount() == 1);

        const Entity recycled = manager.create();

        SPY_CHECK(recycled.index() == first.index());
        SPY_CHECK(recycled.generation() != first.generation());
        SPY_CHECK(manager.isAlive(recycled));
        SPY_CHECK(!manager.isAlive(first));

        manager.clear();
        SPY_CHECK(manager.aliveCount() == 0);
    }

    {
        spy::memory::FreeListAllocator allocator(1U << 20U);
        spy::memory::AllocatorResource resource(allocator);
        spy::ecs::SparseSet<Position> positions(&resource);

        const Entity first(0, 1);
        const Entity second(1, 1);
        const Entity third(2, 1);

        SPY_CHECK((positions.emplace(first, Position{1.0f, 2.0f}) != nullptr));
        SPY_CHECK((positions.emplace(second, Position{2.0f, 4.0f}) != nullptr));
        SPY_CHECK((positions.emplace(third, Position{3.0f, 6.0f}) != nullptr));
        SPY_CHECK((positions.emplace(first, Position{9.0f, 9.0f}) == nullptr));
        SPY_CHECK(positions.size() == 3);
        SPY_CHECK(positions.contains(first));
        SPY_CHECK(positions.get(second).x == 2.0f);
        SPY_CHECK(positions.tryGet(third)->y == 6.0f);

        SPY_CHECK(positions.remove(second));
        SPY_CHECK(!positions.remove(second));
        SPY_CHECK(!positions.contains(second));
        SPY_CHECK(positions.contains(first));
        SPY_CHECK(positions.contains(third));
        SPY_CHECK(positions.size() == 2);
        SPY_CHECK(positions.entities()[0] == first);
        SPY_CHECK(positions.entities()[1] == third);

        const Entity stale(first.index(), first.generation() + 1U);

        SPY_CHECK(!positions.contains(stale));
        SPY_CHECK(positions.tryGet(stale) == nullptr);

        positions.clear();
        SPY_CHECK(positions.empty());
    }

    {
        spy::ecs::Registry registry;
        const Entity entity = registry.createEntity();

        registry.reserve<Position>(16);

        SPY_CHECK(registry.entityCount() == 1);
        SPY_CHECK(registry.isAlive(entity));
        SPY_CHECK(!registry.isAlive(spy::ecs::kInvalidEntity));
        SPY_CHECK((registry.add<Position>(entity, Position{1.0f, 2.0f}) != nullptr));
        SPY_CHECK((registry.add<Position>(entity, Position{3.0f, 4.0f}) == nullptr));
        SPY_CHECK((registry.add<Velocity>(entity, Velocity{5.0f, 6.0f}) != nullptr));
        SPY_CHECK(registry.add<Health>(entity, Health{50}) != nullptr);
        SPY_CHECK(registry.has<Position>(entity));
        SPY_CHECK(!registry.has<Marker>(entity));
        SPY_CHECK(registry.tryGet<Position>(entity)->x == 1.0f);
        SPY_CHECK(registry.tryGet<Health>(entity)->value == 50);
        SPY_CHECK(
            registry.add<Position>(spy::ecs::kInvalidEntity, Position{}) == nullptr);

        const spy::ecs::Registry& constRegistry = registry;

        SPY_CHECK(constRegistry.tryGet<Position>(entity) != nullptr);
        SPY_CHECK(constRegistry.tryGet<Position>(entity)->y == 2.0f);
        SPY_CHECK(constRegistry.tryGet<Marker>(entity) == nullptr);

        SPY_CHECK((registry.replace<Health>(entity, Health{75}) != nullptr));
        SPY_CHECK(registry.tryGet<Health>(entity)->value == 75);
        SPY_CHECK(registry.remove<Health>(entity));
        SPY_CHECK(!registry.remove<Health>(entity));
        SPY_CHECK(!registry.has<Health>(entity));

        SPY_CHECK(registry.destroyEntity(entity));
        SPY_CHECK(!registry.destroyEntity(entity));
        SPY_CHECK(registry.entityCount() == 0);

        const Entity recycled = registry.createEntity();

        SPY_CHECK(recycled.index() == entity.index());
        SPY_CHECK(!registry.has<Position>(recycled));
        SPY_CHECK(!registry.has<Velocity>(recycled));
    }

    {
        spy::ecs::Registry registry;
        std::vector<Entity> moving;

        for (int i = 0; i < 8; ++i)
        {
            const Entity entity = registry.createEntity();

            registry.add<Position>(entity, Position{static_cast<float>(i), 0.0f});
            registry.add<Velocity>(entity, Velocity{1.0f, 2.0f});
            moving.push_back(entity);
        }

        const Entity stationary = registry.createEntity();
        registry.add<Position>(stationary, Position{100.0f, 100.0f});

        const Entity velocityOnly = registry.createEntity();
        registry.add<Velocity>(velocityOnly, Velocity{9.0f, 9.0f});

        SPY_CHECK((registry.view<Position, Velocity>().size() == 8));
        SPY_CHECK((registry.view<Position, const Velocity>().size() == 8));
        SPY_CHECK((registry.view<Position, Marker>().size() == 0));

        std::size_t visited = 0;

        registry.each<Position, const Velocity>(
            [&visited](Entity, Position& position, const Velocity& velocity)
            {
                ++visited;
                position.x += velocity.x;
                position.y += velocity.y;
            });

        SPY_CHECK(visited == 8);
        SPY_CHECK(registry.tryGet<Position>(moving[0])->x == 1.0f);
        SPY_CHECK(registry.tryGet<Position>(moving[0])->y == 2.0f);
        SPY_CHECK(registry.tryGet<Position>(stationary)->x == 100.0f);

        SPY_CHECK(registry.destroyEntity(moving[0]));
        SPY_CHECK((registry.view<Position, Velocity>().size() == 7));

        visited = 0;
        registry.each<Position, Velocity>(
            [&visited](Entity entity, Position&, const Velocity&)
            {
                SPY_CHECK(entity.isValid());
                ++visited;
            });

        SPY_CHECK(visited == 7);
        SPY_CHECK(registry.entityCount() == 9);

        const Entity beforeClear = moving[1];
        registry.clear();
        SPY_CHECK(registry.entityCount() == 0);
        SPY_CHECK(!registry.isAlive(beforeClear));
    }

    {
        spy::ecs::Registry registry;
        const Entity entity = registry.createEntity();

        SPY_CHECK(registry.add<MoveOnly>(entity, MoveOnly(7)) != nullptr);
        SPY_CHECK(registry.tryGet<MoveOnly>(entity)->value == 7);
        SPY_CHECK(registry.remove<MoveOnly>(entity));
    }

    {
        spy::memory::FreeListAllocator allocator(1U << 20U);
        spy::ecs::Registry registry(allocator);
        const Entity entity = registry.createEntity();

        SPY_CHECK((registry.add<Position>(entity, Position{1.0f, 2.0f}) != nullptr));
        SPY_CHECK(registry.tryGet<Position>(entity)->y == 2.0f);
    }

    {
        spy::ecs::Registry registry;
        spy::ecs::SystemScheduler scheduler;

        const Entity entity = registry.createEntity();

        registry.add<Position>(entity, Position{0.0f, 0.0f});
        registry.add<Velocity>(entity, Velocity{4.0f, 6.0f});

        SPY_CHECK(!scheduler.addSystem(nullptr));
        SPY_CHECK(scheduler.addSystem(std::make_unique<MovementSystem>()));
        SPY_CHECK(scheduler.systemCount() == 1);
        SPY_CHECK(scheduler.update(registry, 0.5f));
        SPY_CHECK(registry.tryGet<Position>(entity)->x == 2.0f);
        SPY_CHECK(registry.tryGet<Position>(entity)->y == 3.0f);

        scheduler.clear();
        SPY_CHECK(scheduler.systemCount() == 0);
    }

    {
        spy::ecs::Registry registry;
        std::vector<int> order;
        spy::ecs::SystemScheduler scheduler;

        SPY_CHECK(scheduler.addSystem(
            std::make_unique<RecordingSystem>(order, 1, true),
            spy::ecs::SystemPhase::PostUpdate));
        SPY_CHECK(scheduler.addSystem(
            std::make_unique<RecordingSystem>(order, 2, true),
            spy::ecs::SystemPhase::PreUpdate));
        SPY_CHECK(scheduler.addSystem(
            std::make_unique<RecordingSystem>(order, 3, true),
            spy::ecs::SystemPhase::Update));
        SPY_CHECK(scheduler.addSystem(
            std::make_unique<RecordingSystem>(order, 4, true),
            spy::ecs::SystemPhase::Update));
        SPY_CHECK(scheduler.addSystem(
            std::make_unique<RecordingSystem>(order, 5, false),
            spy::ecs::SystemPhase::Render));

        SPY_CHECK(!scheduler.update(registry, 0.0f));

        const std::vector<int> expectedOrder{2, 3, 4, 1, 5};
        SPY_CHECK(order == expectedOrder);

        order.clear();
        SPY_CHECK(
            scheduler.updatePhase(registry, spy::ecs::SystemPhase::Update, 0.0f));

        const std::vector<int> expectedUpdate{3, 4};
        SPY_CHECK(order == expectedUpdate);

        order.clear();
        SPY_CHECK(
            scheduler.updatePhase(registry, spy::ecs::SystemPhase::FixedUpdate, 0.0f));
        SPY_CHECK(order.empty());
    }

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d check(s) failed\n", g_failures);
        return 1;
    }

    std::puts("all tests passed");
    return 0;
}
