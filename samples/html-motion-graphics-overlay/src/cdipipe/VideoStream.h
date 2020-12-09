#pragma once

#include "Stream.h"

namespace CdiTools
{
    class VideoStream
        : public Stream
    {
    public:
        VideoStream(uint16_t stream_identifier, int frame_width, int frame_height,
            int bytes_per_pixel, int frame_rate_numerator, int frame_rate_denominator)
            : Stream(stream_identifier, frame_width * frame_height * bytes_per_pixel)
            , frame_width_{ frame_width }
            , frame_height_{ frame_height }
            , bytes_per_pixel_{ bytes_per_pixel }
            , frame_rate_numerator_{ frame_rate_numerator }
            , frame_rate_denominator_{ frame_rate_denominator }
        {
        }

        inline PayloadType payload_type() override final { return PayloadType::Video; }
        inline int frame_width() { return frame_width_; }
        inline int frame_height() { return frame_height_; }
        inline int bytes_per_pixel() { return bytes_per_pixel_; }
        inline int frame_rate_numerator() { return frame_rate_numerator_; }
        inline int frame_rate_denominator() { return frame_rate_denominator_; }

    private:
        int frame_width_;
        int frame_height_;
        int bytes_per_pixel_;
        int frame_rate_numerator_;
        int frame_rate_denominator_;
    };
}
