
#include "VideoCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Globals.h"
#include "Controller.h"


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define MINIMUM_CAPTURE_INTERVAL_TO_UPDATE_FPS 10

extern long long g_llFirstFrameReceiveTime;
CVideoCallSession::CVideoCallSession(CController *pController,LongLong fname, CCommonElementsBucket* sharedObject, int nFPS, int *nrDeviceSupportedCallFPS, bool bIsCheckCall, CDeviceCapabilityCheckBuffer *deviceCheckCapabilityBuffer, int nOwnSupportedResolutionFPSLevel) :

m_pCommonElementsBucket(sharedObject),
m_ClientFPS(DEVICE_FPS_MAXIMUM),
m_ClientFPSDiffSum(0),
m_ClientFrameCounter(0),
m_EncodingFrameCounter(0),
m_pEncodedFramePacketizer(NULL),
m_ByteRcvInBandSlot(0),
m_SlotResetLeftRange(0),
m_SlotResetRightRange(nFPS),
m_pVideoEncoder(NULL),
m_bSkipFirstByteCalculation(true),
m_llTimeStampOfFirstPacketRcvd(-1),
m_nFirstFrameEncodingTimeDiff(-1),
m_llShiftedTime(-1),
mt_llCapturePrevTime(0),
m_bResolationCheck(false),
m_bShouldStartCalculation(false),
m_bCaclculationStartTime(0),
m_bHighResolutionSupportedForOwn(false),
m_bHighResolutionSupportedForOpponent(false),
m_bReinitialized(false),
m_bResolutionNegotiationDone(false),
m_pVersionController(NULL),
m_bIsCheckCall(bIsCheckCall),
m_nCallFPS(nFPS),
pnDeviceSupportedFPS(nrDeviceSupportedCallFPS),
m_nOwnVideoCallQualityLevel(nOwnSupportedResolutionFPSLevel),
m_nOpponentVideoCallQualityLevel(VIDEO_CALL_TYPE_UNKNOWN),
m_nCurrentVideoCallQualityLevel(VIDEO_CALL_TYPE_UNKNOWN),
m_pDeviceCheckCapabilityBuffer(deviceCheckCapabilityBuffer),
m_bVideoCallStarted(false),
m_nDeviceCheckFrameCounter(0)

{
    m_pController = pController;
    
	m_miniPacketBandCounter = 0;

	//Resetting Global Variables.

	if (m_bIsCheckCall == LIVE_CALL_MOOD)
	{
		m_bResolationCheck = true;
	}

	m_pFPSController = new CFPSController(nFPS);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::CVideoCallSession");
	m_pVideoCallSessionMutex.reset(new CLockHandler);
	m_lfriendID = fname;
	sessionMediaList.ClearAllFromVideoEncoderList();


	m_SendingBuffer = new CSendingBuffer();
	m_EncodingBuffer = new CEncodingBuffer();
	m_RenderingBuffer = new CRenderingBuffer();

	m_pVideoPacketQueue = new CVideoPacketQueue();
	m_pRetransVideoPacketQueue = new CVideoPacketQueue();
	m_pMiniPacketQueue = new CVideoPacketQueue();

	m_pEncodedFramePacketizer = new CEncodedFramePacketizer(sharedObject, m_SendingBuffer, this);
	m_pEncodedFrameDepacketizer = new CEncodedFrameDepacketizer(sharedObject, this);

	m_BitRateController = new BitRateController(m_nCallFPS);

	m_BitRateController->SetSharedObject(sharedObject);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::CVideoCallSession created");
}

CVideoCallSession::~CVideoCallSession()
{
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");
	m_pVideoEncodingThread->StopEncodingThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
	m_pSendingThread->StopSendingThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 3");
	m_pVideoDepacketizationThread->StopDepacketizationThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");
	m_pVideoDecodingThread->StopDecodingThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");
	m_pVideoRenderingThread->StopRenderingThread();
//    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 6");

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

	if (NULL != m_pSendingThread)
	{
		delete m_pSendingThread;
		m_pSendingThread = NULL;
	}

	if (NULL != m_SendingBuffer)
	{
		delete m_SendingBuffer;
		m_SendingBuffer = NULL;
	}

	if(NULL != m_pFPSController)
	{
		delete m_pFPSController;
		m_pFPSController = NULL;
	}

	m_lfriendID = -1;
    
    if(NULL  != m_pVersionController)
    {
        delete m_pVersionController;
        m_pVersionController = NULL;
        
        
    }
    
	SHARED_PTR_DELETE(m_pVideoCallSessionMutex);
}

