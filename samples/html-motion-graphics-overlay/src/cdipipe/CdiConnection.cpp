#include "Application.h"
#include "CdiConnection.h"
#include "Errors.h"
#include "Configuration.h"
#include "Stream.h"
#include "VideoStream.h"
#include "AudioStream.h"
#include "AncillaryStream.h"

using namespace boost::asio;

CdiTools::CdiConnection::CdiConnection(const std::string& name, const std::string& host_name, unsigned short port_number,
    ConnectionMode connection_mode, ConnectionDirection connection_direction, int buffer_size, io_context& io)
    : Connection(name, host_name, port_number, connection_mode, connection_direction, buffer_size, io)
    , connection_handle_{ NULL }
    , receive_callback_{}
    , connect_callback_{}
    , tx_timeout_{ Configuration::tx_timeout > 0
        ? Configuration::tx_timeout : (1000000 * Configuration::frame_rate_denominator) / Configuration::frame_rate_numerator }
{
}

CdiTools::CdiConnection::~CdiConnection()
{
    LOG_TRACE << "CDI Connection '" << name_ << "' is being destroyed...";
}

void CdiTools::CdiConnection::async_connect(ConnectHandler handler)
{
    if (is_connected()) {
        notify_connection_change(handler, connection_error::already_connected);
        return;
    }

    connect_callback_ = { shared_from_this(), handler };

    CdiLogMethodData log_method_data = {};
    log_method_data.log_method = kLogMethodCallback;
    log_method_data.callback_data.log_msg_cb_ptr = &log_message_callback;
    log_method_data.callback_data.log_user_cb_param = this;

    CdiTxConfigData config_data = {};
    config_data.dest_ip_addr_str = host_name_.c_str();
    config_data.adapter_handle = Application::get()->get_adapter_handle();
    config_data.dest_port = port_number_;
    config_data.thread_core_num = -1;
    config_data.connection_name_str = name_.c_str();
    config_data.connection_log_method_data_ptr = &log_method_data;
    config_data.connection_cb_ptr = &on_connection_change;
    config_data.connection_user_cb_param = &connect_callback_;
    config_data.stats_cb_ptr = NULL;
    config_data.stats_user_cb_param = NULL;
    config_data.stats_config.stats_period_seconds = 0;
#ifdef ENABLE_CLOUDWATCH
    config_data.stats_config.disable_cloudwatch_stats = true;
#else
    config_data.stats_config.disable_cloudwatch_stats = false;
#endif

    LOG_DEBUG << "Waiting to establish CDI connection to " << host_name_ << ":" << port_number_ << "...";

    set_status(ConnectionStatus::Connecting);

    CdiReturnStatus rs = CdiAvmTxCreate(&config_data, &on_payload_transmitted, &connection_handle_);
    if (CdiReturnStatus::kCdiStatusOk != rs) {
        LOG_DEBUG << "CDI connection failure: " << CdiCoreStatusToString(rs) << ", code: " << rs << ".";
        notify_connection_change(handler, connection_error::connection_failure);
        return;
    }
}

void CdiTools::CdiConnection::async_accept(ConnectHandler handler)
{
    if (is_connected()) {
        notify_connection_change(handler, connection_error::already_connected);
        return;
    }

    connect_callback_ = { shared_from_this(), handler };

    CdiLogMethodData log_method_data = {};
    log_method_data.log_method = kLogMethodCallback;
    log_method_data.callback_data.log_msg_cb_ptr = &log_message_callback;
    log_method_data.callback_data.log_user_cb_param = this;

    CdiRxConfigData config_data = {};
    config_data.buffer_delay_ms = Configuration::buffer_delay;
    config_data.rx_buffer_type = CdiBufferType::kCdiSgl;
    config_data.linear_buffer_size = 0;
    config_data.user_cb_param = &receive_callback_;
    config_data.adapter_handle = Application::get()->get_adapter_handle();
    config_data.dest_port = port_number_;
    config_data.thread_core_num = -1;
    config_data.connection_name_str = name_.c_str();
    config_data.connection_log_method_data_ptr = &log_method_data;
    config_data.connection_cb_ptr = &on_connection_change;
    config_data.connection_user_cb_param = &connect_callback_;
    config_data.stats_cb_ptr = NULL;
    config_data.stats_user_cb_param = NULL;
    config_data.stats_config.stats_period_seconds = 0;
#ifdef ENABLE_CLOUDWATCH
    config_data.stats_config.disable_cloudwatch_stats = true;
#else
    config_data.stats_config.disable_cloudwatch_stats = false;
#endif

    LOG_DEBUG << "Listening for CDI connections at " << Application::get()->get_adapter_ip_address() << ":" << port_number_ << "...";

    set_status(ConnectionStatus::Connecting);

    CdiReturnStatus rs = CdiAvmRxCreate(&config_data, &on_payload_received, &connection_handle_);
    if (CdiReturnStatus::kCdiStatusOk != rs) {
        LOG_DEBUG << "CDI connection failure: " << CdiCoreStatusToString(rs) << ", code: " << rs << ".";
        notify_connection_change(handler, connection_error::connection_failure);
        return;
    }
}

