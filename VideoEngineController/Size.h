//
// Created by Fahad-PC on 12/29/2015.
//

#ifndef ANDROIDTESTCLIENTVE_FTEST_SIZE_H
#define ANDROIDTESTCLIENTVE_FTEST_SIZE_H

#define BITRATE_MAX 1000 * 5000
#define FRAME_RATE 30
#define I_INTRA_PERIOD 8

#define FPS_CHANGE_SIGNALING
#define RETRANSMISSION_ENABLED

#define VIDEO_VERSION_CODE 0

#define MAX_VIDEO_PACKET_QUEUE_SIZE 1000
#define MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE 100
#define MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE 1000
#define MAX_VIDEO_PACKET_SENDING_PACKET_SIZE 1024


#define MAX_VIDEO_PACKET_SIZE 508

#define RESENDING_BUFFER_SIZE 500

#define MAX_NUMBER_OF_PACKETS 100
#define MAX_PACKET_SIZE_WITHOUT_HEADER 494
#define PACKET_HEADER_LENGTH 14

#define PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE (PACKET_HEADER_LENGTH + 1)
#define BYTE_SIZE 8


#define MINI_PACKET_LENGTH 12
#define MINI_PACKET_LENGTH_WITH_MEDIA_TYPE (MINI_PACKET_LENGTH + 1)

#define VIDEO_PACKET_MEDIA_TYPE 39
#define AUDIO_PACKET_MEDIA_TYPE 0

#define ENCODER_KEY_FRAME_RATE  8
#define MAX_BLOCK_RETRANSMISSION 4

#define FPS_BEGINNING   12
#define FPS_MAXIMUM     15
#define FPS_MINIMUM     8
#define FPS_COMPARISON_EPS 0.5
#define MAX_DIFF_TO_DROP_FPS 10
#define TIME_DELAY_FOR_RETRANSMISSION 1.5

#ifdef RETRANSMISSION_ENABLED
	#define DEPACKETIZATION_BUFFER_SIZE 30
#else
	#define DEPACKETIZATION_BUFFER_SIZE 15
#endif


#define SIGNAL_BYTE_INDEX_WITHOUT_MEDIA 10
#define RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA 11

#define RESEND_QUEUE_MAX_SIZE 300
#define EXPECTED_FRAME_PACKET_QUEUE_SIZE 5

#define SENDING_INTERVAL_FOR_15_FPS 5
#define MAX_FPS FPS_MAXIMUM

#define RESEND_INFO_SIZE 8
#define RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE 17
#define SIZE_OF_INT_MINUS_8 24


#define CRASH_CHECK
#define PACKET_SEND_STATISTICS_ENABLED
//#define DUMP_DECODED_AUDIO

#endif //ANDROIDTESTCLIENTVE_FTEST_SIZE_H
