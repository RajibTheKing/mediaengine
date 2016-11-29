#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"

//#define __AUDIO_SELF_CALL__
//#define FIRE_ENC_TIME


#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

#define OPUS_ENABLE
//#define __DUMP_FILE__

#ifdef __DUMP_FILE__
FILE *FileInput;
FILE *FileOutput;
#endif

//extern int g_StopVideoSending;



CAudioCallSession::CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, bool bIsCheckCall) :
m_pCommonElementsBucket(pSharedObject),
m_bIsCheckCall(bIsCheckCall)

{
	m_pAudioCallSessionMutex.reset(new CLockHandler);
	m_FriendID = llFriendID;

	StartEncodingThread();
	StartDecodingThread();

	SendingHeader = new CAudioPacketHeader();
	ReceivingHeader = new CAudioPacketHeader();
	m_AudioHeadersize = SendingHeader->GetHeaderSize();

	m_iPacketNumber = 0;
	m_iLastDecodedPacketNumber = -1;
	m_iSlotID = 0;
	m_iPrevRecvdSlotID = -1;
	m_iCurrentRecvdSlotID = -1;
	m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;
	m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;
	m_nMaxAudioPacketNumber = ((1 << HeaderBitmap[PACKETNUMBER]) / AUDIO_SLOT_SIZE) * AUDIO_SLOT_SIZE;
	m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;

	m_bUsingLoudSpeaker = false;
	m_bEchoCancellerEnabled = false;



#ifdef USE_AECM
	m_bNoDataFromFarendYet = true;
	m_pEcho = new CEcho();
	m_pEcho2 = new CEcho();
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
	delete m_pEcho2;
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
	this->m_pAudioCodec = new CAudioCodec(m_pCommonElementsBucket, this);
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

int CAudioCallSession::EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");

	int returnedValue = m_AudioEncodingBuffer.Queue(psaEncodingAudioData, unLength);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

	return returnedValue;
}

