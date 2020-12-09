#include "ChannelRole.h"

enum_map<CdiTools::ChannelRole> CdiTools::channel_role_map{
    { "None", ChannelRole::None },
    { "Transmitter", ChannelRole::Transmitter },
    { "Receiver", ChannelRole::Receiver },
    { "Bridge", ChannelRole::Bridge }
};
