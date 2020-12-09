#include "StreamOptions.h"

enum_map<AudioChannelGrouping> audio_channel_grouping_map{
    { "Mono", AudioChannelGrouping::Mono},
    { "Stereo", AudioChannelGrouping::Stereo }
};

enum_map<AudioSamplingRate> audio_samplingRate_map{
    { "Mono", AudioSamplingRate::Rate48kHz },
    { "Stereo", AudioSamplingRate::Rate96kHz }
};
