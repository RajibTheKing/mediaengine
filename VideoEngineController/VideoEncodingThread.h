
#ifndef _VIDEO_ENCODING_THREAD_H_
#define _VIDEO_ENCODING_THREAD_H_

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "EncodingBuffer.h"
#include "BitRateController.h"
#include "ColorConverter.h"
#include "EncodedFrameDepacketizer.h"

#include <thread>

class CVideoEncodingThread
{

public:

	CVideoEncodingThread(LongLong friendID, CEncodingBuffer *encodingBuffer, BitRateController *bitRateController, CColorConverter *colorConverter, CVideoEncoder *videoEncoder, CEncodedFramePacketizer *encodedFramePacketizer);
	~CVideoEncodingThread();

	void StartEncodingThread();
	void StopEncodingThread();
	void EncodingThreadProcedure();
	static void *CreateVideoEncodingThread(void* param);

	int orientation_type;

private:

	CEncodingBuffer *m_EncodingBuffer;						// bring
	BitRateController *m_BitRateController;					// bring

	unsigned char m_EncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_ConvertedEncodingFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];
	unsigned char m_EncodedFrame[MAX_VIDEO_ENCODER_FRAME_SIZE];

	CColorConverter *m_pColorConverter;						// bring
	CVideoEncoder *m_pVideoEncoder;							// bring
	CEncodedFramePacketizer *m_pEncodedFramePacketizer;		// bring

	int m_iFrameNumber;
	LongLong m_FriendID;									// bring

	bool bEncodingThreadRunning;
	bool bEncodingThreadClosed;

	Tools m_Tools;

	SmartPointer<std::thread> pEncodingThread;
};

#endif 