void CdiTools::CdiConnection::disconnect(std::error_code& ec)
{
    if (connection_handle_ != NULL) {
        CdiReturnStatus rs = CdiCoreConnectionDestroy(connection_handle_);
        if (CdiReturnStatus::kCdiStatusOk == rs) {
            LOG_DEBUG << "CDI connection to " << host_name_ << ":" << port_number_ << " was closed.";
        }
        else {
            LOG_DEBUG << "Error closing CDI connection: " << CdiCoreStatusToString(rs) << ", code: " << rs << ".";
        }

        connection_handle_ = NULL;
    }
}

void CdiTools::CdiConnection::async_receive(ReceiveHandler handler)
{
    if (!is_connected()) {
        notify_payload_received(handler, connection_error::not_connected, nullptr);
        return;
    }

    receive_callback_ = { shared_from_this(), handler };
}

void CdiTools::CdiConnection::async_transmit(Payload payload, TransmitHandler handler)
{
    if (!is_connected()) {
        notify_payload_transmitted(handler, connection_error::not_connected);
        return;
    }

    void* callback_data = new TransmitCallbackData{ shared_from_this(), handler };

    CdiAvmTxPayloadConfig payload_config = {};
    payload_config.core_config_data.user_cb_param = callback_data;
    payload_config.avm_extra_data.stream_identifier = payload->stream_identifier();
    payload_config.core_config_data.unit_size = 0;
#ifdef TRACE_PAYLOADS
    payload_config.core_config_data.core_extra_data.payload_user_data = payload->sequence();
#else
    payload_config.core_config_data.core_extra_data.payload_user_data = 0;
#endif

    Cdi::set_ptp_timestamp(payload_config.core_config_data.core_extra_data.origination_ptp_timestamp);

    CdiReturnStatus rs;
    CdiAvmConfig avm_config;
    CdiAvmConfig* avm_config_ptr = nullptr;

    auto stream = get_stream(payload->stream_identifier());
    if (stream->get_payloads_transmitted() == 0) {
        switch (stream->get_type()) {
        case PayloadType::Video:
            rs = Cdi::create_stream_configuration(std::dynamic_pointer_cast<VideoStream>(stream), payload_config, avm_config);
            break;

        case PayloadType::Audio:
            rs = Cdi::create_stream_configuration(std::dynamic_pointer_cast<AudioStream>(stream), payload_config, avm_config);
            break;

        case PayloadType::Ancillary:
            rs = Cdi::create_stream_configuration(std::dynamic_pointer_cast<AncillaryStream>(stream), payload_config, avm_config);
            break;

        default:
            rs = Cdi::create_stream_configuration(stream, payload_config, avm_config);
            break;
        }

        if (CdiReturnStatus::kCdiStatusOk != rs) {
            LOG_ERROR << "Failure converting baseline configuration: " << CdiCoreStatusToString(rs) << ", code: " << rs << ".";
        }

        avm_config_ptr = &avm_config;
    }

    LOG_TRACE << "CDI transmitting payload #" << payload->stream_identifier() << ":" << payloads_transmitted_ + 1
#ifdef TRACE_PAYLOADS
        << " (" << payload->sequence() << ")"
#endif
        << "...";

    do {
        rs = CdiAvmTxPayload(connection_handle_, &payload_config, avm_config_ptr, payload.get(), tx_timeout_);
    } while (CdiReturnStatus::kCdiStatusQueueFull == rs);
}

void CdiTools::CdiConnection::on_connection_change(const CdiCoreConnectionCbData* cb_data_ptr)
{
    auto callback_data = static_cast<ConnectCallbackData*>(cb_data_ptr->connection_user_cb_param);
    auto self = std::dynamic_pointer_cast<CdiConnection>(callback_data->connection);
    assert(self != nullptr);

    auto connection_status = cb_data_ptr->status_code;

    self->set_status(
        CdiConnectionStatus::kCdiConnectionStatusConnected == connection_status ? ConnectionStatus::Open : ConnectionStatus::Closed);

    if (self->is_connected()) {
        self->logger_.info() << "CDI connection to " << self->host_name_ << ":" << self->port_number_ << " was established.";
    }
    else {
        if (cb_data_ptr->err_msg_str == nullptr) 
            self->logger_.info() << "CDI connection to " << self->host_name_ << ":" << self->port_number_ << " was closed.";
        else
            self->logger_.error() << "CDI connection connection failure for stream [" << cb_data_ptr->stream_identifier << "]: " << cb_data_ptr->err_msg_str << ".";
    }

    // TODO: review: this is using is_connected as error code.
    self->notify_connection_change(callback_data->handler, self->is_connected() ? std::error_code() : connection_error::not_connected);
    callback_data->clear_handler();
}

