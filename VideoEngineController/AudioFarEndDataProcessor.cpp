#include "AudioFarEndDataProcessor.h"
#include "CommonElementsBucket.h"
#include "LiveAudioParser.h"
#include "LiveAudioParserForCallee.h"
#include "LiveAudioParserForPublisher.h"
#include "LiveAudioParserForChannel.h"
#include "AudioMixer.h"
#include "InterfaceOfAudioVideoEngine.h"



#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CAudioFarEndDataProcessor::CAudioFarEndDataProcessor(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning) :
	m_nServiceType(nServiceType),
	m_nEntityType(nEntityType),
	m_llFriendID(llFriendID),
	m_pAudioCallSession(pAudioCallSession),
	m_pCommonElementsBucket(pCommonElementsBucket),
	m_bIsLiveStreamingRunning(bIsLiveStreamingRunning),
	m_bAudioDecodingThreadRunning(false),
	m_bAudioDecodingThreadClosed(true),
	m_llLastTime(-1)
{

	m_pAudioMixer = new AudioMixer(BITS_USED_FOR_AUDIO_MIXING, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING); //Need Remove Magic Numbers.

	for (int i = 0; i < MAX_NUMBER_OF_CALLEE; i++){
		m_vAudioFarEndBufferVector.push_back(new LiveAudioDecodingQueue() );	//Need to delete.
	}

	m_pLiveAudioParser = nullptr;

	if (SERVICE_TYPE_LIVE_STREAM == m_nServiceType || SERVICE_TYPE_SELF_STREAM == m_nServiceType)
	{
		if (ENTITY_TYPE_PUBLISHER == m_nEntityType || ENTITY_TYPE_PUBLISHER_CALLER == m_nEntityType)
		{
			m_pLiveAudioParser = new CLiveAudioParserForPublisher(m_vAudioFarEndBufferVector);
		}
		else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			m_pLiveAudioParser = new CLiveAudioParserForCallee(m_vAudioFarEndBufferVector);
		}

		m_pLiveAudioReceivedQueue = new LiveAudioDecodingQueue();
		//m_pLiveReceiverAudio = new LiveReceiver(m_pCommonElementsBucket, m_pAudioCallSession);
		//m_pLiveReceiverAudio->SetAudioDecodingQueue(m_pLiveAudioReceivedQueue);
	}
	else if (SERVICE_TYPE_CHANNEL == m_nServiceType)
	{
		m_pLiveAudioParser = new CLiveAudioParserForChannel(m_vAudioFarEndBufferVector);
	}

	m_ReceivingHeader = new CAudioPacketHeader();
	m_pAudioDePacketizer = new AudioDePacketizer(m_pAudioCallSession);

	m_cAac = new CAac();
	m_cAac->SetParameters(44100, 2);
	m_pGomGomGain = new CGomGomGain(123);

	StartDecodingThread();
}

CAudioFarEndDataProcessor::~CAudioFarEndDataProcessor()
{
	StopDecodingThread();

	if (m_pGomGomGain)
	{
		delete m_pGomGomGain;
		m_pGomGomGain = nullptr;
	}
	if (m_pAudioDePacketizer)
	{
		delete m_pAudioDePacketizer;
		m_pAudioDePacketizer = nullptr;
	}
	if (m_ReceivingHeader)
	{
		delete m_ReceivingHeader;
		m_ReceivingHeader = nullptr;
	}
	if (m_pLiveAudioReceivedQueue)
	{
		delete m_pLiveAudioReceivedQueue;
		m_pLiveAudioReceivedQueue = NULL;
	}
	if (m_pLiveAudioParser)
	{
		delete m_pLiveAudioParser;
		m_pLiveAudioParser = NULL;
	}

	if (m_cAac)
	{
		delete m_cAac;
		m_cAac = nullptr;
	}
}

void CAudioFarEndDataProcessor::StartDecodingThread()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 1");


	m_bAudioDecodingThreadRunning = true;
	m_bAudioDecodingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t DecodeThreadQ = dispatch_queue_create("DecodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(DecodeThreadQ, ^{
		this->DecodingThreadProcedure();
	});

#else

	std::thread myThread(CreateAudioDecodingThread, this);
	myThread.detach();

#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread Decoding Thread started");

	return;
}

void *CAudioFarEndDataProcessor::CreateAudioDecodingThread(void* param)
{
	CAudioFarEndDataProcessor *pThis = (CAudioFarEndDataProcessor*)param;
	pThis->DecodingThreadProcedure();

	return NULL;
}

