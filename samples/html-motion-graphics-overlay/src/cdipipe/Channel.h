#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

#include "ChannelType.h"
#include "ChannelRole.h"
#include "Connection.h"
#include "Stream.h"

namespace CdiTools
{
    class PayloadBuffer;

    class Channel
        : public std::enable_shared_from_this<Channel>
    {
    public:
        typedef std::function<void(const std::error_code& ec)> ChannelHandler;

        Channel(const std::string& name);
        ~Channel();

        void start(ChannelHandler handler, int thread_pool_size = 0);
        void shutdown();
        std::shared_ptr<IConnection> add_input(ConnectionType connection_type, const std::string& name, const std::string& host_name,
            unsigned short port_number, ConnectionMode connection_mode, size_t buffer_size);
        std::shared_ptr<IConnection> add_output(ConnectionType connection_type, const std::string& name, const std::string& host_name,
            unsigned short port_number, ConnectionMode connection_mode, size_t buffer_size);
        std::shared_ptr<Stream> add_video_stream(uint16_t stream_identifier, int frame_width, int frame_height, int bytes_per_pixel, int frame_rate_numerator, int frame_rate_denominator);
        std::shared_ptr<Stream> add_audio_stream(uint16_t stream_identifier, AudioChannelGrouping channel_grouping, AudioSamplingRate audio_sampling_rate, int bytes_per_sample, const std::string& language);
        std::shared_ptr<Stream> add_ancillary_stream(uint16_t stream_identifier);
        void map_stream(uint16_t stream_identifier, const std::string& connection_name);
        inline const std::string& get_name() { return name_; }
        inline bool is_active() { return active_ != nullptr; }
        void validate_configuration();
        void show_configuration();

    private:
        std::shared_ptr<Stream> get_stream(uint16_t stream_identifier);
        std::vector<std::shared_ptr<IConnection>> get_stream_connections(uint16_t stream_identifier, ConnectionDirection direction = ConnectionDirection::Both);
        std::vector<std::shared_ptr<Stream>> get_connection_streams(const std::string& connection_name);
        void show_stream_connections(uint16_t stream_identifier, ConnectionDirection direction = ConnectionDirection::Both);
        void open_connections(ChannelHandler handler);
        void async_read(const std::shared_ptr<IConnection>& connection, const std::error_code& ec, ChannelHandler handler);
        void read_complete(std::shared_ptr<IConnection> connection, const std::error_code& ec, Payload payload, ChannelHandler handler);
        void async_write(const std::shared_ptr<IConnection>& connection, const std::error_code& ec, ChannelHandler handler);
        void write_complete(std::shared_ptr<IConnection> connection, std::shared_ptr<Stream> stream, const std::error_code& ec, ChannelHandler handler);
        PayloadBuffer& get_connection_buffer(const std::string& connection_name);

        std::string name_;
        boost::asio::io_context io_;
        std::unique_ptr<boost::asio::io_context::work> active_;
        std::vector<std::shared_ptr<IConnection>> connections_;
        std::map<std::string, std::pair<PayloadBuffer, bool>> connection_buffers_;
        std::vector<std::shared_ptr<Stream>> streams_;
        boost::bimap<boost::bimaps::multiset_of<std::string>, boost::bimaps::multiset_of<uint16_t>> channel_map_;
        Logger logger_;
    };
}
