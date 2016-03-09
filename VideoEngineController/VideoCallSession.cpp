#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "Globals.h"
#include "ResendingBuffer.h"
//#include "Helper_IOS.h"

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
map<int, int> g_TraceRetransmittedFrame;
#endif



#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif
//#include <android/log.h>

//#define LOG_TAG "NewTest"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//int FPS=0;
//int fpsCnt=0;

deque<pair<int, int>> ExpectedFramePacketDeQueue;
extern long long g_FriendID;
extern CFPSController g_FPSController;


//int countFrame = 0;
//int countFrameFor15 = 0;
//int countFrameSize = 0;
//long long encodeTimeStampFor15;
//int g_iPacketCounterSinceNotifying = FPS_SIGNAL_IDLE_FOR_PACKETS;
//bool gbStopFPSSending = false;

#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8

extern bool g_bIsVersionDetectableOpponent;
extern unsigned char g_uchSendPacketVersion;
extern int g_uchOpponentVersion;
extern CResendingBuffer g_ResendBuffer;

int g_OppNotifiedByterate = 0;

//extern int g_MY_FPS;

CVideoCallSession *g_VideoCallSession;

CVideoCallSession::CVideoCallSession(LongLong fname, CCommonElementsBucket* sharedObject) :

m_pCommonElementsBucket(sharedObject),
m_iFrameNumber(0),
m_ClientFPS(FPS_BEGINNING),
m_ClientFPSDiffSum(0),
m_ClientFrameCounter(0),
m_EncodingFrameCounter(0),
m_ll1stFrameTimeStamp(0),
m_bFirstFrame(true),
m_iTimeStampDiff(0),
m_b1stDecodedFrame(true),
m_ll1stDecodedFrameTimeStamp(0),
m_pEncodedFramePacketizer(NULL),
m_ByteRcvInBandSlot(0),
m_SlotResetLeftRange(GetUniquePacketID(0, 0)),
m_SlotResetRightRange(GetUniquePacketID(FRAME_RATE, 0)),
m_pVideoEncoder(NULL),
m_bSkipFirstByteCalculation(true),
m_bGotOppBandwidth(0),
m_ByteRcvInSlotInverval(0),
m_ByteSendInSlotInverval(0),
m_RecvMegaSlotInvervalCounter(0),
m_SendMegaSlotInervalCounter(0),
m_ByteSendInMegaSlotInverval(0),
m_ByteRecvInMegaSlotInterval(0),
m_SlotIntervalCounter(0),
m_bMegSlotCounterShouldStop(true),
m_bsetBitrateCalled(false),
m_iConsecutiveGoodMegaSlot(0),
m_iPreviousByterate(BITRATE_MAX / 8),
m_LastSendingSlot(0),
m_iDePacketizeCounter(0),
m_TimeFor100Depacketize(0)
{
	m_miniPacketBandCounter = 0;

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
	g_TraceRetransmittedFrame.clear();
#endif

#ifdef FIRST_BUILD_COMPATIBLE
	g_bIsVersionDetectableOpponent = false;
	g_uchSendPacketVersion = 0;
#else
	g_bIsVersionDetectableOpponent = true;
	g_uchSendPacketVersion = 1;
#endif

	//Resetting Global Variables.
	//	countFrame = 0;
	//	countFrameFor15 = 0;
	//	countFrameSize = 0;
	//	encodeTimeStampFor15 = 0;
//	g_iPacketCounterSinceNotifying = FPS_SIGNAL_IDLE_FOR_PACKETS;
	g_ResendBuffer.Reset();
	//gbStopFPSSending = false;

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
	g_TraceRetransmittedFrame.clear();
#endif



	fpsCnt = 0;
	g_FPSController.Reset();
	//	g_MY_FPS =
	opponentFPS = ownFPS = FPS_BEGINNING;
	m_iCountReQResPack = 0;

	//	FPS=10;

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::CVideoCallSession");
	m_pSessionMutex.reset(new CLockHandler);
	friendID = fname;
	sessionMediaList.ClearAllFromVideoEncoderList();

	m_pEncodedFrameDepacketizer = NULL;
	m_pEncodedFramePacketizer = new CEncodedFramePacketizer(sharedObject);
	m_pEncodedFrameDepacketizer = new CEncodedFrameDepacketizer(sharedObject, this);

	m_BitRateController = new BitRateController();
	m_EncodingBuffer = new CEncodingBuffer();
	m_RenderingBuffer = new CRenderingBuffer();

	m_pVideoPacketQueue = new CVideoPacketQueue();
	m_pRetransVideoPacketQueue = new CVideoPacketQueue();
	m_pMiniPacketQueue = new CVideoPacketQueue();

	g_FriendID = fname;

	ExpectedFramePacketPair.first = 0;
	ExpectedFramePacketPair.second = 0;
	iNumberOfPacketsInCurrentFrame = 0;

	g_VideoCallSession = this;

	m_FrameCounterbeforeEncoding = 0;

	m_iGoodSlotCounter = 0;
	m_iNormalSlotCounter = 0;
	m_SlotCounter = 0;

	m_BitRateController->SetSharedObject(sharedObject);

	m_BandWidthRatioHelper.clear();
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::CVideoCallSession created");
}

