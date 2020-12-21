#include "StreamOptions.h"

enum_map<AudioChannelGrouping> audio_channel_grouping_map{
    { "Mono", AudioChannelGrouping::Mono},
    { "DualMono", AudioChannelGrouping::DualMono},
    { "Stereo", AudioChannelGrouping::Stereo },
    { "MatrixStereo", AudioChannelGrouping::MatrixStereo },
    { "Surround_5_1", AudioChannelGrouping::Surround_5_1 },
    { "Surround_7_1", AudioChannelGrouping::Surround_7_1 },
    { "Surround_22_2", AudioChannelGrouping::Surround_22_2 },
    { "SDI", AudioChannelGrouping::Sdi },
};

enum_map<AudioSamplingRate> audio_samplingRate_map{
    { "Mono", AudioSamplingRate::Rate48kHz },
    { "Stereo", AudioSamplingRate::Rate96kHz }
};
