#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class ChannelType
    {
        Tcp,
        Cdi,
        CdiStream
    };

    extern enum_map<ChannelType> channel_type_map;
}