CVideoCallSession::~CVideoCallSession()
{

	StopDepacketizationThread();
	StopDecodingThread();
	StopEncodingThread();
	StopRenderingThread();

	if (NULL != m_pVideoEncodingThread)
	{
		delete m_pVideoEncodingThread;
		m_pVideoEncodingThread = NULL;
	}

	if (NULL != m_pVideoDepacketizationThread)
	{
		delete m_pVideoDepacketizationThread;
		m_pVideoDepacketizationThread = NULL;
	}

	if (NULL != m_pVideoDecodingThread)
	{
		delete m_pVideoDecodingThread;
		m_pVideoDecodingThread = NULL;
	}

	if (NULL != m_pVideoRenderingThread)
	{
		delete m_pVideoRenderingThread;
		m_pVideoRenderingThread = NULL;
	}

	if (NULL != m_BitRateController)
	{
		delete m_BitRateController;
		m_BitRateController = NULL;
	}

	if (NULL != m_pVideoPacketQueue)
	{
		delete m_pVideoPacketQueue;
		m_pVideoPacketQueue = NULL;
	}

	if (NULL != m_pRetransVideoPacketQueue)
	{
		delete m_pRetransVideoPacketQueue;
		m_pRetransVideoPacketQueue = NULL;
	}

	if (NULL != m_pMiniPacketQueue)
	{
		delete m_pMiniPacketQueue;
		m_pMiniPacketQueue = NULL;
	}

	if (NULL != m_RenderingBuffer)
	{
		delete m_RenderingBuffer;
		m_RenderingBuffer = NULL;
	}

	if (NULL != m_EncodingBuffer)
	{
		delete m_EncodingBuffer;
		m_EncodingBuffer = NULL;
	}

	if (NULL != m_pVideoEncoder)
	{
		delete m_pVideoEncoder;
		m_pVideoEncoder = NULL;
	}

	if (NULL != m_pEncodedFramePacketizer)
	{
		delete m_pEncodedFramePacketizer;
		m_pEncodedFramePacketizer = NULL;
	}

	if (NULL != m_pEncodedFrameDepacketizer)
	{
		delete m_pEncodedFrameDepacketizer;
		m_pEncodedFrameDepacketizer = NULL;
	}

	if (NULL != m_pVideoDecoder)
	{
		delete m_pVideoDecoder;

		m_pVideoDecoder = NULL;
	}

	if (NULL != m_pColorConverter)
	{
		delete m_pColorConverter;

		m_pColorConverter = NULL;
	}
	friendID = -1;

	SHARED_PTR_DELETE(m_pSessionMutex);
}

LongLong CVideoCallSession::GetFriendID()
{
	return friendID;
}

void CVideoCallSession::InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession");

	if (sessionMediaList.IsVideoEncoderExist(iVideoHeight, iVideoWidth))
	{
		return;
	}

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession 2");

	this->m_pVideoEncoder = new CVideoEncoder(m_pCommonElementsBucket);

	m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth);

	g_FPSController.SetEncoder(m_pVideoEncoder);
	m_BitRateController->SetEncoder(m_pVideoEncoder);

	this->m_pVideoDecoder = new CVideoDecoder(m_pCommonElementsBucket, &m_DecodingBuffer);

	m_pVideoDecoder->CreateVideoDecoder();

	this->m_pColorConverter = new CColorConverter(iVideoHeight, iVideoWidth);

	m_pVideoEncodingThread = new CVideoEncodingThread(lFriendID, m_EncodingBuffer, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer);
	m_pVideoRenderingThread = new CVideoRenderingThread(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket);
	m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, &g_FPSController);
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(lFriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter);

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;

	StartRenderingThread();
	StartEncodingThread();
	StartDepacketizationThread();
	StartDecodingThread();

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession session initialized");
}

