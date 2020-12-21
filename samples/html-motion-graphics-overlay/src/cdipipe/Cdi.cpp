#include <iomanip>

#include "Cdi.h"
#include "Exceptions.h"
#include "Stream.h"
#include "VideoStream.h"
#include "AudioStream.h"
#include "AncillaryStream.h"
#include "Configuration.h"

using CdiTools::PayloadType;

static const char* large_payload_pool_name = "Large Buffer";
static const char* small_payload_pool_name = "Small Buffer";

LogLevel CdiTools::Cdi::map_log_level(CdiLogLevel log_level)
{
    switch (log_level)
    {
    case CdiLogLevel::kLogVerbose: return LogLevel::Trace;
    case CdiLogLevel::kLogDebug: return LogLevel::Debug;
    case CdiLogLevel::kLogInfo: return LogLevel::Info;
    case CdiLogLevel::kLogWarning: return LogLevel::Warning;
    case CdiLogLevel::kLogError: return LogLevel::Error;
    default: return LogLevel::Info;
    }
}

CdiLogLevel CdiTools::Cdi::map_log_level(LogLevel log_level)
{
    switch (log_level)
    {
    case LogLevel::Trace: return CdiLogLevel::kLogVerbose;
    case LogLevel::Debug: return CdiLogLevel::kLogDebug;
    case LogLevel::Info: return CdiLogLevel::kLogInfo;
    case LogLevel::Warning: return CdiLogLevel::kLogWarning;
    case LogLevel::Error: return CdiLogLevel::kLogError;
    default: return CdiLogLevel::kLogInfo;
    }
}

CdiAdapterTypeSelection CdiTools::Cdi::map_adapter_type(NetworkAdapterType adapter_type)
{
    switch (adapter_type)
    {
    case NetworkAdapterType::Efa: return CdiAdapterTypeSelection::kCdiAdapterTypeEfa;
    case NetworkAdapterType::Socket: return CdiAdapterTypeSelection::kCdiAdapterTypeSocket;
    case NetworkAdapterType::SocketLibFabric: return CdiAdapterTypeSelection::kCdiAdapterTypeSocketLibfabric;
    default: return CdiAdapterTypeSelection::kCdiAdapterTypeEfa;
    }
}

PayloadType CdiTools::Cdi::map_payload_type(const CdiBaselineAvmPayloadType payload_type)
{
    switch (payload_type) {
    case CdiBaselineAvmPayloadType::kCdiAvmVideo:
        return PayloadType::Video;

    case CdiBaselineAvmPayloadType::kCdiAvmAudio:
        return PayloadType::Audio;

    case CdiBaselineAvmPayloadType::kCdiAvmAncillary:
        return PayloadType::Ancillary;

    case CdiBaselineAvmPayloadType::kCdiAvmNotBaseline:
    default:
        return PayloadType::Unspecified;
    }
}

void CdiTools::Cdi::set_ptp_timestamp(CdiPtpTimestamp& timestamp)
{
    struct timespec time;

    CdiCoreGetUtcTime(&time);
    timestamp.seconds = (uint32_t)time.tv_sec;
    timestamp.nanoseconds = time.tv_nsec;
}

CdiAvmAudioChannelGrouping CdiTools::Cdi::map_channel_grouping(AudioChannelGrouping channel_grouping)
{
    switch (channel_grouping) {
    case AudioChannelGrouping::Mono: return kCdiAvmAudioM;
    default:
    case AudioChannelGrouping::Stereo: return kCdiAvmAudioST;
    }
}

CdiAvmAudioSampleRate CdiTools::Cdi::map_audio_sampling_rate(AudioSamplingRate sampling_rate)
{
    switch (sampling_rate) {
    case AudioSamplingRate::Rate96kHz: return kCdiAvmAudioSampleRate96kHz;
    default:
    case AudioSamplingRate::Rate48kHz: return kCdiAvmAudioSampleRate48kHz;
    }
}

