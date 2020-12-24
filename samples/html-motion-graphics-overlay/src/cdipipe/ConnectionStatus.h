#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class ConnectionStatus
    {
        Closed,
        Connecting,
        Open
    };

    extern enum_map<ConnectionStatus> connection_status_map;
}
