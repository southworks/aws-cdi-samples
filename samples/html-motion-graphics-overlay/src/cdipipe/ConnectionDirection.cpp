#include "ConnectionDirection.h"

enum_map<CdiTools::ConnectionDirection> CdiTools::connection_direction_map{
    { "In", ConnectionDirection::In },
    { "Out", ConnectionDirection::Out },
    { "Both", ConnectionDirection::Both }
};