CdiReturnStatus CdiTools::Cdi::create_stream_configuration(
    std::shared_ptr<VideoStream> stream,
    CdiAvmTxPayloadConfig& payload_config,
    CdiAvmConfig& avm_config)
{
    CdiAvmBaselineConfig baseline_config = {};
    baseline_config.payload_type = CdiBaselineAvmPayloadType::kCdiAvmVideo;
    CdiAvmVideoConfig& video_config = baseline_config.video_config;
    video_config = { 0 };
    video_config.width = stream->frame_width();
    video_config.height = stream->frame_height();
    video_config.sampling = CdiAvmVideoSampling::kCdiAvmVidRGB;
    video_config.alpha_channel = CdiAvmVideoAlphaChannel::kCdiAvmAlphaUnused;
    video_config.depth = CdiAvmVideoBitDepth::kCdiAvmVidBitDepth10;
    video_config.frame_rate_num = stream->frame_rate_numerator();
    video_config.frame_rate_den = stream->frame_rate_denominator();
    video_config.colorimetry = CdiAvmColorimetry::kCdiAvmVidColorimetryBT709;
    video_config.interlace = false;
    video_config.segmented = false;
    video_config.tcs = CdiAvmVideoTcs::kCdiAvmVidTcsSDR;
    video_config.range = CdiAvmVideoRange::kCdiAvmVidRangeFull;
    video_config.par_width = 1;
    video_config.par_height = 1;
    video_config.start_vertical_pos = 0;
    video_config.vertical_size = 0;
    video_config.start_horizontal_pos = 0;
    video_config.horizontal_size = 0;

    avm_config = { 0 };
    CdiReturnStatus rs = CdiAvmMakeBaselineConfiguration(&baseline_config, &avm_config, &payload_config.core_config_data.unit_size);

    return rs;
}

CdiReturnStatus CdiTools::Cdi::create_stream_configuration(
    std::shared_ptr<AudioStream> stream,
    CdiAvmTxPayloadConfig& payload_config,
    CdiAvmConfig& avm_config)
{
    CdiAvmBaselineConfig baseline_config = {};
    baseline_config.payload_type = CdiBaselineAvmPayloadType::kCdiAvmAudio;
    CdiAvmAudioConfig& audio_config = baseline_config.audio_config;
    audio_config = { 0 };

    audio_config.grouping = map_channel_grouping(stream->channel_grouping());
    memset(audio_config.language, 0, sizeof(audio_config.language));
    stream->language().copy(audio_config.language, sizeof(audio_config.language));
    audio_config.sample_rate_khz = map_audio_sampling_rate(stream->sampling_rate());

    avm_config = { 0 };
    CdiReturnStatus rs = CdiAvmMakeBaselineConfiguration(&baseline_config, &avm_config, &payload_config.core_config_data.unit_size);

    return rs;
}

CdiReturnStatus CdiTools::Cdi::create_stream_configuration(
    std::shared_ptr<AncillaryStream> stream,
    CdiAvmTxPayloadConfig& payload_config,
    CdiAvmConfig& avm_config)
{
    CdiAvmBaselineConfig baseline_config = {};
    baseline_config.payload_type = CdiBaselineAvmPayloadType::kCdiAvmAncillary;
    CdiAvmAncillaryDataConfig& ancillary_data_config = baseline_config.ancillary_data_config;
    ancillary_data_config = { 0 };

    avm_config = { 0 };
    CdiReturnStatus rs = CdiAvmMakeBaselineConfiguration(&baseline_config, &avm_config, &payload_config.core_config_data.unit_size);

    return rs;
}

CdiReturnStatus CdiTools::Cdi::create_stream_configuration(
    std::shared_ptr<Stream> stream,
    CdiAvmTxPayloadConfig& payload_config,
    CdiAvmConfig& avm_config)
{
    CdiAvmBaselineConfig baseline_config = {};
    baseline_config.payload_type = CdiBaselineAvmPayloadType::kCdiAvmNotBaseline;

    avm_config = { 0 };
    CdiReturnStatus rs = CdiAvmMakeBaselineConfiguration(&baseline_config, &avm_config, &payload_config.core_config_data.unit_size);

    return rs;
}

