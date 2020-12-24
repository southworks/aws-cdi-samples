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
SET ROLE_OPTIONS="source transmitter receiver both"
SET CHANNEL_OPTIONS="cdistream cdi tcp"
SET ADAPTER_OPTIONS="efa socketlibfabric"
SET FORMAT_OPTIONS="rgb mp4"
SET LOG_LEVEL_OPTIONS="trace debug info warning error"

:: default values
SET "DEFAULT_CHANNEL_TYPE=cdistream"
SET "DEFAULT_ADAPTER_TYPE=socketlibfabric"
SET "DEFAULT_IP=127.0.0.1"
SET "DEFAULT_VIDEO_IN_PORT=1000"
SET "DEFAULT_AUDIO_IN_PORT=1001"
SET "DEFAULT_VIDEO_OUT_PORT=3000"
SET "DEFAULT_AUDIO_OUT_PORT=3001"
SET "DEFAULT_PORT_NUMBER=2000"
SET "DEFAULT_LOG_LEVEL=info"
SET "VIDEO_QUEUE_SIZE=1500"
SET "AUDIO_QUEUE_SIZE=2000"
SET "HTMLSRC_QUEUE_SIZE=2000"
SET "VIDEO_STREAM_INDEX=0"
SET "AUDIO_STREAM_INDEX=0"
SET "TIME_OFFSET=0.9"
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
SET "SCHEME=!PARAMETER:~0,6!"
IF /I "!SCHEME!"=="udp://" (
    SET "INPUT_SOURCE=!PARAMETER!?overrun_nonfatal=1&fifo_size=5000000"
) ELSE IF /I "!SCHEME!"=="rtp://" (
    SET "INPUT_SOURCE=!PARAMETER!?overrun_nonfatal=1&fifo_size=5000000"
) ELSE (
    SET "INPUT_SOURCE=!MEDIA_PATH!!PARAMETER!"
)

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

    IF "!PARAMETER_NAME!"=="show" (
        SET "SHOW_SETTINGS=1" & GOTO next
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

    IF "!PARAMETER_NAME!"=="audio_stream" (
        CALL :ParseValue AUDIO_STREAM_INDEX !PARAMETER_NAME! !PARAMETER_VALUE!
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
        SET "FFMPEG_CMD=!FFMPEG_CMD! -loglevel quiet 2>NUL"
        SET "FFPLAY_CMD=!FFPLAY_CMD! -loglevel quiet 2>NUL"
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
IF /I "!ADAPTER_TYPE!"=="efa" (SET USE_NETWORK=1)
IF NOT DEFINED ADAPTER_TYPE IF /I "!DEFAULT_ADAPTER_TYPE!"=="efa" (SET USE_NETWORK=1)
IF DEFINED LOCAL_IP (SET USE_NETWORK=1)
IF DEFINED REMOTE_IP (SET USE_NETWORK=1)
IF DEFINED USE_NETWORK (FOR /f "tokens=2 delims=\[\]" %%A IN ('ping -n 1 -4 "%COMPUTERNAME%"') DO SET IP_ADDRESS=%%A) 

IF /I "!ROLE!"=="receiver" (
    IF NOT DEFINED INPUT_SOURCE (
        CALL :ParseValue VIDEO_WIDTH width !VIDEO_WIDTH! !VIDEO_WIDTH!
        IF "!ERRORLEVEL!"=="1" (GOTO exit)

        CALL :ParseValue VIDEO_HEIGHT height !VIDEO_HEIGHT! !VIDEO_HEIGHT!
        IF "!ERRORLEVEL!"=="1" (GOTO exit)

        CALL :ParseValue VIDEO_AVG_FRAME_RATE framerate !VIDEO_AVG_FRAME_RATE! !VIDEO_AVG_FRAME_RATE!
        IF "!ERRORLEVEL!"=="1" (GOTO exit)

        GOTO validation_1
    )
)

CALL :ParseValue INPUT_SOURCE source !INPUT_SOURCE! !INPUT_SOURCE!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

:validation_1
CALL :ParseOptions RECEIVER_MODE mode !RECEIVER_MODE_OPTIONS! !RECEIVER_MODE!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions ROLE role !ROLE_OPTIONS! !ROLE! both
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions OUTPUT_FORMAT output_format !FORMAT_OPTIONS! !OUTPUT_FORMAT!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions TIME_OFFSET time_offset !TIME_OFFSET! !TIME_OFFSET!
IF "!ERRORLEVEL!"=="1" (GOTO exit)

CALL :ParseOptions LOG_LEVEL log_level !LOG_LEVEL_OPTIONS! !LOG_LEVEL! info
IF "!ERRORLEVEL!"=="1" (GOTO exit)

IF /I NOT "!ROLE!"=="transmitter" (
    IF "!OUTPUT!"=="" (
        IF /I "!ROLE!"=="source" (
            ECHO ERROR: Must specify an output when the role is '!ROLE!'.
            GOTO exit
        )

        IF /I NOT "!RECEIVER_MODE!"=="play" (
            ECHO ERROR: Must specify an output when the receiver mode is '!RECEIVER_MODE!'.
            GOTO exit
        )
    )

    SET "SCHEME=!OUTPUT:~0,7!"    
    IF /I "!RECEIVER_MODE!"=="stream" (
        ::IF NOT "!SCHEME!"=="http://" (
        ::    ECHO ERROR: Output URL must start with 'http://' when receiver mode is set to 'stream'.
        ::    GOTO exit
        ::)
        ::SET "STREAMER_OUTPUT=!OUTPUT:~7!"
        ECHO.
    ) ELSE IF /I "!RECEIVER_MODE!"=="store" (
        IF /I "!SCHEME!"=="http://" (
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
    IF /I "!ROLE!"=="receiver" (GOTO invalid_overlay_parameter)
    IF /I "!ROLE!"=="source" (GOTO invalid_overlay_parameter)
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

:invalid_overlay_parameter    
    ECHO ERROR: Overlay parameter is not valid in !ROLE! mode.
    GOTO exit
)

:start
:: analize input source
IF DEFINED INPUT_SOURCE (
    ECHO Analyzing input source...
    SET "FFPROBE_CMD=!FFPROBE_CMD! 2^>NUL"
    SET "VIDEO_METADATA=stream=codec_name,width,height,display_aspect_ratio,pix_fmt,r_frame_rate,avg_frame_rate,duration,duration_ts,bit_rate,bits_per_raw_sample,nb_frames : format=duration"
    SET "ANALYZE_VIDEO=!FFPROBE_CMD! -hide_banner -v error -select_streams v:0 -show_entries "!VIDEO_METADATA!" -of default=noprint_wrappers=1 "!INPUT_SOURCE!""
    FOR /F "tokens=1,2,3 delims=^=" %%G IN ('!ANALYZE_VIDEO!') DO (SET "VIDEO_%%G=%%H")

    SET "AUDIO_METADATA=stream=codec_name,sample_rate,channels,channel_layout,bits_per_sample,duration,duration_ts,bit_rate,max_bit_rate,bits_per_raw_sample : format=duration"
    SET "ANALYZE_AUDIO=!FFPROBE_CMD! -hide_banner -v error -select_streams a:!AUDIO_STREAM_INDEX! -show_entries "!AUDIO_METADATA!" -of default=noprint_wrappers=1 "!INPUT_SOURCE!""
    FOR /F "tokens=1,2,3 delims=^=" %%G IN ('!ANALYZE_AUDIO!') DO (SET "AUDIO_%%G=%%H")
)

:show_settings
IF DEFINED QUIET_MODE GOTO quiet

ECHO Current options:
ECHO   Role                   : !ROLE!
ECHO   Mode                   : !RECEIVER_MODE!
ECHO   Log level              : !LOG_LEVEL!
ECHO.
IF DEFINED CHANNEL_TYPE (
    ECHO   Channel                : !CHANNEL_TYPE!
) ELSE (
    ECHO   Channel                : !DEFAULT_CHANNEL_TYPE!
)
IF DEFINED ADAPTER_TYPE (
    ECHO     Adapter Type         : !ADAPTER_TYPE!
) ELSE (
    ECHO     Adapter Type         : !DEFAULT_ADAPTER_TYPE!
)
IF DEFINED LOCAL_IP (
    ECHO     Local IP Address     : !LOCAL_IP!
) ELSE (
    ECHO     Local IP Address     : !DEFAULT_IP!
)
IF DEFINED REMOTE_IP (
    ECHO     Remote IP Address    : !REMOTE_IP!
) ELSE (
    ECHO     Remote IP Address    : !DEFAULT_IP!
)
IF DEFINED PORT_NUMBER (
    ECHO     Port Number          : !PORT_NUMBER!
) ELSE (
    ECHO     Port Number          : !DEFAULT_PORT_NUMBER!
)
ECHO.
IF /I NOT "!ROLE!"=="receiver" (
ECHO   Source                 : !INPUT_SOURCE!
) ELSE (
ECHO   Source                 : channel
)
ECHO     Width                : !VIDEO_WIDTH!
ECHO     Height               : !VIDEO_HEIGHT!
ECHO     Frame Rate           : !VIDEO_AVG_FRAME_RATE!
IF /I NOT "!ROLE!"=="receiver" (
    ECHO     Audio Codec          : !AUDIO_CODEC_NAME!
    ECHO     Audio Channels       : !AUDIO_CHANNELS!
    ECHO     Audio Sampling Rate  : !AUDIO_SAMPLE_RATE!
)
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
IF DEFINED SHOW_SETTINGS GOTO exit

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
IF /I "!ROLE!"=="source" (
    SET "LIVE_SOURCE=!FFMPEG_CMD!!FFMPEG_GLOBAL_OPTIONS! -re -stream_loop -1 -i "!INPUT_SOURCE!" -c:a copy -c:v copy -f rtp_mpegts !OUTPUT!"
    GOTO run
)

IF /I NOT "!ROLE!"=="receiver" (
    SET "FILTER_DELIMITER="
    SET "INPUT_STREAM= -re -i "!INPUT_SOURCE!""
    SET "BIT_RATE= -b:v 2M -maxrate 1M -bufsize 1M"
    IF /I "!AUDIO_CODEC_NAME!"=="aac" (SET "AUDIO_FORMAT=adts") ELSE (SET "AUDIO_FORMAT=!AUDIO_CODEC_NAME!")
    IF DEFINED VIDEO_IN_PORT (SET "VIDEO_IN_ENDPOINT=tcp://127.0.0.1:!VIDEO_IN_PORT!") ELSE (SET "VIDEO_IN_ENDPOINT=tcp://127.0.0.1:!DEFAULT_VIDEO_IN_PORT!")
    IF DEFINED AUDIO_IN_PORT (SET "AUDIO_IN_ENDPOINT=tcp://127.0.0.1:!AUDIO_IN_PORT!") ELSE (SET "AUDIO_IN_ENDPOINT=tcp://127.0.0.1:!DEFAULT_AUDIO_IN_PORT!")
    SET "TX_VIDEO_STREAM= -map 0:v:!VIDEO_STREAM_INDEX! -c:v rawvideo -pix_fmt rgb24 -video_size !VIDEO_WIDTH!x!VIDEO_HEIGHT! -r !VIDEO_AVG_FRAME_RATE!!BIT_RATE! -f rawvideo !VIDEO_IN_ENDPOINT!"
    SET "TX_AUDIO_STREAM= -map 0:a:!AUDIO_STREAM_INDEX! -c:a copy -f !AUDIO_FORMAT! !AUDIO_IN_ENDPOINT!"
    IF DEFINED OVERLAY_FILTER (SET "TX_FILTER=!TX_FILTER!!FILTER_DELIMITER!!OVERLAY_FILTER!") & SET "FILTER_DELIMITER=, ")
    IF DEFINED TX_TIMESTAMP (SET "TX_FILTER=!TX_FILTER!!FILTER_DELIMITER!!PTS_OPTIONS!:x=20:y=20:fontcolor=white" & SET "FILTER_DELIMITER=, ")
    IF DEFINED TX_FILTER (SET "TX_FILTER= -filter_complex "!TX_FILTER!"")
    SET "TX_PROCESSOR=!FFMPEG_CMD!!FFMPEG_GLOBAL_OPTIONS!!INPUT_STREAM!!OVERLAY_STREAM!!TX_FILTER!!TX_VIDEO_STREAM!!TX_AUDIO_STREAM!"
    IF DEFINED OVERLAY_INPUT (SET "COMPOSER=!OVERLAY_INPUT! | !TX_PROCESSOR!") ELSE (SET "COMPOSER=!TX_PROCESSOR!")
)

:: set up encoder
IF /I NOT "!ROLE!"=="transmitter" (
    IF /I "!RECEIVER_MODE!"=="stream" (
        SET "ENCODER_OUTPUT= -c:a copy -c:v libx264 -x264opts sliced-threads -pix_fmt yuv420p -crf 21 -preset veryfast -tune zerolatency -vsync cfr -g 10 -b:a 128k -f hls -hls_time 10 -hls_playlist_type event"
    ) ELSE (
        IF /I "!OUTPUT_FORMAT!"=="rgb" (
            SET "ENCODER_FORMAT= -f rawvideo"
            SET "ENCODER_OUTPUT= -an -c:v rawvideo -pix_fmt rgb24 -video_size !VIDEO_WIDTH!x!VIDEO_HEIGHT! -r !VIDEO_AVG_FRAME_RATE!!BIT_RATE!!ENCODER_FORMAT!"
        ) ELSE IF /I "!OUTPUT_FORMAT!"=="mp4" (
            IF /I "!RECEIVER_MODE!"=="store" (SET "ENCODER_FORMAT= -f mp4") ELSE (SET "ENCODER_FORMAT= -f mpegts")
            SET "ENCODER_OUTPUT= -c:a copy -c:v libx264 -x264opts sliced-threads -pix_fmt yuv420p -preset ultrafast -tune zerolatency -vsync cfr -g 10!ENCODER_FORMAT!"
        )
    )

    SET "FILTER_DELIMITER="
    IF DEFINED VIDEO_OUT_PORT (SET "VIDEO_OUT_ENDPOINT=tcp://127.0.0.1:!VIDEO_OUT_PORT!") ELSE (SET "VIDEO_OUT_ENDPOINT=tcp://127.0.0.1:!DEFAULT_VIDEO_OUT_PORT!")
    IF DEFINED AUDIO_OUT_PORT (SET "AUDIO_OUT_ENDPOINT=tcp://127.0.0.1:!AUDIO_OUT_PORT!") ELSE (SET "AUDIO_OUT_ENDPOINT=tcp://127.0.0.1:!DEFAULT_AUDIO_OUT_PORT!")
    SET "RX_VIDEO_STREAM= -itsoffset !TIME_OFFSET! -thread_queue_size !VIDEO_QUEUE_SIZE! -pixel_format rgb24 -video_size !VIDEO_WIDTH!x!VIDEO_HEIGHT! -framerate !VIDEO_AVG_FRAME_RATE! -f rawvideo -i !VIDEO_OUT_ENDPOINT!"
    SET "RX_AUDIO_STREAM= -thread_queue_size !AUDIO_QUEUE_SIZE! -i !AUDIO_OUT_ENDPOINT!"
    IF DEFINED RX_TIMESTAMP (SET "RX_FILTER=!RX_FILTER!!FILTER_DELIMITER!!PTS_OPTIONS!:x=(w-tw-20):y=20:fontcolor=white" & SET "FILTER_DELIMITER=, ")
    IF DEFINED RX_FILTER (SET "RX_FILTER= -filter_complex "!RX_FILTER!"")
    SET "ENCODER=!FFMPEG_CMD!!FFMPEG_GLOBAL_OPTIONS!!RX_AUDIO_STREAM!!RX_VIDEO_STREAM!!RX_FILTER!!ENCODER_OUTPUT!"
)

IF /I "!ADAPTER_TYPE!"=="efa" (IF NOT DEFINED LOCAL_IP (SET "LOCAL_IP=!IP_ADDRESS!"))
IF NOT DEFINED ADAPTER_TYPE IF /I "!DEFAULT_ADAPTER_TYPE!"=="efa" (IF NOT DEFINED LOCAL_IP (SET "LOCAL_IP=!IP_ADDRESS!"))
IF DEFINED REMOTE_IP IF /I NOT "!REMOTE_IP!"=="!DEFAULT_IP!" IF NOT DEFINED LOCAL_IP (SET "LOCAL_IP=!IP_ADDRESS!")
IF DEFINED CHANNEL_TYPE IF /I NOT "!CHANNEL_TYPE!"=="!DEFAULT_CHANNEL_TYPE!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -channel !CHANNEL_TYPE!")
IF DEFINED ADAPTER_TYPE IF /I NOT "!ADAPTER_TYPE!"=="!DEFAULT_ADAPTER_TYPE!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -adapter !ADAPTER_TYPE!")
IF DEFINED LOCAL_IP IF /I NOT "!LOCAL_IP!"=="!DEFAULT_LOCAL_IP!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -local_ip !LOCAL_IP!")
IF DEFINED REMOTE_IP IF /I NOT "!REMOTE_IP!"=="!DEFAULT_IP!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -remote_ip !REMOTE_IP!")
IF DEFINED PORT_NUMBER IF /I NOT "!PORT_NUMBER!"=="!DEFAULT_PORT_NUMBER!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -port !PORT_NUMBER!")
IF DEFINED VIDEO_IN_PORT IF /I NOT "!VIDEO_IN_PORT!"=="!DEFAULT_VIDEO_IN_PORT!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -video_in_port !VIDEO_IN_PORT!")
IF DEFINED VIDEO_OUT_PORT IF /I NOT "!VIDEO_OUT_PORT!"=="!DEFAULT_VIDEO_OUT_PORT!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -video_out_port !VIDEO_OUT_PORT!")
IF DEFINED AUDIO_IN_PORT IF /I NOT "!AUDIO_IN_PORT!"=="!DEFAULT_AUDIO_IN_PORT!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -audio_in_port !AUDIO_IN_PORT!")
IF DEFINED AUDIO_OUT_PORT IF /I NOT "!AUDIO_OUT_PORT!"=="!DEFAULT_AUDIO_OUT_PORT!" (SET "NETWORK_PARAMETERS=!NETWORK_PARAMETERS! -audio_out_port !AUDIO_OUT_PORT!")
IF DEFINED LOG_LEVEL IF /I NOT "!LOG_LEVEL!"=="!DEFAULT_LOG_LEVEL!" (SET "LOG= -log_level !LOG_LEVEL!")

