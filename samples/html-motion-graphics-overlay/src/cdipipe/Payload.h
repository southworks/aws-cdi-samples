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
        PayloadData(uint16_t stream_identifier, size_t size);
        PayloadData(CdiSgList sgl, uint16_t stream_identifier);
        ~PayloadData();

        inline int stream_identifier() const { return stream_identifier_; }
        inline int size() const { return total_data_size; }
    #ifdef TRACE_PAYLOADS
        inline int sequence() const { return sequence_number_; }
    #endif

    private:
        enum class PayloadClass
        {
            Sgl,
            Buffer
        };

        uint16_t stream_identifier_;
        PayloadClass payload_class_;
        CdiSglEntry sgl_entry_;

        static Logger logger_;

    #ifdef TRACE_PAYLOADS
        int sequence_number_;
        static std::atomic_int next_sequence_number_;
    #endif
    };

    typedef std::shared_ptr<PayloadData> Payload;
}