CVideoEncoder* CVideoCallSession::GetVideoEncoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoEncoder;
}

bool CVideoCallSession::PushPacketForMerging(unsigned char *in_data, unsigned int in_size)
{

#ifdef FIRST_BUILD_COMPATIBLE
	if (!g_bIsVersionDetectableOpponent && (in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA] & 0xC0) == 0xC0)
	{
		g_bIsVersionDetectableOpponent = true;
		g_uchSendPacketVersion = VIDEO_VERSION_CODE;
		//CLogPrinter_WriteSpecific(CLogPrinter::INFO, "$$$# ######################################## Version #################################################");		
	}
#endif

#ifdef	RETRANSMISSION_ENABLED
	if (((in_data[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_RETRANS_PACKET) & 1) /* ||  ((in_data[4] >> 6) & 1) */) //If MiniPacket or RetransMitted packet
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT RETRANSMITTED PACKET");
		m_pRetransVideoPacketQueue->Queue(in_data, in_size);
	}
	else if (((in_data[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_MINI_PACKET) & 1)) // It is a minipacket
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT MINI PACKET");
		m_pMiniPacketQueue->Queue(in_data, in_size);
	}
	else
#endif
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT Original PACKET");

		/*
		int frameNumber = ((int)in_data[1]<<16) + ((int)in_data[2]<<8) + in_data[3];
		int length ;

		if(g_bIsVersionDetectableOpponent && in_data[VERSION_BYTE_INDEX])
		{
		length = ((int)in_data[12]<<8) + in_data[13];

		}
		else
		{
		length = ((int)in_data[14]<<8) + in_data[15];
		}
		*/




#ifdef BITRATE_CONTROL_BASED_ON_BANDWIDTH
		CPacketHeader NowRecvHeader;
		NowRecvHeader.setPacketHeader(in_data);

		if (GetUniquePacketID(NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber()) >= m_SlotResetLeftRange
			&& GetUniquePacketID(NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber()) < m_SlotResetRightRange)
		{
			m_ByteRcvInBandSlot += (NowRecvHeader.getPacketLength() - PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
		}
		else if (GetUniquePacketID(NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber()) >= m_SlotResetRightRange)
		{

			int SlotResetLeftRangeInFrame = (NowRecvHeader.getFrameNumber() - (NowRecvHeader.getFrameNumber() % FRAME_RATE));
			m_SlotResetLeftRange = GetUniquePacketID(SlotResetLeftRangeInFrame, 0);


			int SlotResetRightRangeInFrame = SlotResetLeftRangeInFrame + FRAME_RATE;
			m_SlotResetRightRange = GetUniquePacketID(SlotResetRightRangeInFrame, 0);


			if (m_bSkipFirstByteCalculation == true)
			{
				m_bSkipFirstByteCalculation = false;
			}
			else
			{
				m_miniPacketBandCounter = SlotResetLeftRangeInFrame - FRAME_RATE;//if we miss all frames of the previous slot it will be wrong
				m_miniPacketBandCounter = m_miniPacketBandCounter / FRAME_RATE;
				CreateAndSendMiniPacket((m_ByteRcvInBandSlot), INVALID_PACKET_NUMBER);
			}

			m_ByteRcvInBandSlot = NowRecvHeader.getPacketLength() - PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE;

			//printf("VampireEnggUpt--> m_SlotLeft, m_SlotRight = (%d, %d)........ m_ByteReceived = %d\nCurr(FN,PN) = (%d,%d)\n", m_SlotResetLeftRange/MAX_PACKET_NUMBER, m_SlotResetRightRange/MAX_PACKET_NUMBER, m_ByteRcvInBandSlot, NowRecvHeader.getFrameNumber(), NowRecvHeader.getPacketNumber());


		}

#endif



		m_pVideoPacketQueue->Queue(in_data, in_size);
	}

	return true;
}