void CAudioFarEndDataProcessor::StopDecodingThread()
{
	m_bAudioDecodingThreadRunning = false;

	while (!m_bAudioDecodingThreadClosed)
		Tools::SOSleep(5);
}

int CAudioFarEndDataProcessor::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > &vMissingFrames)
{
	if (m_bIsLiveStreamingRunning)
	{
		/*
			TODO: 
				1. Here we assume that right now there is a single caller.
					So we use default FarEnd data sender id zero.
					if there is multiple caller then you need to give an ID for the data sender.
		*/
		m_pLiveAudioParser->ProcessLiveAudio(0, nOffset, pucaDecodingAudioData, unLength, frameSizes, numberOfFrames, vMissingFrames);
		return 1;
	}

	return  m_AudioReceivedBuffer.EnQueue(pucaDecodingAudioData, unLength);
}

void CAudioFarEndDataProcessor::StartCallInLive()
{
	if (m_pAudioCallSession->GetRole() == VIEWER_IN_CALL)
	{
		m_vAudioFarEndBufferVector[0]->ResetBuffer(); //Contains Data From Live Stream
	}
	m_AudioReceivedBuffer.ResetBuffer();
}

void CAudioFarEndDataProcessor::StopCallInLive()
{
	m_vAudioFarEndBufferVector[0]->ResetBuffer();
}

bool CAudioFarEndDataProcessor::IsQueueEmpty()
{
	if (m_bIsLiveStreamingRunning)
	{
#ifdef LOCAL_SERVER_LIVE_CALL
		if ((m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL || m_pAudioCallSession->GetRole() == VIEWER_IN_CALL) && m_AudioReceivedBuffer.GetQueueSize() == 0)	//EncodedData
		{
			Tools::SOSleep(5);
			return true;
		}
		else if (m_pAudioCallSession->GetRole() != PUBLISHER_IN_CALL && m_vAudioFarEndBufferVector[0]->GetQueueSize() == 0)	//All Viewers ( including callee)
		{
			Tools::SOSleep(5);
			return true;
		}
#else
		if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL && m_AudioReceivedBuffer.GetQueueSize() == 0)	//EncodedData
		{
			Tools::SOSleep(5);
			return true;
		}
		else if (m_pAudioCallSession->GetRole() != PUBLISHER_IN_CALL && m_vAudioFarEndBufferVector[0]->GetQueueSize() == 0)	//All Viewers ( including callee)
		{
			Tools::SOSleep(5);
			return true;
		}
#endif
	}
	else if (m_AudioReceivedBuffer.GetQueueSize() == 0)
	{
		Tools::SOSleep(10);
		return true;
	}
	return false;
}

void CAudioFarEndDataProcessor::DequeueData(int &decodingFrameSize)
{
	if (m_bIsLiveStreamingRunning)
	{
#ifndef LOCAL_SERVER_LIVE_CALL
		if (m_pAudioCallSession->GetRole() != PUBLISHER_IN_CALL)
		{
			decodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);
		}
		else
		{
			decodingFrameSize = m_AudioReceivedBuffer.DeQueue(m_ucaDecodingFrame);
		}
#else
		if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL || m_pAudioCallSession->GetRole() == VIEWER_IN_CALL)
		{
			decodingFrameSize = m_AudioReceivedBuffer.DeQueue(m_ucaDecodingFrame);
		}
		else
		{
			decodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame);
		}
#endif
	}
	else
	{
		decodingFrameSize = m_AudioReceivedBuffer.DeQueue(m_ucaDecodingFrame);
	}
}

bool CAudioFarEndDataProcessor::IsPacketNumberProcessable(int &iPacketNumber)
{
	if (false == m_bIsLiveStreamingRunning && m_iLastDecodedPacketNumber >= iPacketNumber) {
		PRT("@@@@########Skipped Packet: %d", iPacketNumber);
		return false;
	}
	return true;
}

