#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class ConnectionMode
    {
        Listener,
        Client
    };

    extern enum_map<ConnectionMode> connection_mode_map;
}
