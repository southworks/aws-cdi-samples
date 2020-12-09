#pragma once
//#define BLOCKING_MODE

#include <mutex>
#include <boost/circular_buffer.hpp>

#include "Payload.h"

namespace CdiTools
{
    class PayloadBuffer
    {
    public:
        PayloadBuffer();
        PayloadBuffer(size_t buffer_capacity);
    #ifdef BLOCKING_MODE
        ~PayloadBuffer();
    #endif

        bool enqueue(const Payload& item);
        Payload front();
        void pop_front();
        Payload dequeue();
        void clear();
        size_t size();
        bool is_full();
        bool is_empty();
        inline size_t capacity() const { return buffer_.capacity(); }
    #ifdef BLOCKING_MODE
        void set_complete();
    #endif

    private:
        std::mutex gate_;
    #ifdef BLOCKING_MODE
        std::condition_variable data_ready_;
        std::atomic_bool complete_;
    #endif
        boost::circular_buffer<Payload> buffer_;
    };
}
