//
// Created by Fahad-PC on 12/29/2015.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_SIZE_H
#define ANDROIDTESTCLIENTVE_FTEST_SIZE_H

/*LIVERY MOODS*/

#define FPS_CHANGE_SIGNALING

#define __MEDIA_DATA_SIZE_IN_LIVE_PACKET__ 512
#define NUMBER_OF_HEADER_FOR_STREAMING 3

///Packet Types

#define NORMAL_PACKET 1
#define MINI_PACKET 2

#define NEW_HEADER_FORMAT


#define IFRAME_INTERVAL 5


#define __SKIPPED_PACKET_TYPE 0
#define __VIDEO_PACKET_TYPE 1
#define __NEGOTIATION_PACKET_TYPE 2
#define __BITRATE_CONTROLL_PACKET_TYPE 3
#define __NETWORK_INFO_PACKET_TYPE 4

#define __MIN_PACKET_TYPE   1
#define __MAX_PACKET_TYPE   4

#define HIGH_QUALITY_FPS 30
#define LOW_QUALITY_FPS 15

#define LOW_FRAME_RATE 25
#define HIGH_FRAME_RATE 40

#define BLANK_DATA_MOOD true
#define VIDEO_DATA_MOOD false

#define LEAST_MEMORY_OF_STRONG_DEVICE 10

#define FRAME_RATE FPS_MAXIMUM
#define I_INTRA_PERIOD FPS_MAXIMUM / 2 +1
#define BITRATE_DECREMENT_FACTOR 0.8
#define NORMAL_BITRATE_RATIO_IN_MEGA_SLOT 90
#define GOOD_BITRATE_RATIO_IN_MEGA_SLOT 95
#define GOOD_MEGASLOT_TO_UP 10
#define GOOD_MEGASLOT_TO_UP_TOLERANCE 0.9
#define GOOD_MEGASLOT_TO_UP_SAFE 150
#define GOOD_MEGASLOT_TO_UP_LIMIT_TO_BITRATE_JUMP 6

#define SOUL_SELF_DEVICE_CHECK

//#define BITRATE_INCREAMENT_FACTOR 1.2
#define BITRATE_INCREAMENT_DIFFERENCE 25000


#if defined(TARGET_OS_WINDOWS_PHONE)
#define BITRATE_MAX 500000
#else
#define BITRATE_MAX 1000000
#endif

#define BITRATE_BEGIN_FOR_STREAM 380000

#define BITRATE_MIN 75000
#define BITRATE_MIN_FOR_2G 25000
#define BITRATE_BEGIN_FOR_2G 25000
#define BITRATE_MAX_FOR_2G 50000
#define BITRATE_MID ((BITRATE_MAX + BITRATE_MIN)/2)

#if defined(TARGET_OS_WINDOWS_PHONE)
#define BITRATE_BEGIN 200000
#else
#define BITRATE_BEGIN BITRATE_MAX
#endif
#define BITRATE_CHECK_CAPABILITY 5000000
#define MAX_BITRATE_TOLERANCE 100000
#define MAX_BITRATE_MULTIPLICATION_FACTOR 1.25
#define BITRATE_LOW 150000
#define BITRATE_AVERAGE_TIME 60
#define STOP_NOTIFICATION_SENDING_COUNTER 3
#define MAX_MINIPACKET_WAIT_TIME 2000
#define MAX_1ST_MINIPACKET_WAIT_TIME 500

#define VIDEO_START_WITHOUT_VERSION_TIMEOUT_COUNTER 150

#define BITRATE_CHANGE_NO 0
#define BITRATE_CHANGE_UP 1
#define BITRATE_CHANGE_UP_JUMP 2
#define BITRATE_CHANGE_DOWN -1

#define VIDEO_CALL_TYPE_UNKNOWN -1
#define VIDEO_CALL_TYPE_UNCHECKED 0
#define VIDEO_CALL_TYPE_352_15FPS 1
#define VIDEO_CALL_TYPE_352_25FPS 2
#define VIDEO_CALL_TYPE_640_25FPS 3

#define STATUS_UNCHECKED 0
#define STATUS_ABLE 1
#define STATUS_UNABLE 2

#define START_DEVICE_CHECK 1
#define STOP_DEVICE_CHECK 2

#define __LIVE_FIRST_FRAME_SLEEP_TIME__ 300

#define DEVICE_ABILITY_CHECK_MOOD true
#define LIVE_CALL_MOOD false

