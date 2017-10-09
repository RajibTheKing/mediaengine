//
// Created by Fahad-PC on 12/29/2015.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_SIZE_H
#define ANDROIDTESTCLIENTVE_FTEST_SIZE_H

/*LIVERY MOODS*/

#define FPS_CHANGE_SIGNALING

#define MEDIA_DATA_SIZE_IN_LIVE_PACKET 512
#define NUMBER_OF_HEADER_FOR_STREAMING 3

///Packet Types

#define NORMAL_PACKET 1
#define MINI_PACKET 2

#define CHUNK_DELAY_TOLERANCE 100

#define MAXIMUM_LUMINANCE_VALUE 255

#define IFRAME_INTERVAL 15

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_FRAME_HEIGHT 352
#define MAX_FRAME_WIDTH 288
#else
#define MAX_FRAME_HEIGHT 640
#define MAX_FRAME_WIDTH 480
#endif

#define SKIPPED_PACKET_TYPE 0
#define VIDEO_PACKET_TYPE 1
#define NEGOTIATION_PACKET_TYPE 2
#define BITRATE_CONTROLL_PACKET_TYPE 3
#define NETWORK_INFO_PACKET_TYPE 4
#define IDR_FRAME_CONTROL_INFO_TYPE 5

#define MIN_PACKET_TYPE   1
#define MAX_PACKET_TYPE   5

#define HIGH_QUALITY_FPS 30
#define LOW_QUALITY_FPS 15

#define LOW_FRAME_RATE 25
#define HIGH_FRAME_RATE 45

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

#define MEDIA_CHUNK_TIME_SLOT 200

//#define BITRATE_INCREAMENT_FACTOR 1.2
#define BITRATE_INCREAMENT_DIFFERENCE 25000
#define BITRATE_JUMP_DIFFERENCE 200000

#if defined(TARGET_OS_WINDOWS_PHONE)
#define BITRATE_MAX 500000
#else
#define BITRATE_MAX 700000
#endif

#define LIVE_START_HEIGHEST_WAITTIME 5000
#define CALL_START_HIGHEST_WAITTIME 500

#define BITRATE_BEGIN_FOR_STREAM 580000

#define BITRATE_FOR_MEDIUM_STREAM 480000
#define BITRATE_FOR_LOW_STREAM 380000

#define BITRATE_FOR_INSET_STREAM 210000

#define BITRATE_MIN  300000
#define BITRATE_MIN_FOR_2G 25000
#define BITRATE_BEGIN_FOR_2G 25000
#define BITRATE_MAX_FOR_2G 50000
#define BITRATE_MID ((BITRATE_MAX + BITRATE_MIN)/2)

#if defined(TARGET_OS_WINDOWS_PHONE)
#define BITRATE_BEGIN 200000
#else
#define BITRATE_BEGIN (BITRATE_MAX/2)
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

#define NETWORK_TYPE_2G 1
#define NETWORK_TYPE_NOT_2G 0
#define NETWORK_TYPE_UNKNOWN -1

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

#define OLD_SENDING_THREAD
#define OLD_ENCODING_THREAD
#define OLD_DECODING_THREAD
#define OLD_RENDERING_THREAD

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

#ifdef DESKTOP_C_SHARP 
#define ULTRA_MAX_VIDEO_DECODER_FRAME_SIZE 720 * 720 * 3
#else 
#define ULTRA_MAX_VIDEO_DECODER_FRAME_SIZE 720 * 720 * 3 / 2
#endif

#ifdef DESKTOP_C_SHARP 
#define MAX_VIDEO_ENCODER_FRAME_SIZE (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3) +1
#define MAX_VIDEO_DECODER_FRAME_SIZE (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3) +1
#define MAX_VIDEO_RENDERER_FRAME_SIZE (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3) +1
#else 
#define MAX_VIDEO_ENCODER_FRAME_SIZE (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3)/2+1
#define MAX_VIDEO_DECODER_FRAME_SIZE (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3)/2+1
#define MAX_VIDEO_RENDERER_FRAME_SIZE (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3)/2+1
#endif

#if defined(TARGET_OS_WINDOWS_PHONE)
#define MAX_VIDEO_RENDERER_BUFFER_SIZE 2
#else
#define MAX_VIDEO_RENDERER_BUFFER_SIZE 5
#endif

#define LIVE_HEADER_VERSION 3

#define STREAM_TYPE_LIVE_STREAM 11
#define STREAM_TYPE_CHANNEL 12

#define AUDIO_MAX_FRAME_LENGTH_IN_BYTE 4096

#define MEGA_SLOT_INTERVAL 1

#define MIN_BITRATE_FOR_SHARPING 700000

#define LIVE_MEDIA_UNIT_VERSION_BLOCK_POSITION 0
#define LIVE_MEDIA_UNIT_HEADER_LENGTH_BLOCK_POSITION 1
#define LIVE_MEDIA_UNIT_CHUNK_DURATION_BLOCK_POSITION 3
#define LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_POSITION 5
#define LIVE_MEDIA_UNIT_STREAM_TYPE_BLOCK_POSITION 10
#define LIVE_MEDIA_UNIT_BLOCK_INFO_POSITION_BLOCK_POSITION 11
#define LIVE_MEDIA_UNIT_AUDIO_STARTING_POSITION_BLOCK_POSITION 12
#define LIVE_MEDIA_UNIT_VIDEO_STARTING_POSITION_BLOCK_POSITION 15
#define LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_POSITION 18
#define LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_POSITION 21
#define LIVE_MEDIA_UNIT_CHUNK_NUMBER_BLOCK_POSITION	24
#define LIVE_MEDIA_UNIT_SERVICE_TYPE_BLOCK_POSITION	27
#define LIVE_MEDIA_UNIT_ENTITY_TYPE_BLOCK_POSITION	28

