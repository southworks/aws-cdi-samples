#include <iostream>
#include <iomanip>
#include <cassert>

#include <boost/asio.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/thread.hpp>

#include "Application.h"
#include "PayloadBuffer.h"
#include "Channel.h"
#include "Errors.h"
#include "Exceptions.h"
#include "VideoStream.h"
#include "AudioStream.h"
#include "AncillaryStream.h"

using boost::asio::steady_timer;

CdiTools::Channel::Channel(const std::string& name)
    : name_{ name }
    , logger_{ name }
{
}

CdiTools::Channel::~Channel()
{
    shutdown();
}

void CdiTools::Channel::start(ChannelHandler handler, int thread_pool_size)
{
    active_ = std::make_unique<boost::asio::io_context::work>(io_);

    LOG_INFO << "Waiting for channel connections to be ready...";
    open_connections(handler);

    if (thread_pool_size > 1) {
        boost::thread_group pool;
        for (int i = 0; i < thread_pool_size; i++) {
            pool.create_thread([&]() { io_.run(); });
        }

        pool.join_all();
    }
    else {
        io_.run();
    }

    LOG_INFO << "Channel shut down sucessfully.";
}

void CdiTools::Channel::open_connections(ChannelHandler handler)
{
    for (auto&& connection : connections_) {
        auto connection_handler = [=](const std::error_code& ec) {
            if (!ec) {
                LOG_INFO << "Connection '" << connection->get_name() << "' established successfully.";
                if (is_active() && ConnectionDirection::In == connection->get_direction()) {
                    if (connection->get_type() != ConnectionType::Cdi) {
                        // wait until all input connections for stream are ready
                        bool stream_ready = true;
                        for (auto&& stream : get_connection_streams(connection->get_name())) {
                            for (auto&& input : get_stream_connections(stream->id(), ConnectionDirection::In)) {
                                stream_ready = stream_ready && input->is_connected();
                            }
                        }

                        if (stream_ready) {
                            // start the read loop for the input connections
                            for (auto&& stream : get_connection_streams(connection->get_name())) {
                                for (auto&& input : get_stream_connections(stream->id(), ConnectionDirection::In)) {
                                    async_read(input, std::error_code(), handler);
                                }
                            }
                        }
                    }
                    else {
                        // set the receive handler for CDI, which starts to receive as soon as the connection is opened
                        connection->async_receive(
                            std::bind(&Channel::read_complete, shared_from_this(), connection, std::placeholders::_1, std::placeholders::_2, handler));
                    }

                    // clear output buffer for any stream associated with this connection
                    for (auto&& stream : get_connection_streams(connection->get_name())) {
                        for (auto&& output : get_stream_connections(stream->id(), ConnectionDirection::Out)) {
                            output->get_buffer().clear();
                        }
                    }
                }
                else {
                    async_write(connection, std::error_code(), handler);
                }
            }
            else {
                LOG_ERROR << "Connection '" << connection->get_name() << "' failed: " << ec.message() << ".";
                handler(ec);
            }
        };

        if (connection->get_status() == ConnectionStatus::Closed) {
            LOG_DEBUG << "Opening connection '" << connection->get_name() << "'...";
            if (connection->get_mode() == ConnectionMode::Client) {
                connection->async_connect(connection_handler);
            }
            else {
                connection->async_accept(connection_handler);
            }
        }
    }
}

void CdiTools::Channel::async_read(
    std::shared_ptr<IConnection> connection,
    const std::error_code& ec,
    ChannelHandler handler,
    std::shared_ptr<steady_timer> timer)
{
    if (!is_active()) return;

    if (ec) {
        if (connection->get_status() != ConnectionStatus::Open) {
            LOG_WARNING << "Output connection '" << connection->get_name() << "' is not ready.";
            open_connections(handler);
            return;
        }

        LOG_WARNING << "Error receiving a payload: " << ec.message();
    }

    auto payload_size = connection->get_stream(0)->payload_size();
    if (Application::get()->get_pool_free_buffer_count(payload_size) == 0) {
        if (timer == nullptr) {
            LOG_WARNING << "Memory pool '" << Application::get()->get_pool_name(payload_size) << "' is exhausted"
                << ". Throttling input '" << connection->get_name() << "'...";
        }

        if (timer == nullptr) {
            timer = std::make_shared<steady_timer>(io_);
        }

        timer->expires_from_now(std::chrono::milliseconds(300)); // TODO: determine suitable value
        timer->async_wait(std::bind(&Channel::async_read, shared_from_this(), connection, std::placeholders::_1, handler, timer));
        return;
    }

    // receiving next payload for this connection
    connection->async_receive(
        std::bind(&Channel::read_complete, shared_from_this(), connection, std::placeholders::_1, std::placeholders::_2, handler));
}