void CAudioFarEndDataProcessor::DecodeAndPostProcessIfNeeded(int &iPacketNumber, int &nCurrentPacketHeaderLength, int &nCurrentAudioPacketType)
{
	m_iLastDecodedPacketNumber = iPacketNumber;
	LOGEF("Role %d, Before decode", m_iRole);
	if (!m_bIsLiveStreamingRunning)
	{
#ifdef OPUS_ENABLED
		m_nDecodedFrameSize = m_pAudioCallSession->GetAudioCodec()->decodeAudio(m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize, m_saDecodedFrame);
		ALOG("#A#DE#--->> Self#  PacketNumber = " + Tools::IntegertoStringConvert(iPacketNumber));
		LOGEF("Role %d, done decode", m_iRole);

#else
		m_nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize, m_saDecodedFrame);
#endif


#ifdef USE_AGC
		m_pAudioCallSession->m_pRecorderGain->AddFarEnd(m_saDecodedFrame, m_nDecodedFrameSize);
		m_pAudioCallSession->m_pPlayerGain->AddGain(m_saDecodedFrame, m_nDecodedFrameSize, m_bIsLiveStreamingRunning);
#endif
	}
	else 
	{
		if (AUDIO_CHANNEL_PACKET_TYPE == nCurrentAudioPacketType)	//Only for channel
		{
			long long llNow = Tools::CurrentTimestamp();
			m_cAac->DecodeFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize, m_saDecodedFrame, m_nDecodedFrameSize);
			LOG_AAC("$@@@@@@@@@--> AAC_DecodingFrameSize: %d, DecodedFrameSize: %d", m_nDecodingFrameSize, m_nDecodedFrameSize);
		}
		else
		{
			memcpy(m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize);
			m_nDecodedFrameSize = m_nDecodingFrameSize / sizeof(short);
			LOGEF("Role %d, no viewers in call", m_iRole);
		}
	}
}

bool CAudioFarEndDataProcessor::IsPacketTypeSupported(int &nCurrentAudioPacketType)
{
	if (!m_bIsLiveStreamingRunning)
	{
		if (m_ReceivingHeader->IsPacketTypeSupported(nCurrentAudioPacketType))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}

void CAudioFarEndDataProcessor::SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llLastTime, int iCurrentPacketNumber)
{
	long long llNow = 0;

	if (m_bIsLiveStreamingRunning == true) {

		llNow = Tools::CurrentTimestamp();

		__LOG("!@@@@@@@@@@@  #WQ     FN: %d -------- Receiver Time Diff : %lld    DataLength: %d",
			iPacketNumber, llNow - llLastTime, nSentFrameSize);

		llLastTime = llNow;
		if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL) {
			LOG18("#18@# PUb enq , packet type %d", iCurrentPacketNumber);
			short tmpBuffer[2048];
			int iStartIndex = 0;
			int iEndIndex = 1599;
			int iCalleeId = 1;
			int iTotalBlocks = 16;
			int iFrameSize = 800;
			int iMuxHeaderSize = 3;
			m_pAudioMixer->genCalleeChunkHeader((unsigned char*)tmpBuffer, iStartIndex, iEndIndex, iCalleeId, iCurrentPacketNumber, iFrameSize, iTotalBlocks, m_vFrameMissingBlocks);
			memcpy(tmpBuffer+3, pshSentFrame, nSentFrameSize* sizeof(short));
			m_pAudioCallSession->m_AudioDecodedBuffer.EnQueue(tmpBuffer, nSentFrameSize + iMuxHeaderSize, iCurrentPacketNumber);
		}

		HITLER("*STP -> PN: %d, FS: %d, STime: %lld", iCurrentPacketNumber, nSentFrameSize, Tools::CurrentTimestamp());
#ifdef __ANDROID__
		if (m_bIsLiveStreamingRunning && m_pAudioCallSession->GetRole() != CALL_NOT_RUNNING)
		{
			m_pGomGomGain->AddGain(pshSentFrame, nSentFrameSize);
		}
#endif
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_llFriendID,
			SERVICE_TYPE_LIVE_STREAM,
			nSentFrameSize,
			pshSentFrame);
	}
	else
	{
		HITLER("*STP -> PN: %d, FS: %d", iCurrentPacketNumber, m_nDecodedFrameSize);
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_llFriendID, SERVICE_TYPE_CALL, nSentFrameSize, pshSentFrame);
	}

}

void CAudioFarEndDataProcessor::DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize)
{
#ifdef DUMP_FILE
	fwrite(psDecodedFrame, 2, nDecodedFrameSize, m_pAudioCallSession->FileOutput);
#endif
}