#define LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION 29

#define LIVE_MEDIA_UNIT_HEADER_LENGTH_BLOCK_SIZE 2
#define LIVE_MEDIA_UNIT_CHUNK_DURATION_BLOCK_SIZE 2
#define LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_SIZE 5
#define LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE 3
#define LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE 3
#define LIVE_MEDIA_UNIT_AUDIO_STARTING_POSITION_BLOCK_SIZE 3
#define LIVE_MEDIA_UNIT_VIDEO_STARTING_POSITION_BLOCK_SIZE 3
#define LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE 1
#define LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE 1
#define LIVE_MEDIA_UNIT_CHUNK_NUMBER_BLOCK_SIZE 3
#define LIVE_MEDIA_UNIT_SERVICE_TYPE_BLOCK_SIZE 1
#define LIVE_MEDIA_UNIT_ENTITY_TYPE_BLOCK_SIZE 1

#define CAMARA_VIDEO_DATA 51
#define SCREEN_VIDEO_DATA 52

#define SERVICE_TYPE_CALL 11
#define SERVICE_TYPE_LIVE_STREAM 12

#define ENTITY_TYPE_CALLER 31

#define SERVICE_TYPE_SELF_CALL 13
#define SERVICE_TYPE_SELF_STREAM 14

#define SERVICE_TYPE_CHANNEL 16

#define ENTITY_TYPE_PUBLISHER 31
#define ENTITY_TYPE_VIEWER 32
#define ENTITY_TYPE_VIEWER_CALLEE 2
#define ENTITY_TYPE_PUBLISHER_CALLER 1

#define MAX_PACKET_NUMBER 200

#define NORMAL_PACKET_TYPE 1
#define MINI_PACKET_TYPE 2
#define RETRANSMITTED_PACKET_TYPE 3

#define VIDEO_VERSION_CODE 3

#define DEVICE_TYPE_CHECK_START_VERSION 1

#define DEVICE_TYPE_UNKNOWN -1
#define DEVICE_TYPE_DESKTOP 1
#define DEVICE_TYPE_IOS 2
#define DEVICE_TYPE_ANDROID 3
#define DEVICE_TYPE_WINDOWS_PHONE 4

#define MAX_VIDEO_PACKET_SENDING_SLEEP_MS   5
#define MIN_VIDEO_PACKET_SENDING_SLEEP_MS   1
#define REQUIRED_BITRATE_FOR_UNIT_SLEEP     100000

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

#define PACKET_HEADER_LENGTH 22
#define MAX_VIDEO_PACKET_SIZE 1000
#define MAX_PACKET_SIZE_WITHOUT_HEADER 		(MAX_VIDEO_PACKET_SIZE - PACKET_HEADER_LENGTH - 1)

#if defined(TARGET_OS_WINDOWS_PHONE)

#define MAX_VIDEO_PACKET_QUEUE_SIZE 500
#define MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE 1
#define MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE 100
#define MAX_VIDEO_PACKET_SENDING_PACKET_SIZE 50000 + PACKET_HEADER_LENGTH + 20

#define MAX_VIDEO_DATA_TO_SEND_SIZE (10000 + PACKET_HEADER_LENGTH + 20)*10
#define MAX_AUDIO_DATA_TO_SEND_SIZE 30000
#define MAX_AUDIO_VIDEO_DATA_TO_SEND_SIZE MAX_AUDIO_DATA_TO_SEND_SIZE + MAX_VIDEO_DATA_TO_SEND_SIZE

#else

#define MAX_VIDEO_PACKET_QUEUE_SIZE 5000
#define MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE 100

#define MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE 100

#if defined(DESKTOP_C_SHARP)
	#define MAX_VIDEO_PACKET_SENDING_PACKET_SIZE 500000 + PACKET_HEADER_LENGTH + 20
	#define MAX_VIDEO_DATA_TO_SEND_SIZE (500000 + PACKET_HEADER_LENGTH + 20)*10
#else
	#define MAX_VIDEO_PACKET_SENDING_PACKET_SIZE 50000 + PACKET_HEADER_LENGTH + 20
	#define MAX_VIDEO_DATA_TO_SEND_SIZE (50000 + PACKET_HEADER_LENGTH + 20)*10
#endif

#define MAX_AUDIO_DATA_TO_SEND_SIZE 30000
#define MAX_AUDIO_VIDEO_DATA_TO_SEND_SIZE MAX_AUDIO_DATA_TO_SEND_SIZE + MAX_VIDEO_DATA_TO_SEND_SIZE

#endif


#define PACKET_TYPE_INDEX 0
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

#define CRASH_CHECK
#define PACKET_SEND_STATISTICS_ENABLED
#define FRAME_USAGE_STATISTICS_ENABLED
//#define SEND_VIDEO_TO_SELF
//#define DUMP_DECODED_AUDIO

//#define BANDWIDTH_CONTROLLING_TEST

#define FPS_TOLERANCE_FOR_HIGH_RESOLUTION 2
#define FPS_TOLERANCE_FOR_FPS 2

#define TIMEOUT_START_SENDING_VIDEO_DATA 5000

#define ORIENTATION_SCREEN 11
#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8

#define ORIENTATION_BACK_270_MIRRORED 9

//#define CALL_IN_LIVE_INSET_LOWER_PADDING 24

#if defined(DESKTOP_C_SHARP)
	#define BRIGHTNESS_SCALE 20
#else
	#define BRIGHTNESS_SCALE 10
#endif



#endif //ANDROIDTESTCLIENTVE_FTEST_SIZE_H
