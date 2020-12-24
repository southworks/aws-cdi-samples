#pragma once

#include <memory>
#include <cassert>

#include <cdi_core_api.h>

#include "Logger.h"
#include "PayloadType.h"

namespace CdiTools
{
    class PayloadData : public CdiSgList
    {
    public:
        ~PayloadData();

        inline int stream_identifier() const { return stream_identifier_; }
        inline int get_size() const { return total_data_size; }
        inline void set_size(int size) { total_data_size = size; sgl_entry_.size_in_bytes = size; }
    #ifdef TRACE_PAYLOADS
        inline int sequence() const { return sequence_number_; }
    #endif

    static std::shared_ptr<PayloadData> create(uint16_t stream_identifier, size_t size);
    static std::shared_ptr<PayloadData> create(CdiSgList sgl, uint16_t stream_identifier);

    private:
        PayloadData(uint16_t stream_identifier, void* buffer_ptr, size_t size);
        PayloadData(CdiSgList sgl, uint16_t stream_identifier);

        uint16_t stream_identifier_;
        size_t allocated_size_;
        CdiSglEntry sgl_entry_;

        static Logger logger_;

    #ifdef TRACE_PAYLOADS
        int sequence_number_;
        static std::atomic_int next_sequence_number_;
    #endif
    };

    typedef std::shared_ptr<PayloadData> Payload;
}