LongLong CVideoCallSession::GetFriendID()
{
	return m_lfriendID;
}

void CVideoCallSession::InitializeVideoSession(LongLong lFriendID, int iVideoHeight, int iVideoWidth, int iNetworkType)
{
	m_nVideoCallHeight = iVideoHeight;
	m_nVideoCallWidth = iVideoWidth;

    m_pVersionController = new CVersionController();
    
    g_llFirstFrameReceiveTime = 0;

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession");

	if (sessionMediaList.IsVideoEncoderExist(iVideoHeight, iVideoWidth))
	{
		return;
	}

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession 2");

	this->m_pVideoEncoder = new CVideoEncoder(m_pCommonElementsBucket);

	m_pVideoEncoder->CreateVideoEncoder(iVideoHeight, iVideoWidth, m_nCallFPS, m_nCallFPS /2 + 1);

	m_pFPSController->SetEncoder(m_pVideoEncoder);
	m_BitRateController->SetEncoder(m_pVideoEncoder);

	this->m_pVideoDecoder = new CVideoDecoder(m_pCommonElementsBucket);

	m_pVideoDecoder->CreateVideoDecoder();

	this->m_pColorConverter = new CColorConverter(iVideoHeight, iVideoWidth);

	m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, this, m_bIsCheckCall);
	m_pVideoEncodingThread = new CVideoEncodingThread(lFriendID, m_EncodingBuffer, m_pCommonElementsBucket, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this, m_nCallFPS, m_bIsCheckCall);
	m_pVideoRenderingThread = new CVideoRenderingThread(lFriendID, m_RenderingBuffer, m_pCommonElementsBucket, this, m_bIsCheckCall);
	m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, this, m_bIsCheckCall, m_nCallFPS);
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(lFriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter, m_pVersionController, this);

	m_pCommonElementsBucket->m_pVideoEncoderList->AddToVideoEncoderList(lFriendID, m_pVideoEncoder);

	m_ClientFrameCounter = 0;
	m_EncodingFrameCounter = 0;
	m_llFirstFrameCapturingTimeStamp = -1;
	m_pSendingThread->StartSendingThread();
	
	m_pVideoEncodingThread->StartEncodingThread();
	
	m_pVideoRenderingThread->StartRenderingThread();	
	m_pVideoDepacketizationThread->StartDepacketizationThread();
	m_pVideoDecodingThread->StartDecodingThread();

	m_BitRateController->SetOwnNetworkType(iNetworkType);
	CreateAndSendMiniPacket(iNetworkType, NETWORK_TYPE_MINIPACKET);

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InitializeVideoSession session initialized");
}

CVideoEncoder* CVideoCallSession::GetVideoEncoder()
{
	//	return sessionMediaList.GetFromVideoEncoderList(mediaName);

	return m_pVideoEncoder;
}

long long CVideoCallSession::GetFirstVideoPacketTime(){
	return m_llTimeStampOfFirstPacketRcvd;
}

void CVideoCallSession::SetFirstVideoPacketTime(long long llTimeStamp){
	m_llTimeStampOfFirstPacketRcvd = llTimeStamp;
}

void CVideoCallSession::SetFirstFrameEncodingTime(int time){
	m_nFirstFrameEncodingTimeDiff = time;
}

int CVideoCallSession::GetFirstFrameEncodingTime(){
	return m_nFirstFrameEncodingTimeDiff;
}

