#pragma once

#include "PayloadType.h"
#include "StreamOptions.h"

namespace CdiTools
{
    class IConnection;

    class Stream
    {
    public:
        Stream(uint16_t stream_identifier, int payload_size)
            : stream_identifier_{ stream_identifier }
            , payload_size_{ payload_size }
            , payloads_received_{ 0 }
            , payloads_transmitted_{ 0 }
            , payload_errors_{ 0 }
        {
        }

        virtual PayloadType payload_type() = 0;
        inline uint16_t id() { return stream_identifier_; }
        inline int payload_size() { return payload_size_; }
        inline int received_payload() { return ++payloads_received_; }
        inline int get_payloads_received() { return payloads_received_; }
        inline int transmitted_payload() { return ++payloads_transmitted_; }
        inline int get_payloads_transmitted() { return payloads_transmitted_; }
        inline int payload_error() { return ++payload_errors_; }
        inline int get_payload_errors() { return payload_errors_; }

    private:
        uint16_t stream_identifier_;
        int payload_size_;
        std::atomic_int payloads_received_;
        std::atomic_int payloads_transmitted_;
        std::atomic_int payload_errors_;
    };
}
