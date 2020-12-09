#include <cassert>

#include <boost/asio.hpp>

#include "TestConnection.h"
#include "Errors.h"
#include "Exceptions.h"
#include "Stream.h"
#include "VideoStream.h"
#include "AudioStream.h"
#include "AncillaryStream.h"

using namespace boost::asio;

static unsigned char colors[] = {
    0x64, 0x64, 0x61,
    0xCB, 0xCB, 0xC8,
    0xC9, 0xCA, 0x00,
    0x00, 0xCA, 0xC9,
    0x00, 0xC9, 0x00,
    0xC7, 0x00, 0xC8,
    0xC6, 0x00, 0x00,
    0x00, 0x00, 0xC5,
    0x64, 0x64, 0x61
};

namespace Test
{
    using namespace CdiTools;

    void start_stream(
        std::shared_ptr<VideoStream> stream,
        unsigned char* buffer,
        std::chrono::milliseconds& period)
    {
        int frame_width = stream->frame_width();
        int frame_height = stream->frame_height();
        int bytes_per_pixel = stream->bytes_per_pixel();
        int num_bars = sizeof(colors) / (bytes_per_pixel * sizeof(unsigned char));
        int stride = frame_width * bytes_per_pixel;
        int bar_width = stride / num_bars;

        for (int y = 0; y < frame_height; y++) {
            int line_offset = y * stride;
            for (int x = 0, color_offset = 0, color_index = 0; x < frame_width; x++) {
                int pixel_offset = x * bytes_per_pixel;
                int offset = line_offset + pixel_offset;
                if (pixel_offset % bar_width == 0 && color_index < num_bars) {
                    color_offset = color_index++ * bytes_per_pixel;
                }

                for (int b = 0; b < bytes_per_pixel; b++) {
                    buffer[offset + b] = colors[color_offset + b];
                }
            }
        }

        period = std::chrono::milliseconds(1000 * stream->frame_rate_denominator() / stream->frame_rate_numerator());
    }

    void start_stream(
        std::shared_ptr<AudioStream> stream,
        unsigned char* buffer,
        std::chrono::milliseconds& period)
    {
        // TODO: implement
    }

    void start_stream(
        std::shared_ptr<AncillaryStream> stream,
        unsigned char* buffer,
        std::chrono::milliseconds& period)
    {
        // TODO: implement
    }
}

CdiTools::TestConnection::TestConnection(const std::string& name, const std::string& host_name, unsigned short port_number,
    ConnectionMode connection_mode, ConnectionDirection connection_direction, io_context& io)
    : Connection(name, host_name, port_number, connection_mode, connection_direction, io)
    , timer_{ io }
    , period_{ 0 }
{
}

CdiTools::TestConnection::~TestConnection()
{
    LOG_TRACE << "TEST Connection '" << name_ << "' is being destroyed...";

    std::error_code ec;
    disconnect(ec);
}

void CdiTools::TestConnection::async_connect(ConnectHandler handler)
{
    if (is_connected()) {
        notify_connection_change(handler, connection_error::already_connected);
        return;
    }

    LOG_DEBUG << "Waiting to establish TEST connection to " << host_name_ << ":" << port_number_ << "...";

    set_status(ConnectionStatus::Connecting);
    start_stream();
    set_status(ConnectionStatus::Open);

    notify_connection_change(handler, std::error_code());

    LOG_DEBUG << "TEST connection to " << host_name_ << ":" << port_number_ << " was established.";

    //timer_.expires_at(timer_.expiry() + period_);
    //timer_.expires_from_now(period_);
    timer_.expires_after(period_);
}

void CdiTools::TestConnection::async_accept(ConnectHandler handler)
{
    if (is_connected()) {
        notify_connection_change(handler, connection_error::already_connected);
        return;
    }

    LOG_DEBUG << "Listening for TEST connections...";

    set_status(ConnectionStatus::Connecting);
    start_stream();
    set_status(ConnectionStatus::Open);

    notify_connection_change(handler, std::error_code());

    LOG_DEBUG << "TEST Connection accepted from " << host_name_ << ":" << port_number_ << ".";

    //timer_.expires_at(timer_.expiry() + period_);
    //timer_.expires_from_now(period_);
    timer_.expires_after(period_);
}

