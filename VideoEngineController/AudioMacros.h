#ifndef AUDIO_MACROS_H
#define AUDIO_MACROS_H


#define PUBLISHER_IN_CALL 1
#define VIEWER_IN_CALL 2
//#define VIEWER_NOT_IN_CALL 3 //never used so far
#define CALL_NOT_RUNNING 4

/*		*VERSION   */
#define AUDIO_CALL_VERSION				0

#define AUDIO_LIVE_STREAMING_VERSION	0

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_AUDIO_ENCODING_BUFFER_SIZE 5
#define MAX_AUDIO_ENCODING_FRAME_SIZE 3096

#define MAX_AUDIO_DECODER_BUFFER_SIZE 5
#define MAX_AUDIO_DECODER_FRAME_SIZE 3096

#else
#define MAX_AUDIO_ENCODING_BUFFER_SIZE 5
#define MAX_AUDIO_ENCODING_FRAME_SIZE 2048

#define MAX_AUDIO_DECODER_BUFFER_SIZE 30
#define MAX_AUDIO_DECODER_FRAME_SIZE 4096
#endif

#define AUDIO_MAX_BITRATE 24000
#define AUDIO_MIN_BITRATE 16000
#define AUDIO_LOW_BITRATE AUDIO_MIN_BITRATE
#define AUDIO_BITRATE_INIT 24000

#define MAX_AUDIO_PACKET_SIZE 508

#define AUDIO_SLOT_SIZE 30
#define AUDIO_BITRATE_UP_STEP 1000
#define AUDIO_MAX_NO_LOSS_SLOT 3
#define AUDIO_MAX_HUGE_LOSS_SLOT 60
#define AUDIO_MAX_TOLERABLE_ENCODING_TIME 40

#define AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING 800
#define AUDIO_FRAME_SAMPLE_SIZE_FOR_CALL 800
#define MAX_AUDIO_FRAME_SAMPLE_SIZE (AUDIO_FRAME_SAMPLE_SIZE_FOR_CALL > AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING ? AUDIO_FRAME_SAMPLE_SIZE_FOR_CALL : AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING)
#define CURRENT_AUDIO_FRAME_SAMPLE_SIZE(IS_LIVE_STREAM_RUNNING) (IS_LIVE_STREAM_RUNNING ? AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING : AUDIO_FRAME_SAMPLE_SIZE_FOR_CALL)

#define AUDIO_FRAME_SIZE 160
#define AUDIO_SAMPLE_RATE 8000
#define AUDIO_CHANNELS 1
#define AUDIO_APPLICATION OPUS_APPLICATION_VOIP

#define AUDIO_MAX_FRAME_SIZE 6*960
#define AUDIO_MAX_PACKET_SIZE (3*1276)

#define AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO 1
#define AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO 2
#define AUDIO_EVENT_FIRE_ENCODING_TIME 3
#define AUDIO_EVENT_FIRE_AVG_ENCODING_TIME 4

#define HALF_FRAME_DURATION_IN_MS 50


#define MINIMUM_AUDIO_HEADER_SIZE 20

#define STANDARD_CHUNK_DURATION 200

//#define USE_ECHO2
#define __TIMESTAMP_MOD__ 100000
#define __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ 2
#define __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ 10
#define CONSECUTIVE_AUDIO_PACKET_DELY 25
#define OPUS_ENABLED

#endif