#include <iostream>
#include <cassert>
#include <conio.h>

#include <cdi_core_api.h>
#include <cdi_pool_api.h>

#include "Application.h"
#include "Cdi.h"
#include "Configuration.h"
#include "Channel.h"

static const char* logger_name = "Application";

CdiTools::Application* CdiTools::Application::instance_ = nullptr;

CdiTools::Application& CdiTools::Application::start(
    const char* adapter_ip_address,
    NetworkAdapterType adapter_type,
    uint32_t large_buffer_pool_item_size,
    uint32_t large_buffer_pool_max_items,
    uint32_t small_buffer_pool_item_size,
    uint32_t small_buffer_pool_max_items,
    LogLevel log_level,
    const char* log_file_name)
{
    static Application instance{ adapter_ip_address, adapter_type, large_buffer_pool_item_size,
        large_buffer_pool_max_items, small_buffer_pool_item_size, small_buffer_pool_max_items,
        log_level, log_file_name };
    
    if (instance_ == nullptr) instance_ = &instance;

    return instance;
}

CdiTools::Application::Application(
    const char* adapter_ip_address,
    NetworkAdapterType adapter_type,
    uint32_t large_buffer_pool_item_size,
    uint32_t large_buffer_pool_max_items,
    uint32_t small_buffer_pool_item_size,
    uint32_t small_buffer_pool_max_items,
    LogLevel log_level,
    const char* log_file_name)
    : logger_{ logger_name }
    , cdi_logger_{ &logger_ }
    , adapter_handle_{ NULL }
    , large_buffer_pool_handle_{ NULL }
    , small_buffer_pool_handle_{ NULL }
    , large_buffer_pool_item_size_{ large_buffer_pool_item_size }
    , small_buffer_pool_item_size_{ small_buffer_pool_item_size }
{
    Cdi::initialize(cdi_logger_, log_level, log_file_name);
    Cdi::initialize_adapter(adapter_ip_address, adapter_type,
        large_buffer_pool_item_size, large_buffer_pool_max_items,
        small_buffer_pool_item_size, small_buffer_pool_max_items,
        adapter_handle_, large_buffer_pool_handle_, small_buffer_pool_handle_);
}

CdiTools::Application::~Application()
{
    Cdi::shutdown();
}

void* CdiTools::Application::get_pool_buffer(size_t payload_size)
{
    void* buffer_ptr = nullptr;

    CdiPoolHandle pool_handle = get_pool_handle(payload_size);
    assert(pool_handle != NULL);

    if (NULL == pool_handle || !CdiPoolGet(pool_handle, &buffer_ptr)) {
        LOG_DEBUG << "Failed to allocate a payload buffer from the '" << CdiPoolGetName(pool_handle) << "' pool"
            << ", requested size: " << std::to_string(CdiPoolGetItemSize(pool_handle))
            << ", free items: " << std::to_string(CdiPoolGetFreeItemCount(pool_handle))
            << ".";
    }

    return buffer_ptr;
}

void CdiTools::Application::free_pool_buffer(void* buffer_ptr, size_t payload_size)
{
    CdiPoolHandle pool_handle = get_pool_handle(payload_size);
    assert(pool_handle != NULL);

    CdiPoolPut(pool_handle, buffer_ptr);
}

int CdiTools::Application::get_pool_free_buffer_count(size_t payload_size)
{
    CdiPoolHandle pool_handle = get_pool_handle(payload_size);
    assert(pool_handle != NULL);

    int count = CdiPoolGetFreeItemCount(pool_handle);

    return count;
}

const char* CdiTools::Application::get_pool_name(size_t payload_size)
{
    CdiPoolHandle pool_handle = get_pool_handle(payload_size);
    assert(pool_handle != NULL);

    const char* name = CdiPoolGetName(pool_handle);

    return name;
}

