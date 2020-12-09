#pragma once

#include "Logger.h"
#include "ChannelType.h"
#include "ChannelRole.h"
#include "NetworkAdapterType.h"
#include "StreamOptions.h"

namespace CdiTools
{
    struct Configuration
    {
        static ChannelType channel_type;
        static NetworkAdapterType adapter_type;
        static std::string local_ip;
        static std::string remote_ip;
        static LogLevel log_level;
        static std::string log_file;
        static bool inline_handlers;
        static bool disable_audio;
        static int num_threads;
        static int buffer_delay;

        // buffer pool configuration
        static const uint32_t large_buffer_pool_item_size;
        static const uint32_t small_buffer_pool_item_size;
        static unsigned int large_buffer_pool_max_items;
        static unsigned int small_buffer_pool_max_items;

        // input/output port configurations
        static unsigned short port_number;
        static unsigned short video_in_port;
        static unsigned short audio_in_port;
        static unsigned short video_out_port;
        static unsigned short audio_out_port;

        // video configuration settings
        static uint16_t video_stream_id;
        static int frame_width;
        static int bytes_per_pixel;
        static int frame_height;
        static int frame_rate_numerator;
        static int frame_rate_denominator;

        // audio configuration settings
        static uint16_t audio_stream_id;
        static AudioChannelGrouping audio_channel_grouping;
        static AudioSamplingRate audio_sampling_rate;
        static int audio_bytes_per_sample;
        static std::string audio_stream_language;
    };
}