:: set up channel
IF /I NOT "!ROLE!"=="receiver" (
    SET "CHANNEL_TRANSMITTER=!CDIPIPE_CMD! -role transmitter!NETWORK_PARAMETERS! -frame_width !VIDEO_WIDTH! -frame_height !VIDEO_HEIGHT!!LOG!"
)

IF /I NOT "!ROLE!"=="transmitter" (
    SET "CHANNEL_RECEIVER=!CDIPIPE_CMD! -role receiver!NETWORK_PARAMETERS!!LOG!"
)

:: set up player/streamer
IF /I NOT "!ROLE!"=="transmitter" (
    IF /I "!RECEIVER_MODE!"=="play" (
        IF DEFINED CHANNEL_TYPE (SET "WINDOW_TITLE= -window_title "CHANNEL TYPE: !CHANNEL_TYPE!"") ELSE (SET "WINDOW_TITLE= -window_title "CHANNEL TYPE: !DEFAULT_CHANNEL_TYPE!"")
        SET "PLAYER=!FFPLAY_CMD!!FFPLAY_GLOBAL_OPTIONS!!WINDOW_TITLE! -i -"
        SET "DESTINATION=!ENCODER! - | !PLAYER!"
    ) ELSE (
        SET "DESTINATION=!ENCODER! !OUTPUT!"
    )
)

