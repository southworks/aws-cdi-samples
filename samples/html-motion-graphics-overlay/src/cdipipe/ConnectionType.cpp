#include "ConnectionType.h"

enum_map<CdiTools::ConnectionType> CdiTools::connection_type_map{
    { "Tcp", ConnectionType::Tcp },
    { "Cdi", ConnectionType::Cdi }
};