void CdiTools::Channel::read_complete(
    std::shared_ptr<IConnection> connection,
    const std::error_code& ec,
    Payload payload,
    ChannelHandler handler)
{
    // determine the payload stream and retrieve its output connections
    auto stream = get_stream(payload->stream_identifier());
    auto payloads_received = stream->received_payload();
    if (ec) {
        stream->payload_error();
    }

    if (!ec) {
        // queue the payload for transmission by each output connection in stream
        auto connections = get_stream_connections(payload->stream_identifier(), ConnectionDirection::Out);
        for (auto&& output_connection : connections) {
            if (ConnectionStatus::Open != output_connection->get_status()) {
                open_connections(handler);
                continue;
            }

            auto& buffer = output_connection->get_buffer();
            if (buffer.is_full()) {
                stream->payload_error();
            }

            buffer.enqueue(payload);
            LOG_DEBUG << "Received payload #" << payload->stream_identifier() << ":" << payloads_received
#ifdef TRACE_PAYLOADS
                << " (" << payload->sequence() << ")"
#endif
                << ", size: " << payload->get_size()
                << ", queue length/size: " << buffer.size() << "/" << buffer.capacity()
                << ".";
        }
    }

    // resume read loop
    if (connection->get_type() != ConnectionType::Cdi) {
        async_read(connection, ec, handler);
    }
}

void CdiTools::Channel::async_write(
    std::shared_ptr<IConnection> connection,
    const std::error_code& ec,
    ChannelHandler handler,
    std::shared_ptr<steady_timer> timer)
{
    if (!is_active()) return;

    if (ec) {
        if (connection->get_status() != ConnectionStatus::Open) {
            LOG_WARNING << "Output connection '" << connection->get_name() << "' is not ready.";
            open_connections(handler);
            return;
        }

        LOG_WARNING << "Error transmitting a payload: " << ec.message();
    }

    auto& buffer = connection->get_buffer();
    if (buffer.is_empty()) {
        if (timer == nullptr) {
            timer = std::make_shared<steady_timer>(io_);
        }

        timer->expires_from_now(std::chrono::milliseconds(15));
        timer->async_wait(std::bind(&Channel::async_write, shared_from_this(), connection, std::error_code(), handler, timer));
        return;
    }

    auto payload = buffer.front();
    auto stream = get_stream(payload->stream_identifier());
    // TODO: payloads transmitted might be wrong if there are multiple outputs
    LOG_TRACE << "Transmitting payload #" << payload->stream_identifier() << ":" << stream->get_payloads_transmitted() + 1
#ifdef TRACE_PAYLOADS
        << " (" << payload->sequence() << ")"
#endif
        << ", size: " << payload->get_size()
        << ", queue length/size: " << buffer.size() << "/" << buffer.capacity()
        << "...";

    connection->async_transmit(
        payload,
        std::bind(&Channel::write_complete, shared_from_this(), connection, stream, std::placeholders::_1, handler));
}

void CdiTools::Channel::write_complete(
    std::shared_ptr<IConnection> connection,
    std::shared_ptr<Stream> stream,
    const std::error_code& ec,
    ChannelHandler handler)
{
    auto payloads_transmitted = stream->transmitted_payload();
    if (ec) {
        stream->payload_error();
    }

    auto& buffer = connection->get_buffer();
    buffer.pop_front();

    if (!ec) {
#ifdef TRACE_PAYLOADS
        auto payload = buffer.front();
        auto sequence = payload != nullptr ? payload->sequence() : 0;
        auto size = payload != nullptr ? payload->get_size() : 0;
#endif

        LOG_DEBUG << "Transmitted payload #" << stream->id() << ":" << payloads_transmitted
#ifdef TRACE_PAYLOADS
            << " (" << sequence << ")"
            << ", size: " << size
#endif
            << ", queue length/size: " << buffer.size() << "/" << buffer.capacity()
            << ".";
    }

    async_write(connection, ec, handler);
}