bool CVideoCallSession::PushPacketForMerging(unsigned char *in_data, unsigned int in_size, bool bSelfData)
{
    
#ifdef FIRST_BUILD_COMPATIBLE
    
	if (!m_pVersionController->GetOpponentVersionCompatibleFlag() && (in_data[SIGNAL_BYTE_INDEX_WITHOUT_MEDIA] & 0xC0) == 0xC0)
	{
		//g_bIsVersionDetectableOpponent = true;
		//g_uchSendPacketVersion = VIDEO_VERSION_CODE;
		//CLogPrinter_WriteSpecific(CLogPrinter::INFO, "$$$# ######################################## Version #################################################");
        
        
        m_pVersionController->SetOpponentVersionCompatibleFlag(true);
	}
#endif

	if (((in_data[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_MINI_PACKET) & 1)) // It is a minipacket
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "PKTTYPE --> GOT MINI PACKET");
		m_pMiniPacketQueue->Queue(in_data, in_size);
	}
	else
	{
        /*if(bSelfData == false && m_bResolutionNegotiationDone == false)
        {
            OperationForResolutionControl(in_data,in_size);
        }*/
        
		unsigned int unFrameNumber = m_PacketHeader.GetFrameNumberDirectly(in_data);
        
        

		if (unFrameNumber >= m_SlotResetLeftRange && unFrameNumber < m_SlotResetRightRange)
		{
			m_ByteRcvInBandSlot += (in_size - PACKET_HEADER_LENGTH);
		}
		else
		{
			if (m_bSkipFirstByteCalculation == true)
			{
				m_bSkipFirstByteCalculation = false;
			}
			else
			{
				m_miniPacketBandCounter = m_SlotResetLeftRange / m_nCallFPS;
                
                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "ReceivingSide: SlotIndex = " + m_Tools.IntegertoStringConvert(m_miniPacketBandCounter) + ", ReceivedBytes = " + m_Tools.IntegertoStringConvert(m_ByteRcvInBandSlot));

				CreateAndSendMiniPacket(m_ByteRcvInBandSlot, BITRATE_TYPE_MINIPACKET);
			}

			m_SlotResetLeftRange = unFrameNumber - (unFrameNumber % m_nCallFPS);
            
            
			m_SlotResetRightRange = m_SlotResetLeftRange + m_nCallFPS;

			m_ByteRcvInBandSlot = in_size - PACKET_HEADER_LENGTH;
		}

		m_pVideoPacketQueue->Queue(in_data, in_size);
	}

	return true;
}

map<int, long long> g_TimeTraceFromCaptureToSend;
int g_CapturingFrameCounter = 0;

int CVideoCallSession::PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size, int device_orientation)
{
    /*if(m_bIsCheckCall==true)
    {
        m_nDeviceCheckFrameCounter++;
        if(m_nDeviceCheckFrameCounter>75) return m_nDeviceCheckFrameCounter;
    }*/
    
    
	if(GetVersionController()->GetCurrentCallVersion() == -1 && m_bIsCheckCall == false)
    {
        return 1;
    }

	if (m_bVideoCallStarted == false && m_bIsCheckCall == false)
	{
		return 1;
	}
    
	if (m_pVideoEncodingThread->IsThreadStarted() == false)
		return 1;

    if(g_llFirstFrameReceiveTime == 0) g_llFirstFrameReceiveTime = m_Tools.CurrentTimestamp();
    
	
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding");

	LongLong currentTimeStamp = m_Tools.CurrentTimestamp();

	if (m_ClientFrameCounter++)
	{
		m_ClientFPSDiffSum += currentTimeStamp - m_LastTimeStampClientFPS;

		{//Block for LOCK
			int  nApproximateAverageFrameInterval = m_ClientFPSDiffSum / m_ClientFrameCounter;
			if(nApproximateAverageFrameInterval > MINIMUM_CAPTURE_INTERVAL_TO_UPDATE_FPS) {
				Locker lock(*m_pVideoCallSessionMutex);
				m_pFPSController->SetClientFPS(1000 / nApproximateAverageFrameInterval);
			}
		}
	}

	m_LastTimeStampClientFPS = currentTimeStamp;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding 2");
	//this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::PushIntoBufferForEncoding Converted to 420");

#endif

	if(-1 == m_llFirstFrameCapturingTimeStamp)
		m_llFirstFrameCapturingTimeStamp = currentTimeStamp;

	int nCaptureTimeDiff = currentTimeStamp - m_llFirstFrameCapturingTimeStamp;
    
    g_TimeTraceFromCaptureToSend[g_CapturingFrameCounter] = m_Tools.CurrentTimestamp();

    if(g_CapturingFrameCounter<30)
        printf("Frame %d --> Trying to Set --> %d..... Capture Time = %lld\n", g_CapturingFrameCounter, nCaptureTimeDiff, m_Tools.CurrentTimestamp());
    
    g_CapturingFrameCounter++;
    
	int returnedValue = m_EncodingBuffer->Queue(in_data, in_size, nCaptureTimeDiff, device_orientation);
    
    //CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, " nCaptureTimeDiff = " +  m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - mt_llCapturePrevTime));
    mt_llCapturePrevTime = m_Tools.CurrentTimestamp();
    
    
    
//	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoCallSession::PushIntoBufferForEncoding Queue packetSize " + Tools::IntegertoStringConvert(in_size));

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

CEncodedFrameDepacketizer * CVideoCallSession::GetEncodedFrameDepacketizer()
{
	return m_pEncodedFrameDepacketizer;
}

void CVideoCallSession::CreateAndSendMiniPacket(int nByteReceivedOrNetworkType, int nMiniPacketType)
{
	unsigned char uchVersion = (unsigned char)GetVersionController()->GetCurrentCallVersion();
    
	CPacketHeader PacketHeader;

	if (nMiniPacketType == BITRATE_TYPE_MINIPACKET)
	{
		if(0 == uchVersion) return;

		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Byte Received*/, 0, 0, 0);
	}
	else if (nMiniPacketType == NETWORK_TYPE_MINIPACKET)
	{
		//if(0 == uchVersion) return;

		PacketHeader.setPacketHeader(uchVersion, m_miniPacketBandCounter/*SlotID*/, 0, nMiniPacketType, nByteReceivedOrNetworkType/*Network Type*/, 0, 0, 0);
	}

	m_miniPacket[0] = (int)VIDEO_PACKET_MEDIA_TYPE;

	PacketHeader.GetHeaderInByteArray(m_miniPacket + 1);

	m_miniPacket[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA + 1] |= 1<<BIT_INDEX_MINI_PACKET; 

	if(uchVersion)
		m_pCommonElementsBucket->SendFunctionPointer(m_lfriendID, 2, m_miniPacket,PACKET_HEADER_LENGTH + 1);
	else
		m_pCommonElementsBucket->SendFunctionPointer(m_lfriendID, 2, m_miniPacket,PACKET_HEADER_LENGTH_NO_VERSION + 1);
}

