@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET "SCRIPT_NAME=EXECUTE"
IF NOT DEFINED CONSOLE_MODE (SET "CONSOLE_MODE=/C")
IF DEFINED FFMPEG_PATH IF NOT "!FFMPEG_PATH:~-1!" == "\" SET "FFMPEG_PATH=!FFMPEG_PATH!\")
IF DEFINED TOOLS_PATH IF NOT "!TOOLS_PATH:~-1!" == "\" SET "TOOLS_PATH=!TOOLS_PATH!\")
IF DEFINED HTMLSRC_PATH (IF NOT "!HTMLSRC_PATH:~-1!" == "\" SET "HTMLSRC_PATH=!HTMLSRC_PATH!\") ELSE (SET "HTMLSRC_PATH=!TOOLS_PATH!")
IF DEFINED MEDIA_PATH IF NOT "!MEDIA_PATH:~-1!" == "\" SET "MEDIA_PATH=!MEDIA_PATH!\")

SET FFMPEG_CMD=!FFMPEG_PATH!ffmpeg
SET FFPLAY_CMD=!FFMPEG_PATH!ffplay
SET FFPROBE_CMD=!FFMPEG_PATH!ffprobe
SET HTMLSRC_CMD=!HTMLSRC_PATH!htmlsrc
SET CDIPIPE_CMD=!TOOLS_PATH!cdipipe -tx_timeout 80000

SET "FFMPEG_GLOBAL_OPTIONS= -hide_banner -loglevel info"
SET "FFPLAY_GLOBAL_OPTIONS= -hide_banner -loglevel info"
::SET "FFPLAY_GLOBAL_OPTIONS=-hide_banner -probesize 32 -sync ext -infbuf -autoexit  -genpts -fast -rw_timeout 60000000"

SET RECEIVER_MODE_OPTIONS="play stream store"
SET ROLE_OPTIONS="transmitter receiver both"
SET CHANNEL_OPTIONS="cdistream cdi tcp"
SET ADAPTER_OPTIONS="efa socket socketlibfabric"
SET FORMAT_OPTIONS="rgb mp4"
SET LOG_LEVEL_OPTIONS="trace debug info warning error"

:: default values
SET "VIDEO_IN_PORT=1000"
SET "AUDIO_IN_PORT=1001"
SET "VIDEO_OUT_PORT=3000"
SET "AUDIO_OUT_PORT=3001"
SET "PORT_NUMBER=2000"
SET "VIDEO_QUEUE_SIZE=1500"
SET "AUDIO_QUEUE_SIZE=2000"
SET "HTMLSRC_QUEUE_SIZE=2000"
SET "TIME_OFFSET=0.3"
SET "CHANNEL_TYPE=cdistream"
SET "ADAPTER_TYPE=socketlibfabric"
SET "ROLE=both"
SET "RECEIVER_MODE=play"
SET "OUTPUT_FORMAT=mp4"
SET "DEFAULT_VIDEO_WIDTH=none"
SET "DEFAULT_VIDEO_HEIGHT=none"
SET "DEFAULT_VIDEO_AVG_FRAME_RATE=none"
SET "DEFAULT_OVERLAY_WINDOW_SIZE=none"
SET "DEFAULT_OVERLAY_VIEWPORT_ORIGIN=0 0"
SET "DEFAULT_OVERLAY_VIEWPORT_SIZE=window size"
SET "DEFAULT_OVERLAY_POSITION=bottom-left"
SET "DEFAULT_OVERLAY_FRAME_RATE=30"
SET "DEFAULT_OVERLAY_SCALE_FACTOR=1"
SET "DEFAULT_OVERLAY_CHROMA_COLOR=none"
SET "DEFAULT_OVERLAY_BACKGROUND_COLOR=none"
SET "DEFAULT_TX_TIMESTAMP=off"
SET "DEFAULT_RX_TIMESTAMP=off"
SET "DEFAULT_KEEP_WINDOWS=false"

:: process command line
IF "%~1"=="" (GOTO usage)
SET PARAMETER=%~1
IF "!PARAMETER:~0,1!"=="-" GOTO loop
SET "INPUT_SOURCE=!MEDIA_PATH!%1"
SHIFT
:loop
IF NOT "%~1"=="" (
    SET CURRENT=%1
    SET PARAMETER_NAME=!CURRENT:~1!
    SET PARAMETER_VALUE=%2
    SET IS_VALID_PARAMETER=
    SET IS_INVALID_PARAMETER=

    IF "!PARAMETER_NAME!"=="help" (
        GOTO usage
    )

    IF "!PARAMETER_NAME!"=="examples" (
        GOTO examples
    )

    IF "!PARAMETER_NAME!"=="role" (
        CALL :ParseOptions ROLE !PARAMETER_NAME! !ROLE_OPTIONS! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="mode" (
        CALL :ParseOptions RECEIVER_MODE !PARAMETER_NAME! !RECEIVER_MODE_OPTIONS! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="channel" (
        CALL :ParseOptions CHANNEL_TYPE !PARAMETER_NAME! !CHANNEL_OPTIONS! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="width" (
        CALL :ParseValue VIDEO_WIDTH !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="height" (
        CALL :ParseValue VIDEO_HEIGHT !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="framerate" (
        CALL :ParseValue VIDEO_AVG_FRAME_RATE !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )
    
    IF "!PARAMETER_NAME!"=="adapter" (
        CALL :ParseOptions ADAPTER_TYPE !PARAMETER_NAME! !ADAPTER_OPTIONS! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="local_ip" (
        CALL :ParseValue LOCAL_IP !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="remote_ip" (
        CALL :ParseValue REMOTE_IP !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="port" (
        CALL :ParseValue PORT_NUMBER !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="video_in_port" (
        CALL :ParseValue VIDEO_IN_PORT !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="audio_in_port" (
        CALL :ParseValue AUDIO_IN_PORT !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="video_out_port" (
        CALL :ParseValue VIDEO_OUT_PORT !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="audio_out_port" (
        CALL :ParseValue AUDIO_OUT_PORT !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="overlay" (
        CALL :ParseValue OVERLAY_SOURCE !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="overlay_window_size" (
        SET OVERLAY_WINDOW_WIDTH=%2
        SET OVERLAY_WINDOW_HEIGHT=%3
        IF "!OVERLAY_WINDOW_WIDTH!"=="" (SET IS_INVALID_PARAMETER=1)
        IF "!OVERLAY_WINDOW_WIDTH:~0,1!"=="-" (SET IS_INVALID_PARAMETER=1)
        IF DEFINED IS_INVALID_PARAMETER (
            ECHO ERROR: Missing or invalid '!PARAMETER_NAME!' dimensions. Must specify as: -!PARAMETER_NAME! ^<width^> ^<height^>.
            GOTO exit
        )
        SHIFT & SHIFT & GOTO next
    )

    IF "!PARAMETER_NAME!"=="overlay_viewport_origin" (
        SET OVERLAY_VIEWPORT_LEFT=%2
        SET OVERLAY_VIEWPORT_TOP=%3
        IF "!OVERLAY_VIEWPORT_LEFT!"=="" (SET IS_INVALID_PARAMETER=1)
        IF "!OVERLAY_VIEWPORT_LEFT:~0,1!"=="-" (SET IS_INVALID_PARAMETER=1)
        IF DEFINED IS_INVALID_PARAMETER (
            ECHO ERROR: Missing or invalid '!PARAMETER_NAME!' coordinates. Must specify as: -!PARAMETER_NAME! ^<left^> ^<top^>.
            GOTO exit
        )
        SHIFT & SHIFT & GOTO next
    )

    IF "!PARAMETER_NAME!"=="overlay_viewport_size" (
        SET OVERLAY_VIEWPORT_WIDTH=%2
        SET OVERLAY_VIEWPORT_HEIGHT=%3
        IF "!OVERLAY_VIEWPORT_WIDTH!"=="" (SET MISSING_ORIGIN=1)
        IF "!OVERLAY_VIEWPORT_WIDTH:~0,1!"=="-" (SET MISSING_ORIGIN=1)
        IF DEFINED IS_INVALID_PARAMETER (
            ECHO ERROR: Missing or invalid '!PARAMETER_NAME!' dimensions. Must specify as: -!PARAMETER_NAME! ^<width^> ^<height^>.
            GOTO exit
        )
        SHIFT & SHIFT & GOTO next
    )

    IF "!PARAMETER_NAME!"=="overlay_chroma_color" (
        CALL :ParseValue OVERLAY_CHROMA_COLOR !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="overlay_background_color" (
        CALL :ParseValue OVERLAY_BACKGROUND_COLOR !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="overlay_scale_factor" (
        CALL :ParseValue OVERLAY_SCALE_FACTOR !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="overlay_frame_rate" (
        CALL :ParseValue OVERLAY_FRAME_RATE !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="overlay_position" (
        CALL :ParseValue OVERLAY_POSITION !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="output_format" (
        CALL :ParseOptions OUTPUT_FORMAT !PARAMETER_NAME! !FORMAT_OPTIONS! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="output" (
        CALL :ParseValue OUTPUT !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="time_offset" (
        CALL :ParseValue TIME_OFFSET !PARAMETER_NAME! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="quiet" (
        SET "FFMPEG_CMD=!FFMPEG_CMD! -loglevel quiet"
        SET "FFPLAY_CMD=!FFPLAY_CMD! -loglevel quiet"
        SET "FFPROBE_CMD=!FFPROBE_CMD! -loglevel quiet"
        SET "HTMLSRC_CMD=!HTMLSRC_CMD! --quiet -l error"
        ::SET "CDIPIPE_CMD=!CDIPIPE_CMD! -quiet"
        SET "QUIET_MODE=1"
        GOTO next
    )

    IF "!PARAMETER_NAME!"=="log_level" (
        CALL :ParseOptions LOG_LEVEL !PARAMETER_NAME! !LOG_LEVEL_OPTIONS! !PARAMETER_VALUE!
        IF "!ERRORLEVEL!"=="0" (SHIFT & GOTO next) ELSE (GOTO exit)
    )

    IF "!PARAMETER_NAME!"=="tx_timestamp" (SET TX_TIMESTAMP=1 & GOTO next)
    IF "!PARAMETER_NAME!"=="rx_timestamp" (SET RX_TIMESTAMP=1 & GOTO next)
    IF "!PARAMETER_NAME!"=="keep" (SET "CONSOLE_MODE=/k" & GOTO next)

    ECHO ERROR: Unknown argument '!PARAMETER_NAME!' specified.
    GOTO usage

:next
    SHIFT
    GOTO :loop
)

:validation
IF DEFINED LOCAL_IP (SET USE_NETWORK=1)
IF DEFINED REMOTE_IP (SET USE_NETWORK=1)
IF DEFINED USE_NETWORK (FOR /f "tokens=2 delims=\[\]" %%A IN ('ping -n 1 -4 "%COMPUTERNAME%"') DO SET IP_ADDRESS=%%A) ELSE (SET IP_ADDRESS=127.0.0.1)

IF /I "!ROLE!"=="receiver" (
    CALL :ParseValue VIDEO_WIDTH width !VIDEO_WIDTH! !VIDEO_WIDTH!
    IF "!ERRORLEVEL!"=="1" (GOTO exit)

    CALL :ParseValue VIDEO_HEIGHT height !VIDEO_HEIGHT! !VIDEO_HEIGHT!
    IF "!ERRORLEVEL!"=="1" (GOTO exit)

    CALL :ParseValue VIDEO_AVG_FRAME_RATE framerate !VIDEO_AVG_FRAME_RATE! !VIDEO_AVG_FRAME_RATE!
    IF "!ERRORLEVEL!"=="1" (GOTO exit)
) ELSE (
    CALL :ParseValue INPUT_SOURCE source !INPUT_SOURCE! !INPUT_SOURCE!
    IF "!ERRORLEVEL!"=="1" (GOTO exit)
)

CALL :ParseOptions RECEIVER_MODE mode !RECEIVER_MODE_OPTIONS! !RECEIVER_MODE!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions ROLE role !ROLE_OPTIONS! !ROLE! both
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions CHANNEL_TYPE channel !CHANNEL_OPTIONS! !CHANNEL_TYPE!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions ADAPTER_TYPE adapter !ADAPTER_OPTIONS! !ADAPTER_TYPE!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseValue LOCAL_IP local_ip !LOCAL_IP! !IP_ADDRESS!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseValue REMOTE_IP remote_ip !REMOTE_IP! !IP_ADDRESS!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseValue PORT_NUMBER port !PORT_NUMBER!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions OUTPUT_FORMAT output_format !FORMAT_OPTIONS! !OUTPUT_FORMAT!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions TIME_OFFSET time_offset !TIME_OFFSET! !TIME_OFFSET!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions LOG_LEVEL log_level !LOG_LEVEL_OPTIONS! !LOG_LEVEL! info
IF "!ERRORLEVEL!"=="1" (GOTO exit)

IF /I NOT "!ROLE!"=="transmitter" (
    IF NOT "!RECEIVER_MODE!"=="play" IF "!OUTPUT!"=="" (
        ECHO ERROR: Must specify an output when the receiver mode is set to '!RECEIVER_MODE!'.
        GOTO exit
    )

    SET "SCHEME=!OUTPUT:~0,7!"    
    IF "!RECEIVER_MODE!"=="stream" (
        ::IF NOT "!SCHEME!"=="http://" (
        ::    ECHO ERROR: Output URL must start with 'http://' when receiver mode is set to 'stream'.
        ::    GOTO exit
        ::)
        ::SET "STREAMER_OUTPUT=!OUTPUT:~7!"
        ECHO.
    ) ELSE IF "!RECEIVER_MODE!"=="store" (
        IF "!SCHEME!"=="http://" (
            ECHO ERROR: Output URL must be a file path when receiver mode is set to 'store'.
            GOTO exit
        )
    )
) ELSE (
    IF DEFINED VIDEO_WIDTH (GOTO ignore_dimensions)
    IF DEFINED VIDEO_HEIGHT (GOTO ignore_dimensions)
    IF NOT DEFINED VIDEO_AVG_FRAME_RATE (GOTO check_overlay)
:ignore_dimensions
    ECHO ERROR: Parameters 'width', 'height', and 'framerate' are ignored in transmitter mode. They are obtained from the input source.
)

:check_overlay
IF DEFINED OVERLAY_SOURCE (
    :: overlay window size is required for the time being to compute overlay frame size
    IF NOT DEFINED OVERLAY_WINDOW_WIDTH GOTO missing_overlay_dimensions
    IF NOT DEFINED OVERLAY_WINDOW_HEIGHT GOTO missing_overlay_dimensions
    IF NOT DEFINED OVERLAY_VIEWPORT_LEFT (SET "OVERLAY_VIEWPORT_LEFT=0")
    IF NOT DEFINED OVERLAY_VIEWPORT_TOP (SET "OVERLAY_VIEWPORT_TOP=0")
    IF NOT DEFINED OVERLAY_VIEWPORT_WIDTH (SET "OVERLAY_VIEWPORT_WIDTH=!OVERLAY_WINDOW_WIDTH!")
    IF NOT DEFINED OVERLAY_VIEWPORT_HEIGHT (SET "OVERLAY_VIEWPORT_HEIGHT=!OVERLAY_WINDOW_HEIGHT!")
    IF NOT DEFINED OVERLAY_FRAME_RATE GOTO missing_frame_rate
    IF NOT DEFINED OVERLAY_POSITION (SET "OVERLAY_POSITION=!DEFAULT_OVERLAY_POSITION!")
    GOTO start
:missing_overlay_dimensions
    ECHO ERROR: Missing overlay window dimensions. Specify overlay window width and height using the '-overlay_window_size' parameter.
    GOTO exit
:missing_frame_rate
    ECHO ERROR: Missing overlay frame rate. Specify the rate using the '-overlay_frame_rate' parameter.
    GOTO exit
) ELSE (
    IF DEFINED OVERLAY_WINDOW_WIDTH GOTO missing_overlay_source
    IF DEFINED OVERLAY_WINDOW_HEIGHT GOTO missing_overlay_source
    IF DEFINED OVERLAY_VIEWPORT_LEFT GOTO missing_overlay_source
    IF DEFINED OVERLAY_VIEWPORT_TOP GOTO missing_overlay_source
    IF DEFINED OVERLAY_VIEWPORT_HEIGHT GOTO missing_overlay_source
    IF DEFINED OVERLAY_VIEWPORT_WIDTH GOTO missing_overlay_source
    IF DEFINED OVERLAY_FRAME_RATE GOTO missing_overlay_source
    IF DEFINED OVERLAY_SCALE_FACTOR GOTO missing_overlay_source
    IF DEFINED OVERLAY_CHROMA_COLOR GOTO missing_overlay_source
    IF DEFINED OVERLAY_BACKGROUND_COLOR GOTO missing_overlay_source
    IF DEFINED OVERLAY_POSITION GOTO missing_overlay_source
    GOTO start
:missing_overlay_source
    ECHO ERROR: One or more overlay parameters were provided but overlay source is missing. Specify source URL using the '-overlay' parameter.
    GOTO exit
)

:start
:: analize input source
IF DEFINED INPUT_SOURCE (
    IF DEFINED QUIET_MODE SET "FFPROBE_CMD=!FFPROBE_CMD! 2^>NUL"
    SET "VIDEO_METADATA=stream=codec_name,width,height,display_aspect_ratio,pix_fmt,r_frame_rate,avg_frame_rate,duration,duration_ts,bit_rate,bits_per_raw_sample,nb_frames : format=duration"
    SET "ANALYZE_VIDEO=!FFPROBE_CMD! -hide_banner -v error -select_streams v:0 -show_entries "!VIDEO_METADATA!" -of default=noprint_wrappers=1 !INPUT_SOURCE!"
    FOR /F "tokens=1,2,3 delims=^=" %%G IN ('!ANALYZE_VIDEO!') DO (SET "VIDEO_%%G=%%H")

    SET "AUDIO_METADATA=stream=codec_name,sample_rate,channels,channel_layout,bits_per_sample,duration,duration_ts,bit_rate,max_bit_rate,bits_per_raw_sample : format=duration"
    SET "ANALYZE_AUDIO=!FFPROBE_CMD! -hide_banner -v error -select_streams a:!AUDIO_STREAM_ID! -show_entries "!AUDIO_METADATA!" -of default=noprint_wrappers=1 !INPUT_SOURCE!"
    FOR /F "tokens=1,2,3 delims=^=" %%G IN ('!ANALYZE_AUDIO!') DO (SET "AUDIO_%%G=%%H")
)

:show_options
IF DEFINED QUIET_MODE GOTO quiet

ECHO.
ECHO Current options:
ECHO   Role                   : !ROLE!
ECHO   Mode                   : !RECEIVER_MODE!
ECHO   Log level              : !LOG_LEVEL!
ECHO.
ECHO   Channel                : !CHANNEL_TYPE!
ECHO     Adapter Type         : !ADAPTER_TYPE!
ECHO     Local IP Address     : !LOCAL_IP!
ECHO     Remote IP Address    : !REMOTE_IP!
ECHO     Port Number          : !PORT_NUMBER!
ECHO.
IF /I NOT "!ROLE!"=="receiver" (
ECHO   Source                 : !INPUT_SOURCE!
) ELSE (
ECHO   Source                 : channel
)
ECHO     Width                : !VIDEO_WIDTH!
ECHO     Height               : !VIDEO_HEIGHT!
ECHO     Frame Rate           : !VIDEO_AVG_FRAME_RATE!
ECHO.
IF DEFINED OVERLAY_SOURCE (
ECHO   Overlay source         : !OVERLAY_SOURCE!
    ECHO     Window Width         : !OVERLAY_WINDOW_WIDTH!
    ECHO     Window Height        : !OVERLAY_WINDOW_HEIGHT!
    ECHO     Viewport Top         : !OVERLAY_VIEWPORT_LEFT!
    ECHO     Viewport Left        : !OVERLAY_VIEWPORT_TOP!
    ECHO     Viewport Width       : !OVERLAY_VIEWPORT_WIDTH!
    ECHO     Viewport Height      : !OVERLAY_VIEWPORT_HEIGHT!
    ECHO     Position             : !OVERLAY_POSITION!
    IF DEFINED OVERLAY_FRAME_RATE (
        ECHO     Frame Rate           : !OVERLAY_FRAME_RATE!
    ) ELSE (
        ECHO     Frame Rate           : !DEFAULT_OVERLAY_FRAME_RATE!
    )
    IF DEFINED OVERLAY_SCALE_FACTOR (
        ECHO     Scale                : !OVERLAY_SCALE_FACTOR!
    ) ELSE (
        ECHO     Scale                : !DEFAULT_OVERLAY_SCALE_FACTOR!
    )
    IF DEFINED OVERLAY_CHROMA_COLOR (
        ECHO     Chroma Color         : !OVERLAY_CHROMA_COLOR!
    ) ELSE (
        ECHO     Chroma Color         : !DEFAULT_OVERLAY_CHROMA_COLOR!
    )
    IF DEFINED OVERLAY_BACKGROUND_COLOR (
        ECHO     Background color     : !OVERLAY_BACKGROUND_COLOR!
    ) ELSE (
        ECHO     Background color     : !DEFAULT_OVERLAY_BACKGROUND_COLOR!
    )
) ELSE (
    ECHO   Overlay                : no
)

IF DEFINED OUTPUT (
    ECHO   Output                 : !OUTPUT!
    ECHO   Output format          : !OUTPUT_FORMAT!
)

ECHO.
IF DEFINED TX_TIMESTAMP (
    ECHO   Tx timestamp           : yes
) ELSE (
    ECHO   Tx timestamp           : no
)
IF DEFINED RX_TIMESTAMP (
    ECHO   Rx timestamp           : yes
) ELSE (
    ECHO   Rx timestamp           : no
)
ECHO.

:quiet
:: set up overlay
IF DEFINED OVERLAY_SOURCE (
    SET "OVERLAY_INPUT=!HTMLSRC_CMD! "!OVERLAY_SOURCE!""
    IF DEFINED OVERLAY_WINDOW_WIDTH (SET "OVERLAY_INPUT=!OVERLAY_INPUT! -ws !OVERLAY_WINDOW_WIDTH!,!OVERLAY_WINDOW_HEIGHT!")
    IF DEFINED OVERLAY_VIEWPORT_WIDTH (SET "OVERLAY_INPUT=!OVERLAY_INPUT! -vo !OVERLAY_VIEWPORT_LEFT!,!OVERLAY_VIEWPORT_TOP! -vs !OVERLAY_VIEWPORT_WIDTH!,!OVERLAY_VIEWPORT_HEIGHT!")
    IF DEFINED OVERLAY_CHROMA_COLOR (SET "OVERLAY_INPUT=!OVERLAY_INPUT! -c !OVERLAY_CHROMA_COLOR!")
    IF DEFINED OVERLAY_BACKGROUND_COLOR (SET "OVERLAY_INPUT=!OVERLAY_INPUT! -b !OVERLAY_BACKGROUND_COLOR!")
    IF DEFINED OVERLAY_SCALE_FACTOR (SET "OVERLAY_INPUT=!OVERLAY_INPUT! -sf !OVERLAY_SCALE_FACTOR!")
    SET /A OVERLAY_FRAME_SIZE=!OVERLAY_VIEWPORT_WIDTH! * !OVERLAY_VIEWPORT_HEIGHT! * 4
    SET "OVERLAY_STREAM= -thread_queue_size !HTMLSRC_QUEUE_SIZE! -f rawvideo -pixel_format bgra -video_size !OVERLAY_VIEWPORT_WIDTH!x!OVERLAY_VIEWPORT_HEIGHT! -framerate !OVERLAY_FRAME_RATE! -i -"
    IF /I "!OVERLAY_POSITION!" == "top-left" (
        SET "OVERLAY_FILTER=!OVERLAY_FILTER!!FILTER_DELIMITER![0]setpts=PTS-STARTPTS[mn];[1]setpts=PTS-STARTPTS,fps=!VIDEO_AVG_FRAME_RATE![ov];[mn][ov]overlay=10:10:eof_action=endall" & SET "FILTER_DELIMITER=, "
    ) ELSE IF /I "!OVERLAY_POSITION!"=="bottom-left" (
        SET "OVERLAY_FILTER=!OVERLAY_FILTER!!FILTER_DELIMITER![0]setpts=PTS-STARTPTS[mn];[1]setpts=PTS-STARTPTS,fps=!VIDEO_AVG_FRAME_RATE![ov];[mn][ov]overlay=10:main_h-overlay_h:eof_action=endall" & SET "FILTER_DELIMITER=, "
    ) ELSE IF /I "!OVERLAY_POSITION!"=="top-right" (
        SET "OVERLAY_FILTER=!OVERLAY_FILTER!!FILTER_DELIMITER![0]setpts=PTS-STARTPTS[mn];[1]setpts=PTS-STARTPTS,fps=!VIDEO_AVG_FRAME_RATE![ov];[mn][ov]overlay=main_w-overlay_w:10:eof_action=endall" & SET "FILTER_DELIMITER=, "
    ) ELSE IF /I "!OVERLAY_POSITION!"=="bottom-right" (
        SET "OVERLAY_FILTER=!OVERLAY_FILTER!!FILTER_DELIMITER![0]setpts=PTS-STARTPTS[mn];[1]setpts=PTS-STARTPTS,fps=!VIDEO_AVG_FRAME_RATE![ov];[mn][ov]overlay=main_w-overlay_w:main_h-overlay_h:eof_action=endall" & SET "FILTER_DELIMITER=, "
    )
)

:: timestamp
SET "PTS_OPTIONS=drawtext=fontfile=/Windows/Fonts/arial.ttf:fontsize=36:start_number=1:text='%%{pts\:gmtime\:0\:%%H\\\:%%M\\\:%%S} %%{n}:bordercolor=black:borderw=1'"
SET "TIMECODE=drawtext=font=Lucida Sans:fontsize=36:start_number=1:timecode='00\:00\:00\:00': r=!VIDEO_AVG_FRAME_RATE!: bordercolor=black:borderw=1'"

:: set up source
IF /I NOT "!ROLE!"=="receiver" (
    SET "INPUT_STREAM= -re -i !INPUT_SOURCE!"
    SET "BIT_RATE= -b:v 2M -maxrate 1M -bufsize 1M"
    SET "TX_VIDEO_STREAM= -vcodec rawvideo -pix_fmt rgb24 -video_size !VIDEO_WIDTH!x!VIDEO_HEIGHT! -r !VIDEO_AVG_FRAME_RATE!!BIT_RATE! -f rawvideo tcp://127.0.0.1:!VIDEO_IN_PORT!"
    SET "TX_AUDIO_STREAM= -acodec pcm_s16le -f s16le tcp://127.0.0.1:!AUDIO_IN_PORT!"
    IF DEFINED TX_TIMESTAMP (SET "TRANSMIT_PTS=!FILTER_DELIMITER!!PTS_OPTIONS!:x=20:y=20:fontcolor=white" & SET "FILTER_DELIMITER=, ")
    IF DEFINED FILTER_DELIMITER (SET "TX_FILTER= -filter_complex "!OVERLAY_FILTER!!TRANSMIT_PTS!"")
    SET "TX_PROCESSOR=!FFMPEG_CMD!!FFMPEG_GLOBAL_OPTIONS!!INPUT_STREAM!!OVERLAY_STREAM!!TX_FILTER!!TX_VIDEO_STREAM!!TX_AUDIO_STREAM!"
    IF DEFINED OVERLAY_INPUT (SET "SOURCE=!OVERLAY_INPUT! | !TX_PROCESSOR!") ELSE (SET "SOURCE=!TX_PROCESSOR!")
)

:: set up encoder
IF /I NOT "!ROLE!"=="transmitter" (
    SET "FILTER_DELIMITER="
    IF /I "!OUTPUT_FORMAT!"=="rgb" (
        SET "ENCODER_FORMAT= -f rawvideo"
    ) ELSE IF /I "!OUTPUT_FORMAT!"=="mp4" (
        IF /I "!RECEIVER_MODE!"=="store" (SET "ENCODER_FORMAT= -f mp4") ELSE (SET "ENCODER_FORMAT= -f mpegts")
    )

    SET "ENCODER_RAW= -an -c:v rawvideo -pix_fmt rgb24 -video_size !VIDEO_WIDTH!x!VIDEO_HEIGHT! -r !VIDEO_AVG_FRAME_RATE!!BIT_RATE!!OUTPUT_FORMAT!"
    IF /I "!RECEIVER_MODE!"=="stream" (
        SET "ENCODER_OUTPUT= -ac 2 -c:v libx264 -x264opts sliced-threads -pix_fmt yuv420p -crf 21 -preset veryfast -tune zerolatency -vsync cfr -g 10 -b:a 128k -f hls -hls_time 10 -hls_playlist_type event"
    ) ELSE (
        SET "ENCODER_OUTPUT= -ac 2 -c:v libx264 -x264opts sliced-threads -pix_fmt yuv420p -preset ultrafast -tune zerolatency -vsync cfr -g 10!ENCODER_FORMAT!"
    )

    SET "RX_VIDEO_STREAM= -thread_queue_size !VIDEO_QUEUE_SIZE! -pixel_format rgb24 -video_size !VIDEO_WIDTH!x!VIDEO_HEIGHT! -framerate !VIDEO_AVG_FRAME_RATE! -f rawvideo -i tcp://127.0.0.1:!VIDEO_OUT_PORT!"
    SET "RX_AUDIO_STREAM= -itsoffset !TIME_OFFSET! -thread_queue_size !AUDIO_QUEUE_SIZE! -f s16le -sample_rate 44100 -channels 2 -i tcp://127.0.0.1:!AUDIO_OUT_PORT!"
    IF DEFINED RX_TIMESTAMP (SET "RECEIVE_PTS=!PTS_OPTIONS!:x=(w-tw-20):y=20:fontcolor=white" & SET "FILTER_DELIMITER=, ")
    IF DEFINED FILTER_DELIMITER (SET "RX_FILTER= -vf "!RECEIVE_PTS!"")
    SET "ENCODER=!FFMPEG_CMD!!FFMPEG_GLOBAL_OPTIONS!!RX_VIDEO_STREAM!!RX_AUDIO_STREAM!!RX_FILTER!!ENCODER_OUTPUT!"
)

:: set up channel
IF /I NOT "!ROLE!"=="receiver" (
    :: -frame_rate !VIDEO_AVG_FRAME_RATE!
    SET "CHANNEL_TRANSMITTER=!CDIPIPE_CMD! -role transmitter -channel !CHANNEL_TYPE! -adapter !ADAPTER_TYPE! -local_ip !LOCAL_IP! -remote_ip !REMOTE_IP! -port !PORT_NUMBER! -video_in_port !VIDEO_IN_PORT! -audio_in_port !AUDIO_IN_PORT! -frame_width !VIDEO_WIDTH! -frame_height !VIDEO_HEIGHT! -log_level !LOG_LEVEL!"
)

IF /I NOT "!ROLE!"=="transmitter" (
    SET "CHANNEL_RECEIVER=!CDIPIPE_CMD! -role receiver -channel !CHANNEL_TYPE! -adapter !ADAPTER_TYPE! -local_ip !LOCAL_IP! -port !PORT_NUMBER! -video_out_port !VIDEO_OUT_PORT! -audio_out_port !AUDIO_OUT_PORT! -log_level !LOG_LEVEL!"
)

:: set up player/streamer
IF /I NOT "!ROLE!"=="transmitter" (
    IF /I "!RECEIVER_MODE!"=="play" (
        SET "WINDOW_TITLE= -window_title "CHANNEL TYPE - !CHANNEL_TYPE!""
        SET "PLAYER=!FFPLAY_CMD!!FFPLAY_GLOBAL_OPTIONS!!WINDOW_TITLE! -i -"
        SET "DESTINATION=!ENCODER! - | !PLAYER!"
    ) ELSE (
        SET "DESTINATION=!ENCODER! !OUTPUT!"
    )
)

IF DEFINED DEBUG (
    IF DEFINED SOURCE (ECHO ## SOURCE & ECHO !SOURCE! & ECHO.)
    IF DEFINED DESTINATION (ECHO ## OUTPUT & ECHO !DESTINATION! & ECHO.)
    IF DEFINED CHANNEL_RECEIVER (ECHO ## RECEIVER & ECHO !CHANNEL_RECEIVER! & ECHO.)
    IF DEFINED CHANNEL_TRANSMITTER (ECHO ## TRANSMITTER & ECHO !CHANNEL_TRANSMITTER! & ECHO.)
    GOTO exit
) 

IF /I "!ROLE!"=="both" (GOTO receiver) ELSE (IF /I "!ROLE!"=="receiver" GOTO receiver)
:check_tx
IF /I "!ROLE!"=="both" (GOTO transmitter) ELSE (IF /I "!ROLE!"=="transmitter" GOTO transmitter)
GOTO exit

:receiver
START !WINDOW_MODE! "CHANNEL RECEIVER" CMD !CONSOLE_MODE! "PROMPT RECEIVER$G&&!CHANNEL_RECEIVER!"
START !WINDOW_MODE! "ENCODER/PLAYER" CMD !CONSOLE_MODE! "PROMPT PLAYER$G&&!DESTINATION!"
GOTO check_tx

:transmitter
START !WINDOW_MODE! "CHANNEL TRANSMITTER" CMD !CONSOLE_MODE! "PROMPT TRANSMITTER$G&&!CHANNEL_TRANSMITTER!"
TIMEOUT 3 > nul
START !WINDOW_MODE! "SOURCE" CMD !CONSOLE_MODE! "PROMPT SOURCE$G&&!SOURCE!"
GOTO exit

:usage
ECHO.
ECHO Usage:
ECHO   !SCRIPT_NAME! ^<source^> [^<options...^>]
ECHO     ^<source^>                              : source file path or URL (required)
ECHO.
ECHO Options:
ECHO     -role ^<type^>                          : type of role: transmitter ^| receiver ^| both (optional, default: !ROLE!)
ECHO     -mode ^<option^>                        : receiver mode: play ^| stream ^| store (optional, default: !RECEIVER_MODE!)
ECHO     -log_level ^<value^>                    : log level : trace ^| debug ^| info ^| warning ^| error (optional, default: !LOG_LEVEL!)
ECHO     -channel ^<type^>                       : type of channel: cdi ^| cdistream ^| tcp (optional, default: !CHANNEL_TYPE!)
ECHO     -width ^<value^>                        : input source frame width (required in receiver mode, default: !DEFAULT_VIDEO_WIDTH!)
ECHO     -height ^<value^>                       : input source frame height (required in receiver mode, default: !DEFAULT_VIDEO_HEIGHT!)
ECHO     -framerate ^<value^>                    : input source frame rate (required in receiver mode, default: !DEFAULT_VIDEO_AVG_FRAME_RATE!)
ECHO     -adapter ^<type^>                       : type of adapter: efa ^| socket ^| socketlibfabric
ECHO                                             (optional, default: !ADAPTER_TYPE!)
ECHO     -local_ip ^<ip_address^>                : local IP address (optional, default 127.0.0.1 or IP address 
ECHO                                             of first local adapter when -remote_ip is specified)
ECHO     -remote_ip ^<ip_address^>               : remote IP address (optional, applies to transmitter only, default 127.0.0.1
ECHO                                             or IP address of first local adapter when -local_ip is specified)
ECHO     -port ^<port_number^>                   : destination port number (optional, default !PORT_NUMBER!)
ECHO     -output ^<destination^>                 : output file name or URL (required for stream and store receiver modes)
ECHO     -output_format ^<type^>                 : output format type: rgb ^| mp4 (optional, default: !OUTPUT_FORMAT!)
ECHO.
ECHO Overlay options:
ECHO     -overlay ^<overlay_source^>             : file path or URL of the overlay page (optional, default: no overlay)
ECHO     -overlay_window_size ^<width height^>   : overlay window pixel width and height (optional, default: !DEFAULT_OVERLAY_WINDOW_SIZE!)
ECHO     -overlay_viewport_origin ^<left top^>   : overlay viewport origin left top coordinates (optional, default: !DEFAULT_OVERLAY_VIEWPORT_ORIGIN!)
ECHO     -overlay_viewport_size ^<width height^> : overlay viewport width and height (optional, default: !DEFAULT_OVERLAY_VIEWPORT_SIZE!)
ECHO     -overlay_position ^<position^>          : overlay position [top-left ^| top-right ^| bottom-left ^| bottom-right]
ECHO                                             (optional, default: !DEFAULT_OVERLAY_POSITION!)
ECHO     -overlay_framerate ^<rate^>             : overlay frame rate (optional, default: !DEFAULT_OVERLAY_FRAME_RATE!)
ECHO     -overlay_scale_factor ^<scale^>         : overlay scale factor (optional, default: !DEFAULT_OVERLAY_SCALE_FACTOR!)
ECHO     -overlay_chroma_color ^<color^>         : overlay transparent color (optional, default: !DEFAULT_OVERLAY_CHROMA_COLOR!)
ECHO     -overlay_background_color ^<color^>     : overlay background color (optional, default: !DEFAULT_OVERLAY_BACKGROUND_COLOR!)
ECHO.
ECHO Advanced options:
ECHO     -video_in_port ^<port_number^>          : video input port number (optional, default: !VIDEO_IN_PORT!)
ECHO     -audio_in_port ^<port_number^>          : audio input port number (optional, default: !AUDIO_IN_PORT!)
ECHO     -video_out_port ^<port_number^>         : video input port number (optional, default: !VIDEO_OUT_PORT!)
ECHO     -audio_out_port ^<port_number^>         : audio input port number (optional, default: !AUDIO_OUT_PORT!)
ECHO     -time_offset ^<seconds^>                : audio/video stream time offset (optional, default: !TIME_OFFSET!)
ECHO     -rx_timestamp                         : display receiver timestamp overlay (optional, default: !DEFAULT_RX_TIMESTAMP!)
ECHO     -tx_timestamp                         : display transmitter timestamp overlay (optional, default: !DEFAULT_TX_TIMESTAMP!)
ECHO     -keep                                 : keep tool windows open (optional, default: !DEFAULT_KEEP_WINDOWS!)
ECHO.
ECHO IMPORTANT: set the following environment variables before using or configure all required executables in the path.
ECHO     FFMPEG_PATH : path to ffmpeg.exe, ffplay.exe, and ffprobe.exe
ECHO     TOOLS_PATH  : path to htmlsrc.exe and cdipipe.exe
ECHO.
ECHO To see some example command lines, type !SCRIPT_NAME! -examples
ECHO.
ECHO KNOWN ISSUES:
ECHO.   - To stop, press 'q' with the focus on the player window. Then, press CTRL + C in the transmitter and receiver windows.
ECHO.
GOTO exit

:ParseOptions
SET "%1=%~4"
SET PARAMETER_VALUE=%4
SET OPTIONS_LIST=

SETLOCAL ENABLEDELAYEDEXPANSION
FOR %%A IN (%~3) DO (
    SET OPTIONS_LIST=!OPTIONS_LIST!!DELIMITER!'%%A'
    IF /I '!PARAMETER_VALUE!'=='%%A' (SET IS_VALID_PARAMETER=1)
    SET "DELIMITER=, "
)

IF NOT DEFINED IS_VALID_PARAMETER (
    ECHO ERROR: Missing or invalid '%~2' argument value. Valid values are: !OPTIONS_LIST!
    ENDLOCAL & EXIT /B 1
)
ENDLOCAL
EXIT /B 0

:ParseValue
SET "%1=%~3"
SET PARAMETER_VALUE=%3

SETLOCAL ENABLEDELAYEDEXPANSION
IF "!PARAMETER_VALUE!"=="" (SET IS_INVALID_PARAMETER=1)
IF "!PARAMETER_VALUE:~0,1!"=="-" (SET IS_INVALID_PARAMETER=1)
IF DEFINED IS_INVALID_PARAMETER (
    ECHO ERROR: Missing or invalid value for parameter '%~2'.
    ENDLOCAL & EXIT /B 1
)
ENDLOCAL
EXIT /B 0

:exit
ENDLOCAL
