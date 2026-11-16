#include "core/events/event_bus.hpp"

namespace spy::events {

void EventBus::clear()
{
    m_dispatcher.clear();
    m_queue.clear();
}

}