long long CVideoCallSession::GetShiftedTime()
{
	return m_llShiftedTime;
}
void CVideoCallSession::SetShiftedTime(long long llTime)
{
	m_llShiftedTime = llTime;
}

bool  CVideoCallSession::GetResolationCheck()
{
    
    return m_bResolationCheck;
}

void CVideoCallSession::SetCalculationStartMechanism(bool s7)
{
    m_bShouldStartCalculation = s7;
    if(m_bCaclculationStartTime == 0)
    {
        m_bCaclculationStartTime = m_Tools.CurrentTimestamp();
    }
}

bool CVideoCallSession::GetCalculationStatus()
{
    return m_bShouldStartCalculation;
}

long long CVideoCallSession::GetCalculationStartTime()
{
    return m_bCaclculationStartTime;
}

void CVideoCallSession::DecideHighResolatedVideo(bool bValue)
{
    m_bResolationCheck = true;
    
    if(bValue)
    {
        m_bHighResolutionSupportedForOwn = true;
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Decision Supported = " + m_Tools.IntegertoStringConvert(bValue));
        m_pDeviceCheckCapabilityBuffer->Queue(m_lfriendID, STOP_DEVICE_CHECK, DEVICE_CHECK_SUCCESS, m_nVideoCallHeight, m_nVideoCallWidth);
        
        
        
        
        //ReInitializeVideoLibrary(352, 288);
        //StopDeviceAbilityChecking();
        //m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_640x480);
        
    }
    else
    {
        //Not Supported
        m_bHighResolutionSupportedForOwn = false;
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Decision NotSupported = " + m_Tools.IntegertoStringConvert(bValue));
        
        m_pDeviceCheckCapabilityBuffer->Queue(m_lfriendID, STOP_DEVICE_CHECK, DEVICE_CHECK_FAILED, m_nVideoCallHeight, m_nVideoCallWidth);
        
        
        
        
		//StopDeviceAbilityChecking();
        //m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_25FPS);
        
        //ReInitializeVideoLibrary(352, 288);

		//if (m_nVideoCallWidth < 640)
			//*pnDeviceSupportedFPS = 15;


    }
}

bool CVideoCallSession::GetHighResolutionSupportStatus()
{
    return m_bHighResolutionSupportedForOwn;
}