void CdiTools::Cdi::show_stream_configuration(uint16_t stream_identifier, Logger& logger, const CdiAvmBaselineConfig& config)
{
    using std::setw;
    using std::left;

    const int lvl = 2;
    switch (config.payload_type) {
    case CdiBaselineAvmPayloadType::kCdiAvmVideo:
        CdiAvmVideoConfig video_config = config.video_config;
        logger.info()
            << "Received configuration for stream #" << stream_identifier << ", type: video\n"
            << left << setw(lvl) << "Version: " << video_config.version.major << "." << video_config.version.minor << "\n"
            << left << setw(lvl) << "Width: " << video_config.width << "\n"
            << left << setw(lvl) << "Height: " << video_config.height << "\n"
            << left << setw(lvl) << "Sampling: " << video_config.sampling << "\n"
            << left << setw(lvl) << "Alpha channel: " << video_config.alpha_channel << "\n"
            << left << setw(lvl) << "Depth: " << video_config.depth << "\n"
            << left << setw(lvl) << "Frame rate numerator: " << video_config.frame_rate_num << "\n"
            << left << setw(lvl) << "Frame rate denominator: " << video_config.frame_rate_den << "\n"
            << left << setw(lvl) << "Colorimetry: " << video_config.colorimetry << "\n"
            << left << setw(lvl) << "Interlace: " << std::boolalpha << video_config.interlace << "\n"
            << left << setw(lvl) << "Segmented: " << std::boolalpha << video_config.segmented << "\n"
            << left << setw(lvl) << "Transfer characteristic system: " << video_config.tcs << "\n"
            << left << setw(lvl) << "Signal encoding range: " << video_config.range << "\n";
        break;

    case CdiBaselineAvmPayloadType::kCdiAvmAudio:
        CdiAvmAudioConfig audio_config = config.audio_config;
        logger.info()
            << "Received configuration for stream #" << stream_identifier << ", type: audio\n"
            << left << setw(lvl) << "Version: " << audio_config.version.major << "." << audio_config.version.minor << "\n"
            << left << setw(lvl) << "Grouping: " << audio_config.grouping << "\n"
            << left << setw(lvl) << "Language: " << audio_config.language << "\n"
            << left << setw(lvl) << "Sample rate (kHz): " << audio_config.sample_rate_khz << "\n";
        break;

    case CdiBaselineAvmPayloadType::kCdiAvmAncillary:
        CdiAvmAncillaryDataConfig ancillary_data_config = config.ancillary_data_config;
        logger.info()
            << "Received configuration for stream #" << stream_identifier << ", type: ancillary\n"
            << left << setw(lvl) << "Version: " << ancillary_data_config.version.major << "." << ancillary_data_config.version.minor << "\n";
        break;

    case CdiBaselineAvmPayloadType::kCdiAvmNotBaseline:
        logger.info()
            << "Received configuration for stream #" << stream_identifier << ", type: not baseline\n"
            << left << setw(lvl) << "Version: " << ancillary_data_config.version.major << "." << ancillary_data_config.version.minor << "\n";
        break;
    }
}

void CdiTools::Cdi::initialize(
    CdiLogger& logger,
    LogLevel log_level,
    const char* log_file_name)
{
    CdiCoreConfigData core_config = {};
    core_config.default_log_level = map_log_level(log_level);
    core_config.global_log_method_data_ptr = &logger;
    CloudWatchConfigData cloudwatch_config = {};
#ifdef ENABLE_CLOUDWATCH
    cloudwatch_config.dimension_domain_str = Configuration::cloudwatch_domain.c_str();
    cloudwatch_config.namespace_str = Configuration::cloudwatch_namespace.c_str();
    cloudwatch_config.region_str = Configuration::cloudwatch_region.c_str();
    core_config.cloudwatch_config_ptr = &cloudwatch_config;
#else
    core_config.cloudwatch_config_ptr = NULL;
#endif

    CdiReturnStatus rs = CdiCoreInitialize(&core_config);
    if (CdiReturnStatus::kCdiStatusOk != rs) {
        throw CdiInitializationException(std::string("Failed to initialize the CDI core: ") + CdiCoreStatusToString(rs));
    }
}

