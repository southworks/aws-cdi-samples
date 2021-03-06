#include "Configuration.h"
#include "Stream.h"

using namespace CdiTools;

// general settings
LogLevel Configuration::log_level{ LogLevel::Info };
std::string Configuration::log_file;

// channel settings
ChannelType Configuration::channel_type{ ChannelType::CdiStream };
bool Configuration::inline_handlers{ false };
int Configuration::num_threads{ 1 };

// CDI settings
NetworkAdapterType Configuration::adapter_type{ NetworkAdapterType::SocketLibFabric };
std::string Configuration::local_ip{ "127.0.0.1" };
std::string Configuration::remote_ip{ "127.0.0.1" };
int Configuration::buffer_delay{ 0 };
int Configuration::tx_timeout{ 0 };

// CloudWatch settings
#ifdef ENABLE_CLOUDWATCH
std::string Configuration::cloudwatch_domain;
std::string Configuration::cloudwatch_namespace;
std::string Configuration::cloudwatch_region;
#endif

// buffer pool configuration
const uint32_t Configuration::large_buffer_pool_item_size = 1920 * 1080 * 3;
const uint32_t Configuration::small_buffer_pool_item_size = 2 * 2304;
unsigned int Configuration::large_buffer_pool_max_items = 40;
unsigned int Configuration::small_buffer_pool_max_items = 60;

// input/output port configurations
unsigned short Configuration::port_number = 2000;
unsigned short Configuration::video_in_port = 1000;
unsigned short Configuration::audio_in_port = video_in_port + 1;
unsigned short Configuration::video_out_port = 3000;
unsigned short Configuration::audio_out_port = video_out_port + 1;

// video configuration settings
uint16_t Configuration::video_stream_id = 1;
int Configuration::frame_width = 1280;
int Configuration::bytes_per_pixel = 3;
int Configuration::frame_height = 534;
int Configuration::frame_rate_numerator = 24;
int Configuration::frame_rate_denominator = 1;

// audio configuration settings
bool Configuration::disable_audio{ false };
uint16_t Configuration::audio_stream_id = 2;
AudioChannelGrouping Configuration::audio_channel_grouping = AudioChannelGrouping::Stereo;
AudioSamplingRate Configuration::audio_sampling_rate = AudioSamplingRate::Rate48kHz;
int Configuration::audio_bytes_per_sample = 2;
std::string Configuration::audio_stream_language = "en";
