#pragma once

#include <boost/asio/io_context.hpp>

#include "IConnection.h"
#include "Logger.h"

namespace CdiTools
{
    class Connection
        : public IConnection
        , public std::enable_shared_from_this<Connection>
    {
    public:
        Connection(const std::string& name, const std::string& host_name, unsigned short port_number,
            ConnectionMode connection_mode, ConnectionDirection connection_direction, int buffer_size, boost::asio::io_context& io);
        ~Connection() override;

        inline bool is_connected() const override { return ConnectionStatus::Open == status_; }
        inline ConnectionStatus get_status() const { return status_; }
        void disconnect(std::error_code& ec) override;
        inline const std::string& get_name() const override { return name_; }
        inline ConnectionDirection get_direction() const override { return direction_; }
        inline ConnectionMode get_mode() const override { return mode_; }
        inline int get_payloads_received() const override { return payloads_received_; }
        inline int get_payloads_transmitted() const override { return payloads_transmitted_; }
        void add_stream(std::shared_ptr<Stream> stream) override;
        std::shared_ptr<Stream> get_stream(uint16_t stream_identifier) override;
        PayloadBuffer& get_buffer() override;

        static std::shared_ptr<IConnection> get_connection(ConnectionType connection_type, const std::string& name, 
            const std::string& host_name, unsigned short port_number, ConnectionMode connection_mode,
            ConnectionDirection connection_direction, int buffer_size, boost::asio::io_context& io_context);

    protected:
        inline void set_status(ConnectionStatus status) { status_ = status; }
        void notify_connection_change(ConnectHandler handler, const std::error_code& ec);
        void notify_payload_received(ReceiveHandler handler, const std::error_code& ec, Payload payload);
        void notify_payload_transmitted(TransmitHandler handler, const std::error_code& ec);

        std::string name_;
        std::string host_name_;
        unsigned short port_number_;
        ConnectionMode mode_;
        ConnectionDirection direction_;
        std::vector<std::shared_ptr<Stream>> streams_;
        boost::asio::io_context& io_;
        Logger logger_;
        std::atomic_int payloads_received_;
        std::atomic_int payloads_transmitted_;
        std::atomic_int payload_errors_;
        PayloadBuffer payload_buffer_;
        bool suppress_buffer_notifications_;

    private:
        ConnectionStatus status_;
    };
}
