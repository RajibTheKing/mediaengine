#ifndef _VIDEO_PACKET_DEPACKETIZER_H_
#define _VIDEO_PACKET_DEPACKETIZER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "VideoPacketBuffer.h"
#include "AudioVideoEngineDefinitions.h"
#include "Size.h"
#include "Tools.h"
#include "PacketHeader.h"


namespace IPV
{
	class thread;
}

class CCommonElementsBucket;
class CVideoCallSession;

class CEncodedFrameDepacketizer
{

public:

	CEncodedFrameDepacketizer(CCommonElementsBucket* sharedObject, CVideoCallSession *pVideoCallSession);
	~CEncodedFrameDepacketizer();

	int Depacketize(unsigned char *in_data, unsigned int in_size, int PacketType, CPacketHeader &packetHeader);
	void MoveForward(int frame);
	int CreateNewIndex(int frame);
	void ClearFrame(int index, int frame);
	int GetReceivedFrame(unsigned char* data,int &nFramNumber,int &nEcodingTime,int nExpectedTime,int nRight);

	map<int, int> m_mFrameTimeStamp;

	Tools m_Tools;

private:
	int ProcessFrame(unsigned char *data,int index,int frameNumber,int &nFramNumber);
	int GetEncodingTime(int nFrameNumber);
	bool m_bIsDpkgBufferFilledUp;
	int m_iFirstFrameReceived;

	int SafeFinder(int Data);

	int m_iMaxFrameNumRecvd;
	long long m_FirstFrameEncodingTime;

	std::map<int, int> m_FrameTracker;
	int m_FrontFrame;
	int m_BackFrame;
	int m_Counter;
	int m_BufferSize;
	std::set<int> m_AvailableIndexes;

	CCommonElementsBucket* m_pCommonElementsBucket;
	CVideoCallSession* m_VideoCallSession;

	CVideoPacketBuffer m_CVideoPacketBuffer[DEPACKETIZATION_BUFFER_SIZE + 1];

	CPacketHeader PacketHeader;

	unsigned char * m_pPacketToResend;

protected:

	SmartPointer<CLockHandler> m_pEncodedFrameDepacketizerMutex;

};

#endif