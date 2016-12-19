#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "InterfaceOfAudioVideoEngine.h"

//#define __AUDIO_SELF_CALL__
//#define FIRE_ENC_TIME

#ifdef USE_AECM
#include "Echo.h"
#endif
#ifdef USE_ANS
#include "Noise.h"
#endif
#ifdef USE_AGC
#include "Gain.h"
#endif
#ifdef USE_VAD
#include "Voice.h"
#endif


#define PUBLISHER_IN_CALL 1
#define VIEWER_IN_CALL 2
#define VIEWER_NOT_IN_CALL 3 //never used so far
#define CALL_NOT_RUNNING 4


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define OPUS_ENABLE
//#define __DUMP_FILE__

#ifdef __DUMP_FILE__
FILE *FileInput;
FILE *FileOutput;
#endif


//#define USE_ECHO2

#define __TIMESTUMP_MOD__ 100000

#define __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ 0
#define __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ 2



CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, int nServiceType, bool bIsCheckCall) :
m_pCommonElementsBucket(pSharedObject),
m_bIsCheckCall(bIsCheckCall),
m_nServiceType(nServiceType)
{
	m_iRole = CALL_NOT_RUNNING;
	m_bLiveAudioStreamRunning = false;

	if (m_nServiceType == SERVICE_TYPE_LIVE_STREAM || m_nServiceType == SERVICE_TYPE_SELF_STREAM)
	{
		m_bLiveAudioStreamRunning = true;
		m_pLiveAudioReceivedQueue = new LiveAudioDecodingQueue();
		m_pLiveReceiverAudio = new LiveReceiver(m_pCommonElementsBucket);
		m_pLiveReceiverAudio->SetAudioDecodingQueue(m_pLiveAudioReceivedQueue);
	}


	m_pAudioCallSessionMutex.reset(new CLockHandler);
	m_FriendID = llFriendID;

	StartEncodingThread();
	StartDecodingThread();

	SendingHeader = new CAudioLiveHeader();
	ReceivingHeader = new CAudioLiveHeader();

	if (m_bLiveAudioStreamRunning)
	{
		m_AudioHeadersize = SendingHeader->GetHeaderSize();
	}
	else
	{
		m_AudioHeadersize = m_sendingHeaderOld.GetHeaderSize();
	}

	m_llEncodingTimeStampOffset = m_Tools.CurrentTimestamp();
	m_llDecodingTimeStampOffset = -1;
	m_iPacketNumber = 0;
	m_iLastDecodedPacketNumber = -1;
	m_iSlotID = 0;
	m_iPrevRecvdSlotID = -1;
	m_iCurrentRecvdSlotID = -1;
	m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;
	m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
	m_nCompressedFrameSize = 0;
	m_nRawFrameSize = 0;
	if (m_bLiveAudioStreamRunning)
	{
		m_llMaxAudioPacketNumber = ((1 << HeaderBitmap[PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;
	}
	else
	{
		m_llMaxAudioPacketNumber = ((1 << HeaderBitmapOld[PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;
	}

	if (m_bLiveAudioStreamRunning)
	{
		m_iRawDataSendIndex = 0;
		m_iCompressedDataSendIndex = 0;
		m_vRawFrameLength.clear();
	}

	m_bUsingLoudSpeaker = false;
	m_bEchoCancellerEnabled = false;

	m_bLoudSpeakerEnabled = false;

#ifdef USE_AECM
	m_pEcho = new CEcho(66);
#ifdef USE_ECHO2
	m_pEcho2 = new CEcho(77);
#endif
#endif

#ifdef USE_ANS
	m_pNoise = new CNoise();
#endif

#ifdef USE_AGC
	m_pRecorderGain = new CGain();
	m_pPlayerGain = new CGain();
#endif

#ifdef USE_VAD
	m_pVoice = new CVoice();
#endif


	m_iAudioVersionFriend = -1;
	m_iAudioVersionSelf = __AUDIO_CALL_VERSION__;


	CLogPrinter_Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
}

CAudioCallSession::~CAudioCallSession()
{
	StopDecodingThread();
	StopEncodingThread();

#ifdef OPUS_ENABLE
	delete m_pAudioCodec;
#else
	delete m_pG729CodecNative;
#endif
#ifdef USE_AECM
	delete m_pEcho;
#ifdef USE_ECHO2
	delete m_pEcho2;
#endif
#endif
#ifdef USE_ANS
	delete m_pNoise;
#endif
#ifdef USE_AGC
	delete m_pRecorderGain;
	delete m_pPlayerGain;
#endif
#ifdef USE_VAD
	delete m_pVoice;
#endif




	if (m_bLiveAudioStreamRunning)
	{
		if (NULL != m_pLiveReceiverAudio)
		{
			delete m_pLiveReceiverAudio;
			m_pLiveReceiverAudio = NULL;
		}

		if (NULL != m_pLiveAudioReceivedQueue)
		{
			delete m_pLiveAudioReceivedQueue;

			m_pLiveAudioReceivedQueue = NULL;
		}
	}



	m_FriendID = -1;
#ifdef __DUMP_FILE__
	fclose(FileOutput);
	fclose(FileInput);
#endif

	SHARED_PTR_DELETE(m_pAudioCallSessionMutex);
}

void CAudioCallSession::InitializeAudioCallSession(LongLong llFriendID)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");

	//this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket);

	//m_pAudioCodec->CreateAudioEncoder();

	//m_pAudioDecoder->CreateAudioDecoder();
#ifdef OPUS_ENABLE
	this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket, this, llFriendID);
	m_pAudioCodec->CreateAudioEncoder();
#else
	m_pG729CodecNative = new G729CodecNative();
	int iRet = m_pG729CodecNative->Open();
#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession session initialized, iRet = " + m_Tools.IntegertoStringConvert(iRet));

}


void CAudioCallSession::SetEchoCanceller(bool bOn)
{
#ifdef USE_AECM
	m_bEchoCancellerEnabled = bOn;
#endif
}


void CAudioCallSession::StartCallInLive(int iRole)
{
	m_iRole = iRole;
	m_llDecodingTimeStampOffset = -1;
}

void CAudioCallSession::EndCallInLive()
{
	m_iRole = CALL_NOT_RUNNING;
	m_llDecodingTimeStampOffset = -1;
	m_pLiveAudioReceivedQueue->ResetBuffer();
	m_AudioDecodingBuffer.ResetBuffer();
}

int CAudioCallSession::EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength)
{
	if (AUDIO_CLIENT_SAMPLES_IN_FRAME != unLength)
	{
		ALOG("Invalid Audio Frame Length");
		return -1;
	}
	//	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");
	long long llCurrentTime = Tools::CurrentTimestamp();

#ifdef USE_AECM
	if (!m_bLiveAudioStreamRunning && m_bEchoCancellerEnabled)
	{
#ifdef USE_ECHO2
		m_pEcho2->AddFarEnd(psaEncodingAudioData, unLength);
#endif
		m_pEcho->CancelEcho(psaEncodingAudioData, unLength);
	}
#endif

	int returnedValue = m_AudioEncodingBuffer.EnQueue(psaEncodingAudioData, unLength, llCurrentTime);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

	return returnedValue;
}

int CAudioCallSession::CancelAudioData(short *psaPlayingAudioData, unsigned int unLength)
{
#ifdef USE_AECM
	if (!m_bLiveAudioStreamRunning && m_bEchoCancellerEnabled)
	{
#ifdef USE_ECHO2
		m_pEcho2->CancelEcho(psaPlayingAudioData, unLength);
#endif
		m_pEcho->AddFarEnd(psaPlayingAudioData, unLength, m_bLoudSpeakerEnabled);
	}
#endif
	return true;
}

void CAudioCallSession::SetVolume(int iVolume, bool bRecorder)
{
#ifdef USE_AGC
	if (!m_bLiveAudioStreamRunning)
	{
		if (bRecorder)
		{
			m_pRecorderGain->SetGain(iVolume);
		}
		else
		{
			m_pPlayerGain->SetGain(iVolume);
		}
	}
#endif
}

void CAudioCallSession::SetLoudSpeaker(bool bOn)
{
#ifdef USE_AGC
	/*if (m_bUsingLoudSpeaker != bOn)
	{
	m_bUsingLoudSpeaker = bOn;
	if (bOn)
	{
	m_iVolume = m_iVolume * 1.0 / LS_RATIO;
	}
	else
	{
	m_iVolume *= LS_RATIO;
	}
	}*/
	//This method may be used in future.
#endif
	m_bUsingLoudSpeaker = bOn;
	/*
	#ifdef USE_AECM
	delete m_pEcho;
	m_pEcho = new CEcho(66);
	#ifdef USE_ECHO2
	delete m_pEcho2;
	m_pEcho2 = new CEcho(77);
	#endif
	#endif*/
}

int CAudioCallSession::DecodeAudioDataVector(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	//    ALOG("#H#Received PacketType: "+m_Tools.IntegertoStringConvert(pucaDecodingAudioData[0]));
	if (m_bLiveAudioStreamRunning)
	{
		m_pLiveReceiverAudio->ProcessAudioStreamVector(nOffset, pucaDecodingAudioData, unLength, frameSizes, numberOfFrames, vMissingFrames);
		return 1;
	}

	int returnedValue = m_AudioDecodingBuffer.EnQueue(pucaDecodingAudioData, unLength);

	return returnedValue;
}


int CAudioCallSession::DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
	//    ALOG("#H#Received PacketType: "+m_Tools.IntegertoStringConvert(pucaDecodingAudioData[0]));
	if (m_bLiveAudioStreamRunning)
	{
		m_pLiveReceiverAudio->ProcessAudioStream(nOffset, pucaDecodingAudioData, unLength, frameSizes, numberOfFrames, missingFrames, numberOfMissingFrames);

		return 1;
	}

	int returnedValue = m_AudioDecodingBuffer.EnQueue(pucaDecodingAudioData, unLength);

	return returnedValue;
}

CAudioCodec* CAudioCallSession::GetAudioCodec()
{
	return m_pAudioCodec;
}

void CAudioCallSession::StopEncodingThread()
{
	//if (pInternalThread.get())
	{
		m_bAudioEncodingThreadRunning = false;

		while (!m_bAudioEncodingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();
}

void CAudioCallSession::MuxAudioData(short * pData1, short * pData2, short * pMuxedData, int iDataLength)
{
	for (int i = 0; i < iDataLength; i++)
	{
		pMuxedData[i] = pData1[i] + pData2[i];
	}
}

void CAudioCallSession::StartEncodingThread()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartEncodingThread 1");

	if (m_pAudioEncodingThread.get())
	{
		m_pAudioEncodingThread.reset();

		return;
	}

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

void *CAudioCallSession::CreateAudioEncodingThread(void* param)
{
	CAudioCallSession *pThis = (CAudioCallSession*)param;
	pThis->EncodingThreadProcedure();

	return NULL;
}

#ifdef FIRE_ENC_TIME
int encodingtimetimes = 0, cumulitiveenctime = 0;
#endif

void CAudioCallSession::MuxIfNeeded()
{
	long long lastDecodedTimeStamp;
	if (m_bLiveAudioStreamRunning && m_iRole == PUBLISHER_IN_CALL)
	{
		int nLastDecodedFrameSize = 0;
		if (m_AudioDecodedBuffer.GetQueueSize() != 0)
		{
			nLastDecodedFrameSize = m_AudioDecodedBuffer.DeQueue(m_saAudioPrevDecodedFrame, lastDecodedTimeStamp);
			if (nLastDecodedFrameSize == AUDIO_CLIENT_SAMPLES_IN_FRAME) //Both must be 800
			{
				MuxAudioData(m_saAudioRecorderFrame, m_saAudioPrevDecodedFrame, m_saAudioMUXEDFrame, nLastDecodedFrameSize);
			}
			else
			{
				nLastDecodedFrameSize = 0;
			}
		}
		if (nLastDecodedFrameSize == 0)
		{
			memcpy(m_saAudioMUXEDFrame, m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME * sizeof(short));
		}
	}
}

void CAudioCallSession::DumpEncodingFrame()
{
#ifdef __DUMP_FILE__
	fwrite(m_saAudioRecorderFrame, 2, AUDIO_CLIENT_SAMPLES_IN_FRAME, FileInput);
#endif
}

void CAudioCallSession::PrintRelativeTime(int &cnt, long long &llLasstTime, int &countFrame, int & nCurrentTimeStamp, long long & timeStamp)
{

	__LOG("#WQ Relative Time Counter: %d-------------------------------- NO: %lld\n", cnt, timeStamp - llLasstTime);
	cnt++;
	llLasstTime = timeStamp;
	countFrame++;
	nCurrentTimeStamp = timeStamp - m_llEncodingTimeStampOffset;
}

bool CAudioCallSession::PreProcessAudioBeforeEncoding()
{
	if (!m_bLiveAudioStreamRunning)
	{
#ifdef USE_VAD			
		if (!m_pVoice->HasVoice(m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME))
		{
			return false;
		}
#endif


#ifdef USE_AGC
		m_pPlayerGain->AddFarEnd(m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
		m_pRecorderGain->AddGain(m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
#endif


#ifdef USE_ANS
		memcpy(m_saAudioEncodingTempFrame, m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME * sizeof(short));
		m_pNoise->Denoise(m_saAudioEncodingTempFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME, m_saAudioEncodingDenoisedFrame);
#ifdef USE_AECM

		memcpy(m_saAudioRecorderFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
#else
		memcpy(m_saAudioRecorderFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
#endif
#endif
	}
	return true;
}

void CAudioCallSession::EncodeIfNeeded(long long &timeStamp, int &encodingTime, double &avgCountTimeStamp)
{
#ifdef OPUS_ENABLE
	if (m_bLiveAudioStreamRunning == false)
	{
		m_nCompressedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME, &m_ucaCompressedFrame[1 + m_AudioHeadersize]);

		ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
		encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
		m_pAudioCodec->DecideToChangeComplexity(encodingTime);
	}
	else 
	{
		if (m_iRole == PUBLISHER_IN_CALL)
		{
			m_nRawFrameSize = AUDIO_CLIENT_SAMPLES_IN_FRAME * sizeof(short);
			memcpy(&m_ucaRawFrame[1 + m_AudioHeadersize], m_saAudioMUXEDFrame, m_nRawFrameSize);

			m_nCompressedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME, &m_ucaCompressedFrame[1 + m_AudioHeadersize]);

			ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(AUDIO_CLIENT_SAMPLES_IN_FRAME) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
			encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
			m_pAudioCodec->DecideToChangeComplexity(encodingTime);
			
		}
		else if (m_iRole == VIEWER_IN_CALL)
		{
			m_nCompressedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioRecorderFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME, &m_ucaCompressedFrame[1 + m_AudioHeadersize]);

			ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(AUDIO_CLIENT_SAMPLES_IN_FRAME) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
			encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
			m_pAudioCodec->DecideToChangeComplexity(encodingTime);
		}
		else //Should only work for PUBLISHER when CALL_NOT_RUNNING
		{
			m_nRawFrameSize = AUDIO_CLIENT_SAMPLES_IN_FRAME * sizeof(short);
			memcpy(&m_ucaRawFrame[1 + m_AudioHeadersize], m_saAudioRecorderFrame, m_nRawFrameSize);			
		}
	}
	avgCountTimeStamp += encodingTime;

	if (!m_bLiveAudioStreamRunning) {
#ifdef FIRE_ENC_TIME
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_ENCODING_TIME, encodingTime, 0);
		cumulitiveenctime += encodingTime;
		encodingtimetimes++;
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_AVG_ENCODING_TIME, cumulitiveenctime * 1.0 / encodingtimetimes, 0);
#endif
	}
#else
	nEncodedFrameSize = m_pG729CodecNative->Encode(m_saAudioRecorderFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
	encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
	avgCountTimeStamp += encodingTime;
#endif
	//            if (countFrame % 100 == 0)
	//                ALOG("#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize)
	//                     + " nEncodedFrameSize = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize) + " ratio: " + m_Tools.DoubleToString((nEncodedFrameSize * 100) / nEncodingFrameSize)
	//                     + " EncodeTime: " + m_Tools.IntegertoStringConvert(encodingTime)
	//                     + " AvgTime: " + m_Tools.DoubleToString(avgCountTimeStamp / countFrame)
	//                     + " MaxFrameNumber: " + m_Tools.IntegertoStringConvert(m_nMaxAudioPacketNumber));
}

void CAudioCallSession::AddHeader(int &version, int &nCurrentTimeStamp)
{
	m_iSlotID = m_iPacketNumber / AUDIO_SLOT_SIZE;
	m_iSlotID %= SendingHeader->GetFieldCapacity(SLOTNUMBER);

	if (!m_bLiveAudioStreamRunning)
	{
		BuildAndGetHeaderInArray(m_iNextPacketType, 0, m_iSlotID, m_iPacketNumber, m_nCompressedFrameSize,
			m_iPrevRecvdSlotID, m_iReceivedPacketsInPrevSlot, 0, version, nCurrentTimeStamp, &m_ucaCompressedFrame[1]);
	}
	else
	{
		if (m_iRole == PUBLISHER_IN_CALL)
		{
			BuildAndGetHeaderInArray(AUDIO_OPUS_PACKET_TYPE, 0, m_iSlotID, m_iPacketNumber, m_nCompressedFrameSize,
				m_iPrevRecvdSlotID, m_iReceivedPacketsInPrevSlot, 0, version, nCurrentTimeStamp, &m_ucaCompressedFrame[1]);
			BuildAndGetHeaderInArray(AUDIO_MUXED_PACKET_TYPE, 0, m_iSlotID, m_iPacketNumber, m_nRawFrameSize,
				m_iPrevRecvdSlotID, m_iReceivedPacketsInPrevSlot, 0, version, nCurrentTimeStamp, &m_ucaRawFrame[1]);
		}
		else if (m_iRole == VIEWER_IN_CALL)
		{
			BuildAndGetHeaderInArray(AUDIO_OPUS_PACKET_TYPE, 0, m_iSlotID, m_iPacketNumber, m_nCompressedFrameSize,
				m_iPrevRecvdSlotID, m_iReceivedPacketsInPrevSlot, 0, version, nCurrentTimeStamp, &m_ucaCompressedFrame[1]);
		}
		else
		{
			BuildAndGetHeaderInArray(AUDIO_NONMUXED_PACKET_TYPE, 0, m_iSlotID, m_iPacketNumber, m_nRawFrameSize,
				m_iPrevRecvdSlotID, m_iReceivedPacketsInPrevSlot, 0, version, nCurrentTimeStamp, &m_ucaRawFrame[1]);
		}
	}
}

void CAudioCallSession::SetAudioIdentifierAndNextPacketType()
{
	m_ucaRawFrame[0] = 0;   //Setting Audio packet type( = 0).
	m_ucaCompressedFrame[0] = 0;   //Setting Audio packet type( = 0).

	++m_iPacketNumber;
	if (m_iPacketNumber == m_llMaxAudioPacketNumber)
	{
		m_iPacketNumber = 0;
	}

	if (false == m_bLiveAudioStreamRunning && m_iNextPacketType == AUDIO_NOVIDEO_PACKET_TYPE)
	{
		m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
	}
}

void CAudioCallSession::SendAudioData()
{
#ifdef  __AUDIO_SELF_CALL__
	if (m_bLiveAudioStreamRunning == false)
	{
		ALOG("#A#EN#--->> Self#  PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
		DecodeAudioData(0, m_ucaCompressedFrame + 1, m_nCompressedFrameSize + m_AudioHeadersize);
		return;
	}
#endif
	if (m_bIsCheckCall != LIVE_CALL_MOOD)	//Capability test call
	{
		return;
	}


	if (m_bLiveAudioStreamRunning)
	{
		if (m_iRole == PUBLISHER_IN_CALL)
		{
			Locker lock(*m_pAudioCallSessionMutex);
			if ((m_iRawDataSendIndex + m_nRawFrameSize + m_AudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
			{

				memcpy(m_ucaRawDataToSend + m_iRawDataSendIndex, m_ucaRawFrame, m_nRawFrameSize + m_AudioHeadersize + 1);
				m_iRawDataSendIndex += (m_nRawFrameSize + m_AudioHeadersize + 1);
				m_vRawFrameLength.push_back(m_nRawFrameSize + m_AudioHeadersize + 1);
			}

			if ((m_iCompressedDataSendIndex + m_nCompressedFrameSize + m_AudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
			{
				memcpy(m_ucaCompressedDataToSend + m_iCompressedDataSendIndex, m_ucaCompressedFrame, m_nCompressedFrameSize + m_AudioHeadersize + 1);
				m_iCompressedDataSendIndex += (m_nCompressedFrameSize + m_AudioHeadersize + 1);
				m_vCompressedFrameLength.push_back(m_nCompressedFrameSize + m_AudioHeadersize + 1);
			}
		}
		else if (m_iRole == VIEWER_IN_CALL)
		{
#ifndef LOCAL_SERVER_LIVE
			m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaCompressedFrame, m_nCompressedFrameSize + m_AudioHeadersize + 1, 0);	//Need to check send type.
#else
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, m_nCompressedFrameSize + m_AudioHeadersize + 1, m_ucaCompressedFrame);
#endif
		}
		else
		{
			Locker lock(*m_pAudioCallSessionMutex);
			if ((m_iRawDataSendIndex + m_nRawFrameSize + m_AudioHeadersize + 1) < MAX_AUDIO_DATA_TO_SEND_SIZE)
			{
				memcpy(m_ucaRawDataToSend + m_iRawDataSendIndex, m_ucaRawFrame, m_nRawFrameSize + m_AudioHeadersize + 1);
				m_iRawDataSendIndex += (m_nRawFrameSize + m_AudioHeadersize + 1);
				m_vRawFrameLength.push_back(m_nRawFrameSize + m_AudioHeadersize + 1);
			}
		}

	}
	else
	{
		m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaCompressedFrame, m_nCompressedFrameSize + m_AudioHeadersize + 1, 0);

#ifdef  __DUPLICATE_AUDIO__
#ifndef MULTIPLE_HEADER
		if (false == m_bLiveAudioStreamRunning && m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning())
		{
			toolsObject.SOSleep(5);
			m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1, 0);
			//                    ALOG("#2AE# Sent Second Times");
		}
#endif
#endif
	}
	
}

void CAudioCallSession::EncodingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
#ifdef __DUMP_FILE__
	FileInput = fopen("/storage/emulated/0/InputPCMN.pcm", "w");
	//    FileInput = fopen("/stcard/emulated/0/InputPCM.pcm", "w");
#endif
	Tools toolsObject;
	int encodingTime = 0;
	long long timeStamp = 0;
	double avgCountTimeStamp = 0;
	int countFrame = 0;
	int version = 0;
	int nCurrentTimeStamp;

	long long currentTime = m_Tools.CurrentTimestamp() / 1000;
	//StartCallInLive(PUBLISHER_IN_CALL);

	long long llLasstTime = -1;
	int cnt = 1;
	while (m_bAudioEncodingThreadRunning)
	{
		if (m_AudioEncodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			m_AudioEncodingBuffer.DeQueue(m_saAudioRecorderFrame, timeStamp);
			MuxIfNeeded();
			DumpEncodingFrame();
			PrintRelativeTime(cnt, llLasstTime, countFrame, nCurrentTimeStamp, timeStamp);

			if (false == PreProcessAudioBeforeEncoding())
			{
				continue;
			}

			EncodeIfNeeded(timeStamp, encodingTime, avgCountTimeStamp);
			AddHeader(version, nCurrentTimeStamp);
			SetAudioIdentifierAndNextPacketType();
			SendAudioData();
			toolsObject.SOSleep(0);
		}
	}

	m_bAudioEncodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

void CAudioCallSession::StopDecodingThread()
{
	//if (m_pAudioDecodingThread.get())
	{
		m_bAudioDecodingThreadRunning = false;

		while (!m_bAudioDecodingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//m_pAudioDecodingThread.reset();
}

void CAudioCallSession::StartDecodingThread()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::StartDecodingThread 1");

	if (m_pAudioDecodingThread.get())
	{
		m_pAudioDecodingThread.reset();

		return;
	}

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

void *CAudioCallSession::CreateAudioDecodingThread(void* param)
{
	CAudioCallSession *pThis = (CAudioCallSession*)param;
	pThis->DecodingThreadProcedure();

	return NULL;
}

bool CAudioCallSession::IsQueueEmpty(Tools &toolsObject)
{
	if (m_bLiveAudioStreamRunning && m_iRole != PUBLISHER_IN_CALL && m_pLiveAudioReceivedQueue->GetQueueSize() == 0)
	{
		toolsObject.SOSleep(1);
		return true;
	}
	else if (m_bLiveAudioStreamRunning && m_iRole == PUBLISHER_IN_CALL && m_AudioDecodingBuffer.GetQueueSize() == 0)
	{
		toolsObject.SOSleep(1);
		return true;
	}
	else if (false == m_bLiveAudioStreamRunning && m_AudioDecodingBuffer.GetQueueSize() == 0)
	{
		toolsObject.SOSleep(10);
		return true;
	}
	return false;
}

void CAudioCallSession::DequeueData(int &nDecodingFrameSize)
{
	if (m_bLiveAudioStreamRunning)
	{
		if (m_iRole != PUBLISHER_IN_CALL)
		{
			nDecodingFrameSize = m_pLiveAudioReceivedQueue->DeQueue(m_ucaDecodingFrame);
		}
		else
		{
			nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);
		}
	}
	else
	{
		nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);
	}
}

bool CAudioCallSession::IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType)
{
	if (m_bLiveAudioStreamRunning)
	{
		if (m_iRole == VIEWER_IN_CALL && nCurrentAudioPacketType != AUDIO_OPUS_PACKET_TYPE)
		{
			return false;
		}
		else if (m_iRole != VIEWER_IN_CALL && nCurrentAudioPacketType == AUDIO_OPUS_PACKET_TYPE)
		{
			return false;
		}
	}
	return true;
}


bool CAudioCallSession::IsPacketNumberProcessable(int &iPacketNumber)
{
#ifdef  __DUPLICATE_AUDIO__
#ifdef MULTIPLE_HEADER
	if (false == m_bLiveAudioStreamRunning && m_iLastDecodedPacketNumber == iPacketNumber) {
		LOGEF("Role %d, pack=ipack", m_iRole);
		return false;
	}
#else
	//iPacketNumber rotates
	if (false == m_bLiveAudioStreamRunning && m_iLastDecodedPacketNumber >= iPacketNumber) {
		return false;
	}
#endif
	ALOG("#2A#RCV---------> Decoding = " + m_Tools.IntegertoStringConvert(iPacketNumber));
#endif
	return true;
}

bool CAudioCallSession::IsPacketTypeProcessable(int &nCurrentAudioPacketType, int &nVersion, Tools &toolsObject)
{
	bool bIsProcessablePacket;
	if (AUDIO_SKIP_PACKET_TYPE == nCurrentAudioPacketType)
	{
		//ALOG("#V#TYPE# ############################################### SKIPPET");
		toolsObject.SOSleep(0);
		bIsProcessablePacket = false;
	}
	else if (AUDIO_NOVIDEO_PACKET_TYPE == nCurrentAudioPacketType)
	{
		//g_StopVideoSending = 1;*/
		if (false == m_bLiveAudioStreamRunning)
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
		bIsProcessablePacket = true;
	}
	else if (AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType ||
		AUDIO_OPUS_PACKET_TYPE == nCurrentAudioPacketType ||
		AUDIO_NONMUXED_PACKET_TYPE == nCurrentAudioPacketType ||
		AUDIO_MUXED_PACKET_TYPE == nCurrentAudioPacketType)
	{
		bIsProcessablePacket = true;
#ifdef  __DUPLICATE_AUDIO__
		if (false == m_bLiveAudioStreamRunning) {
			m_iAudioVersionFriend = nVersion;
		}
#endif
	}
	return bIsProcessablePacket;
}

void CAudioCallSession::DecodeAndPostProcessIfNeeded()
{
	LOGEF("Role %d, Before decode", m_iRole);
	if (!m_bLiveAudioStreamRunning)
	{
#ifdef OPUS_ENABLE
		nDecodedFrameSize = m_pAudioCodec->decodeAudio(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
		ALOG("#A#DE#--->> Self#  PacketNumber = " + m_Tools.IntegertoStringConvert(iPacketNumber));
		LOGEF("Role %d, done decode", m_iRole);

#else
		nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
#endif


#ifdef USE_AGC
		m_pRecorderGain->AddFarEnd(m_saDecodedFrame, nDecodedFrameSize);
		m_pPlayerGain->AddGain(m_saDecodedFrame, nDecodedFrameSize);
#endif
	}
	else
	{

		if (m_iRole != VIEWER_IN_CALL && m_iRole != PUBLISHER_IN_CALL)
		{
			memcpy(m_saDecodedFrame, m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize);
			nDecodedFrameSize = nDecodingFrameSize / sizeof(short);
			LOGEF("Role %d, no viewers in call", m_iRole);
		}
		else
		{
#ifdef OPUS_ENABLE
			nDecodedFrameSize = m_pAudioCodec->decodeAudio(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
			ALOG("#A#DE#--->> Self#  PacketNumber = " + m_Tools.IntegertoStringConvert(iPacketNumber));
			LOGEF("Role %d, after decode", m_iRole);
#else
			nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
#endif
		}
	}
}

void CAudioCallSession::DumpDecodedFrame()
{
#ifdef __DUMP_FILE__
	fwrite(m_saDecodedFrame, 2, nDecodedFrameSize, FileOutput);
#endif
}

void CAudioCallSession::PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
	int &iFrameCounter, long long &nDecodingTime, double &dbTotalTime, long long &timeStamp)
{
	if (!m_bLiveAudioStreamRunning)
	{
		llNow = m_Tools.CurrentTimestamp();
		//            ALOG("#DS Size: "+m_Tools.IntegertoStringConvert(nDecodedFrameSize));
		if (llNow - llTimeStamp >= 1000)
		{
			//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Num AudioDataDecoded = " + m_Tools.IntegertoStringConvert(iDataSentInCurrentSec));
			iDataSentInCurrentSec = 0;
			llTimeStamp = llNow;
		}
		iDataSentInCurrentSec++;

		++iFrameCounter;
		nDecodingTime = m_Tools.CurrentTimestamp() - timeStamp;
		dbTotalTime += nDecodingTime;
		//            if(iFrameCounter % 100 == 0)
		//                ALOG( "#DE#--->> Size " + m_Tools.IntegertoStringConvert(nDecodedFrameSize) + " DecodingTime: "+ m_Tools.IntegertoStringConvert(nDecodingTime) + "A.D.Time : "+m_Tools.DoubleToString(dbTotalTime / iFrameCounter));
	}
}

void CAudioCallSession::SendToPlayer(long long &llNow, long long &llLastTime)
{
	if (m_bLiveAudioStreamRunning == true)
	{

		llNow = Tools::CurrentTimestamp();

		__LOG("!@@@@@@@@@@@  #WQ     FN: %d -------- Receiver Time Diff : %lld    DataLength: %d", iPacketNumber, llNow - llLastTime, nDecodedFrameSize);

		llLastTime = llNow;
		if (m_iRole == PUBLISHER_IN_CALL)
		{
			m_AudioDecodedBuffer.EnQueue(m_saDecodedFrame, nDecodedFrameSize, 0);
		}
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_FriendID,
			SERVICE_TYPE_LIVE_STREAM,
			nDecodedFrameSize,
			m_saDecodedFrame);
	}
	else
	{
		m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_FriendID, SERVICE_TYPE_CALL, nDecodedFrameSize, m_saDecodedFrame);
	}

}

void CAudioCallSession::DecodingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Started DecodingThreadProcedure method.");

	Tools toolsObject;
	bool bIsProcessablePacket;
	int iFrameCounter = 0, nCurrentAudioPacketType, iPacketNumber;
	long long timeStamp, nDecodingTime = 0;
	double dbTotalTime = 0;

	int iDataSentInCurrentSec = 0, iPlaiedFrameCounter = 0;
	long long llTimeStamp = 0;
	long long llNow = 0;
	long long llExpectedEncodingTimeStamp = 0, llWaitingTime = 0;
	long long llLastDecodedTime = -1;
	long long llLastDecodedFrameEncodedTimeStamp = -1;
	long long llFirstSentTime = -1;
	long long llFirstSentFrame = -1;

#ifdef __DUMP_FILE__
	FileOutput = fopen("/storage/emulated/0/OutputPCMN.pcm", "w");
#endif

	long long llLastTime = -1, llDiffTimeNow = -1;
	long long iTimeStampOffset = 0;
	int TempOffset = 0;
	long long currentTime = m_Tools.CurrentTimestamp() / 1000;
	while (m_bAudioDecodingThreadRunning)
	{		
		if (!IsQueueEmpty(toolsObject))
		{
			DequeueData(nDecodingFrameSize);
			timeStamp = m_Tools.CurrentTimestamp();

			int dummy;
			int nSlotNumber, packetLength, recvdSlotNumber, nChannel, nVersion;
			ParseHeaderAndGetValues(nCurrentAudioPacketType, dummy, nSlotNumber, iPacketNumber, dummy, recvdSlotNumber, m_iOpponentReceivedPackets,
				nChannel, nVersion, iTimeStampOffset, m_ucaDecodingFrame);
			LOGEF("Role %d packettype %d", m_iRole, nCurrentAudioPacketType);

			if (!IsPacketProcessableBasedOnRole(nCurrentAudioPacketType))
			{
				continue;
			}

			if (!IsPacketNumberProcessable(iPacketNumber))
			{
				continue;
			}

			if (!ReceivingHeader->IsPacketTypeSupported(nCurrentAudioPacketType))
			{
				continue;
			}

			if (!IsPacketTypeProcessable(nCurrentAudioPacketType, nVersion, toolsObject))
			{
				continue;
			}

			if (!IsPacketProcessableBasedOnRelativeTime(iTimeStampOffset))
			{
				continue;
			}


			llNow = m_Tools.CurrentTimestamp();

			SetSlotStatesAndDecideToChangeBitRate(iPacketNumber, nSlotNumber);

			nDecodingFrameSize -= m_AudioHeadersize;

			DecodeAndPostProcessIfNeeded();
			DumpDecodedFrame();
			PrintDecodingTimeStats(llNow, llTimeStamp, iDataSentInCurrentSec, iFrameCounter, nDecodingTime, dbTotalTime, timeStamp);

			if (nDecodedFrameSize < 1)
			{
				ALOG("#EXP# Decoding Failed.");
				continue;
			}

			SendToPlayer(llNow, llLastTime);
			toolsObject.SOSleep(0);
		}
	}

	m_bAudioDecodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}

bool CAudioCallSession::IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime)
{
	if (m_bLiveAudioStreamRunning)
	{
		if (-1 == m_llDecodingTimeStampOffset)
		{
			m_Tools.SOSleep(__LIVE_FIRST_FRAME_SLEEP_TIME__);
			m_llDecodingTimeStampOffset = m_Tools.CurrentTimestamp() - llCurrentFrameRelativeTime;
		}
		else
		{
			long long llNow = m_Tools.CurrentTimestamp();
			long long llExpectedEncodingTimeStamp = llNow - m_llDecodingTimeStampOffset;
			long long llWaitingTime = llCurrentFrameRelativeTime - llExpectedEncodingTimeStamp;

			if (llExpectedEncodingTimeStamp - __AUDIO_DELAY_TIMESTAMP_TOLERANCE__ > llCurrentFrameRelativeTime) {
				__LOG("@@@@@@@@@@@@@@@@@--> New*********************************************** FrameNumber: %d [%lld]\t\tDELAY FRAME: %lld  Now: %lld", iPacketNumber, iTimeStampOffset, llWaitingTime, llNow % __TIMESTUMP_MOD__);
				return false;
			}

			while (llExpectedEncodingTimeStamp + __AUDIO_PLAY_TIMESTAMP_TOLERANCE__ < llCurrentFrameRelativeTime)
			{
				m_Tools.SOSleep(1);
				llExpectedEncodingTimeStamp = m_Tools.CurrentTimestamp() - m_llDecodingTimeStampOffset;
			}
		}
		return true;
	}
	else
	{
		return true;
	}
}

void CAudioCallSession::SetSlotStatesAndDecideToChangeBitRate(int &iPacketNumber, int &nSlotNumber)
{
	if (nSlotNumber != m_iCurrentRecvdSlotID)
	{
		m_iPrevRecvdSlotID = m_iCurrentRecvdSlotID;
		if (m_iPrevRecvdSlotID != -1)
		{
			m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot;
		}

		m_iCurrentRecvdSlotID = nSlotNumber;
		m_iReceivedPacketsInCurrentSlot = 0;

#ifdef OPUS_ENABLE
		if (false == m_bLiveAudioStreamRunning) {
			m_pAudioCodec->DecideToChangeBitrate(m_iOpponentReceivedPackets);
		}
#endif
	}

	m_iLastDecodedPacketNumber = iPacketNumber;
	m_iReceivedPacketsInCurrentSlot++;
}

void CAudioCallSession::GetAudioSendToData(unsigned char * pAudioRawDataToSend, int &RawLength, std::vector<int> &vRawDataLengthVector,
	std::vector<int> &vCompressedDataLengthVector, int &CompressedLength, unsigned char * pAudioCompressedDataToSend = 0)
{
	Locker lock(*m_pAudioCallSessionMutex);

	vRawDataLengthVector = m_vRawFrameLength;
	m_vRawFrameLength.clear();
	memcpy(pAudioRawDataToSend, m_ucaRawDataToSend, m_iRawDataSendIndex);
	RawLength = m_iRawDataSendIndex;
	m_iRawDataSendIndex = 0;

	vCompressedDataLengthVector = m_vCompressedFrameLength;
	m_vCompressedFrameLength.clear();
	memcpy(pAudioCompressedDataToSend, m_ucaCompressedDataToSend, m_iCompressedDataSendIndex);
	CompressedLength = m_iCompressedDataSendIndex;
	m_iCompressedDataSendIndex = 0;
}

void CAudioCallSession::GetAudioSendToData(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector)
{
	Locker lock(*m_pAudioCallSessionMutex);

	vCombinedDataLengthVector = m_vRawFrameLength;
	m_vRawFrameLength.clear();
	memcpy(pAudioCombinedDataToSend, m_ucaRawDataToSend, m_iRawDataSendIndex);
	CombinedLength = m_iRawDataSendIndex;

	if (m_iRole == PUBLISHER_IN_CALL)
	{
		for (int a : m_vCompressedFrameLength)
		{
			vCombinedDataLengthVector.push_back(a);
		}
		m_vCompressedFrameLength.clear();
		memcpy(pAudioCombinedDataToSend + m_iRawDataSendIndex, m_ucaCompressedDataToSend, m_iCompressedDataSendIndex);
		CombinedLength += m_iCompressedDataSendIndex;
		m_iCompressedDataSendIndex = 0;
	}
	m_iRawDataSendIndex = 0;
}


int CAudioCallSession::GetServiceType()
{
	return m_nServiceType;
}

void CAudioCallSession::BuildAndGetHeaderInArray(int packetType, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
	int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header)
{
	//LOGEF("##EN### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
	//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);

	if (m_bLiveAudioStreamRunning)
	{
		SendingHeader->SetInformation(packetType, PACKETTYPE);
		SendingHeader->SetInformation(packetNumber, PACKETNUMBER);
		SendingHeader->SetInformation(slotNumber, SLOTNUMBER);
		SendingHeader->SetInformation(packetLength, PACKETLENGTH);
		SendingHeader->SetInformation(recvSlotNumber, RECVDSLOTNUMBER);
		SendingHeader->SetInformation(numPacketRecv, NUMPACKETRECVD);
		SendingHeader->SetInformation(version, VERSIONCODE);
		SendingHeader->SetInformation(timestamp, TIMESTAMP);
		SendingHeader->SetInformation(networkType, NETWORKTYPE);
		SendingHeader->SetInformation(channel, CHANNELS);

		SendingHeader->GetHeaderInByteArray(header);
	}
	else
	{
		m_sendingHeaderOld.SetInformation(packetType, PACKETTYPE);
		m_sendingHeaderOld.SetInformation(packetNumber, PACKETNUMBER);
		m_sendingHeaderOld.SetInformation(slotNumber, SLOTNUMBER);
		m_sendingHeaderOld.SetInformation(packetLength, PACKETLENGTH);
		m_sendingHeaderOld.SetInformation(recvSlotNumber, RECVDSLOTNUMBER);
		m_sendingHeaderOld.SetInformation(numPacketRecv, NUMPACKETRECVD);
		m_sendingHeaderOld.SetInformation(version, VERSIONCODE);
		m_sendingHeaderOld.SetInformation(networkType, NETWORKTYPE);
		m_sendingHeaderOld.SetInformation(channel, CHANNELS);

		m_sendingHeaderOld.GetHeaderInByteArray(header);
	}
}

void CAudioCallSession::ParseHeaderAndGetValues(int &packetType, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
	int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header)
{
	if (m_bLiveAudioStreamRunning)
	{
		ReceivingHeader->CopyHeaderToInformation(header);

		packetType = ReceivingHeader->GetInformation(PACKETTYPE);
		networkType = ReceivingHeader->GetInformation(NETWORKTYPE);
		slotNumber = ReceivingHeader->GetInformation(SLOTNUMBER);
		packetNumber = ReceivingHeader->GetInformation(PACKETNUMBER);
		packetLength = ReceivingHeader->GetInformation(PACKETLENGTH);
		recvSlotNumber = ReceivingHeader->GetInformation(RECVDSLOTNUMBER);
		numPacketRecv = ReceivingHeader->GetInformation(NUMPACKETRECVD);
		channel = ReceivingHeader->GetInformation(CHANNELS);
		version = ReceivingHeader->GetInformation(VERSIONCODE);
		timestamp = ReceivingHeader->GetInformation(TIMESTAMP);
	}
	else
	{
		m_receivingHeaderOld.CopyHeaderToInformation(header);

		packetType = m_receivingHeaderOld.GetInformation(PACKETTYPE);
		networkType = m_receivingHeaderOld.GetInformation(NETWORKTYPE);
		slotNumber = m_receivingHeaderOld.GetInformation(SLOTNUMBER);
		packetNumber = m_receivingHeaderOld.GetInformation(PACKETNUMBER);
		packetLength = m_receivingHeaderOld.GetInformation(PACKETLENGTH);
		recvSlotNumber = m_receivingHeaderOld.GetInformation(RECVDSLOTNUMBER);
		numPacketRecv = m_receivingHeaderOld.GetInformation(NUMPACKETRECVD);
		channel = m_receivingHeaderOld.GetInformation(CHANNELS);
		version = m_receivingHeaderOld.GetInformation(VERSIONCODE);
	}

	//LOGEF("##DE### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
	//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);
}
