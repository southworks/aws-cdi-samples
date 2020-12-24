#pragma once

#include "Stream.h"

namespace CdiTools
{
    class AncillaryStream
        : public Stream
    {
    public:
        AncillaryStream(uint16_t stream_identifier)
            : Stream(stream_identifier, 1000)           // TODO: review payload size
        {
        }

        inline PayloadType get_type() override final { return PayloadType::Ancillary; }
    };
}