void CVideoCallSession::SetOpponentHighResolutionSupportStatus(bool bValue)
{
    m_bHighResolutionSupportedForOpponent = bValue;
}

bool CVideoCallSession::GetOpponentHighResolutionSupportStatus()
{
    return m_bHighResolutionSupportedForOpponent;
}

bool CVideoCallSession::GetReinitializationStatus()
{
    return m_bReinitialized;
}


void CVideoCallSession::OperationForResolutionControl(unsigned char* in_data, int in_size)
{
    //Opponent resolution support checking
    CPacketHeader RcvPakcetHeader;
    int gotResSt = RcvPakcetHeader.GetOpponentResolution(in_data);
    
    if(gotResSt == 2)
    {
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent High supported, flag =  "+ m_Tools.IntegertoStringConvert(gotResSt) );
        
        SetOpponentHighResolutionSupportStatus(true);
        m_bResolutionNegotiationDone = true;
        
        
    }
    else if(gotResSt == 1)
    {
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent High NotSupported = "+ m_Tools.IntegertoStringConvert(gotResSt));
        
        
        SetOpponentHighResolutionSupportStatus(false);
        
        if(m_bHighResolutionSupportedForOwn == false || m_bHighResolutionSupportedForOpponent == false)
        {
            printf("m_bReinitialized SET_CAMERA_RESOLUTION_352x288_OR_320x240  = %d\n", m_bReinitialized);
            
            //m_pCommonElementsBucket->m_pEventNotifier->fireVideoNotificationEvent(m_lfriendID, m_pCommonElementsBucket->m_pEventNotifier->SET_CAMERA_RESOLUTION_352x288_OR_320x240);
            
            //ReInitializeVideoLibrary(352, 288);
        }
        
        m_pVideoDecodingThread->Reset();
        m_bResolutionNegotiationDone = true;
        
    }
    else
    {
//        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Opponent not set = " + m_Tools.IntegertoStringConvert(gotResSt));
    }
    
    
    //End of  Opponent resolution support checking
}
CVersionController* CVideoCallSession::GetVersionController()
{
    return m_pVersionController;
    
}
bool CVideoCallSession::GetResolutionNegotiationStatus()
{
    return m_bResolutionNegotiationDone;
}

void CVideoCallSession::StopDeviceAbilityChecking()
{
	long long llReinitializationStartTime = m_Tools.CurrentTimestamp();

//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");
	m_pVideoEncodingThread->StopEncodingThread();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
	m_pVideoDepacketizationThread->StopDepacketizationThread();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");
	m_pVideoDecodingThread->InstructionToStop();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");
	m_pVideoRenderingThread->StopRenderingThread();
//	CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 6");
}

int CVideoCallSession::GetOwnVideoCallQualityLevel(){
	return m_nOwnVideoCallQualityLevel;	
}

int CVideoCallSession::GetOpponentVideoCallQualityLevel(){
	return m_nOpponentVideoCallQualityLevel;
}

void CVideoCallSession::SetOwnVideoCallQualityLevel(int nVideoCallQualityLevel){
	m_nOwnVideoCallQualityLevel = nVideoCallQualityLevel;
}

void CVideoCallSession::SetOpponentVideoCallQualityLevel(int nVideoCallQualityLevel){
	m_nOpponentVideoCallQualityLevel = nVideoCallQualityLevel;
}

int CVideoCallSession::GetCurrentVideoCallQualityLevel(){
	return m_nCurrentVideoCallQualityLevel;
}