void CdiTools::Channel::shutdown()
{
    if (active_ == nullptr) return;

    LOG_DEBUG << "Channel is shutting down...";

    active_.reset();

    for (auto&& connection : connections_) {
        std::error_code ec;
        connection->disconnect(ec);
        if (ec) {
            LOG_ERROR << "Connection'" << connection->get_name() << "' could not be closed: " << ec.message() << ", code: " << ec.value() << ".";
        }
        else {
            LOG_INFO << "Connection '" << connection->get_name() << "' closed successfully.";
        }

        connection->get_buffer().clear();
    }

    io_.stop();
}

std::shared_ptr<CdiTools::IConnection> CdiTools::Channel::add_input(ConnectionType connection_type, const std::string& name,
    const std::string& host_name, unsigned short port_number, ConnectionMode connection_mode, int buffer_size)
{
    auto connection = Connection::get_connection(connection_type, name, host_name, port_number, connection_mode, ConnectionDirection::In, buffer_size, io_);

    connections_.push_back(connection);

    return connection;
}

std::shared_ptr<CdiTools::IConnection> CdiTools::Channel::add_output(ConnectionType connection_type, const std::string& name,
    const std::string& host_name, unsigned short port_number, ConnectionMode connection_mode, int buffer_size)
{
    auto connection = Connection::get_connection(connection_type, name, host_name, port_number, connection_mode, ConnectionDirection::Out, buffer_size, io_);

    connections_.push_back(connection);

    return connection;
}

std::shared_ptr<CdiTools::Stream> CdiTools::Channel::add_video_stream(
    uint16_t stream_identifier, int frame_width, int frame_height,
    int bytes_per_pixel, int frame_rate_numerator, int frame_rate_denominator)
{
    // TODO: validate that stream_identifier is not already defined
    auto stream = std::make_shared<VideoStream>(stream_identifier, frame_width,
        frame_height, bytes_per_pixel, frame_rate_numerator, frame_rate_denominator);
    streams_.push_back(stream);

    return stream;
}

std::shared_ptr<CdiTools::Stream> CdiTools::Channel::add_audio_stream(
    uint16_t stream_identifier, AudioChannelGrouping channel_grouping,
    AudioSamplingRate audio_sampling_rate, int bytes_per_sample, const std::string& language)
{
    // TODO: validate that stream_identifier is not already defined
    auto stream = std::make_shared<AudioStream>(stream_identifier, channel_grouping, audio_sampling_rate, bytes_per_sample, language);
    streams_.push_back(stream);

    return stream;
}

std::shared_ptr<CdiTools::Stream> CdiTools::Channel::add_ancillary_stream(uint16_t stream_identifier)
{
    // TODO: validate that stream_identifier is not already defined
    auto stream = std::make_shared<AncillaryStream>(stream_identifier);
    streams_.push_back(stream);

    return stream;
}

void CdiTools::Channel::map_stream(uint16_t stream_identifier, const std::string& connection_name)
{
    auto connection = std::find_if(connections_.begin(), connections_.end(),
        [&](const auto& connection) { return connection->get_name() == connection_name; });
    if (connection == connections_.end()) {
        throw InvalidConfigurationException(std::string("Failed to map unknown connection '" + connection_name + "'."));
    }

    if ((*connection)->get_direction() == ConnectionDirection::In) {
        auto stream_connections = get_stream_connections(stream_identifier, ConnectionDirection::In);
        if (stream_connections.size() > 0) {
            throw InvalidConfigurationException(
                std::string("Stream [") + std::to_string(stream_identifier) + "] is already assigned to connection '" + stream_connections[0]->get_name()
                + "' and cannot also be assigned to connection '" + connection_name + "'. Only a single input connection is allowed per stream.");
        }
    }

    (*connection)->add_stream(get_stream(stream_identifier));

    channel_map_.insert({ connection_name, stream_identifier });
}

void CdiTools::Channel::validate_configuration()
{
    for (auto&& connection : connections_) {
        auto streams = channel_map_.left.equal_range(connection->get_name());
        if (streams.first == streams.second) {
            throw InvalidConfigurationException(
                std::string("Connection '") + connection->get_name() + "' has no stream assigned.");
        }
    }
}

