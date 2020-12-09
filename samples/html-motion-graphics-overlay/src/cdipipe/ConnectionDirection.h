#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class ConnectionDirection
    {
        In,
        Out,
        Both
    };

    extern enum_map<ConnectionDirection> connection_direction_map;
}
