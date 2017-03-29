
#include "AudioNearEndDataProcessor.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "CommonElementsBucket.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioPacketizer.h"
#include "Gain.h"


CAudioNearEndDataProcessor::CAudioNearEndDataProcessor(long long llFriendID, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning) :
m_bIsReady(false),
m_llFriendID(llFriendID),
m_pAudioCallSession(pAudioCallSession),
m_pCommonElementsBucket(pCommonElementsBucket),
m_pAudioEncodingBuffer(pAudioEncodingBuffer),
m_bIsLiveStreamingRunning(bIsLiveStreamingRunning),
m_bAudioEncodingThreadRunning(false),
m_bAudioEncodingThreadClosed(true),
m_nEncodedFrameSize(0),
m_iPacketNumber(0),
m_iRawDataSendIndexViewer(0),
m_iRawDataSendIndexCallee(0)
{
	LOGT("##NF## anedp created.");
	m_pAudioEncodingMutex.reset(new CLockHandler);
	m_pAudioCodec = pAudioCallSession->GetAudioCodec();
	m_llMaxAudioPacketNumber = ((1LL << HeaderBitmap[INF_PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;

	m_pAudioPacketHeader = new CAudioPacketHeader();

	m_MyAudioHeadersize = m_pAudioPacketHeader->GetHeaderSize();
	m_llEncodingTimeStampOffset = Tools::CurrentTimestamp();

	m_bIsReady = true;

	StartEncodingThread();
}

CAudioNearEndDataProcessor::~CAudioNearEndDataProcessor(){
	//TODO: delete all new's inside cons.
	StopEncodingThread();
	delete m_pAudioPacketHeader;
}

void CAudioNearEndDataProcessor::StartEncodingThread()
{
	m_bAudioEncodingThreadRunning = true;
	m_bAudioEncodingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
	dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(EncodeThreadQ, ^{
		this->EncodingThreadProcedure();
	});
#else
	std::thread myThread(CreateAudioEncodingThread, this);
	myThread.detach();
#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread Encoding Thread started");

	return;
}

void *CAudioNearEndDataProcessor::CreateAudioEncodingThread(void* param)
{
	CAudioNearEndDataProcessor *pThis = (CAudioNearEndDataProcessor*)param;
	pThis->EncodingThreadProcedure();

	return NULL;
}

void CAudioNearEndDataProcessor::EncodingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
#ifdef __DUMP_FILE__
	FileInput = fopen("/sdcard/InputPCMN.pcm", "wb");
	FileInputWithEcho = fopen("/sdcard/InputPCMN_WITH_ECHO.pcm", "wb");
	FileInputPreGain = fopen("/sdcard/InputPCMNPreGain.pcm", "wb");
#endif
	Tools toolsObject;
	long long encodingTime = 0;
	long long llCapturedTime = 0;
	double avgCountTimeStamp = 0;
	int countFrame = 0;
	int version = 0;
	long long llRelativeTime = 0;

	long long llLasstTime = -1;
	int cnt = 1;
	LOGT("##NF## encoder started");
	while (m_bAudioEncodingThreadRunning)
	{
		if (m_pAudioEncodingBuffer->GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			m_pAudioEncodingBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
			LOGT("##NF## encoder got job. time:%lld", llCapturedTime);
			MuxIfNeeded();
			DumpEncodingFrame();
			PrintRelativeTime(cnt, llLasstTime, countFrame, llRelativeTime, llCapturedTime);

			if (false == PreProcessAudioBeforeEncoding())
			{
				continue;
			}

			EncodeIfNeeded(llCapturedTime, encodingTime, avgCountTimeStamp);

			if (m_bIsLiveStreamingRunning && VIEWER_IN_CALL == m_pAudioCallSession->GetRole())
			{

				m_pAudioCallSession->GetAudioPacketizer()->Packetize(
					true /*bool bShouldPacketize*/,
					m_ucaRawFrameNonMuxed + 1 + m_MyAudioHeadersize /*unsigned char* uchData*/,
					m_nRawFrameSize /*int nDataLength*/,
					m_iPacketNumber /*int nFrameNumber*/,
					AUDIO_LIVE_CALLEE_PACKET_TYPE /*int packetType*/,
					0 /*int networkType*/,
					version /*int version*/,
					llRelativeTime /*long long llRelativeTime*/,
					0 /*int channel*/,
					m_pAudioCallSession->m_iPrevRecvdSlotID /*int iPrevRecvdSlotID*/,
					m_pAudioCallSession->m_iReceivedPacketsInPrevSlot /*int nReceivedPacketsInPrevSlot*/,
					m_llFriendID /*long long llFriendID*/);

				toolsObject.SOSleep(10);

				m_pAudioCallSession->GetAudioPacketizer()->Packetize(
						true /*bool bShouldPacketize*/,
						m_ucaRawFrameNonMuxed + 1 + m_MyAudioHeadersize /*unsigned char* uchData*/,
						m_nRawFrameSize /*int nDataLength*/,
						m_iPacketNumber /*int nFrameNumber*/,
						AUDIO_LIVE_CALLEE_PACKET_TYPE /*int packetType*/,
						0 /*int networkType*/,
						version /*int version*/,
						llRelativeTime /*long long llRelativeTime*/,
						0 /*int channel*/,
						m_pAudioCallSession->m_iPrevRecvdSlotID /*int iPrevRecvdSlotID*/,
						m_pAudioCallSession->m_iReceivedPacketsInPrevSlot /*int nReceivedPacketsInPrevSlot*/,
						m_llFriendID /*long long llFriendID*/);

				SetAudioIdentifierAndNextPacketType();
			}
			else {
				AddHeader(version, llRelativeTime);
				SetAudioIdentifierAndNextPacketType();
				EnqueueReadyToSendData(toolsObject);
			}

			toolsObject.SOSleep(0);
		}
	}

	m_bAudioEncodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

void CAudioNearEndDataProcessor::EnqueueReadyToSendData(Tools toolsObject)
{
#ifdef  __AUDIO_SELF_CALL__
	//Todo: m_AudioReceivedBuffer fix. not member of this class
	if (m_bIsLiveStreamingRunning == false)
	{
		ALOG("#A#EN#--->> Self#  PacketNumber = " + Tools::IntegertoStringConvert(m_iPacketNumber));
		m_AudioReceivedBuffer.EnQueue(m_ucaEncodedFrame + 1, m_nEncodedFrameSize + m_MyAudioHeadersize);
		return;
	}
#endif
	if (m_pAudioCallSession->m_bIsCheckCall != LIVE_CALL_MOOD)	//Capability test call
	{
		return;
	}


	if (m_bIsLiveStreamingRunning)
	{
		if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL)
		{
			LOGT("###NF### EnqueReadyToSendData.");
			Locker lock(*m_pAudioEncodingMutex);
			if ((m_iRawDataSendIndexViewer + m_nRawFrameSize + m_MyAudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
			{
				memcpy(m_ucaRawDataToSendViewer + m_iRawDataSendIndexViewer, m_ucaRawFrameMuxed, m_nRawFrameSize + m_MyAudioHeadersize + 1);
				m_iRawDataSendIndexViewer += (m_nRawFrameSize + m_MyAudioHeadersize + 1);
				m_vRawFrameLengthViewer.push_back(m_nRawFrameSize + m_MyAudioHeadersize + 1);
			}
#ifndef LOCAL_SERVER_LIVE_CALL
			if ((m_iRawDataSendIndexCallee + m_nRawFrameSize + m_MyAudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
			{
				memcpy(m_ucaRawDataToSendCallee + m_iRawDataSendIndexCallee, m_ucaRawFrameNonMuxed, m_nRawFrameSize + m_MyAudioHeadersize + 1);
				m_iRawDataSendIndexCallee += (m_nRawFrameSize + m_MyAudioHeadersize + 1);
				m_vRawFrameLengthCallee.push_back(m_nRawFrameSize + m_MyAudioHeadersize + 1);
			}
#else
			m_clientSocket->SendToServer(m_ucaCompressedFrame, m_nCompressedFrameSize + m_MyAudioHeadersize + 1);
#endif
		}
#if 0		
		else if (m_iRole == VIEWER_IN_CALL)
		{
#ifndef LOCAL_SERVER_LIVE_CALL
#ifndef NO_CONNECTIVITY			
			m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, MEDIA_TYPE_LIVE_CALL_AUDIO, m_ucaRawFrameNonMuxed, (m_nRawFrameSize / 2) + m_MyAudioHeadersize + 1, 0, std::vector< std::pair<int, int> >());	//Need to check send type.
#else
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, (m_nRawFrameSize / 2) + m_MyAudioHeadersize + 1, m_ucaRawFrameNonMuxed);
#endif
#else
			m_clientSocket->SendToServer(m_ucaCompressedFrame, m_nCompressedFrameSize + m_MyAudioHeadersize + 1);
#endif
		}
#endif
		else
		{
			Locker lock(*m_pAudioEncodingMutex);
			if ((m_iRawDataSendIndexViewer + m_nRawFrameSize + m_MyAudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
			{
				memcpy(m_ucaRawDataToSendViewer + m_iRawDataSendIndexViewer, m_ucaRawFrameNonMuxed, m_nRawFrameSize + m_MyAudioHeadersize + 1);
				m_iRawDataSendIndexViewer += (m_nRawFrameSize + m_MyAudioHeadersize + 1);
				m_vRawFrameLengthViewer.push_back(m_nRawFrameSize + m_MyAudioHeadersize + 1);
			}
		}

	}
	else
	{
#ifndef NO_CONNECTIVITY
		m_pCommonElementsBucket->SendFunctionPointer(m_llFriendID, MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, 0, std::vector< std::pair<int, int> >());
#else
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
#endif

#ifdef  __DUPLICATE_AUDIO__
		if (false == m_bIsLiveStreamingRunning && m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning())
		{
			toolsObject.SOSleep(5);
#ifndef NO_CONNECTIVITY
			m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, MEDIA_TYPE_AUDIO, m_ucaEncodedFrame, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, 0, std::vector< std::pair<int, int> >());
#else
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, m_nEncodedFrameSize + m_MyAudioHeadersize + 1, m_ucaEncodedFrame);
#endif
		}
#endif
	}

}

void CAudioNearEndDataProcessor::AddHeader(int &version, long long &llRelativeTime)
{
	m_iSlotID = m_iPacketNumber / AUDIO_SLOT_SIZE;
	m_iSlotID %= m_pAudioPacketHeader->GetFieldCapacity(INF_SLOTNUMBER);

	if (!m_bIsLiveStreamingRunning)
	{
		BuildAndGetHeaderInArray(m_iNextPacketType, m_MyAudioHeadersize, 0, m_iSlotID, m_iPacketNumber, m_nEncodedFrameSize,
			m_pAudioCallSession->m_iPrevRecvdSlotID, m_pAudioCallSession->m_iReceivedPacketsInPrevSlot, 0, version, llRelativeTime, &m_ucaEncodedFrame[1]);
	}
	else
	{
		if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL)
		{
			BuildAndGetHeaderInArray(AUDIO_NONMUXED_LIVE_CALL_PACKET_TYPE, m_MyAudioHeadersize, 0, m_iSlotID, m_iPacketNumber, m_nRawFrameSize,
				m_pAudioCallSession->m_iPrevRecvdSlotID, m_pAudioCallSession->m_iReceivedPacketsInPrevSlot, 0, version, llRelativeTime, &m_ucaRawFrameNonMuxed[1]);
			BuildAndGetHeaderInArray(AUDIO_MUXED_PACKET_TYPE, m_MyAudioHeadersize, 0, m_iSlotID, m_iPacketNumber, m_nRawFrameSize,
				m_pAudioCallSession->m_iPrevRecvdSlotID, m_pAudioCallSession->m_iReceivedPacketsInPrevSlot, 0, version, llRelativeTime, &m_ucaRawFrameMuxed[1]);
		}
		else if (m_pAudioCallSession->GetRole() == VIEWER_IN_CALL)
		{
			BuildAndGetHeaderInArray(AUDIO_LIVE_CALLEE_PACKET_TYPE, m_MyAudioHeadersize, 0, m_iSlotID, m_iPacketNumber, m_nRawFrameSize / 2,
				m_pAudioCallSession->m_iPrevRecvdSlotID, m_pAudioCallSession->m_iReceivedPacketsInPrevSlot, 0, version, llRelativeTime, &m_ucaRawFrameNonMuxed[1]);
		}
		else
		{
			BuildAndGetHeaderInArray(AUDIO_NONMUXED_LIVE_NONCALL_PACKET_TYPE, m_MyAudioHeadersize, 0, m_iSlotID, m_iPacketNumber, m_nRawFrameSize,
				m_pAudioCallSession->m_iPrevRecvdSlotID, m_pAudioCallSession->m_iReceivedPacketsInPrevSlot, 0, version, llRelativeTime, &m_ucaRawFrameNonMuxed[1]);
		}
	}

	m_ucaRawFrameNonMuxed[0] = 0;   //Setting Audio packet type( = 0).
	m_ucaRawFrameMuxed[0] = 0;   //Setting Audio packet type( = 0).
	m_ucaEncodedFrame[0] = 0;   //Setting Audio packet type( = 0).
}

void CAudioNearEndDataProcessor::BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
	int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header)
{
	//LOGEF("##EN### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
	//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);

	m_pAudioPacketHeader->SetInformation(packetType, INF_PACKETTYPE);
	m_pAudioPacketHeader->SetInformation(nHeaderLength, INF_HEADERLENGTH);
	m_pAudioPacketHeader->SetInformation(packetNumber, INF_PACKETNUMBER);
	m_pAudioPacketHeader->SetInformation(slotNumber, INF_SLOTNUMBER);
	m_pAudioPacketHeader->SetInformation(packetLength, INF_BLOCK_LENGTH);
	m_pAudioPacketHeader->SetInformation(recvSlotNumber, INF_RECVDSLOTNUMBER);
	m_pAudioPacketHeader->SetInformation(numPacketRecv, INF_NUMPACKETRECVD);
	m_pAudioPacketHeader->SetInformation(version, INF_VERSIONCODE);
	m_pAudioPacketHeader->SetInformation(timestamp, INF_TIMESTAMP);
	m_pAudioPacketHeader->SetInformation(networkType, INF_NETWORKTYPE);
	m_pAudioPacketHeader->SetInformation(channel, INF_CHANNELS);

	m_pAudioPacketHeader->SetInformation(0, INF_PACKET_BLOCK_NUMBER);
	m_pAudioPacketHeader->SetInformation(1, INF_TOTAL_PACKET_BLOCKS);
	m_pAudioPacketHeader->SetInformation(0, INF_BLOCK_OFFSET);
	m_pAudioPacketHeader->SetInformation(packetLength, INF_FRAME_LENGTH);

	m_pAudioPacketHeader->showDetails("@#BUILD");

	m_pAudioPacketHeader->GetHeaderInByteArray(header);
}

void CAudioNearEndDataProcessor::SetAudioIdentifierAndNextPacketType()
{
	++m_iPacketNumber;
	if (m_iPacketNumber == m_llMaxAudioPacketNumber)
	{
		m_iPacketNumber = 0;
	}

	if (false == m_bIsLiveStreamingRunning && m_iNextPacketType == AUDIO_NOVIDEO_PACKET_TYPE)
	{
		m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
	}
}

void CAudioNearEndDataProcessor::EncodeIfNeeded(long long &llCapturedTime, long long &encodingTime, double &avgCountTimeStamp)
{

	if (m_bIsLiveStreamingRunning == false)
	{
		m_nEncodedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), &m_ucaEncodedFrame[1 + m_MyAudioHeadersize]);

		//ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
		encodingTime = Tools::CurrentTimestamp() - llCapturedTime;
		m_pAudioCodec->DecideToChangeComplexity(encodingTime);
	}
	else
	{
		if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL)
		{
			m_nRawFrameSize = CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short);
			memcpy(&m_ucaRawFrameMuxed[1 + m_MyAudioHeadersize], m_saAudioMUXEDFrame, m_nRawFrameSize);
			memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saAudioRecorderFrame, m_nRawFrameSize);
		}
		else if (m_pAudioCallSession->GetRole() == VIEWER_IN_CALL)
		{
			m_nRawFrameSize = CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short);
			memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saAudioRecorderFrame, m_nRawFrameSize);
		}
		else //Should only work for PUBLISHER when CALL_NOT_RUNNING
		{
			m_nRawFrameSize = CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short);
			memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saAudioRecorderFrame, m_nRawFrameSize);
		}
	}
	avgCountTimeStamp += encodingTime;

	if (!m_bIsLiveStreamingRunning) {
#ifdef FIRE_ENC_TIME
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_ENCODING_TIME, encodingTime, 0);
		cumulitiveenctime += encodingTime;
		encodingtimetimes++;
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_AVG_ENCODING_TIME, cumulitiveenctime * 1.0 / encodingtimetimes, 0);
#endif
	}
	//            if (countFrame % 100 == 0)
	//                ALOG("#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize)
	//                     + " nEncodedFrameSize = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize) + " ratio: " + m_Tools.DoubleToString((nEncodedFrameSize * 100) / nEncodingFrameSize)
	//                     + " EncodeTime: " + m_Tools.IntegertoStringConvert(encodingTime)
	//                     + " AvgTime: " + m_Tools.DoubleToString(avgCountTimeStamp / countFrame)
	//                     + " MaxFrameNumber: " + m_Tools.IntegertoStringConvert(m_nMaxAudioPacketNumber));
}

