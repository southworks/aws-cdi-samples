#include "NetworkAdapterType.h"

enum_map<CdiTools::NetworkAdapterType> CdiTools::adapter_type_map{
    { "Efa", NetworkAdapterType::Efa },
    //{ "Socket", NetworkAdapterType::Socket },
    { "SocketLibFabric", NetworkAdapterType::SocketLibFabric }
};
