#include <cassert>

#include "PayloadBuffer.h"

CdiTools::PayloadBuffer::PayloadBuffer(size_t buffer_capacity)
    : buffer_{ buffer_capacity }
{
}

bool CdiTools::PayloadBuffer::enqueue(const Payload& item)
{
    bool buffer_overrun = false;
    {
        std::lock_guard<std::mutex> lock(gate_);
        buffer_overrun = buffer_.full();
        buffer_.push_back(item);
    }

    return !buffer_overrun;
}

CdiTools::Payload CdiTools::PayloadBuffer::front()
{
    std::lock_guard<std::mutex> lock(gate_);

    auto payload = buffer_.empty() ? nullptr : buffer_.front();

    return payload;
}

void CdiTools::PayloadBuffer::pop_front()
{
    std::lock_guard<std::mutex> lock(gate_);
    assert(!buffer_.empty());
    if (!buffer_.empty()) {
        buffer_.pop_front();
    }
}

void CdiTools::PayloadBuffer::clear()
{
    std::lock_guard<std::mutex> lock(gate_);
    buffer_.clear();
}

size_t CdiTools::PayloadBuffer::size()
{
    std::lock_guard<std::mutex> lock(gate_);

    return buffer_.size();
}

bool CdiTools::PayloadBuffer::is_full()
{
    std::lock_guard<std::mutex> lock(gate_);

    return buffer_.full();
}

bool CdiTools::PayloadBuffer::is_empty()
{
    std::lock_guard<std::mutex> lock(gate_);

    return buffer_.empty();
}