void CdiTools::Channel::show_configuration()
{
    std::vector<std::shared_ptr<IConnection>> inputs;
    std::vector<std::shared_ptr<IConnection>> outputs;
    std::partition_copy(begin(connections_), end(connections_),
        std::back_inserter(inputs), std::back_inserter(outputs),
        [](const auto& item) { return item->get_direction() == ConnectionDirection::In; });

    std::cout << "# Inputs\n";
    for (auto&& connection : inputs) {
        std::cout << "  [" << std::setw(12) << std::left << connection->get_name() << "] "
            << "type: " << typeid(*connection).name()
            << "\n";
        for (auto&& stream : get_connection_streams(connection->get_name())) {
            std::cout << "    stream: " << stream->id() << "\n";
        }
    }

    std::cout << "\n# Outputs\n";
    for (auto&& connection : outputs) {
        std::cout << "  [" << std::setw(12) << std::left << connection->get_name() << "] "
            << "type: " << typeid(*connection).name()
            << "\n";
        for (auto&& stream : get_connection_streams(connection->get_name())) {
            std::cout << "    stream: " << stream->id() << "\n";
        }
    }
}

void CdiTools::Channel::show_status()
{
    for (auto&& stream : streams_) {
        std::ostringstream queue_length;
        for (auto&& connection : get_stream_connections(stream->id(), ConnectionDirection::Out)) {
            auto& buffer = connection->get_buffer();
            queue_length << (queue_length.tellp() > 0 ? ", " : "") << connection->get_name() 
                << ": " << buffer.size() << "/" << buffer.capacity();
        }

        LOG_INFO << "Stream #" << stream->id()
            << " - Rx payloads: " << stream->get_payloads_received()
            << ", Tx payloads: " << stream->get_payloads_transmitted()
            << ", errors: " << stream->get_payload_errors()
            << ", queues: " << queue_length.str();
    }
}

std::shared_ptr<CdiTools::Stream> CdiTools::Channel::get_stream(uint16_t stream_identifier)
{
    auto stream = std::find_if(streams_.begin(), streams_.end(),
        [=](const auto& stream) { return stream->id() == stream_identifier; });

    if (stream == streams_.end()) {
        throw InvalidConfigurationException(
            std::string("An unrecognized stream [") + std::to_string(stream_identifier) + "] was specified.");
    }

    return *stream;
}

std::vector<std::shared_ptr<CdiTools::IConnection>> CdiTools::Channel::get_stream_connections(uint16_t stream_identifier, ConnectionDirection direction)
{
    std::vector<std::shared_ptr<IConnection>> stream_connections;
    for (auto&& map_entry : boost::make_iterator_range(channel_map_.right.equal_range(stream_identifier))) {
        auto connection = std::find_if(connections_.begin(), connections_.end(),
            [&](const auto& connection) { return connection->get_name() == map_entry.second; });
        if (connection == connections_.end())
        {
            throw InvalidConfigurationException(
                std::string("Stream [") + std::to_string(stream_identifier) + "] is mapped to an unknown connection '" + map_entry.second + "'.");
        }

        if (ConnectionDirection::Both == direction || (*connection)->get_direction() == direction) {
            stream_connections.push_back(*connection);
        }
    }

    return stream_connections;
}

std::vector<std::shared_ptr<CdiTools::Stream>> CdiTools::Channel::get_connection_streams(const std::string& connection_name)
{
    std::vector<std::shared_ptr<Stream>> connection_streams;
    for (auto&& map_entry : boost::make_iterator_range(channel_map_.left.equal_range(connection_name))) {
        auto stream = std::find_if(streams_.begin(), streams_.end(),
            [&](const auto& stream) { return stream->id() == map_entry.second; });
        if (stream == streams_.end())
        {
            throw InvalidConfigurationException(
                std::string("Connection '") + connection_name + "' is mapped to an unknown Stream [" + std::to_string(map_entry.second) + "].");
        }

        connection_streams.push_back(*stream);
    }

    return connection_streams;
}

void CdiTools::Channel::show_stream_connections(uint16_t stream_identifier, ConnectionDirection direction)
{
    std::cout << "stream: " << stream_identifier << "\n";
    const std::vector<std::shared_ptr<IConnection>>& connections = get_stream_connections(stream_identifier, direction);
    for (auto&& connection : connections) {
        std::cout << "connection: " << connection->get_name() << " (" << (connection->get_direction() == ConnectionDirection::In ? "input" : "output") << ")\n";
    }

    std::cout << "\n";
}