void CVideoCallSession::SetCurrentVideoCallQualityLevel(int nVideoCallQualityLevel)
{
	m_nCurrentVideoCallQualityLevel = nVideoCallQualityLevel;
    
    if(m_bIsCheckCall == true)
        return;

	if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_640_25)
	{
        

		m_nVideoCallHeight = m_pController->m_Quality[1].iHeight;
		m_nVideoCallWidth = m_pController->m_Quality[1].iWidth;
        
		m_nCallFPS = 25;
		m_SlotResetRightRange = 25;
	}
	else if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_352_25)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
		m_nCallFPS = 25;
		m_SlotResetRightRange = 25;
	}
	else if (m_nCurrentVideoCallQualityLevel == SUPPORTED_RESOLUTION_FPS_352_15)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
        
		m_nCallFPS = 15;
		m_SlotResetRightRange = 15;
	}
	else if (m_nCurrentVideoCallQualityLevel == RESOLUTION_FPS_SUPPORT_NOT_TESTED)
	{
        m_nVideoCallHeight = m_pController->m_Quality[0].iHeight;
        m_nVideoCallWidth = m_pController->m_Quality[0].iWidth;
		m_nCallFPS = 15;
		m_SlotResetRightRange = 15;
	}

	m_BitRateController->SetCallFPS(m_nCallFPS);
	m_pVideoEncodingThread->SetCallFPS(m_nCallFPS);
    m_pVideoEncodingThread->SetFrameNumber(m_nCallFPS);
	m_pVideoDecodingThread->SetCallFPS(m_nCallFPS);

	this->m_pColorConverter->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth);
	this->m_pVideoEncoder->SetHeightWidth(m_nVideoCallHeight, m_nVideoCallWidth, m_nCallFPS, m_nCallFPS / 2 + 1);

	m_pVideoEncodingThread->SetNotifierFlag(true);
}

BitRateController* CVideoCallSession::GetBitRateController(){
	return m_BitRateController;
}

void CVideoCallSession::ReInitializeVideoLibrary(int iHeight, int iWidth)
{
    return;
    
    printf("Reinitializing........\n");
    long long llReinitializationStartTime = m_Tools.CurrentTimestamp();
    

    
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 1");
    m_pVideoEncodingThread->StopEncodingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 2");
    m_pVideoDepacketizationThread->StopDepacketizationThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 4");
    //m_pVideoDecodingThread->StopDecodingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 5");
    m_pVideoRenderingThread->StopRenderingThread();
    CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Video call session destructor 6");
    

	m_pVideoEncoder->CreateVideoEncoder(iHeight, iWidth, m_nCallFPS, m_nCallFPS/2 + 1);
	m_pColorConverter->SetHeightWidth(iHeight, iWidth);

	m_SendingBuffer->ResetBuffer();
	//g_FPSController.Reset(m_nCallFPS;);
	m_EncodingBuffer->ResetBuffer();
	//m_BitRateController
	m_RenderingBuffer->ResetBuffer();
	m_pVideoPacketQueue->ResetBuffer();
	m_pMiniPacketQueue->ResetBuffer();

	m_BitRateController = new BitRateController(m_nCallFPS);
    m_BitRateController->SetEncoder(m_pVideoEncoder);
    m_BitRateController->SetSharedObject(m_pCommonElementsBucket);
    
    m_pVideoEncodingThread->ResetVideoEncodingThread(m_BitRateController);
    
//  m_pSendingThread = new CSendingThread(m_pCommonElementsBucket, m_SendingBuffer, &g_FPSController, this);
//	m_pVideoEncodingThread = new CVideoEncodingThread(m_lfriendID, m_EncodingBuffer, m_BitRateController, m_pColorConverter, m_pVideoEncoder, m_pEncodedFramePacketizer, this);
//	m_pVideoRenderingThread = new CVideoRenderingThread(m_lfriendID, m_RenderingBuffer, m_pCommonElementsBucket, this);
//   m_pVideoDecodingThread = new CVideoDecodingThread(m_pEncodedFrameDepacketizer, m_RenderingBuffer, m_pVideoDecoder, m_pColorConverter, &g_FPSController, this);
    
	m_pVideoDepacketizationThread = new CVideoDepacketizationThread(m_lfriendID, m_pVideoPacketQueue, m_pRetransVideoPacketQueue, m_pMiniPacketQueue, m_BitRateController, m_pEncodedFrameDepacketizer, m_pCommonElementsBucket, &m_miniPacketBandCounter, m_pVersionController, this);
    

    
    g_CapturingFrameCounter = 0;
    
    m_pVideoEncodingThread->StartEncodingThread();
    m_pVideoRenderingThread->StartRenderingThread();
    m_pVideoDepacketizationThread->StartDepacketizationThread();
    //m_pVideoDecodingThread->StartDecodingThread();
    m_pVideoDecodingThread->Reset();
    
    m_bReinitialized = true;
    
    printf("TheKing--> Reinitialization time = %lld\n",m_Tools.CurrentTimestamp() - llReinitializationStartTime);
    
    
}

CFPSController* CVideoCallSession::GetFPSController()
{
	return m_pFPSController;
}

