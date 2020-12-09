#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class PayloadType
    {
        Unspecified,
        Video,
        Audio,
        Ancillary
    };

    extern enum_map<PayloadType> payload_type_map;
}