#pragma once

#include <cdi_core_api.h>
#include <cdi_pool_api.h>

#include "CdiLogger.h"
#include "NetworkAdapterType.h"
#include "ChannelRole.h"

namespace CdiTools
{
    class Channel;

    class Application
    {
    public:
        Application(const char* adapter_ip_address,
            NetworkAdapterType adapter_type,
            uint32_t large_buffer_pool_item_size,
            uint32_t large_buffer_pool_max_items,
            uint32_t small_buffer_pool_item_size,
            uint32_t small_buffer_pool_max_items,
            LogLevel log_level,
            const char* log_file_name = nullptr);

        ~Application();

        static Application& start(
            const char* adapter_ip_address,
            NetworkAdapterType adapter_type,
            uint32_t large_buffer_pool_item_size,
            uint32_t large_buffer_pool_max_items,
            uint32_t small_buffer_pool_item_size,
            uint32_t small_buffer_pool_max_items,
            LogLevel log_level,
            const char* log_file_name = nullptr);

        inline CdiAdapterHandle get_adapter_handle() { return adapter_handle_; }
        inline const char* get_adapter_ip_address() { return ip_address_.c_str(); }
        void* get_pool_buffer(size_t payload_size);
        void free_pool_buffer(void* buffer_ptr, size_t payload_size);
        int get_pool_free_buffer_count(size_t payload_size);
        const char* get_pool_name(size_t payload_size);
        static Application* get() { return instance_; }
        static int run(ChannelRole channel_role, bool show_channel_config);

    private:
        CdiPoolHandle get_pool_handle(size_t payload_size);
        static std::shared_ptr<Channel> configure_channel(ChannelRole channel_role);
        static CdiTools::Application* instance_;

        Logger logger_;
        CdiLogger cdi_logger_;
        std::string ip_address_;
        uint32_t large_buffer_pool_item_size_;
        uint32_t small_buffer_pool_item_size_;
        CdiAdapterHandle adapter_handle_;
        CdiPoolHandle large_buffer_pool_handle_;
        CdiPoolHandle small_buffer_pool_handle_;
    };
}