void CAudioFarEndDataProcessor::ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
	int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength)
{
	m_ReceivingHeader->CopyHeaderToInformation(header);

	packetType = m_ReceivingHeader->GetInformation(INF_PACKETTYPE);
	nHeaderLength = m_ReceivingHeader->GetInformation(INF_HEADERLENGTH);
	networkType = m_ReceivingHeader->GetInformation(INF_NETWORKTYPE);
	slotNumber = m_ReceivingHeader->GetInformation(INF_SLOTNUMBER);
	packetNumber = m_ReceivingHeader->GetInformation(INF_PACKETNUMBER);
	packetLength = m_ReceivingHeader->GetInformation(INF_BLOCK_LENGTH);
	recvSlotNumber = m_ReceivingHeader->GetInformation(INF_RECVDSLOTNUMBER);
	numPacketRecv = m_ReceivingHeader->GetInformation(INF_NUMPACKETRECVD);
	channel = m_ReceivingHeader->GetInformation(INF_CHANNELS);
	version = m_ReceivingHeader->GetInformation(INF_VERSIONCODE);
	timestamp = m_ReceivingHeader->GetInformation(INF_TIMESTAMP);


	iBlockNumber = m_ReceivingHeader->GetInformation(INF_PACKET_BLOCK_NUMBER);
	nNumberOfBlocks = m_ReceivingHeader->GetInformation(INF_TOTAL_PACKET_BLOCKS);
	iOffsetOfBlock = m_ReceivingHeader->GetInformation(INF_BLOCK_OFFSET);
	nFrameLength = m_ReceivingHeader->GetInformation(INF_FRAME_LENGTH);

	if (iBlockNumber == -1)
	{
		iBlockNumber = 0;
	}

	if (nNumberOfBlocks == -1)
	{
		nNumberOfBlocks = 1;
		iOffsetOfBlock = 0;
		nFrameLength = packetLength;
	}
}

bool CAudioFarEndDataProcessor::IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType)
{
	LOGENEW("m_iRole = %d, nCurrentAudioPacketType = %d\n", m_pAudioCallSession->GetRole(), nCurrentAudioPacketType);
	
	if (SERVICE_TYPE_CHANNEL == m_nServiceType)	//Channel
	{
		if (AUDIO_CHANNEL_PACKET_TYPE == nCurrentAudioPacketType)
		{
			return true;
		}
		return false;
	}
	else if (SERVICE_TYPE_LIVE_STREAM == m_nServiceType || SERVICE_TYPE_SELF_STREAM == m_nServiceType)	//LiveStreaming.
	{
		if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL)	//
		{
			if (nCurrentAudioPacketType == AUDIO_LIVE_CALLEE_PACKET_TYPE)
			return true;
		}		
		else if (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType)
		{
			if (AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED == nCurrentAudioPacketType || AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED == nCurrentAudioPacketType)
			{
				return true;
			}
		}

	}	

	return false;
}