CdiPoolHandle CdiTools::Application::get_pool_handle(size_t payload_size)
{
    if (payload_size <= small_buffer_pool_item_size_) {
        return small_buffer_pool_handle_;
    }

    if (payload_size <= large_buffer_pool_item_size_) {
        return large_buffer_pool_handle_;
    }

    // TODO: make pool item sizes command line arguments?
    LOG_ERROR << "Requested payload size exceeds maximum allowed (" << large_buffer_pool_item_size_ << " bytes).";

    return NULL;
}

std::shared_ptr<CdiTools::Channel> CdiTools::Application::configure_channel(ChannelRole channel_role)
{
    auto channel = std::make_shared<Channel>(enum_name(channel_role_map, channel_role));
    auto endpoint_connection_type = ConnectionType::Tcp;
    auto channel_connection_type = ChannelType::Cdi == Configuration::channel_type || ChannelType::CdiStream == Configuration::channel_type
        ? ConnectionType::Cdi : ConnectionType::Tcp;

    auto input_connection_type = ChannelRole::Transmitter == channel_role ? endpoint_connection_type : channel_connection_type;
    auto output_connection_type = ChannelRole::Receiver == channel_role ? endpoint_connection_type : channel_connection_type;

    // set up channel streams
    channel->add_video_stream(Configuration::video_stream_id, Configuration::frame_width, Configuration::frame_height,
        Configuration::bytes_per_pixel, Configuration::frame_rate_numerator, Configuration::frame_rate_denominator);
    if (!Configuration::disable_audio) {
        channel->add_audio_stream(Configuration::audio_stream_id, Configuration::audio_channel_grouping,
            Configuration::audio_sampling_rate, Configuration::Configuration::audio_bytes_per_sample, Configuration::audio_stream_language);
    }

    // configure channel connections
    auto video_buffer_size = static_cast<unsigned int>(Configuration::large_buffer_pool_max_items * 0.9);
    auto audio_buffer_size = static_cast<unsigned int>(Configuration::small_buffer_pool_max_items * 0.9);
    if (ChannelRole::Transmitter == channel_role) {
        channel->add_input(input_connection_type, "video_in", "127.0.0.1", Configuration::video_in_port, ConnectionMode::Listener, 0);
        channel->add_output(output_connection_type, ChannelType::CdiStream != Configuration::channel_type ? "video_out" : "avid_out",
            Configuration::remote_ip, Configuration::port_number, ConnectionMode::Client,
            ChannelType::CdiStream != Configuration::channel_type ? video_buffer_size : video_buffer_size + audio_buffer_size);

        if (!Configuration::disable_audio) {
            channel->add_input(input_connection_type, "audio_in", "127.0.0.1", Configuration::audio_in_port, ConnectionMode::Listener, 0);
            if (ChannelType::CdiStream != Configuration::channel_type) {
                channel->add_output(output_connection_type, "audio_out", Configuration::remote_ip, Configuration::port_number + 1, ConnectionMode::Client, audio_buffer_size);
            }
        }

        // map streams to connections
        channel->map_stream(Configuration::video_stream_id, "video_in");
        channel->map_stream(Configuration::video_stream_id, ChannelType::CdiStream != Configuration::channel_type ? "video_out" : "avid_out");
        if (!Configuration::disable_audio) {
            channel->map_stream(Configuration::audio_stream_id, "audio_in");
            channel->map_stream(Configuration::audio_stream_id, ChannelType::CdiStream != Configuration::channel_type ? "audio_out" : "avid_out");
        }
    }
    else if (ChannelRole::Receiver == channel_role) {
        channel->add_input(input_connection_type, ChannelType::CdiStream != Configuration::channel_type ? "video_in" : "avid_in",
            Configuration::remote_ip, Configuration::port_number, ConnectionMode::Listener, 0);
        channel->add_output(output_connection_type, "video_out", "127.0.0.1", Configuration::video_out_port, ConnectionMode::Listener, video_buffer_size);

        if (!Configuration::disable_audio) {
            if (ChannelType::CdiStream != Configuration::channel_type) {
                channel->add_input(input_connection_type, "audio_in", Configuration::remote_ip, Configuration::port_number + 1, ConnectionMode::Listener, 0);
            }

            channel->add_output(output_connection_type, "audio_out", "127.0.0.1", Configuration::audio_out_port, ConnectionMode::Listener, audio_buffer_size);
        }

        // map streams to connections
        channel->map_stream(Configuration::video_stream_id, ChannelType::CdiStream != Configuration::channel_type ? "video_in" : "avid_in");
        channel->map_stream(Configuration::video_stream_id, "video_out");
        if (!Configuration::disable_audio) {
            channel->map_stream(Configuration::audio_stream_id, ChannelType::CdiStream != Configuration::channel_type ? "audio_in" : "avid_in");
            channel->map_stream(Configuration::audio_stream_id, "audio_out");
        }
    }
    else if (ChannelRole::Bridge == channel_role) {
        channel->add_input(input_connection_type, "video_in", "127.0.0.1", Configuration::port_number, ConnectionMode::Listener, 0);
        channel->add_output(output_connection_type, "video_out", "127.0.0.1", Configuration::video_out_port, ConnectionMode::Listener, video_buffer_size);
        if (!Configuration::disable_audio) {
            channel->add_input(input_connection_type, "audio_in", "127.0.0.1", Configuration::port_number + 1, ConnectionMode::Listener, 0);
            channel->add_output(output_connection_type, "audio_out", "127.0.0.1", Configuration::audio_out_port, ConnectionMode::Listener, audio_buffer_size);
        }

        // map streams to connections
        channel->map_stream(Configuration::video_stream_id, "video_in");
        channel->map_stream(Configuration::video_stream_id, "video_out");
        if (!Configuration::disable_audio) {
            channel->map_stream(Configuration::audio_stream_id, "audio_in");
            channel->map_stream(Configuration::audio_stream_id, "audio_out");
        }
    }
    else {
        std::cout << "ERROR: Channel role is not supported.\n";
    }

    channel->validate_configuration();

    return channel;
}