void CdiTools::TestConnection::disconnect(std::error_code& ec)
{
    if (is_connected()) {
        timer_.cancel();
        set_status(ConnectionStatus::Closed);
        LOG_DEBUG << "TEST connection to " << host_name_ << ":" << port_number_ << " was closed.";
    }
}

void CdiTools::TestConnection::async_receive(ReceiveHandler handler)
{
    if (!is_connected()) {
        notify_payload_received(handler, connection_error::not_connected, nullptr);
        return;
    }

    LOG_TRACE << "TEST waiting for payload #" << payload_->stream_identifier() << "/" << payloads_received_ + 1 << "...";
    timer_.async_wait([&, handler](const  boost::system::error_code& ec) {
        if (is_connected()) {
            //timer_.expires_after(period_);
            timer_.expires_at(timer_.expiry() + period_);
        }

        auto payloads_received = ++payloads_received_;

        if (ec) {
            auto payload_errors = ++payload_errors_;
            LOG_DEBUG << "TEST connection timer failure: " << ec.message() << ", code: " << ec.value() << ", total errors: " << payload_errors << ".";
        }
        else {
            LOG_TRACE << "TEST received payload #" << payload_->stream_identifier() << ":" << payloads_received
#ifdef TRACE_PAYLOADS
                << " (" << payloads_received + payload_errors_ << ")"
#endif
                << ", size:" << payload_->size() << "...";
            //async_receive(handler);

            notify_payload_received(handler, std::error_code(), payload_);
        }
    });
}

void CdiTools::TestConnection::async_transmit(Payload payload, TransmitHandler handler)
{
    if (is_connected()) {
        LOG_TRACE << "TEST transmitting payload #" << payload->stream_identifier() << "/" << payloads_transmitted_ + 1 
#ifdef TRACE_PAYLOADS
            << " (" << payload->sequence() << ")"
#endif
            << "...";
        post(io_, [&, handler, payload]() {
            //std::this_thread::sleep_for(payload->size() > 10000 ? std::chrono::milliseconds(300) : std::chrono::microseconds(10));
            auto payloads_transmitted = ++payloads_transmitted_;
            LOG_TRACE << "TEST transmitted payload #" << payload->stream_identifier() << ":" << payloads_transmitted 
#ifdef TRACE_PAYLOADS
                << " (" << payload->sequence() << ")"
#endif
                << "...";

            notify_payload_transmitted(handler, std::error_code());
        });
    }
}

void CdiTools::TestConnection::add_stream(std::shared_ptr<Stream> stream)
{
    //if (streams_.size() > 0) {
    //    throw InvalidConfigurationException(
    //        std::string("Connection '" + name_ + "' has already been assigned to stream ["
    //            + std::to_string(streams_[0]->id()) + "]. Test connections support a single stream only."));
    //}

    Connection::add_stream(stream);
}

void CdiTools::TestConnection::start_stream()
{
    // TODO: using default stream, review
    auto stream = get_stream(0);
    payload_ = std::make_shared<PayloadData>(stream->id(), stream->payload_size());
    if (payload_->total_data_size == 0) {
        LOG_ERROR << "Failed to obtain a payload buffer of size " << stream->payload_size() << " from the pool.";
        return;
    }

    unsigned char* buffer = (unsigned char*)payload_->sgl_head_ptr->address_ptr;
    switch (stream->payload_type()) {
    case PayloadType::Video:
        Test::start_stream(std::dynamic_pointer_cast<VideoStream>(stream), buffer, period_);
        break;

    case PayloadType::Audio:
        Test::start_stream(std::dynamic_pointer_cast<AudioStream>(stream), buffer, period_);
        break;

    case PayloadType::Ancillary:
        Test::start_stream(std::dynamic_pointer_cast<AncillaryStream>(stream), buffer, period_);
        break;

    default:
        assert(false);
    }
}