bool CAudioNearEndDataProcessor::PreProcessAudioBeforeEncoding()
{
	//if (!m_bIsLiveStreamingRunning)
	{
#ifdef USE_VAD			
		if (!m_pVoice->HasVoice(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning)))
		{
			return false;
		}
#endif


#ifdef USE_AGC
		m_pAudioCallSession->m_pPlayerGain->AddFarEnd(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning));
		m_pAudioCallSession->m_pRecorderGain->AddGain(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), m_bIsLiveStreamingRunning);
#endif
		 

#ifdef USE_ANS
		memcpy(m_saAudioEncodingTempFrame, m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short));
		m_pNoise->Denoise(m_saAudioEncodingTempFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), m_saAudioEncodingDenoisedFrame, m_bIsLiveStreamingRunning);
#ifdef USE_AECM

		memcpy(m_saAudioRecorderFrame, m_saAudioEncodingDenoisedFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning));
#else
		memcpy(m_saAudioRecorderFrame, m_saAudioEncodingDenoisedFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning));
#endif
#endif
	}
	return true;
}

void CAudioNearEndDataProcessor::GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
	int &sendingLengthViewer, int &sendingLengthCallee)
{
	Locker lock(*m_pAudioEncodingMutex);

	vCombinedDataLengthVector = m_vRawFrameLengthViewer;
	m_vRawFrameLengthViewer.clear();
	memcpy(pAudioCombinedDataToSend, m_ucaRawDataToSendViewer, m_iRawDataSendIndexViewer);
	CombinedLength = m_iRawDataSendIndexViewer;
	sendingLengthViewer = m_iRawDataSendIndexViewer;
	LOGT("##NF##GetAudioData## viewerlength:%d calleelength:%d", m_iRawDataSendIndexViewer, m_iRawDataSendIndexCallee);

	if (m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL)
	{
		for (int a : m_vRawFrameLengthCallee)
		{
			vCombinedDataLengthVector.push_back(a);
		}
		m_vRawFrameLengthCallee.clear();
		memcpy(pAudioCombinedDataToSend + m_iRawDataSendIndexViewer, m_ucaRawDataToSendCallee, m_iRawDataSendIndexCallee);
		CombinedLength += m_iRawDataSendIndexCallee;
		sendingLengthCallee = m_iRawDataSendIndexCallee;
		m_iRawDataSendIndexCallee = 0;
	}

	m_iRawDataSendIndexViewer = 0;
}

