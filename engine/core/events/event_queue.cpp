#include "core/events/event_queue.hpp"

#include "core/utilities/scope/scope_guard.hpp"

#include <cstddef>

namespace spy::events {

void EventQueue::process(EventDispatcher& dispatcher)
{
    if (m_processingActive)
    {
        return;
    }

    m_processingActive = true;
    auto resetProcessing = makeScopeGuard([this]() noexcept { m_processingActive = false; });
    auto clearProcessing = makeScopeGuard([this]() noexcept { m_processing.clear(); });

    m_processing.swap(m_pending);

    for (const auto& event : m_processing)
    {
        event->dispatch(dispatcher);
    }
}

void EventQueue::clear()
{
    m_pending.clear();

    if (!m_processingActive)
    {
        m_processing.clear();
    }
}

bool EventQueue::empty() const noexcept
{
    return m_pending.empty() && m_processing.empty();
}

std::size_t EventQueue::size() const noexcept
{
    return m_pending.size() + m_processing.size();
}

bool EventQueue::processing() const noexcept
{
    return m_processingActive;
}

}
