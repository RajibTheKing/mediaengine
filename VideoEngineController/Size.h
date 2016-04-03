//
// Created by Fahad-PC on 12/29/2015.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_SIZE_H
#define ANDROIDTESTCLIENTVE_FTEST_SIZE_H

#define FIRST_BUILD_COMPATIBLE
#define FPS_CHANGE_SIGNALING
//#define RETRANSMISSION_ENABLED
#define BITRATE_ENABLED

///Packet Types
#define NORMAL_PACKET 1
#define MINI_PACKET 2
#define RETRANSMITTED_PACKET 3

#define FRAME_RATE 15
#define I_INTRA_PERIOD 8
#define BITRATE_DECREMENT_FACTOR 0.8
#define NORMAL_BITRATE_RATIO_IN_MEGA_SLOT 90
#define GOOD_BITRATE_RATIO_IN_MEGA_SLOT 95
#define GOOD_MEGASLOT_TO_UP 10
#define GOOD_MEGASLOT_TO_UP_TOLERANCE 0.9
#define GOOD_MEGASLOT_TO_UP_SAFE 150
#define GOOD_MEGASLOT_TO_UP_LIMIT_TO_BITRATE_JUMP 6

//#define BITRATE_INCREAMENT_FACTOR 1.2
#define BITRATE_INCREAMENT_DIFFERENCE 25000
#define BITRATE_MAX 1000000
#define BITRATE_MIN 75000
#define BITRATE_MIN_FOR_2G 25000
#define BITRATE_MID ((BITRATE_MAX + BITRATE_MIN)/2)
#define BITRATE_BEGIN 250000
#define MAX_BITRATE_TOLERANCE 100000
#define MAX_BITRATE_MULTIPLICATION_FACTOR 1.25
#define BITRATE_LOW 150000
#define BITRATE_AVERAGE_TIME 60
#define STOP_NOTIFICATION_SENDING_COUNTER 3
#define MAX_MINIPACKET_WAIT_TIME 2000
#define MAX_1ST_MINIPACKET_WAIT_TIME 500

#define BITRATE_CHANGE_NO 0
#define BITRATE_CHANGE_UP 1
#define BITRATE_CHANGE_UP_JUMP 2
#define BITRATE_CHANGE_DOWN -1




#define MAX_VIDEO_DECODER_BUFFER_SIZE 30
#define MAX_VIDEO_DECODER_FRAME_SIZE 352 * 288 * 3

#define MAX_VIDEO_ENCODER_BUFFER_SIZE 45 
#ifdef _DESKTOP_C_SHARP_ 
#define MAX_VIDEO_ENCODER_FRAME_SIZE (352 * 288 * 3) +1 
#else 
#define MAX_VIDEO_ENCODER_FRAME_SIZE (352 * 288 * 3)/2+1 
#endif

#define MAX_VIDEO_RENDERER_BUFFER_SIZE 30
#define MAX_VIDEO_RENDERER_FRAME_SIZE 352 * 288 * 3


#define MEGA_SLOT_INTERVAL 1

#define BITRATE_CONTROL_BASED_ON_BANDWIDTH 1


#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8





#define MAX_PACKET_NUMBER 200

#define NORMAL_PACKET_TYPE 1
#define MINI_PACKET_TYPE 2
#define RETRANSMITTED_PACKET_TYPE 3

#define BIT_INDEX_RETRANS_PACKET	7
#define BIT_INDEX_MINI_PACKET	6

#define VIDEO_VERSION_CODE 1

#define MAX_VIDEO_PACKET_QUEUE_SIZE 1000
#define MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE 100
#define MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE 1000
#define MAX_VIDEO_PACKET_SENDING_PACKET_SIZE 1024
#define MAX_VIDEO_PACKET_SENDING_SLEEP_MS   5
#define MIN_VIDEO_PACKET_SENDING_SLEEP_MS   5
#define REQUIRED_BITRATE_FOR_UNIT_SLEEP     100000

#define MAX_VIDEO_PACKET_SIZE 508

#define RESENDING_BUFFER_SIZE 500

#define MAX_NUMBER_OF_PACKETS 100

#define PACKET_HEADER_LENGTH 14
#define PACKET_HEADER_LENGTH_NO_VERSION 20

#define MAX_PACKET_SIZE_WITHOUT_HEADER 		(MAX_VIDEO_PACKET_SIZE - PACKET_HEADER_LENGTH - 1)
#define MAX_PACKET_SIZE_WITHOUT_HEADER_NO_VERSION 	487



#define SIGNAL_BYTE_INDEX_WITHOUT_MEDIA 0
#define RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA 4
#define VERSION_BYTE_INDEX 5



#define PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE (PACKET_HEADER_LENGTH + 1)
#define BYTE_SIZE 8


//#define MINI_PACKET_LENGTH 12
//#define MINI_PACKET_LENGTH_WITH_MEDIA_TYPE (MINI_PACKET_LENGTH + 1)

#define VIDEO_PACKET_MEDIA_TYPE 39
#define AUDIO_PACKET_MEDIA_TYPE 0

//#define ENCODER_KEY_FRAME_RATE  8
//#define MAX_BLOCK_RETRANSMISSION 4

#define FPS_MAXIMUM     15
#define FPS_MINIMUM     8
#define FPS_BEGINNING   15

#define BITRATE_BEGINNING   12
#define BITRATE_MAXIMUM     25
#define BITRATE_MINIMUM     1

#define FPS_COMPARISON_EPS 0.5
#define MAX_DIFF_TO_DROP_FPS 10
#define TIME_DELAY_FOR_RETRANSMISSION_IN_MS 300

#define FPS_SIGNAL_DROP 1
#define FPS_SIGNAL_UNCHANGED 0
#define FPS_SIGNAL_INC 2
#define MAX_CONSECUTIVE_UNCHANGED_COUNTER 150

//#ifdef RETRANSMISSION_ENABLED
//	#define DEPACKETIZATION_BUFFER_SIZE 30
//#else
//	#define DEPACKETIZATION_BUFFER_SIZE 15
//#endif

#define DEPACKETIZATION_BUFFER_SIZE 50

#define RESEND_QUEUE_MAX_SIZE 300
#define EXPECTED_FRAME_PACKET_QUEUE_SIZE 5

#define SENDING_INTERVAL_FOR_15_FPS 5
#define MAX_FPS FPS_MAXIMUM

#define RESEND_INFO_SIZE 8
#define RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE 17
#define SIZE_OF_INT_MINUS_8 24

//#define FPS_CHANGE_INTERVAL 500
//#define MAXIMUM_TOLERABLE_FRAME_DROP 1

#define NUMBER_OF_WAIT_SLOT_TO_DETECT_UP_FAIL 5
#define NUMBER_OF_SPIRAL_LIMIT 4

#define INVALID_PACKET_NUMBER 200
#define INVALID_PACKET_NUMBER_FOR_NETWORK_TYPE 201

#define NETWORK_TYPE_2G 1
#define NETWORK_TYPE_NOT_2G 0


#define CRASH_CHECK
#define PACKET_SEND_STATISTICS_ENABLED
#define FRAME_USAGE_STATISTICS_ENABLED
//#define SEND_VIDEO_TO_SELF
//#define DUMP_DECODED_AUDIO

//#define BANDWIDTH_CONTROLLING_TEST

#endif //ANDROIDTESTCLIENTVE_FTEST_SIZE_H