#define DEVICE_CHECK_STARTING 1
#define DEVICE_CHECK_SUCCESS 2
#define DEVICE_CHECK_FAILED 3

#define RESOLUTION_FPS_SUPPORT_NOT_TESTED 0
#define SUPPORTED_RESOLUTION_FPS_352_15 1
#define SUPPORTED_RESOLUTION_FPS_352_25 2
#define SUPPORTED_RESOLUTION_FPS_640_25 3

#if defined(TARGET_OS_WINDOWS_PHONE)
#define HEIGHT 288
#define WIDTH 352
#else
#define HEIGHT 480
#define WIDTH 640
#endif

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_VIDEO_DECODER_BUFFER_SIZE 5
#else
#define MAX_VIDEO_DECODER_BUFFER_SIZE 30
#endif

#if defined SOUL_SELF_DEVICE_CHECK

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_VIDEO_ENCODER_BUFFER_SIZE 5
#else
#define MAX_VIDEO_ENCODER_BUFFER_SIZE 30
#endif

#endif

#ifdef _DESKTOP_C_SHARP_ 
#define MAX_VIDEO_ENCODER_FRAME_SIZE (WIDTH * HEIGHT * 3) +1
#define MAX_VIDEO_DECODER_FRAME_SIZE (WIDTH * HEIGHT * 3) +1
#define MAX_VIDEO_RENDERER_FRAME_SIZE (WIDTH * HEIGHT * 3) +1
#else 
#define MAX_VIDEO_ENCODER_FRAME_SIZE (WIDTH * HEIGHT * 3)/2+1
#define MAX_VIDEO_DECODER_FRAME_SIZE (WIDTH * HEIGHT * 3)/2+1
#define MAX_VIDEO_RENDERER_FRAME_SIZE (WIDTH * HEIGHT * 3)/2+1
#endif

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_VIDEO_RENDERER_BUFFER_SIZE 2
#else
#define MAX_VIDEO_RENDERER_BUFFER_SIZE 5
#endif

#define MAX_AUDIO_FRAME_LENGHT 4096


#define MEGA_SLOT_INTERVAL 1

#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8

#define LIVE_MEDIA_UNIT_VERSION_BLOCK_POSITION 0
#define LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_POSITION 1
#define LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_POSITION 5
#define LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_POSITION 8
#define LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION 11
#define LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_SIZE 4
#define LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE 3
#define LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE 3
#define LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE 1
#define LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE 1

#define MAX_PACKET_NUMBER 200

#define NORMAL_PACKET_TYPE 1
#define MINI_PACKET_TYPE 2
#define RETRANSMITTED_PACKET_TYPE 3

#define VIDEO_VERSION_CODE 0

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_VIDEO_PACKET_QUEUE_SIZE 500
#define MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE 1
#define MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE 200
#define MAX_VIDEO_PACKET_SENDING_PACKET_SIZE 550
#else
#define MAX_VIDEO_PACKET_QUEUE_SIZE 5000
#define MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE 100




#define MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE 100
#define MAX_VIDEO_PACKET_SENDING_PACKET_SIZE 50000 + PACKET_HEADER_LENGTH + 20
#define MAX_VIDEO_DATA_TO_SEND_SIZE (10000 + PACKET_HEADER_LENGTH + 20)*10
#define MAX_AUDIO_DATA_TO_SEND_SIZE 30000
#define MAX_AUDIO_VIDEO_DATA_TO_SEND_SIZE MAX_AUDIO_DATA_TO_SEND_SIZE + MAX_VIDEO_DATA_TO_SEND_SIZE

#endif

#define MAX_VIDEO_PACKET_SENDING_SLEEP_MS   5
#define MIN_VIDEO_PACKET_SENDING_SLEEP_MS   1
#define REQUIRED_BITRATE_FOR_UNIT_SLEEP     100000

#define MAX_VIDEO_PACKET_SIZE 508

#if defined(TARGET_OS_WINDOWS_PHONE)
#define RESENDING_BUFFER_SIZE 1
#else
#define RESENDING_BUFFER_SIZE 500
#endif

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_NUMBER_OF_PACKETS 30
#else
#define MAX_NUMBER_OF_PACKETS 500
#endif

#define PACKET_HEADER_LENGTH 15

