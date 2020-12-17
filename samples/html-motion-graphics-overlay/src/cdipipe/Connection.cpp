#include "Configuration.h"
#include "Connection.h"
#include "Exceptions.h"
#include "Stream.h"

#include "TcpConnection.h"
#include "CdiConnection.h"

using namespace boost::asio;
using namespace boost::asio::ip;

CdiTools::Connection::Connection(const std::string& name, const std::string& host_name, unsigned short port_number,
    ConnectionMode connection_mode, ConnectionDirection connection_direction, int buffer_size, io_context& io)
    : name_{ name }
    , host_name_{ host_name }
    , port_number_{ port_number }
    , mode_{ connection_mode }
    , direction_{ connection_direction }
    , io_{ io }
    , payloads_received_{ 0 }
    , payloads_transmitted_{ 0 }
    , payload_errors_{ 0 }
    , logger_{ name }
    , status_{ ConnectionStatus::Closed }
    , payload_buffer_{ buffer_size }
    , suppress_buffer_notifications_{ false }
{
}

CdiTools::Connection::~Connection()
{
    LOG_TRACE << "Connection '" << name_ << "' is being destroyed...";
}

void CdiTools::Connection::add_stream(std::shared_ptr<CdiTools::Stream> stream)
{
    switch (stream->payload_type()) {
    case PayloadType::Video:
    case PayloadType::Audio:
    case PayloadType::Ancillary:
        break;

    default:
        throw InvalidConfigurationException(std::string("Stream has an unsupported payload type."));
    }

    streams_.push_back(stream);
}

// TODO: stream_identifier 0 means default stream
std::shared_ptr<CdiTools::Stream> CdiTools::Connection::get_stream(uint16_t stream_identifier)
{
    if (stream_identifier != 0) {
        auto stream = std::find_if(begin(streams_), end(streams_),
            [stream_identifier](std::shared_ptr<Stream>& stream) { return stream->id() == stream_identifier; });
        if (stream != end(streams_)) {
            return *stream;
        }
    }

    return *begin(streams_);
}

void CdiTools::Connection::disconnect(std::error_code& ec)
{
    set_status(ConnectionStatus::Closed);
}

std::shared_ptr<CdiTools::IConnection> CdiTools::Connection::get_connection(ConnectionType connection_type,
    const std::string& name, const std::string& host_name, unsigned short port_number, ConnectionMode connection_mode, 
    ConnectionDirection connection_direction, int buffer_size, io_context& io)
{
    std::shared_ptr<Connection> connection;
    switch (connection_type) {
    case ConnectionType::Cdi:
        connection = std::make_shared<CdiConnection>(name, host_name, port_number, connection_mode, connection_direction, buffer_size, io);
        break;
    case ConnectionType::Tcp:
        connection = std::make_shared<TcpConnection>(name, host_name, port_number, connection_mode, connection_direction, buffer_size, io);
        break;
    default:
        throw InvalidConfigurationException(std::string("Failed to create unsupported connection type " + std::to_string(static_cast<int>(connection_type)) + "."));
    }

    return connection;
}

CdiTools::PayloadBuffer& CdiTools::Connection::get_buffer()
{
    size_t buffer_size = payload_buffer_.size();
    if (payload_buffer_.is_full()) {
        if (!suppress_buffer_notifications_) {
            LOG_WARNING << "Receive buffer for connection '" << get_name() << "' is full"
                << ", capacity: " << payload_buffer_.capacity()
                << ". One or more payloads will be discarded.";
            suppress_buffer_notifications_ = true;
        }
    }

    if (suppress_buffer_notifications_) {
        size_t buffer_capacity = payload_buffer_.capacity();
        const size_t low_water_mark = static_cast<size_t>(buffer_capacity * 0.8);
        suppress_buffer_notifications_ = buffer_size > low_water_mark;
    }

    return payload_buffer_;
}

void CdiTools::Connection::notify_connection_change(ConnectHandler handler, const std::error_code& ec)
{
    if (handler != nullptr) {
        if (Configuration::inline_handlers) {
            handler(ec);
        }
        else {
            post(io_, std::bind(handler, ec));
        }
    }
}

void CdiTools::Connection::notify_payload_received(ReceiveHandler handler, const std::error_code& ec, Payload payload)
{
    if (handler != nullptr) {
        if (Configuration::inline_handlers) {
            handler(ec, payload);
        }
        else {
            post(io_, std::bind(handler, ec, payload));
        }
    }
}

void CdiTools::Connection::notify_payload_transmitted(TransmitHandler handler, const std::error_code& ec)
{
    if (handler != nullptr) {
        if (Configuration::inline_handlers) {
            handler(ec);
        }
        else {
            post(io_, std::bind(handler, ec));
        }
    }
}