int CdiTools::Application::run(ChannelRole channel_role, bool show_channel_config)
{
    int exit_code = 0;
    std::shared_ptr<Channel> channel;

    Logger::start(Configuration::log_level, Configuration::log_file);

    try
    {
        channel = configure_channel(channel_role);
        if (show_channel_config) {
            channel->show_configuration();
        }

        if (channel_role == ChannelRole::Receiver
            && (ChannelType::Cdi == Configuration::channel_type || ChannelType::CdiStream == Configuration::channel_type)) {
            start(Configuration::local_ip.c_str(), Configuration::adapter_type, 0, 0, 0, 0, LogLevel::Info);
        }
        else {
            start(Configuration::local_ip.c_str(), Configuration::adapter_type,
                Configuration::large_buffer_pool_item_size, Configuration::large_buffer_pool_max_items,
                Configuration::small_buffer_pool_item_size, Configuration::small_buffer_pool_max_items, LogLevel::Info);
        }

        std::thread shutdown([&]() {
            int key = 0;
            // TODO: this is not portable
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                if (_kbhit()) {
                    key = _getch();
                    if (key == 'q' || key == 'Q') break;
                    if (key == 'i' || key == 'I') Logger::set_level(LogLevel::Info);
                    if (key == 'd' || key == 'D') Logger::set_level(LogLevel::Debug);
                    if (key == 't' || key == 'T') Logger::set_level(LogLevel::Trace);
                    if (key == 's' || key == 'S') channel->show_status();
                }
            }

            channel->shutdown();
        });

        channel->start(channel_role, [](const std::error_code& ec) {
            if (ec) {
                std::cout << "ERROR: " << ec.message() << ".\n";
            }
            else {
                std::cout << "Channel has shut down.\n";
            }
        }, Configuration::num_threads);

        if (shutdown.joinable()) {
            shutdown.join();
        }
    }
    catch (const std::exception& exception)
    {
        std::cout << "ERROR: " << exception.what() << std::endl;
        exit_code = 1;
    }

    Logger::shutdown();

    return exit_code;
}
