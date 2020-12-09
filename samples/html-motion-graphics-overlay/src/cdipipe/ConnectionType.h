#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class ConnectionType
    {
        Tcp,
        Cdi
    };

    extern enum_map<ConnectionType> connection_type_map;
}