void CdiTools::Cdi::shutdown()
{
    CdiCoreShutdown();
}

void CdiTools::Cdi::initialize_adapter(
    const char* adapter_ip_address,
    NetworkAdapterType adapter_type,
    uint32_t large_buffer_pool_item_size,
    uint32_t large_buffer_pool_max_items,
    uint32_t small_buffer_pool_item_size,
    uint32_t small_buffer_pool_max_items,
    CdiAdapterHandle& adapter_handle,
    CdiPoolHandle& large_buffer_pool_handle,
    CdiPoolHandle& small_buffer_pool_handle)
{
    // determine buffer pool sizes
    uint32_t large_buffer_size = 0;
    if (large_buffer_pool_max_items > 0 && large_buffer_pool_item_size > 0
        && !CdiPoolCreateUsingExistingBuffer(nullptr, large_buffer_pool_max_items,
        large_buffer_pool_item_size, true, nullptr, 0, &large_buffer_size, nullptr)) {
        throw CdiInitializationException("Failed to obtain large buffers pool size.");
    }

    uint32_t small_buffer_size = 0;
    if (small_buffer_pool_max_items > 0 && small_buffer_pool_item_size > 0
        && !CdiPoolCreateUsingExistingBuffer(nullptr, small_buffer_pool_max_items,
        small_buffer_pool_item_size, true, nullptr, 0, &small_buffer_size, nullptr)) {
        throw CdiInitializationException("Failed to obtain small buffers pool size.");
    }

    // initialize network adapter
    CdiAdapterData adapter_data = {};
    adapter_data.adapter_ip_addr_str = adapter_ip_address;
    adapter_data.tx_buffer_size_bytes = (uint64_t)large_buffer_size + small_buffer_size;
    adapter_data.adapter_type = map_adapter_type(adapter_type);

    CdiReturnStatus rs = CdiCoreNetworkAdapterInitialize(&adapter_data, &adapter_handle);
    if (CdiReturnStatus::kCdiStatusOk != rs) {
        throw CdiInitializationException(std::string("CDI network adapter initialization failed: ") + CdiCoreStatusToString(rs) + ".");
    }

    void* large_payload_buffer_ptr = adapter_data.ret_tx_buffer_ptr;
    void* small_payload_buffer_ptr = (char*)adapter_data.ret_tx_buffer_ptr + large_buffer_size;

    // create large payload buffer pool
    if (large_buffer_pool_max_items > 0 && large_buffer_pool_item_size > 0
        && !CdiPoolCreateUsingExistingBuffer(large_payload_pool_name, large_buffer_pool_max_items,
            large_buffer_pool_item_size, true, large_payload_buffer_ptr, large_buffer_size, nullptr, &large_buffer_pool_handle)) {
        throw CdiInitializationException(std::string("Failed to allocate the '") + large_payload_pool_name + "' buffer pool.");
    }

    // create small payload buffer pool
    if (small_buffer_pool_max_items > 0 && small_buffer_pool_item_size > 0
        && !CdiPoolCreateUsingExistingBuffer(small_payload_pool_name, small_buffer_pool_max_items,
            small_buffer_pool_item_size, true, small_payload_buffer_ptr, small_buffer_size, nullptr, &small_buffer_pool_handle)) {
        throw CdiInitializationException(std::string("Failed to allocate the '") + small_payload_pool_name + "' buffer pool.");
    }
}
