#pragma once

#include "Connection.h"
#include "Cdi.h"

namespace CdiTools
{
    class CdiConnection
        : public Connection
    {
    public:
        CdiConnection(const std::string& name, const std::string& host_name, unsigned short port_number,
            ConnectionMode connection_mode, ConnectionDirection connection_direction, boost::asio::io_context& io);
        ~CdiConnection() override;

        void async_connect(ConnectHandler handler) override;
        void async_accept(ConnectHandler handler) override;
        void disconnect(std::error_code& ec) override;
        void async_receive(ReceiveHandler handler) override;
        void async_transmit(Payload payload, TransmitHandler handler) override;
        inline ConnectionType get_type() const override { return ConnectionType::Cdi; }

    private:
        ConnectHandler connect_handler_;
        ReceiveHandler receive_handler_;
        TransmitHandler transmit_handler_;

        static void on_connection_change(const CdiCoreConnectionCbData* cb_data_ptr);
        static void on_payload_received(const CdiAvmRxCbData* cb_data_ptr);
        static void on_payload_transmitted(const CdiAvmTxCbData* cb_data_ptr);
        static void log_message_callback(const CdiLogMessageCbData* cb_data_ptr);

        CdiConnectionHandle connection_handle_;
    };
}
