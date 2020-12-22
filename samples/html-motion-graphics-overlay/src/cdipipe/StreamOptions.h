#pragma once

#include "Enum.h"

enum class AudioChannelGrouping
{
    Mono,
    DualMono,
    Stereo,
    MatrixStereo,
    Surround_5_1,
    Surround_7_1,
    Surround_22_2,
    Sdi
};

extern enum_map<AudioChannelGrouping> audio_channel_grouping_map;

enum class AudioSamplingRate
{
    Rate48kHz,
    Rate96kHz
};

extern enum_map<AudioSamplingRate> audio_sampling_rate_map;
