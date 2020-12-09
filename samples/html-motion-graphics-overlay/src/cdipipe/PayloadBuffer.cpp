#include <cassert>

#include "PayloadBuffer.h"

CdiTools::PayloadBuffer::PayloadBuffer()
{
}

CdiTools::PayloadBuffer::PayloadBuffer(size_t buffer_capacity)
    : buffer_{ buffer_capacity }
#ifdef BLOCKING_MODE
    , complete_(false)
#endif
{
}

#ifdef BLOCKING_MODE
CdiTools::PayloadBuffer::~PayloadBuffer()
{
    set_complete();
}
#endif

bool CdiTools::PayloadBuffer::enqueue(const Payload& item)
{
    bool buffer_overrun = false;
    {
        std::lock_guard<std::mutex> lock(gate_);
        buffer_overrun = buffer_.full();
        buffer_.push_back(item);
    }

#ifdef BLOCKING_MODE
    data_ready_.notify_one();
#endif

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

CdiTools::Payload CdiTools::PayloadBuffer::dequeue()
{
#ifdef BLOCKING_MODE
    std::unique_lock<std::mutex> lock(gate_);
    data_ready_.wait(lock, [&]() { return complete_ || !buffer_.empty(); });
#else
    std::lock_guard<std::mutex> lock(gate_);
#endif

    if (buffer_.size() == 0) {
        return nullptr;
    }

    auto item = buffer_.front();
    buffer_.pop_front();

    return item;
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

#ifdef BLOCKING_MODE
void CdiTools::PayloadBuffer::set_complete() {
    complete_ = true;
    {
        std::lock_guard<std::mutex> lock(gate_);
        data_ready_.notify_all();
    }
}
#endif