bool CAudioFarEndDataProcessor::IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion)
{
	if (false == m_bIsLiveStreamingRunning)
	{
		if (AUDIO_SKIP_PACKET_TYPE == nCurrentAudioPacketType)
		{
			//ALOG("#V#TYPE# ############################################### SKIPPET");
			Tools::SOSleep(0);
			return false;
		}
		else if (AUDIO_NOVIDEO_PACKET_TYPE == nCurrentAudioPacketType)
		{
			//g_StopVideoSending = 1;*/
			if (false == m_bIsLiveStreamingRunning)
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
			return true;
		}
		else if (AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
		{
			m_iAudioVersionFriend = nVersion;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}

bool CAudioFarEndDataProcessor::IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType)
{
#ifndef LOCAL_SERVER_LIVE_CALL
	if (m_bIsLiveStreamingRunning)
	{
		if (m_pLiveAudioParser->GetRoleChanging() == true)
		{
			return false;
		}
		if ((m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL) || (m_pAudioCallSession->GetRole() == VIEWER_IN_CALL))
		{
			return true;
		}
		if (-1 == m_llDecodingTimeStampOffset)
		{
			Tools::SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
			m_llDecodingTimeStampOffset = Tools::CurrentTimestamp() - llCurrentFrameRelativeTime;
			LOGENEW("iPacketNumber resyncing");
		}
		else
		{
			long long llNow = Tools::CurrentTimestamp();
			long long llExpectedEncodingTimeStamp = llNow - m_llDecodingTimeStampOffset;
			long long llWaitingTime = llCurrentFrameRelativeTime - llExpectedEncodingTimeStamp;

			LOGENEW("@@@@@@@@@--> relativeTime: [%lld] DELAY FRAME: %lld  currentTime: %lld, iPacketNumber = %d", llCurrentFrameRelativeTime, llWaitingTime, llNow, iPacketNumber);

			if (llExpectedEncodingTimeStamp - __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ > llCurrentFrameRelativeTime)
			{
				CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "CAudioCallSession::IsPacketProcessableBasedOnRelativeTime relativeTime = "
					+ Tools::getText(llCurrentFrameRelativeTime) + " DELAY = " + Tools::getText(llWaitingTime) + " currentTime = " + Tools::getText(llNow)
					+ " iPacketNumber = " + Tools::getText(iPacketNumber));
				//				HITLER("#@#@26022017# ##################################################################### dropping audio data");
				LOG_AAC("#aac#aqa# Frame not received timely: %d", llWaitingTime);
				return false;
			}

			while (llExpectedEncodingTimeStamp + __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ < llCurrentFrameRelativeTime)
			{
				Tools::SOSleep(5);
				llExpectedEncodingTimeStamp = Tools::CurrentTimestamp() - m_llDecodingTimeStampOffset;
			}
		}
		return true;
	}
	else
	{
		return true;
	}
#else
	return true;
#endif
}

void CAudioFarEndDataProcessor::SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber)
{
	if (!m_bIsLiveStreamingRunning)
	{
		if (nSlotNumber != m_iCurrentRecvdSlotID)
		{
			//Todo: m_iPrevRecvdSlotID may be accessed from multiple thread.
			m_pAudioCallSession->m_iPrevRecvdSlotID = m_iCurrentRecvdSlotID;
			if (m_pAudioCallSession->m_iPrevRecvdSlotID != -1)
			{
				m_pAudioCallSession->m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot;
			}

			m_iCurrentRecvdSlotID = nSlotNumber;
			m_iReceivedPacketsInCurrentSlot = 0;

			if (m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning()) {
				m_pAudioCallSession->GetAudioCodec()->DecideToChangeBitrate(m_iOpponentReceivedPackets);
			}
			else if (m_pAudioCallSession->GetAudioCodec()->GetCurrentBitrateOpus() != AUDIO_BITRATE_INIT){
				m_pAudioCallSession->GetAudioCodec()->SetBitrateOpus(AUDIO_BITRATE_INIT);
			}
		}
		m_iReceivedPacketsInCurrentSlot++;
	}
}

void CAudioFarEndDataProcessor::PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
	 long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime)
{
	if (!m_bIsLiveStreamingRunning)
	{
		llNow = Tools::CurrentTimestamp();
		//            ALOG("#DS Size: "+m_Tools.IntegertoStringConvert(m_nDecodedFrameSize));
		if (llNow - llTimeStamp >= 1000)
		{
			//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Num AudioDataDecoded = " + m_Tools.IntegertoStringConvert(iDataSentInCurrentSec));
			iDataSentInCurrentSec = 0;
			llTimeStamp = llNow;
		}
		iDataSentInCurrentSec++;

		
		nDecodingTime = Tools::CurrentTimestamp() - llCapturedTime;
		dbTotalTime += nDecodingTime;
	}
}
/**Decoding*/
void CAudioFarEndDataProcessor::DecodingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Started DecodingThreadProcedure method.");	

#ifdef DUMP_FILE
	m_pAudioCallSession->FileOutput = fopen("/sdcard/OutputPCMN.pcm", "wb");
#endif
		
	while (m_bAudioDecodingThreadRunning)
	{
		if (m_bIsLiveStreamingRunning)
		{
			if ((SERVICE_TYPE_LIVE_STREAM == m_nServiceType || SERVICE_TYPE_SELF_STREAM == m_nServiceType)			
				&& (ENTITY_TYPE_VIEWER == m_nEntityType || ENTITY_TYPE_VIEWER_CALLEE == m_nEntityType))		//Is Viewer or Callee.
			{
				LiveStreamFarEndProcedureViewer();
			}
			else 
			{
				LiveStreamFarEndProcedure();
			}
		}
		else 
		{
			AudioCallFarEndProcedure();
		}
	}

	m_bAudioDecodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}