void CAudioNearEndDataProcessor::PrintRelativeTime(int &cnt, long long &llLasstTime, int &countFrame, long long & llRelativeTime, long long & llCapturedTime)
{

	__LOG("#WQ Relative Time Counter: %d-------------------------------- NO: %lld\n", cnt, llCapturedTime - llLasstTime);
	cnt++;
	llLasstTime = llCapturedTime;
	countFrame++;
	llRelativeTime = llCapturedTime - m_llEncodingTimeStampOffset;
}


void CAudioNearEndDataProcessor::DumpEncodingFrame()
{
#ifdef __DUMP_FILE__
	fwrite(m_saAudioRecorderFrame, 2, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), FileInput);
#endif
}

void CAudioNearEndDataProcessor::MuxIfNeeded()
{
	long long lastDecodedTimeStamp;
	if (m_bIsLiveStreamingRunning && m_pAudioCallSession->GetRole() == PUBLISHER_IN_CALL)
	{
		int nLastDecodedFrameSize = 0;
		if (m_pAudioCallSession->m_AudioDecodedBuffer.GetQueueSize() != 0)
		{
			nLastDecodedFrameSize = m_pAudioCallSession->m_AudioDecodedBuffer.DeQueue(m_saAudioPrevDecodedFrame, lastDecodedTimeStamp);
			if (nLastDecodedFrameSize == CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning)) //Both must be 800
			{
				m_pAudioCallSession->MuxAudioData(m_saAudioRecorderFrame, m_saAudioPrevDecodedFrame, m_saAudioMUXEDFrame, nLastDecodedFrameSize);
			}
			else
			{
				nLastDecodedFrameSize = 0;
			}
		}
		if (nLastDecodedFrameSize == 0)
		{
			memcpy(m_saAudioMUXEDFrame, m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning) * sizeof(short));
		}
#ifdef __DUMP_FILE__
		fwrite(m_saAudioMUXEDFrame, sizeof(short), CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), FileInputMuxed);
#endif
	}
}

void CAudioNearEndDataProcessor::StopEncodingThread()
{
	m_bAudioEncodingThreadRunning = false;

	while (!m_bAudioEncodingThreadClosed)
		Tools::SOSleep(5);
}