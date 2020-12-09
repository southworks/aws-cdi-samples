#pragma once

#include "Enum.h"

enum class AudioChannelGrouping
{
    Mono,
    Stereo
};

extern enum_map<AudioChannelGrouping> audio_channel_grouping_map;

enum class AudioSamplingRate
{
    Rate48kHz,
    Rate96kHz
};

extern enum_map<AudioSamplingRate> audio_samplingRate_map;