void CdiTools::CdiConnection::on_payload_received(const CdiAvmRxCbData* cb_data_ptr)
{
    auto callback_data = static_cast<ReceiveCallbackData*>(cb_data_ptr->core_cb_data.user_cb_param);
    auto self = std::dynamic_pointer_cast<CdiConnection>(callback_data->connection);
    assert(self != nullptr);
    assert(self->receive_callback_.handler != nullptr);

    auto payloads_received = ++self->payloads_received_;
    auto status_code = cb_data_ptr->core_cb_data.status_code;

    // create a payload from the SG list received
    auto payload = PayloadData::create(
        cb_data_ptr->sgl,
        cb_data_ptr->avm_extra_data.stream_identifier);

    if (self->receive_callback_.handler == nullptr) {
        self->logger_.error() << "A receive handler has not set. Payload will be dropped.";
        return;
    }

    if (CdiReturnStatus::kCdiStatusOk != status_code) {
        auto payload_errors = ++self->payload_errors_;
        // seeing some error statuses without a corresponding error message
        const char* err_msg = cb_data_ptr->core_cb_data.err_msg_str != nullptr
            ? cb_data_ptr->core_cb_data.err_msg_str : "No message available";
        self->logger_.error() << "Error receiving a payload: " << err_msg
            << ", code: " << status_code << ", total errors: " << payload_errors << ".";
        self->notify_payload_received(self->receive_callback_.handler, connection_error::receive_error, nullptr);
    }

    // ensure stream is valid
    auto stream = self->get_stream(cb_data_ptr->avm_extra_data.stream_identifier);
    if (stream == nullptr) {
        auto payload_errors = ++self->payload_errors_;
        self->logger_.error() << "Stream identifier #" << cb_data_ptr->avm_extra_data.stream_identifier
            << " is not valid. Payload will be dropped" << ", total errors: " << payload_errors << ".";
        self->notify_payload_received(self->receive_callback_.handler, connection_error::bad_stream_identifier, payload);
        return;
    }

    if (payload != nullptr) {
        self->logger_.trace() << "CDI received payload #" << payload->stream_identifier() << ":" << payloads_received
#ifdef TRACE_PAYLOADS
            << " (" << payload->sequence() << ")"
#endif
            << ", size: " << cb_data_ptr->sgl.total_data_size
            << "...";
        self->notify_payload_received(self->receive_callback_.handler, std::error_code(), payload);
    }

    // log stream configuration, if specified
    if (cb_data_ptr->config_ptr != nullptr) {
        CdiAvmBaselineConfig config;
        CdiReturnStatus rs = CdiAvmParseBaselineConfiguration(cb_data_ptr->config_ptr, &config);
        if (CdiReturnStatus::kCdiStatusOk == rs) {
            Cdi::show_stream_configuration(stream->id(), self->logger_, config);
        }
        else {
            self->logger_.error() << "Error parsing the payload configuration: " << CdiCoreStatusToString(rs) << ", code: " << rs << ".";
        }
    }
}

void CdiTools::CdiConnection::on_payload_transmitted(const CdiAvmTxCbData* cb_data_ptr)
{
    auto callback_data = std::unique_ptr<TransmitCallbackData>(static_cast<TransmitCallbackData*>(cb_data_ptr->core_cb_data.user_cb_param));
    auto self = std::dynamic_pointer_cast<CdiConnection>(callback_data->connection);
    assert(self != nullptr);

    auto status_code = cb_data_ptr->core_cb_data.status_code;
    auto stream_identifier = cb_data_ptr->avm_extra_data.stream_identifier;

    auto payloads_transmitted = ++self->payloads_transmitted_;

    if (CdiReturnStatus::kCdiStatusOk == status_code) {
        self->logger_.trace() << "CDI transmitted payload #" << stream_identifier << ":" << payloads_transmitted
#ifdef TRACE_PAYLOADS
            << " (" << cb_data_ptr->core_cb_data.core_extra_data.payload_user_data << ")"
#endif
            << "...";
    }
    else {
        auto payload_errors = ++self->payload_errors_;
        const char* err_msg = cb_data_ptr->core_cb_data.err_msg_str != nullptr
            ? cb_data_ptr->core_cb_data.err_msg_str : "No message available";
        self->logger_.error() << "CDI transmission error: " << err_msg
            << ", code: " << status_code << ", total errors: " << payload_errors << ".";
    }

    self->notify_payload_transmitted(callback_data->handler,
        CdiReturnStatus::kCdiStatusOk == status_code ? std::error_code() : connection_error::transmit_error);
}

void CdiTools::CdiConnection::log_message_callback(const CdiLogMessageCbData* cb_data_ptr)
{
    auto self = static_cast<CdiConnection*>(cb_data_ptr->log_user_cb_param);
    self->logger_.log(Cdi::map_log_level(cb_data_ptr->log_level)) << cb_data_ptr->message_str;
}
