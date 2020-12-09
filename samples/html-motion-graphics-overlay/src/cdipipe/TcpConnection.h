#pragma once

#include <boost/asio/ip/tcp.hpp>

#include "Connection.h"

namespace CdiTools
{
    class TcpConnection
        : public Connection
    {
    public:
        TcpConnection(const std::string& name, const std::string& host_name, unsigned short port_number,
            ConnectionMode connection_mode, ConnectionDirection connection_direction, boost::asio::io_context& io);
        ~TcpConnection() override;

        void async_connect(ConnectHandler handler) override;
        void async_accept(ConnectHandler handler) override;
        void disconnect(std::error_code& ec) override;
        void async_receive(ReceiveHandler handler) override;
        void async_transmit(Payload payload, TransmitHandler handler) override;
        inline ConnectionType get_type() const override { return ConnectionType::Tcp; }
        void add_stream(std::shared_ptr<Stream> stream) override;

    private:
        boost::asio::ip::tcp::socket socket_;
    };
}