int CVideoCallSession::PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	LongLong currentTimeStamp = m_Tools.CurrentTimestamp();

	if (m_ClientFrameCounter++)
	{
		m_ClientFPSDiffSum += currentTimeStamp - m_LastTimeStampClientFPS;


		{//Block for LOCK
			Locker lock(*m_pSessionMutex);
			g_FPSController.SetClientFPS(1000 / (m_ClientFPSDiffSum / m_ClientFrameCounter));
			//		m_ClientFPS = 1000 / (m_ClientFPSDiffSum / m_ClientFrameCounter);
			//		m_ClientFPS = 1000/(currentTimeStamp - m_LastTimeStampClientFPS);
		}

		m_DropSum = 0;
	}

	m_LastTimeStampClientFPS = currentTimeStamp;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding 2");
	//this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding Converted to 420");

#endif

	int returnedValue = m_EncodingBuffer->Queue(in_data, in_size);

	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding Queue packetSize " + Tools::IntegertoStringConvert(in_size));

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding pushed to encoder queue");

	return returnedValue;
}

CVideoDecoder* CVideoCallSession::GetVideoDecoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoDecoder;
}

CColorConverter* CVideoCallSession::GetColorConverter()
{
	return m_pColorConverter;
}

/*
void CVideoCallSession::ResetAllInMediaList()
{
sessionMediaList.ResetAllInVideoEncoderList();
}
*/

void CVideoCallSession::StopEncodingThread()
{
	m_pVideoEncodingThread->StopEncodingThread();
}

void CVideoCallSession::StartEncodingThread()
{
	m_pVideoEncodingThread->StartEncodingThread();
}

void CVideoCallSession::StopDepacketizationThread()
{
	m_pVideoDepacketizationThread->StopDepacketizationThread();
}

void CVideoCallSession::StartDepacketizationThread()
{
	m_pVideoDepacketizationThread->StartDepacketizationThread();
}

void CVideoCallSession::StopDecodingThread()
{
	m_pVideoDecodingThread->StopDecodingThread();
}

void CVideoCallSession::StartDecodingThread()
{
	m_pVideoDecodingThread->StartDecodingThread();
}

CEncodedFrameDepacketizer * CVideoCallSession::GetEncodedFrameDepacketizer()
{
	return m_pEncodedFrameDepacketizer;
}

void CVideoCallSession::CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber)
{
	unsigned char uchVersion = g_uchSendPacketVersion;

	//    if(INVALID_PACKET_NUMBER !=resendPacketNumber && resendFrameNumber % I_INTRA_PERIOD != 0 ) //
	if (INVALID_PACKET_NUMBER != resendPacketNumber) //
	{
		return;
	}

	int numberOfPackets = 1000; //dummy numberOfPackets

	CPacketHeader PacketHeader;
	if (resendPacketNumber == INVALID_PACKET_NUMBER) {
		//m_miniPacketBandCounter++;
		if (0 == uchVersion) return;

		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, resendPacketNumber/*Invalid_Packet*/, resendFrameNumber/*BandWidth*/, 0, 0, 0);
	}
	else {
		PacketHeader.setPacketHeader(uchVersion, resendFrameNumber, numberOfPackets, resendPacketNumber, 0, 0, 0, 0);
		g_timeInt.setTime(resendFrameNumber, resendPacketNumber);
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

	m_miniPacket[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA + 1] |= 1 << BIT_INDEX_MINI_PACKET; //MiniPacket Flag

	if (uchVersion)
		m_pCommonElementsBucket->SendFunctionPointer(friendID, 2, m_miniPacket, PACKET_HEADER_LENGTH + 1);
	else
		m_pCommonElementsBucket->SendFunctionPointer(friendID, 2, m_miniPacket, PACKET_HEADER_LENGTH_NO_VERSION + 1);

	//m_SendingBuffer.Queue(frameNumber, miniPacket, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
}






















void CVideoCallSession::StopRenderingThread()
{
	m_pVideoRenderingThread->StopRenderingThread();

	/*
	//if (pInternalThread.get())
	{

	bRenderingThreadRunning = false;

	while (!bRenderingThreadClosed)
	{
	m_Tools.SOSleep(5);
	}
	}

	//pInternalThread.reset();
	*/
}

void CVideoCallSession::StartRenderingThread()
{
	m_pVideoRenderingThread->StartRenderingThread();

	/*
	if (pRenderingThread.get())
	{
	pRenderingThread.reset();
	return;
	}

	bRenderingThreadRunning = true;
	bRenderingThreadClosed = false;

	#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t RenderThreadQ = dispatch_queue_create("RenderThreadQ",DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(RenderThreadQ, ^{
	this->RenderingThreadProcedure();
	});

	#else

	std::thread myThread(CreateVideoRenderingThread, this);
	myThread.detach();

	#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartRenderingThread Rendering Thread started");

	return;
	*/
}