void CAudioCallSession::SetVolume(int iVolume, bool bRecorder)
{
#ifdef USE_AGC
	if (bRecorder)
	{
		m_pRecorderGain->SetGain(iVolume);
	}
	else
	{
		m_pPlayerGain->SetGain(iVolume);
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
}

int CAudioCallSession::DecodeAudioData(unsigned char *pucaDecodingAudioData, unsigned int unLength)
{
	//    ALOG("#H#Received PacketType: "+m_Tools.IntegertoStringConvert(pucaDecodingAudioData[0]));
	int returnedValue = m_AudioDecodingBuffer.Queue(&pucaDecodingAudioData[1], unLength - 1);

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

void CAudioCallSession::EncodingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
#ifdef __DUMP_FILE__
	FileInput = fopen("/storage/emulated/0/InputPCMN.pcm", "w");
	//    FileInput = fopen("/stcard/emulated/0/InputPCM.pcm", "w");
#endif
	Tools toolsObject;
	int nEncodingFrameSize, nEncodedFrameSize, encodingTime;
	long long timeStamp;
	double avgCountTimeStamp = 0;
	int countFrame = 0;
    int version = __AUDIO_CALL_VERSION__;

	while (m_bAudioEncodingThreadRunning)
	{
		if (m_AudioEncodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			nEncodingFrameSize = m_AudioEncodingBuffer.DeQueue(m_saAudioEncodingFrame);
			if (nEncodingFrameSize % AUDIO_FRAME_SIZE >0)
			{
				ALOG("#EXP# Client Sample Size not multiple of AUDIO-FRAME-SIZE = " + Tools::IntegertoStringConvert(nEncodingFrameSize));
			}
#ifdef __DUMP_FILE__
			fwrite(m_saAudioEncodingFrame, 2, nEncodingFrameSize, FileInput);
#endif
			int nEncodedFrameSize;

			timeStamp = m_Tools.CurrentTimestamp();
			countFrame++;
#ifdef USE_VAD			
			if (!m_pVoice->HasVoice(m_saAudioEncodingFrame, nEncodingFrameSize))
			{
				continue;
			}
#endif


#ifdef USE_AGC
			m_pPlayerGain->AddFarEnd(m_saAudioEncodingFrame, nEncodingFrameSize);
			m_pRecorderGain->AddGain(m_saAudioEncodingFrame, nEncodingFrameSize);
#endif


#ifdef USE_ANS
			m_pNoise->Denoise(m_saAudioEncodingTempFrame, nEncodingFrameSize, m_saAudioEncodingDenoisedFrame);
#ifdef USE_AECM
			if (m_bNoDataFromFarendYet)
			{
				memcpy(m_saAudioEncodingFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
			}
#else
			memcpy(m_saAudioEncodingFrame, m_saAudioEncodingDenoisedFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
#endif


#endif

#ifdef USE_AECM
			if (m_bEchoCancellerEnabled /*&& !m_bNoDataFromFarendYet*/)
			{
				m_pEcho2->AddFarEnd(m_saAudioEncodingFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
				m_pEcho->CancelEcho(m_saAudioEncodingFrame, AUDIO_CLIENT_SAMPLES_IN_FRAME);
			}
#endif

#ifdef OPUS_ENABLE
			nEncodedFrameSize = m_pAudioCodec->encodeAudio(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
			encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
			m_pAudioCodec->DecideToChangeComplexity(encodingTime);
			avgCountTimeStamp += encodingTime;
#ifdef FIRE_ENC_TIME
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_ENCODING_TIME, encodingTime, 0);
			cumulitiveenctime += encodingTime;
			encodingtimetimes++;
			m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_FIRE_AVG_ENCODING_TIME, cumulitiveenctime * 1.0 / encodingtimetimes, 0);
#endif

#else
			nEncodedFrameSize = m_pG729CodecNative->Encode(m_saAudioEncodingFrame, nEncodingFrameSize, &m_ucaEncodedFrame[1 + m_AudioHeadersize]);
			encodingTime = m_Tools.CurrentTimestamp() - timeStamp;
			avgCountTimeStamp += encodingTime;
#endif
			//            if (countFrame % 100 == 0)
			//                ALOG("#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize)
			//                     + " nEncodedFrameSize = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize) + " ratio: " + m_Tools.DoubleToString((nEncodedFrameSize * 100) / nEncodingFrameSize)
			//                     + " EncodeTime: " + m_Tools.IntegertoStringConvert(encodingTime)
			//                     + " AvgTime: " + m_Tools.DoubleToString(avgCountTimeStamp / countFrame)
			//                     + " MaxFrameNumber: " + m_Tools.IntegertoStringConvert(m_nMaxAudioPacketNumber));

			//m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodedFrame);

			//            SendingHeader->SetInformation( (countFrame%100 == 0)? 0 : 1, PACKETTYPE);

			m_iSlotID = m_iPacketNumber / AUDIO_SLOT_SIZE;
			m_iSlotID %= SendingHeader->GetFieldCapacity(SLOTNUMBER);

			SendingHeader->SetInformation(m_iNextPacketType, PACKETTYPE);
			if (m_iNextPacketType == AUDIO_NOVIDEO_PACKET_TYPE)
			{
				m_iNextPacketType = AUDIO_NORMAL_PACKET_TYPE;
			}

			//			SendingHeader->SetInformation(m_iPacketNumber + (m_iPacketNumber<30?400:0), PACKETNUMBER);
			int nCurrentPacketNumber = m_iPacketNumber;
			SendingHeader->SetInformation(m_iPacketNumber, PACKETNUMBER);
			SendingHeader->SetInformation(m_iSlotID, SLOTNUMBER);
			SendingHeader->SetInformation(nEncodedFrameSize, PACKETLENGTH);
			SendingHeader->SetInformation(m_iPrevRecvdSlotID, RECVDSLOTNUMBER);
			SendingHeader->SetInformation(m_iReceivedPacketsInPrevSlot, NUMPACKETRECVD);
			SendingHeader->SetInformation(version, VERSIONCODE);
			SendingHeader->GetHeaderInByteArray(&m_ucaEncodedFrame[1]);
			//            SendingHeader->CopyHeaderToInformation(&m_ucaEncodedFrame[1]);
			//
			//            int version = SendingHeader->GetInformation(VERSIONCODE);
			//            ALOG( "#2AE#--->> PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber)
			//                  +" m_iAudioVersionSelf = "+ m_Tools.IntegertoStringConvert(version));

			m_ucaEncodedFrame[0] = 0;   //Setting Audio packet type( = 0).

			//            ALOG("#V# E: PacketNumber: "+m_Tools.IntegertoStringConvert(m_iPacketNumber)
			//                + " #V# E: SLOTNUMBER: "+m_Tools.IntegertoStringConvert(m_iSlotID)
			//                + " #V# E: NUMPACKETRECVD: "+m_Tools.IntegertoStringConvert(m_iReceivedPacketsInPrevSlot)
			//                + " #V# E: RECVDSLOTNUMBER: "+m_Tools.IntegertoStringConvert(m_iPrevRecvdSlotID)
			//            );

			//            if(m_iPacketNumber%100 ==0)
			//                ALOG("#2AE#  ------------------------ Version: "+Tools::IntegertoStringConvert(m_iAudioVersionFriend) +" isVideo: "+Tools::IntegertoStringConvert(m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning()));

			if (m_iPacketNumber >= m_nMaxAudioPacketNumber)
				m_iPacketNumber = 0;
			else
				++m_iPacketNumber;

			//            ALOG("#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));
			//            CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "#DE#--->> QUEUE = " + m_Tools.IntegertoStringConvert(nEncodedFrameSize + m_AudioHeadersize + 1));

#ifdef  __AUDIO_SELF_CALL__
			DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
#else
			if (m_bIsCheckCall == LIVE_CALL_MOOD) {
				//                ALOG("#H#Sent PacketType: "+m_Tools.IntegertoStringConvert(m_ucaEncodedFrame[0]));
				m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);

#ifdef  __DUPLICATE_AUDIO__
				if (0 < m_iAudioVersionFriend && m_pCommonElementsBucket->m_pEventNotifier->IsVideoCallRunning()) {
					toolsObject.SOSleep(5);
					m_pCommonElementsBucket->SendFunctionPointer(m_FriendID, 1, m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
					//                    ALOG("#2AE# Sent Second Times");
				}
#endif
			}
			else
				DecodeAudioData(m_ucaEncodedFrame, nEncodedFrameSize + m_AudioHeadersize + 1);
#endif

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

void CAudioCallSession::DecodingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Started DecodingThreadProcedure method.");

	Tools toolsObject;
	bool bIsProcessablePacket;
	int nDecodingFrameSize, nDecodedFrameSize, iFrameCounter = 0, nCurrentAudioPacketType, iPacketNumber;
	long long timeStamp, nDecodingTime = 0;
	double dbTotalTime = 0;
	toolsObject.SOSleep(1000);
	int iDataSentInCurrentSec = 0;
	long long llTimeStamp = 0;
	int nTolarance = m_nMaxAudioPacketNumber / 2;

#ifdef __DUMP_FILE__
	FileOutput = fopen("/storage/emulated/0/OutputPCMN.pcm", "w");
#endif
	//toolsObject.SOSleep(1000);
	while (m_bAudioDecodingThreadRunning)
	{
		if (m_AudioDecodingBuffer.GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			nDecodingFrameSize = m_AudioDecodingBuffer.DeQueue(m_ucaDecodingFrame);
			bIsProcessablePacket = false;
			//            ALOG( "#DE#--->> nDecodingFrameSize = " + m_Tools.IntegertoStringConvert(nDecodingFrameSize));
			timeStamp = m_Tools.CurrentTimestamp();
			ReceivingHeader->CopyHeaderToInformation(m_ucaDecodingFrame);
			//            ALOG("#V# PacketNumber: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(PACKETNUMBER))
			//                    + " #V# SLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(SLOTNUMBER))
			//                    + " #V# NUMPACKETRECVD: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(NUMPACKETRECVD))
			//                    + " #V# RECVDSLOTNUMBER: "+ m_Tools.IntegertoStringConvert(ReceivingHeader->GetInformation(RECVDSLOTNUMBER))
			//            );

			nCurrentAudioPacketType = ReceivingHeader->GetInformation(PACKETTYPE);
			iPacketNumber = ReceivingHeader->GetInformation(PACKETNUMBER);

			ALOG("#2A#RCV---> PacketNumber = " + m_Tools.IntegertoStringConvert(iPacketNumber)
				+ "  Last: " + m_Tools.IntegertoStringConvert(m_iLastDecodedPacketNumber)
				+ " m_iAudioVersionFriend = " + m_Tools.IntegertoStringConvert(m_iAudioVersionFriend));

#ifdef  __DUPLICATE_AUDIO__
			//iPacketNumber rotates
			if (m_iLastDecodedPacketNumber > -1) {
				if (iPacketNumber <= m_iLastDecodedPacketNumber && iPacketNumber + nTolarance > m_iLastDecodedPacketNumber) {
					continue;
				}
				else if (iPacketNumber > m_iLastDecodedPacketNumber && iPacketNumber > nTolarance + m_iLastDecodedPacketNumber) {
					continue;
				}
			}
			ALOG("#2A#RCV---------> Decoding = " + m_Tools.IntegertoStringConvert(iPacketNumber));
#endif


			if (!ReceivingHeader->IsPacketTypeSupported())
			{
				continue;
			}

			if (AUDIO_SKIP_PACKET_TYPE == nCurrentAudioPacketType)
			{
				//                ALOG("#V#TYPE# ############################################### SKIPPET");
				toolsObject.SOSleep(0);
				continue;
			}
			else if (AUDIO_NOVIDEO_PACKET_TYPE == nCurrentAudioPacketType)
			{
				//g_StopVideoSending = 1;*/
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO, 0, 0);
				bIsProcessablePacket = true;
			}
			else if (AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
			{
				bIsProcessablePacket = true;
			}
			else if (AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
			{
				bIsProcessablePacket = true;
				if (-1 == m_iAudioVersionFriend)
					m_iAudioVersionFriend = ReceivingHeader->GetInformation(VERSIONCODE);
				//                ALOG("#2A   m_iAudioVersionFriend = "+ m_Tools.IntegertoStringConvert(m_iAudioVersionFriend));
			}
            else if(AUDIO_NORMAL_PACKET_TYPE == nCurrentAudioPacketType)
            {
                bIsProcessablePacket = true;
                if(-1 == m_iAudioVersionFriend)
                    m_iAudioVersionFriend = ReceivingHeader->GetInformation(VERSIONCODE);
//                ALOG("#2A   m_iAudioVersionFriend = "+ m_Tools.IntegertoStringConvert(m_iAudioVersionFriend));
            }

			if (!bIsProcessablePacket) continue;

			m_iOpponentReceivedPackets = ReceivingHeader->GetInformation(NUMPACKETRECVD);

			if (ReceivingHeader->GetInformation(SLOTNUMBER) != m_iCurrentRecvdSlotID)
			{
				m_iPrevRecvdSlotID = m_iCurrentRecvdSlotID;
				if (m_iPrevRecvdSlotID != -1)
				{
					m_iReceivedPacketsInPrevSlot = m_iReceivedPacketsInCurrentSlot;
				}

				m_iCurrentRecvdSlotID = ReceivingHeader->GetInformation(SLOTNUMBER);
				m_iReceivedPacketsInCurrentSlot = 0;
#ifdef OPUS_ENABLE
				//m_pAudioCodec->DecideToChangeBitrate(m_iOpponentReceivedPackets);
#endif
			}

			m_iLastDecodedPacketNumber = iPacketNumber;
			m_iReceivedPacketsInCurrentSlot ++;
			//continue;
			nDecodingFrameSize -= m_AudioHeadersize;
			//            ALOG("#ES Size: "+m_Tools.IntegertoStringConvert(nDecodingFrameSize));
#ifdef OPUS_ENABLE
			nDecodedFrameSize = m_pAudioCodec->decodeAudio(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);

#else
			nDecodedFrameSize = m_pG729CodecNative->Decode(m_ucaDecodingFrame + m_AudioHeadersize, nDecodingFrameSize, m_saDecodedFrame);
#endif


#ifdef USE_AECM
			if (m_bEchoCancellerEnabled)
			{								
				m_pEcho2->CancelEcho(m_saDecodedFrame, nDecodedFrameSize);
				m_pEcho->AddFarEnd(m_saDecodedFrame, nDecodedFrameSize);
			}			
			m_bNoDataFromFarendYet = false;
#endif

#ifdef USE_AGC
			m_pRecorderGain->AddFarEnd(m_saDecodedFrame, nDecodedFrameSize);
			m_pPlayerGain->AddGain(m_saDecodedFrame, nDecodedFrameSize);
#endif

			
#ifdef __DUMP_FILE__
			fwrite(m_saDecodedFrame, 2, nDecodedFrameSize, FileOutput);
#endif
			long long llNow = m_Tools.CurrentTimestamp();
			//            ALOG("#DS Size: "+m_Tools.IntegertoStringConvert(nDecodedFrameSize));
			if (llNow - llTimeStamp >= 1000)
			{
				//                CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, "Num AudioDataDecoded = " + m_Tools.IntegertoStringConvert(iDataSentInCurrentSec));
				iDataSentInCurrentSec = 0;
				llTimeStamp = llNow;
			}
            iDataSentInCurrentSec ++;

			++iFrameCounter;
			nDecodingTime = m_Tools.CurrentTimestamp() - timeStamp;
			dbTotalTime += nDecodingTime;
			//            if(iFrameCounter % 100 == 0)
			//                ALOG( "#DE#--->> Size " + m_Tools.IntegertoStringConvert(nDecodedFrameSize) + " DecodingTime: "+ m_Tools.IntegertoStringConvert(nDecodingTime) + "A.D.Time : "+m_Tools.DoubleToString(dbTotalTime / iFrameCounter));
#if defined(DUMP_DECODED_AUDIO)
			m_Tools.WriteToFile(m_saDecodedFrame, size);
#endif
            if(nDecodedFrameSize < 1)
			{
				ALOG("#EXP# Decoding Failed.");
				continue;
			}
			if (m_bIsCheckCall == LIVE_CALL_MOOD)
			{
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(m_FriendID, nDecodedFrameSize, m_saDecodedFrame);
			}
			toolsObject.SOSleep(0);
		}
	}

	m_bAudioDecodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}

