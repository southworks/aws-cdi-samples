#include "ChannelType.h"

enum_map<CdiTools::ChannelType> CdiTools::channel_type_map{
    { "Tcp", ChannelType::Tcp },
    { "Cdi", ChannelType::Cdi },
    { "CdiStream", ChannelType::CdiStream }
};