void *CVideoCallSession::CreateVideoRenderingThread(void* param)
{
	/*
	CVideoCallSession *pThis = (CVideoCallSession*)param;
	pThis->RenderingThreadProcedure();
	*/
	return NULL;
}

void CVideoCallSession::RenderingThreadProcedure()
{
	/*
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Started EncodingThreadProcedure.");
	Tools toolsObject;
	int frameSize,nFrameNumber,intervalTime;
	unsigned int nTimeStampDiff;
	long long currentFrameTime,decodingTime,firstFrameEncodingTime;
	int videoHeight, videoWidth;
	long long currentTimeStamp;
	int prevTimeStamp=0;
	int minTimeGap = 51;

	while (bRenderingThreadRunning)
	{
	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::RenderingThreadProcedure");

	if (m_RenderingBuffer->GetQueueSize() == 0)
	toolsObject.SOSleep(10);
	else
	{

	int timeDiffForQueue;

	frameSize = m_RenderingBuffer->DeQueue(nFrameNumber, nTimeStampDiff, m_RenderingFrame, videoHeight, videoWidth, timeDiffForQueue);
	CLogPrinter_WriteForQueueTime(CLogPrinter::DEBUGS, " m_RenderingBuffer "+ toolsObject.IntegertoStringConvert(timeDiffForQueue));

	currentFrameTime = toolsObject.CurrentTimestamp();

	if(m_b1stDecodedFrame)
	{
	m_ll1stDecodedFrameTimeStamp = currentFrameTime;
	firstFrameEncodingTime = nTimeStampDiff;
	m_b1stDecodedFrame = false;
	}
	else
	{
	minTimeGap = nTimeStampDiff - prevTimeStamp ;
	}

	prevTimeStamp = nTimeStampDiff;

	if(frameSize<1 && minTimeGap < 50)
	continue;

	toolsObject.SOSleep(5);

	m_pCommonElementsBucket->m_pEventNotifier->fireVideoEvent(friendID, nFrameNumber, frameSize, m_RenderingFrame, videoHeight, videoWidth);
	}
	}

	bRenderingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::RenderingThreadProcedure() Stopped EncodingThreadProcedure");
	*/
}

int CVideoCallSession::GetUniquePacketID(int fn, int pn)
{
	return fn*MAX_PACKET_NUMBER + pn;
}

int CVideoCallSession::NeedToChangeBitRate(double dataReceivedRatio)
{
	m_SlotCounter++;

	if (dataReceivedRatio < NORMAL_BITRATE_RATIO_IN_MEGA_SLOT)
	{
		m_iGoodSlotCounter = 0;
		m_iNormalSlotCounter = 0;
		m_SlotCounter = 0;

		//m_iConsecutiveGoodMegaSlot = 0;
		m_PrevMegaSlotStatus = dataReceivedRatio;
		return BITRATE_CHANGE_DOWN;

	}
	else if (dataReceivedRatio >= NORMAL_BITRATE_RATIO_IN_MEGA_SLOT && dataReceivedRatio <= GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
	{
		m_iNormalSlotCounter++;
	}
	else if (dataReceivedRatio > GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
	{
		m_iGoodSlotCounter++;
		/*m_iConsecutiveGoodMegaSlot++;
		if(m_iConsecutiveGoodMegaSlot == GOOD_MEGASLOT_TO_UP)
		{
		m_iConsecutiveGoodMegaSlot = 0;
		return BITRATE_CHANGE_UP;
		}*/


	}
	else
	{
		//      m_iConsecutiveGoodMegaSlot = 0;
	}


	if (m_SlotCounter >= GOOD_MEGASLOT_TO_UP)
	{
		int temp = GOOD_MEGASLOT_TO_UP * 0.9;

		if (m_iGoodSlotCounter >= temp && m_PrevMegaSlotStatus>GOOD_BITRATE_RATIO_IN_MEGA_SLOT)
		{
			m_iGoodSlotCounter = 0;
			m_iNormalSlotCounter = 0;
			m_SlotCounter = 0;

			m_PrevMegaSlotStatus = dataReceivedRatio;

			return BITRATE_CHANGE_UP;
		}

	}


	m_PrevMegaSlotStatus = dataReceivedRatio;
	return BITRATE_CHANGE_NO;
}








