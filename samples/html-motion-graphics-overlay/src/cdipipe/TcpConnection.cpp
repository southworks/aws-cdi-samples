#include <boost/asio.hpp>

#include "TcpConnection.h"
#include "Errors.h"
#include "Stream.h"
#include "Exceptions.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using asio_error = boost::system::error_code;

CdiTools::TcpConnection::TcpConnection(const std::string& name, const std::string& host_name, unsigned short port_number,
    ConnectionMode connection_mode, ConnectionDirection connection_direction, io_context& io)
    : Connection(name, host_name, port_number, connection_mode, connection_direction, io)
    , socket_{ io }
{
}

CdiTools::TcpConnection::~TcpConnection()
{
    LOG_TRACE << "TCP Connection '" << name_ << "' is being destroyed...";
}

void CdiTools::TcpConnection::async_connect(ConnectHandler handler)
{
    if (is_connected()) {
        notify_connection_change(handler, connection_error::already_connected);
        return;
    }

    LOG_DEBUG << "Waiting to establish TCP connection to " << host_name_ << ":" << port_number_ << "...";
    set_status(ConnectionStatus::Connecting);
    auto resolver = std::make_shared<tcp::resolver>(io_);
    tcp::resolver::query query(host_name_, std::to_string(port_number_));
    
    resolver->async_resolve(query, [&, resolver, handler](const asio_error& ec, tcp::resolver::results_type results)
    {
        if (ec) {
            LOG_DEBUG << "TCP connection failure: " << ec.message() << ", code: " << ec.value() << ".";
            set_status(ConnectionStatus::Closed);
            notify_connection_change(handler, ec);
            return;
        }

        LOG_TRACE << "Resolved " << host_name_ << ":" << port_number_ << " successfully...";
        boost::asio::async_connect(socket_, results.begin(), [&, handler](const asio_error& ec, tcp::resolver::iterator) {
            set_status(ec ? ConnectionStatus::Closed : ConnectionStatus::Open);
            if (is_connected()) {
                LOG_DEBUG << "TCP connection to " << socket_.remote_endpoint() << " was established.";
            }
            else {
                LOG_ERROR << "TCP connection failure: " << ec.message() << ", code: " << ec.value() << ".";
            }

            notify_connection_change(handler, ec);
        });
    });
}

void CdiTools::TcpConnection::async_accept(ConnectHandler handler)
{
    if (is_connected()) {
        notify_connection_change(handler, connection_error::already_connected);
        return;
    }

    LOG_DEBUG << "Listening for TCP connections at " << host_name_ << ":" << port_number_ << "...";

    set_status(ConnectionStatus::Connecting);
    tcp::endpoint endpoint(address_v4::loopback(), port_number_);
    auto acceptor = std::make_shared<tcp::acceptor>(io_, endpoint);
    acceptor->async_accept(socket_, [&, acceptor, handler](const asio_error& ec) {
        set_status(ec ? ConnectionStatus::Closed : ConnectionStatus::Open);
        if (is_connected()) {
            LOG_DEBUG << "TCP connection accepted from " << socket_.remote_endpoint() << ".";
        }
        else {
            LOG_DEBUG << "TCP connection failure: " << ec.message() << ", code: " << ec.value() << ".";
        }

        notify_connection_change(handler, ec);
    });
}

void CdiTools::TcpConnection::disconnect(std::error_code& ec)
{
    if (socket_.is_open()) {
        auto endpoint = socket_.remote_endpoint();
        socket_.shutdown(socket_base::shutdown_both);
        LOG_DEBUG << "TCP connection to " << endpoint << " was closed.";
    }

    asio_error err;
    socket_.close(err);
    if (err) {
        ec = err;
        LOG_DEBUG << "TCP connection close failure: " << ec.message() << ", code: " << ec.value() << ".";
    }

    set_status(ConnectionStatus::Closed);
}

