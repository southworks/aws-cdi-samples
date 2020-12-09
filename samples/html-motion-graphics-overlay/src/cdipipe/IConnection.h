#pragma once

#include <functional>

#include "Payload.h"
#include "ConnectionType.h"
#include "ConnectionStatus.h"
#include "ConnectionDirection.h"
#include "ConnectionMode.h"

namespace CdiTools
{
    class Stream;

    class IConnection {
    public:
        typedef std::function<void(const std::error_code& ec)> ConnectHandler;
        typedef std::function<void(const std::error_code& ec, Payload payload)> ReceiveHandler;
        typedef std::function<void(const std::error_code& ec)> TransmitHandler;

        virtual ~IConnection() {}

        virtual bool is_connected() const = 0;
        virtual ConnectionStatus get_status() const = 0;
        virtual void async_connect(ConnectHandler handler) = 0;
        virtual void async_accept(ConnectHandler handler) = 0;
        virtual void disconnect(std::error_code& ec) = 0;
        virtual void async_receive(ReceiveHandler handler) = 0;
        virtual void async_transmit(Payload payload, TransmitHandler handler) = 0;
        virtual ConnectionType get_type() const = 0;
        virtual const std::string& get_name() const = 0;
        virtual ConnectionDirection get_direction() const = 0;
        virtual ConnectionMode get_mode() const = 0;
        virtual int get_payloads_received() const = 0;
        virtual int get_payloads_transmitted() const = 0;
        virtual void add_stream(std::shared_ptr<Stream> stream) = 0;
        virtual std::shared_ptr<Stream> get_stream(uint16_t stream_identifier) = 0;
    };
}
