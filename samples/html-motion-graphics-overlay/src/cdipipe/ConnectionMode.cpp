#include "ConnectionMode.h"

enum_map<CdiTools::ConnectionMode> CdiTools::connection_mode_map{
    { "Client", ConnectionMode::Client },
    { "Cdi", ConnectionMode::Listener }
};