void CAudioFarEndDataProcessor::LiveStreamFarEndProcedureViewer()
{
	int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
	long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
	double dbTotalTime = 0; //MeaningLess

	int iDataSentInCurrentSec = 0; //NeedToFix.
	long long llTimeStamp = 0;
	int nQueueSize = m_vAudioFarEndBufferVector[0]->GetQueueSize();
	int nCalleeId = 1;
	m_vFrameMissingBlocks.clear();
	if (nQueueSize> 0)
	{
		m_nDecodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);
		LOG18("#18#FE#Viewer... ");

		if (m_nDecodingFrameSize < 1)
		{
			//LOGE("##DE# CAudioCallSession::DecodingThreadProcedure queue size 0.");
			return;
		}

		/// ----------------------------------------- TEST CODE FOR VIWER IN CALL ----------------------------------------------///

		llCapturedTime = Tools::CurrentTimestamp();

		int dummy;
		int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
		int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
		ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
			nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

		LOG18("XXP@#@#MARUF FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);


		//int iInd = nCurrentPacketHeaderLength;
		//LOG18("#18#FE#Viewer #beg %d %d %d %d %d", (int)m_ucaDecodingFrame[iInd], (int)m_ucaDecodingFrame[iInd+1],
		//	(int)m_ucaDecodingFrame[iInd+2], (int)m_ucaDecodingFrame[iInd+3], (int)m_ucaDecodingFrame[iInd+4]);
		//iInd = nCurrentPacketHeaderLength + nFrameLength - 1;
		//LOG18("#18#FE#Viewer #end %d %d %d %d %d", (int)m_ucaDecodingFrame[iInd], (int)m_ucaDecodingFrame[iInd - 1],
		// (int)m_ucaDecodingFrame[iInd - 2], (int)m_ucaDecodingFrame[iInd - 3], (int)m_ucaDecodingFrame[iInd - 4]);

		if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
		{
			HITLER("XXP@#@#MARUF REMOVED IN BASED ON PACKET PROCESSABLE ON ROLE");
			return;
		}

		if (!IsPacketNumberProcessable(iPacketNumber))
		{
			HITLER("XXP@#@#MARUF REMOVED PACKET PROCESSABLE ON PACKET NUMBER");
			return;
		}

		//bool bIs18BitData = true;
		//
		////GetCallDatr;
		//unsigned char *pDataToBeRemoved;
		//int iTempId = 0;

		//m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, pDataToBeRemoved, iTempId);	//Need To check Casting.

		bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
		llNow = Tools::CurrentTimestamp();
		bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
		HITLER("XXP@#@#MARUF [%d %d]", iPacketNumber, iBlockNumber);
		if (bIsCompleteFrame){
			//m_ucaDecodingFrame
			HITLER("XXP@#@#MARUF Complete[%d %d]", iPacketNumber, iBlockNumber);

			m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
			if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
			{
				HITLER("XXP@#@#MARUF REMOVED ON RELATIVE TIME");
				return;
			}
		}
		llNow = Tools::CurrentTimestamp();		

		if (bIsCompleteFrame){
			HITLER("XXP@#@#MARUF WORKING ON COMPLETE FRAME . ");
			m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
			HITLER("XXP@#@#MARUF  -> HEHE %d %d", m_nDecodingFrameSize, nCurrentPacketHeaderLength);

			//DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);
			
			if (AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED == nCurrentAudioPacketType)
			{
				if (m_pAudioCallSession->GetRole() == VIEWER_IN_CALL)
				{
					nCalleeId = 1;	//Should be fixed.
					long long nGetOwnFrameNumber;
					nGetOwnFrameNumber = m_pAudioMixer->GetAudioFrameByParsingMixHeader(m_ucaDecodingFrame + nCurrentPacketHeaderLength, nCalleeId);
					long long llLastFrameNumber;
					int nSize;
					bool bFound = false;
					while (0 < m_pAudioCallSession->m_ViewerInCallSentDataQueue.GetQueueSize())
					{
						nSize = m_pAudioCallSession->m_ViewerInCallSentDataQueue.DeQueueForCallee(m_saCalleeSentData, llLastFrameNumber, nGetOwnFrameNumber);
						LOG18("#18@# FOUND OWNFrame %lld, queued frame no, %lld", nGetOwnFrameNumber, llLastFrameNumber);
						
						if (nSize == -1) {
							LOG18("#18@# FOUND EITHER CURRENT FRAME IS GREATER THAN FRONT OF QUEUE OR NO DATA IN QUEUE");
							break;
						}

						if (nGetOwnFrameNumber == llLastFrameNumber)
						{
							bFound = true;
							break;
						}
					}
					if (bFound)
					{
						LOG18("#18@# FOUND REMOVED AUDIO DATA");
						m_nDecodingFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
					}
					else
					{
						//Do Some thing;
						LOG18("#18@# FOUND REMOVED AUDIO DATA with -1");
						nCalleeId = -1;
						m_nDecodingFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
					}
				}
				else //For Only Viewers
				{
					LOG18("#18@# FOUND REMOVED AUDIO DATA ONLY VIEWR");
					nCalleeId = -1;
					m_nDecodingFrameSize = m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, (unsigned char *)m_saCalleeSentData, nCalleeId, m_vFrameMissingBlocks) / sizeof(short);
				}
			}
			else
			{
				LOG18("#18@# FOUND REMOVED AUDIO DATA ONLY VIEWR wrong media");
				memcpy(m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, m_nDecodingFrameSize);
				m_nDecodedFrameSize = m_nDecodingFrameSize / sizeof(short);
			}
			
			LOG18("#18#FE#Viewer  m_nDecodingFrameSize = %d", m_nDecodingFrameSize);
			DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
			PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
			HITLER("XXP@#@#MARUF AFTER POST PROCESS ... deoding frame size %d", m_nDecodedFrameSize);
			if (m_nDecodedFrameSize < 1)
			{
				HITLER("XXP@#@#MARUF REMOVED FOR LOW SIZE.");
				return;
			}
			LOG18("#18#FE#Viewer  SendToPlayer");
			SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber);
			Tools::SOSleep(0);
		}
	}
	else {
		Tools::SOSleep(5);
	}
}


