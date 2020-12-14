#include <iostream>

#include "Enum.h"
#include "Configuration.h"
#include "CommandLine.h"
#include "Application.h"
#include "Utils.h"

using namespace CdiTools;

int main(int argc, char* argv[])
{
    bool show_channel_config = false;
    ChannelRole channel_role = ChannelRole::None;
    std::string frame_rate;

    CommandLine command_line{ "CDI receiver/transmitter protocol bridge for video applications" };

    command_line
        .add_option("role",             "Channel endpoint role", channel_role, channel_role_map)
        .add_option("channel",          "Communication channel type", Configuration::channel_type, channel_type_map)
        .add_option("adapter",          "Network adapter type", Configuration::adapter_type, adapter_type_map)
        .add_option("local_ip",         "Local network adapter IP address", Configuration::local_ip)
        .add_option("remote_ip",        "Remote network adapter IP address", Configuration::remote_ip)
        .add_option("port",             "Destination port number", Configuration::port_number)
        //.add_option("show_config",      "Display channel configuration", show_channel_config)
        .add_option("no_audio",         "Disable audio stream", Configuration::disable_audio)
        .add_option("log_level",        "Set the log level", Configuration::log_level, log_level_map)
        .add_option("log_file",         "Log file name", Configuration::log_file)
        .add_option("num_threads",      "Number of channel threads", Configuration::num_threads)
        .add_option("inline_handlers",  "Use inline handlers", Configuration::inline_handlers)
        .add_option("buffer_delay",     "Incoming payload buffer delay (max: " + std::to_string(MAXIMUM_RX_BUFFER_DELAY_MS) + " ms.)", Configuration::buffer_delay)
        .add_option("video_in_port",    "Video input port number", Configuration::video_in_port)
        .add_option("video_out_port",   "Video output port number", Configuration::video_out_port)
        .add_option("audio_in_port",    "Audio input port number", Configuration::audio_in_port)
        .add_option("audio_out_port",   "Audio output port number", Configuration::audio_out_port)
        .add_option("frame_width",      "Input source frame width", Configuration::frame_width)
        .add_option("frame_height",     "Input source frame height", Configuration::frame_height)
        .add_option("frame_rate",       "Input source frame rate", frame_rate)
        .add_option("tx_timeout",       "Payload transmission timeout in microseconds", Configuration::tx_timeout)
        .add_option("large_pool_items", "Large payload pool maximum items", Configuration::large_buffer_pool_max_items)
        .add_option("small_pool_items", "Small payload pool maximum items", Configuration::small_buffer_pool_max_items);

    if (command_line.parse(argc, argv)) {
        if (ChannelRole::None == channel_role) {
            std::cout << "ERROR: must specify a channel endpoint role. Use -help to see available options.\n";
            return 1;
        }

        if (Configuration::buffer_delay < 0 || Configuration::buffer_delay > MAXIMUM_RX_BUFFER_DELAY_MS) {
            std::cout << "ERROR: '-buffer_delay' setting must be a value between 0 and " << MAXIMUM_RX_BUFFER_DELAY_MS << ". Use -help to see available options.\n";
            return 1;
        }

        if (!frame_rate.empty()) {
            std::vector<int> tokens;
            if (!Utils::split<int>(frame_rate, '/', std::back_inserter(tokens)) || tokens.empty() || tokens.size() > 2) {
                std::cout << "ERROR: invalid value '" << frame_rate << "' provided for parameter 'frame_rate'.\n";
                return 1;
            }

            Configuration::frame_rate_numerator = tokens[0];
            Configuration::frame_rate_denominator = tokens.size() > 1 ? tokens[1] : 1;
        }

        return Application::run(channel_role, show_channel_config);
    }

    return 1;
}
