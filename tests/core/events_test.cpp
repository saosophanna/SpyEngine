#include "core/events/events.hpp"

#include <cstddef>
#include <cstdio>
#include <utility>
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

struct AlphaEvent
{
    int value;
};

struct BetaEvent
{
    int value;
};

struct GammaEvent
{
};

}

#define SPY_CHECK(expression) check((expression), #expression, __LINE__)

int main()
{
    using namespace spy::events;
    using spy::uint32;

    static_assert(eventTypeId<AlphaEvent>() == eventTypeId<AlphaEvent>());
    static_assert(eventTypeId<AlphaEvent>() != eventTypeId<BetaEvent>());

    EventDispatcher dispatcher;
    SPY_CHECK(dispatcher.subscriptionCount() == 0);
    SPY_CHECK(dispatcher.eventTypeCount() == 0);
    SPY_CHECK(dispatcher.dispatchDepth() == 0);

    std::vector<int> received;

    EventConnection first = dispatcher.subscribe<AlphaEvent>(
        [&received](const AlphaEvent& event) { received.push_back(event.value * 10); });
    EventConnection second = dispatcher.subscribe<AlphaEvent>(
        [&received](const AlphaEvent& event) { received.push_back(event.value * 10 + 1); });

    SPY_CHECK(first.isConnected());
    SPY_CHECK(second.isConnected());
    SPY_CHECK(dispatcher.subscriptionCount() == 2);
    SPY_CHECK(dispatcher.eventTypeCount() == 1);
    SPY_CHECK(dispatcher.hasSubscribers<AlphaEvent>());
    SPY_CHECK(!dispatcher.hasSubscribers<BetaEvent>());

    dispatcher.dispatch(AlphaEvent{3});
    SPY_CHECK(received.size() == 2);
    SPY_CHECK(received[0] == 30);
    SPY_CHECK(received[1] == 31);

    first.disconnect();
    SPY_CHECK(!first.isConnected());
    SPY_CHECK(dispatcher.subscriptionCount() == 1);

    dispatcher.dispatch(AlphaEvent{4});
    SPY_CHECK(received.size() == 3);
    SPY_CHECK(received[2] == 41);

    std::vector<int> betaReceived;
    EventConnection beta = dispatcher.subscribe<BetaEvent>(
        [&betaReceived](const BetaEvent& event) { betaReceived.push_back(event.value); });

    dispatcher.dispatch(AlphaEvent{5});
    SPY_CHECK(betaReceived.empty());

    dispatcher.dispatch(BetaEvent{6});
    SPY_CHECK(betaReceived.size() == 1);
    SPY_CHECK(betaReceived[0] == 6);
    beta.disconnect();

    EventConnection direct = dispatcher.subscribe<AlphaEvent>([](const AlphaEvent&) {});
    SPY_CHECK(dispatcher.unsubscribe<AlphaEvent>(direct.id()));
    SPY_CHECK(!direct.isConnected());
    SPY_CHECK(!dispatcher.unsubscribe<AlphaEvent>(direct.id()));

    EventConnection typeErased = dispatcher.subscribe<AlphaEvent>([](const AlphaEvent&) {});
    SPY_CHECK(dispatcher.disconnect(typeErased.eventType(), typeErased.id()));
    SPY_CHECK(!typeErased.isConnected());
    SPY_CHECK(!dispatcher.disconnect(typeErased.eventType(), typeErased.id()));

    std::size_t observedDepth = 0;
    dispatcher.subscribe<GammaEvent>([&dispatcher, &observedDepth](const GammaEvent&) {
        observedDepth = dispatcher.dispatchDepth();
    });

    dispatcher.dispatch(GammaEvent{});
    SPY_CHECK(observedDepth == 1);
    SPY_CHECK(dispatcher.dispatchDepth() == 0);

    std::vector<int> nestedOrder;
    EventDispatcher nestedDispatcher;
    nestedDispatcher.subscribe<AlphaEvent>(
        [&nestedDispatcher, &nestedOrder](const AlphaEvent& event) {
            nestedOrder.push_back(event.value);

            if (event.value == 1)
            {
                nestedDispatcher.dispatch(BetaEvent{2});
            }
        });
    nestedDispatcher.subscribe<BetaEvent>(
        [&nestedOrder](const BetaEvent& event) { nestedOrder.push_back(event.value); });
    nestedDispatcher.dispatch(AlphaEvent{1});

    SPY_CHECK(nestedOrder.size() == 2);
    SPY_CHECK(nestedOrder[0] == 1);
    SPY_CHECK(nestedOrder[1] == 2);

    EventDispatcher dynamicDispatcher;
    int lateCalls = 0;
    EventConnection late;
    dynamicDispatcher.subscribe<AlphaEvent>(
        [&dynamicDispatcher, &late, &lateCalls](const AlphaEvent&) {
            if (!late.isConnected())
            {
                late = dynamicDispatcher.subscribe<AlphaEvent>(
                    [&lateCalls](const AlphaEvent&) { ++lateCalls; });
            }
        });

    dynamicDispatcher.dispatch(AlphaEvent{0});
    SPY_CHECK(lateCalls == 0);

    dynamicDispatcher.dispatch(AlphaEvent{0});
    SPY_CHECK(lateCalls == 1);

    EventDispatcher removalDispatcher;
    int firstCalls = 0;
    int secondCalls = 0;
    EventConnection removable;
    removalDispatcher.subscribe<AlphaEvent>([&removable, &firstCalls](const AlphaEvent&) {
        ++firstCalls;
        removable.disconnect();
    });
    removable = removalDispatcher.subscribe<AlphaEvent>(
        [&secondCalls](const AlphaEvent&) { ++secondCalls; });

    removalDispatcher.dispatch(AlphaEvent{0});
    SPY_CHECK(firstCalls == 1);
    SPY_CHECK(secondCalls == 0);
    SPY_CHECK(!removable.isConnected());

    removalDispatcher.dispatch(AlphaEvent{0});
    SPY_CHECK(firstCalls == 2);
    SPY_CHECK(secondCalls == 0);

    EventDispatcher clearDuringDispatch;
    int callsAfterClear = 0;
    clearDuringDispatch.subscribe<AlphaEvent>(
        [&clearDuringDispatch](const AlphaEvent&) { clearDuringDispatch.clear(); });
    clearDuringDispatch.subscribe<AlphaEvent>(
        [&callsAfterClear](const AlphaEvent&) { ++callsAfterClear; });

    clearDuringDispatch.dispatch(AlphaEvent{0});
    SPY_CHECK(callsAfterClear == 0);
    SPY_CHECK(clearDuringDispatch.subscriptionCount() == 0);

    EventBus bus;
    int raiiCalls = 0;

    {
        EventSubscription subscription(bus.subscribe<AlphaEvent>(
            [&raiiCalls](const AlphaEvent&) { ++raiiCalls; }));

        SPY_CHECK(subscription.isConnected());
        bus.publish(AlphaEvent{1});
        SPY_CHECK(raiiCalls == 1);
    }

    bus.publish(AlphaEvent{1});
    SPY_CHECK(raiiCalls == 1);

    EventSubscription moved;
    {
        EventSubscription original(bus.subscribe<AlphaEvent>(
            [&raiiCalls](const AlphaEvent&) { ++raiiCalls; }));

        moved = std::move(original);
        SPY_CHECK(moved.isConnected());
        SPY_CHECK(!original.isConnected());
    }

    bus.publish(AlphaEvent{1});
    SPY_CHECK(raiiCalls == 2);

    moved.disconnect();
    bus.publish(AlphaEvent{1});
    SPY_CHECK(raiiCalls == 2);

    EventBus listenerBus;
    int listenerAlphaCalls = 0;
    int listenerBetaCalls = 0;
    EventListener listener;
    listener.track(listenerBus.subscribe<AlphaEvent>(
        [&listenerAlphaCalls](const AlphaEvent&) { ++listenerAlphaCalls; }));
    listener.track(listenerBus.subscribe<BetaEvent>(
        [&listenerBetaCalls](const BetaEvent&) { ++listenerBetaCalls; }));

    SPY_CHECK(listener.subscriptionCount() == 2);

    listenerBus.publish(AlphaEvent{1});
    listenerBus.publish(BetaEvent{1});
    SPY_CHECK(listenerAlphaCalls == 1);
    SPY_CHECK(listenerBetaCalls == 1);

    listener.disconnectAll();
    SPY_CHECK(listener.empty());

    listenerBus.publish(AlphaEvent{1});
    listenerBus.publish(BetaEvent{1});
    SPY_CHECK(listenerAlphaCalls == 1);
    SPY_CHECK(listenerBetaCalls == 1);

    EventBus eventBus;
    std::vector<int> busReceived;
    EventConnection busConnection = eventBus.subscribe<AlphaEvent>(
        [&busReceived](const AlphaEvent& event) { busReceived.push_back(event.value); });

    SPY_CHECK(busConnection.isConnected());
    eventBus.publish(AlphaEvent{7});
    SPY_CHECK(busReceived.size() == 1);
    SPY_CHECK(busReceived[0] == 7);

    EventConnection busUnsubscribed =
        eventBus.subscribe<BetaEvent>([](const BetaEvent&) {});
    SPY_CHECK(eventBus.unsubscribe<BetaEvent>(busUnsubscribed.id()));
    SPY_CHECK(!busUnsubscribed.isConnected());

    eventBus.enqueue(AlphaEvent{9});
    SPY_CHECK(eventBus.queuedEventCount() == 1);
    SPY_CHECK(eventBus.hasQueuedEvents());
    SPY_CHECK(eventBus.queue().contains<AlphaEvent>());
    SPY_CHECK(!eventBus.queue().contains<BetaEvent>());
    SPY_CHECK(busReceived.size() == 1);

    eventBus.processQueuedEvents();
    SPY_CHECK(busReceived.size() == 2);
    SPY_CHECK(busReceived[1] == 9);
    SPY_CHECK(eventBus.queuedEventCount() == 0);
    SPY_CHECK(!eventBus.hasQueuedEvents());

    EventBus queueBus;
    std::vector<int> queueValues;
    queueBus.subscribe<AlphaEvent>([&queueBus, &queueValues](const AlphaEvent& event) {
        queueValues.push_back(event.value);

        if (event.value == 1)
        {
            queueBus.enqueue(AlphaEvent{2});
        }
    });

    queueBus.enqueue(AlphaEvent{1});
    queueBus.processQueuedEvents();
    SPY_CHECK(queueValues.size() == 1);
    SPY_CHECK(queueValues[0] == 1);
    SPY_CHECK(queueBus.queuedEventCount() == 1);

    queueBus.processQueuedEvents();
    SPY_CHECK(queueValues.size() == 2);
    SPY_CHECK(queueValues[1] == 2);

    EventBus reentrantBus;
    std::vector<int> reentrantValues;
    reentrantBus.subscribe<AlphaEvent>(
        [&reentrantBus, &reentrantValues](const AlphaEvent& event) {
            reentrantValues.push_back(event.value);

            if (event.value == 3)
            {
                reentrantBus.enqueue(AlphaEvent{4});
                reentrantBus.processQueuedEvents();
            }
        });

    reentrantBus.enqueue(AlphaEvent{3});
    reentrantBus.processQueuedEvents();
    SPY_CHECK(reentrantValues.size() == 1);
    SPY_CHECK(reentrantBus.queuedEventCount() == 1);

    reentrantBus.processQueuedEvents();
    SPY_CHECK(reentrantValues.size() == 2);
    SPY_CHECK(reentrantValues[1] == 4);

    eventBus.clear();
    SPY_CHECK(eventBus.subscriptionCount() == 0);
    SPY_CHECK(!eventBus.hasQueuedEvents());

    EventBus clearBus;
    clearBus.enqueue(AlphaEvent{1});
    clearBus.clearQueue();
    SPY_CHECK(!clearBus.hasQueuedEvents());

    EventConnection orphan;
    {
        EventBus scopedBus;
        orphan = scopedBus.subscribe<AlphaEvent>([](const AlphaEvent&) {});
        SPY_CHECK(orphan.isConnected());
    }
    SPY_CHECK(!orphan.isConnected());
    orphan.disconnect();

    EventBus builtinBus;
    uint32 width = 0;
    uint32 height = 0;
    builtinBus.subscribe<WindowResizedEvent>(
        [&width, &height](const WindowResizedEvent& event) {
            width = event.width;
            height = event.height;
        });

    builtinBus.publish(WindowResizedEvent{1920, 1080});
    SPY_CHECK(width == 1920);
    SPY_CHECK(height == 1080);

    builtinBus.emplace<WindowResizedEvent>(uint32{800}, uint32{600});
    builtinBus.processQueuedEvents();
    SPY_CHECK(width == 800);
    SPY_CHECK(height == 600);

    SPY_CHECK(eventCategory<WindowResizedEvent> == EventCategory::Window);
    SPY_CHECK(hasCategory(eventCategory<KeyPressedEvent>, EventCategory::Input));
    SPY_CHECK(!hasCategory(eventCategory<KeyPressedEvent>, EventCategory::Window));
    SPY_CHECK(hasModifier(KeyModifier::Shift | KeyModifier::Control, KeyModifier::Control));
    SPY_CHECK(!hasModifier(KeyModifier::Shift, KeyModifier::Alt));

    int frameCalls = 0;
    builtinBus.subscribe<FrameBeginEvent>([&frameCalls](const FrameBeginEvent& event) {
        if (event.frameIndex == 12)
        {
            ++frameCalls;
        }
    });

    builtinBus.publish(FrameBeginEvent{12, 0.016});
    SPY_CHECK(frameCalls == 1);

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d checks failed\n", g_failures);
        return 1;
    }

    return 0;
}
