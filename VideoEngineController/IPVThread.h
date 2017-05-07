
#ifndef IPV_THREAD_H
#define IPV_THREAD_H

#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include "RenderingBuffer.h"
#include "AverageCalculator.h"

#include <thread>

class CCommonElementsBucket;
class CVideoCallSession;

class CIPVThread
{

public:

	CIPVThread(long long llFriendID, CRenderingBuffer *pcRenderingBuffer, CCommonElementsBucket* pcCommonElementsBucket, CVideoCallSession *pcVideoCallSession, bool bIsCheckCall);
	~CIPVThread();

	void StartThread();
	void StopThread();
	void ThreadRunProcedure();
	static void *CreateThread(void* pParam);

private:

	bool m_bThreadRunning;
	bool m_bThreadClosed;

	bool m_bIsCheckCall;
	int m_nRenderFrameCount;
	int m_nInsetHeight;
	int m_nInsetWidth;
	long long m_llFriendID;
	long long m_lRenderCallTime;
	long long m_llRenderFrameCounter;

	unsigned char m_ucaRenderingFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

	Tools m_Tools;
	CAverageCalculator m_cRenderTimeCalculator;

	CRenderingBuffer *m_pcRenderingBuffer;
	CCommonElementsBucket* m_pcCommonElementsBucket;
    CVideoCallSession *m_pcVideoCallSession;

	SmartPointer<std::thread> pThreadPointer;
};

#endif 