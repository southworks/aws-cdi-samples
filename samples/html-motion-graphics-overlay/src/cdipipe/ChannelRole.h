#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class ChannelRole
    {
        None,
        Transmitter,
        Receiver,
        Bridge
    };

    extern enum_map<ChannelRole> channel_role_map;
}