void CdiTools::TcpConnection::async_receive(ReceiveHandler handler)
{
    if (!is_connected()) {
        notify_payload_received(handler, connection_error::not_connected, nullptr);
        return;
    }

    auto& default_stream = streams_[0];
    auto payload = PayloadData::create(default_stream->id(), default_stream->payload_size());
    if (payload == nullptr) {
        auto payload_errors = ++payload_errors_;
        LOG_DEBUG << "Failed to obtain a payload buffer for #" << payload->stream_identifier() << ":" << payloads_received_ + 1
            << ", size " << default_stream->payload_size()
#ifdef TRACE_PAYLOADS
            << " (" << payload->sequence() << ")"
#endif
            " from the pool, total errors : " << payload_errors << ".";

        notify_payload_received(handler, connection_error::no_buffer_space, payload);
        return;
    }

    std::vector<mutable_buffer> sgl;
    for (CdiSglEntry* sgl_entry_ptr = payload->sgl_head_ptr;
        sgl_entry_ptr != nullptr; sgl_entry_ptr = sgl_entry_ptr->next_ptr) {
        sgl.push_back(mutable_buffer{ sgl_entry_ptr->address_ptr, (size_t)sgl_entry_ptr->size_in_bytes });
    }

    LOG_TRACE << "TCP waiting for payload #" << payload->stream_identifier() << ":" << payloads_received_ + 1
#ifdef TRACE_PAYLOADS
        << " (" << payload->sequence() << ")"
#endif
        << "...";
    if (default_stream->payload_type() == PayloadType::Video) {
        async_read(socket_, sgl, [&, payload, handler](const asio_error& ec, std::size_t bytes_received) {
            auto payloads_received = ++payloads_received_;
            if (ec) {
                auto payload_errors = ++payload_errors_;
                LOG_DEBUG << "TCP receive failure: " << ec.message() << ", code: " << ec.value() << ", total errors: " << payload_errors << ".";
                if (error::connection_reset == ec || error::connection_aborted == ec || error::eof == ec) {
                    std::error_code err;
                    disconnect(err);
                }
            }
            else {
                LOG_TRACE << "TCP received payload #" << payload->stream_identifier() << "/" << payloads_received 
#ifdef TRACE_PAYLOADS
                    << " (" << payload->sequence() << ")"
#endif
                    << ", size:" << bytes_received << "...";
            }

            payload->set_size(static_cast<int>(bytes_received));
            notify_payload_received(handler, ec, payload);
        });
    }
    else {
        socket_.async_read_some(sgl, [&, payload, handler](const asio_error& ec, std::size_t bytes_received) {
            auto payloads_received = ++payloads_received_;
            if (ec) {
                auto payload_errors = ++payload_errors_;
                LOG_DEBUG << "TCP receive failure: " << ec.message() << ", code: " << ec.value() << ", total errors: " << payload_errors << ".";
                if (error::connection_reset == ec || error::connection_aborted == ec || error::eof == ec) {
                    std::error_code err;
                    disconnect(err);
                }
            }
            else {
                LOG_TRACE << "TCP received payload #" << payload->stream_identifier() << ":" << payloads_received 
#ifdef TRACE_PAYLOADS
                    << " (" << payload->sequence() << ")"
#endif
                    << ", size:" << bytes_received << "...";
            }

            payload->set_size(static_cast<int>(bytes_received));
            notify_payload_received(handler, ec, payload);
        });
    }
}

void CdiTools::TcpConnection::async_transmit(Payload payload, TransmitHandler handler)
{
    if (!is_connected()) {
        notify_payload_transmitted(handler, connection_error::not_connected);
        return;
    }

    std::vector<const_buffer> sgl;
    for (CdiSglEntry* sgl_entry_ptr = payload->sgl_head_ptr;
        sgl_entry_ptr != nullptr; sgl_entry_ptr = sgl_entry_ptr->next_ptr) {
        sgl.push_back(const_buffer{ sgl_entry_ptr->address_ptr, (size_t)sgl_entry_ptr->size_in_bytes });
    }

    LOG_TRACE << "TCP transmitting payload #" << payload->stream_identifier() << ":" << payloads_transmitted_ + 1 
#ifdef TRACE_PAYLOADS
        << " (" << payload->sequence() << ")"
#endif
        << "...";
    async_write(socket_, sgl, [&, payload, handler](const asio_error& ec, std::size_t bytes_transferred) {
        auto payloads_transmitted = ++payloads_transmitted_;
        if (ec) {
            auto payload_errors = ++payload_errors_;
            LOG_DEBUG << "TCP transmit failure: " << ec.message() << ", code: " << ec.value() << ", total errors: " << payload_errors <<".";
            if (error::connection_reset == ec || error::connection_aborted == ec || error::eof == ec) {
                std::error_code err;
                disconnect(err);
            }
        }
        else {
            LOG_TRACE << "TCP transmitted payload #" << payload->stream_identifier() << ":" << payloads_transmitted 
#ifdef TRACE_PAYLOADS
                << " (" << payload->sequence() << ")"
#endif
                << "...";
        }

        notify_payload_transmitted(handler, ec);
    });
}

void CdiTools::TcpConnection::add_stream(std::shared_ptr<Stream> stream)
{
    if (streams_.size() > 0) {
        throw InvalidConfigurationException(
            std::string("TCP connection '" + name_ + "' has already been assigned to stream [" + std::to_string(streams_[0]->id()) + "]. TCP connections support a single stream only."));
    }

    Connection::add_stream(stream);
}
