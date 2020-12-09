#pragma once

#include "Enum.h"

namespace CdiTools
{
    enum class NetworkAdapterType
    {
        Efa,
        Socket,
        SocketLibFabric
    };

    extern enum_map<NetworkAdapterType> adapter_type_map;
}