:run
IF DEFINED DEBUG (
    IF DEFINED LIVE_SOURCE (ECHO ## LIVE SOURCE & ECHO !LIVE_SOURCE! & ECHO.)
    IF DEFINED COMPOSER (ECHO ## COMPOSER & ECHO !COMPOSER! & ECHO.)
    IF DEFINED DESTINATION (ECHO ## OUTPUT & ECHO !DESTINATION! & ECHO.)
    IF DEFINED CHANNEL_RECEIVER (ECHO ## RECEIVER & ECHO !CHANNEL_RECEIVER! & ECHO.)
    IF DEFINED CHANNEL_TRANSMITTER (ECHO ## TRANSMITTER & ECHO !CHANNEL_TRANSMITTER! & ECHO.)
    GOTO exit
) 

IF /I "!ROLE!"=="source" (
    START !WINDOW_MODE! "LIVE SOURCE" CMD !CONSOLE_MODE! "PROMPT LIVE_SOURCE$G&&!LIVE_SOURCE!"
    GOTO exit
)

IF /I "!ROLE!"=="both" (GOTO receiver) ELSE (IF /I "!ROLE!"=="receiver" GOTO receiver)
:check_tx
IF /I "!ROLE!"=="both" (GOTO transmitter) ELSE (IF /I "!ROLE!"=="transmitter" GOTO transmitter)
GOTO exit

:receiver
START !WINDOW_MODE! "CHANNEL RECEIVER" CMD !CONSOLE_MODE! "PROMPT RECEIVER$G&&!CHANNEL_RECEIVER!"
START !WINDOW_MODE! "ENCODER" CMD !CONSOLE_MODE! "PROMPT PLAYER$G&&!DESTINATION!"
GOTO check_tx

:transmitter
START !WINDOW_MODE! "CHANNEL TRANSMITTER" CMD !CONSOLE_MODE! "PROMPT TRANSMITTER$G&&!CHANNEL_TRANSMITTER!"
TIMEOUT 3 > nul
START !WINDOW_MODE! "COMPOSER" CMD !CONSOLE_MODE! "PROMPT COMPOSER$G&&!COMPOSER!"
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
ECHO     -channel ^<type^>                       : type of channel: cdi ^| cdistream ^| tcp (optional, default: !DEFAULT_CHANNEL_TYPE!)
ECHO     -width ^<value^>                        : input source frame width (required in receiver mode, default: !DEFAULT_VIDEO_WIDTH!)
ECHO     -height ^<value^>                       : input source frame height (required in receiver mode, default: !DEFAULT_VIDEO_HEIGHT!)
ECHO     -framerate ^<value^>                    : input source frame rate (required in receiver mode, default: !DEFAULT_VIDEO_AVG_FRAME_RATE!)
ECHO     -adapter ^<type^>                       : type of adapter: efa ^| socketlibfabric
ECHO                                             (optional, default: !ADAPTER_TYPE!)
ECHO     -local_ip ^<ip_address^>                : local IP address (optional, default !DEFAULT_IP! or IP address 
ECHO                                             of first local adapter when -remote_ip is specified)
ECHO     -remote_ip ^<ip_address^>               : remote IP address (optional, applies to transmitter only, default !DEFAULT_IP!
ECHO                                             or IP address of first local adapter when -local_ip is specified)
ECHO     -port ^<port_number^>                   : destination port number (optional, default !DEFAULT_PORT_NUMBER!)
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
ECHO     -audio_stream_index ^<index^>           : input audio stream index (optional, default: !AUDIO_STREAM_INDEX!)
ECHO     -time_offset ^<seconds^>                : audio/video stream time offset (optional, default: !TIME_OFFSET!)
ECHO     -show                                 : show current settings and exit (optional, default: false)
ECHO     -keep                                 : keep tool windows open (optional, default: !DEFAULT_KEEP_WINDOWS!)
ECHO.
ECHO IMPORTANT: set the following environment variables before using or configure all required executables in the path.
ECHO     FFMPEG_PATH : path to ffmpeg.exe, ffplay.exe, and ffprobe.exe
ECHO     TOOLS_PATH  : path to htmlsrc.exe and cdipipe.exe
ECHO     MEDIA_PATH  : path to source media files
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
