#pragma once

#include <cdi_core_api.h>
#include <cdi_avm_api.h>
#include <cdi_log_api.h>
#include <cdi_baseline_profile_api.h>
#include <cdi_pool_api.h>
#if SUPPORT_BASELINE_PROFILE_01
#include <cdi_baseline_profile_01_00_api.h>
#endif
#if SUPPORT_BASELINE_PROFILE_02
#include <cdi_baseline_profile_02_00_api.h>
#endif

#include "Logger.h"
#include "PayloadType.h"
#include "NetworkAdapterType.h"
#include "StreamOptions.h"
#include "CdiLogger.h"

namespace CdiTools
{
    class VideoStream;
    class AudioStream;
    class AncillaryStream;
    class Stream;

    namespace Cdi
    {
        void initialize(CdiLogger& logger, LogLevel log_level, const char* log_file_name);
        void initialize_adapter(const char* adapter_ip_address, NetworkAdapterType adapter_type,
            uint32_t large_buffer_pool_item_size, uint32_t large_buffer_pool_max_items,
            uint32_t small_buffer_pool_item_size, uint32_t small_buffer_pool_max_items,
            CdiAdapterHandle& adapter_handle, CdiPoolHandle& large_buffer_pool_handle, CdiPoolHandle& small_buffer_pool_handle);
        void shutdown();
        LogLevel map_log_level(CdiLogLevel log_level);
        CdiLogLevel map_log_level(LogLevel log_level);
        CdiAdapterTypeSelection map_adapter_type(NetworkAdapterType adapter_type);
        PayloadType map_payload_type(const CdiBaselineAvmPayloadType payload_type);
        void set_ptp_timestamp(CdiPtpTimestamp& timestamp);
        CdiAvmAudioChannelGrouping map_channel_grouping(AudioChannelGrouping channel_grouping);
        CdiAvmAudioSampleRate map_audio_sampling_rate(AudioSamplingRate sampling_rate);
        CdiReturnStatus create_stream_configuration(std::shared_ptr<VideoStream> stream,
            CdiAvmTxPayloadConfig& payload_config, CdiAvmConfig& avm_config);
        CdiReturnStatus create_stream_configuration(std::shared_ptr<AudioStream> stream,
            CdiAvmTxPayloadConfig& payload_config, CdiAvmConfig& avm_config);
        CdiReturnStatus create_stream_configuration(std::shared_ptr<AncillaryStream> stream,
            CdiAvmTxPayloadConfig& payload_config, CdiAvmConfig& avm_config);
        CdiReturnStatus create_stream_configuration(std::shared_ptr<Stream> stream,
            CdiAvmTxPayloadConfig& payload_config, CdiAvmConfig& avm_config);
        void show_stream_configuration(uint16_t stream_identifier, Logger& logger, const CdiAvmBaselineConfig& config);
    };
}
