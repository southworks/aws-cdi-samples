#include "PayloadType.h"

enum_map<CdiTools::PayloadType> CdiTools::payload_type_map{
    { "Unspecified", PayloadType::Unspecified },
    { "Video", PayloadType::Video },
    { "Audio", PayloadType::Audio },
    { "Ancillary", PayloadType::Ancillary }
};
