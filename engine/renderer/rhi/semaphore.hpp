#pragma once

namespace spy::rhi
{

class Semaphore
{
public:
    virtual ~Semaphore() = default;

    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    [[nodiscard]] virtual bool isValid() const = 0;

protected:
    Semaphore() = default;
};

}