void CAudioFarEndDataProcessor::LiveStreamFarEndProcedure()
{
	int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
	long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
	double dbTotalTime = 0; //MeaningLess

	int iDataSentInCurrentSec = 0; //NeedToFix.
	long long llTimeStamp = 0;
	int nQueueSize = m_vAudioFarEndBufferVector[0]->GetQueueSize();
	m_vFrameMissingBlocks.clear();
	if ( nQueueSize> 0)
	{
		m_nDecodingFrameSize = m_vAudioFarEndBufferVector[0]->DeQueue(m_ucaDecodingFrame, m_vFrameMissingBlocks);
		
		LOG18("#18#FE#Live..");
		if (m_nDecodingFrameSize < 1)
		{
			//LOGE("##DE# CAudioCallSession::DecodingThreadProcedure queue size 0.");
			return;
		}

		/// ----------------------------------------- TEST CODE FOR VIWER IN CALL ----------------------------------------------///

		llCapturedTime = Tools::CurrentTimestamp();

		int dummy;
		int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
		int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
		ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
			nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

		HITLER("XXP@#@#MARUF FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);
		if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
		{
			HITLER("XXP@#@#MARUF REMOVED IN BASED ON PACKET PROCESSABLE ON ROLE");
			return;
		}

		if (!IsPacketNumberProcessable(iPacketNumber))
		{
			HITLER("XXP@#@#MARUF REMOVED PACKET PROCESSABLE ON PACKET NUMBER");
			return;
		}

		//bool bIs18BitData = true;
		//
		////GetCallDatr;
		//unsigned char *pDataToBeRemoved;
		//int iTempId = 0;

		//m_pAudioMixer->removeAudioData((unsigned char *)m_saDecodedFrame, m_ucaDecodingFrame + nCurrentPacketHeaderLength, pDataToBeRemoved, iTempId);	//Need To check Casting.

		bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
		llNow = Tools::CurrentTimestamp();
		bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
		HITLER("XXP@#@#MARUF [%d %d]", iPacketNumber, iBlockNumber);
		if (bIsCompleteFrame){
			//m_ucaDecodingFrame
			HITLER("XXP@#@#MARUF Complete[%d %d]", iPacketNumber, iBlockNumber);

			m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
			if (!IsPacketProcessableBasedOnRelativeTime(llRelativeTime, iPacketNumber, nCurrentAudioPacketType))
			{
				HITLER("XXP@#@#MARUF REMOVED ON RELATIVE TIME");
				return;
			}
		}
		llNow = Tools::CurrentTimestamp();

		SetSlotStatesAndDecideToChangeBitRate(nSlotNumber);

		if (bIsCompleteFrame){
			HITLER("XXP@#@#MARUF WORKING ON COMPLETE FRAME . ");
			m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
			HITLER("XXP@#@#MARUF  -> HEHE %d %d", m_nDecodingFrameSize, nCurrentPacketHeaderLength);
			DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);
			DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
			PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
			HITLER("XXP@#@#MARUF AFTER POST PROCESS ... deoding frame size %d", m_nDecodedFrameSize);
			if (m_nDecodedFrameSize < 1)
			{
				HITLER("XXP@#@#MARUF REMOVED FOR LOW SIZE.");
				return;
			}
			LOG18("#18#FE#Live SendToPlayer");
			SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber);
			Tools::SOSleep(0);
		}
	}
	else {
		Tools::SOSleep(5);
	}
}