#define MAX_PACKET_SIZE_WITHOUT_HEADER 		(MAX_VIDEO_PACKET_SIZE - PACKET_HEADER_LENGTH - 1)



#define __PACKET_TYPE_INDEX 0
#define SIGNAL_BYTE_INDEX_WITHOUT_MEDIA 1
#define CALL_INFO_BYTE_INDEX_WITHOUT_MEDIA 5
#define VERSION_BYTE_INDEX 6



#define PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE (PACKET_HEADER_LENGTH + 1)
#define BYTE_SIZE 8


#define VIDEO_PACKET_MEDIA_TYPE 39
#define AUDIO_PACKET_MEDIA_TYPE 0

//#define ENCODER_KEY_FRAME_RATE  8
//#define MAX_BLOCK_RETRANSMISSION 4

#define FPS_MAXIMUM     HIGH_QUALITY_FPS
#define FPS_MINIMUM     8

#define FPS_COMPARISON_EPS 0.5
#define MAX_DIFF_TO_DROP_FPS 10
#define TIME_DELAY_FOR_RETRANSMISSION_IN_MS 300

#define FPS_SIGNAL_DROP 1
#define FPS_SIGNAL_UNCHANGED 0
#define FPS_SIGNAL_INC 2
#define MAX_CONSECUTIVE_UNCHANGED_COUNTER 300


#define DEPACKETIZATION_BUFFER_SIZE 300

#define RESEND_QUEUE_MAX_SIZE 300
#define EXPECTED_FRAME_PACKET_QUEUE_SIZE 5

#define SENDING_INTERVAL_FOR_15_FPS 5
#define MAX_FPS FPS_MAXIMUM

#define DEVICE_FPS_MAXIMUM 30

#define RESEND_INFO_SIZE 8
#define RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE 17
#define SIZE_OF_INT_MINUS_8 24

//#define FPS_CHANGE_INTERVAL 500
//#define MAXIMUM_TOLERABLE_FRAME_DROP 1

#define NUMBER_OF_WAIT_SLOT_TO_DETECT_UP_FAIL 5
#define NUMBER_OF_SPIRAL_LIMIT 4

#define NETWORK_TYPE_2G 1
#define NETWORK_TYPE_NOT_2G 0
#define NETWORK_TYPE_UNKNOWN -1


#define CRASH_CHECK
#define PACKET_SEND_STATISTICS_ENABLED
#define FRAME_USAGE_STATISTICS_ENABLED
//#define SEND_VIDEO_TO_SELF
//#define DUMP_DECODED_AUDIO

//#define BANDWIDTH_CONTROLLING_TEST

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_AUDIO_ENCODING_BUFFER_SIZE 5
#define MAX_AUDIO_ENCODING_FRAME_SIZE 3096

#define MAX_AUDIO_DECODER_BUFFER_SIZE 5
#define MAX_AUDIO_DECODER_FRAME_SIZE 3096

#else
#define MAX_AUDIO_ENCODING_BUFFER_SIZE 5
#define MAX_AUDIO_ENCODING_FRAME_SIZE 4096

#define MAX_AUDIO_DECODER_BUFFER_SIZE 5
#define MAX_AUDIO_DECODER_FRAME_SIZE 4096
#endif


#define FPS_TOLERANCE_FOR_HIGH_RESOLUTION 2
#define FPS_TOLERANCE_FOR_FPS 2

#define TIMEOUT_START_SENDING_VIDEO_DATA 5000


#define AUDIO_BITRATE_LIVE 24000

#define AUDIO_MAX_BITRATE 32000
#define AUDIO_MIN_BITRATE 6000
#define AUDIO_LOW_BITRATE AUDIO_MIN_BITRATE
#define AUDIO_BITRATE_INIT 16000



#define AUDIO_SLOT_SIZE 30
#define AUDIO_BITRATE_UP_STEP 1000
#define AUDIO_MAX_NO_LOSS_SLOT 3
#define AUDIO_MAX_HUGE_LOSS_SLOT 60
#define AUDIO_MAX_TOLERABLE_ENCODING_TIME 40

#define AUDIO_CLIENT_SAMPLE_SIZE 800

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


#define SERVICE_TYPE_CALL 11
#define SERVICE_TYPE_LIVE_STREAM 12
#define SERVICE_TYPE_SELF_CALL 13
#define SERVICE_TYPE_SELF_STREAM 14



#endif //ANDROIDTESTCLIENTVE_FTEST_SIZE_H
