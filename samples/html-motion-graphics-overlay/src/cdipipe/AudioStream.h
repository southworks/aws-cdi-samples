#pragma once

#include "Stream.h"

namespace CdiTools
{
    class AudioStream
        : public Stream
    {
    public:
        AudioStream(uint16_t stream_identifier, AudioChannelGrouping channel_grouping,
            AudioSamplingRate sampling_rate, int bytes_per_sample, const std::string& language)
            : Stream(stream_identifier, bytes_per_sample * 2304)            // TODO: review payload size
            , channel_grouping_{ channel_grouping }
            , bytes_per_sample_{ bytes_per_sample }
            , sampling_rate_{ sampling_rate }
            , language_{ language }
        {
        }

        inline PayloadType payload_type() override final { return PayloadType::Audio; }
        inline AudioChannelGrouping channel_grouping() { return channel_grouping_; }
        inline AudioSamplingRate sampling_rate() { return sampling_rate_; }
        inline const std::string& language() { return language_; }

    private:
        int bytes_per_sample_;
        AudioChannelGrouping channel_grouping_;
        AudioSamplingRate sampling_rate_;
        std::string language_;
    };
}