void CAudioFarEndDataProcessor::AudioCallFarEndProcedure()
{
	int nCurrentAudioPacketType = 0, iPacketNumber = 0, nCurrentPacketHeaderLength = 0;
	long long llCapturedTime, nDecodingTime = 0, llRelativeTime = 0, llNow = 0;
	double dbTotalTime = 0; //MeaningLess

	int iDataSentInCurrentSec = 0; //NeedToFix.
	long long llTimeStamp = 0;
	
	if (!IsQueueEmpty())
	{
		DequeueData(m_nDecodingFrameSize);
		LOG18("#18#FE#Call...");
		if (m_nDecodingFrameSize < 1)
		{
			//LOGE("##DE# CAudioCallSession::DecodingThreadProcedure queue size 0.");
			return;
		}
		
		llCapturedTime = Tools::CurrentTimestamp();

		int dummy;
		int nSlotNumber, nPacketDataLength, recvdSlotNumber, nChannel, nVersion;
		int iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength;
		ParseHeaderAndGetValues(nCurrentAudioPacketType, nCurrentPacketHeaderLength, dummy, nSlotNumber, iPacketNumber, nPacketDataLength, recvdSlotNumber, m_iOpponentReceivedPackets,
			nChannel, nVersion, llRelativeTime, m_ucaDecodingFrame, iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);

		HITLER("XXP@#@#MARUF FOUND DATA OF LENGTH -> [%d %d] %d frm len = %d", iPacketNumber, iBlockNumber, nPacketDataLength, nFrameLength);
		
		if (!IsPacketNumberProcessable(iPacketNumber))
		{
			HITLER("XXP@#@#MARUF REMOVED PACKET PROCESSABLE ON PACKET NUMBER");
			return;
		}

		if (!IsPacketTypeSupported(nCurrentAudioPacketType))
		{
			HITLER("XXP@#@#MARUF REMOVED PACKET TYPE SUPPORTED");
			return;
		}

		if (!IsPacketProcessableInNormalCall(nCurrentAudioPacketType, nVersion))
		{
			HITLER("XXP@#@#MARUF REMOVED PACKET PROCESSABLE IN NORMAL CALL");
			return;
		}

		bool bIsCompleteFrame = true;	//(iBlockNumber, nNumberOfBlocks, iOffsetOfBlock, nFrameLength);
		llNow = Tools::CurrentTimestamp();
		bIsCompleteFrame = m_pAudioDePacketizer->dePacketize(m_ucaDecodingFrame + nCurrentPacketHeaderLength, iBlockNumber, nNumberOfBlocks, nPacketDataLength, iOffsetOfBlock, iPacketNumber, nFrameLength, llNow, m_llLastTime);
		HITLER("XXP@#@#MARUF [%d %d]", iPacketNumber, iBlockNumber);
		if (bIsCompleteFrame){
			//m_ucaDecodingFrame
			HITLER("XXP@#@#MARUF Complete[%d %d]", iPacketNumber, iBlockNumber);

			m_nDecodingFrameSize = m_pAudioDePacketizer->GetCompleteFrame(m_ucaDecodingFrame + nCurrentPacketHeaderLength) + nCurrentPacketHeaderLength;
		}
		llNow = Tools::CurrentTimestamp();
		
		if (bIsCompleteFrame){
			HITLER("XXP@#@#MARUF WORKING ON COMPLETE FRAME . ");
			m_nDecodingFrameSize -= nCurrentPacketHeaderLength;
			HITLER("XXP@#@#MARUF  -> HEHE %d %d", m_nDecodingFrameSize, nCurrentPacketHeaderLength);
			DecodeAndPostProcessIfNeeded(iPacketNumber, nCurrentPacketHeaderLength, nCurrentAudioPacketType);
			DumpDecodedFrame(m_saDecodedFrame, m_nDecodedFrameSize);
			PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, nDecodingTime, dbTotalTime, llCapturedTime);
			HITLER("XXP@#@#MARUF AFTER POST PROCESS ... deoding frame size %d", m_nDecodedFrameSize);
			if (m_nDecodedFrameSize < 1)
			{
				HITLER("XXP@#@#MARUF REMOVED FOR LOW SIZE.");
				return;
			}
			LOG18("#18#FE#Call SendToPlayer");
			SendToPlayer(m_saDecodedFrame, m_nDecodedFrameSize, m_llLastTime, iPacketNumber);
			Tools::SOSleep(0);
		}
	}
}




