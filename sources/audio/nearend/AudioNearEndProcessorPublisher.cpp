#include "AudioNearEndProcessorPublisher.h"
#include "AudioShortBufferForPublisherFarEnd.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"
#include "AudioPacketHeader.h"
#include "AudioMixer.h"
#include "MuxHeader.h"
#include "AudioLinearBuffer.h"
#include "AudioEncoderInterface.h"
#include "Tools.h"
#include "AudioDecoderBuffer.h"
#include "AudioDeviceInformation.h"



namespace MediaSDK
{

	AudioNearEndProcessorPublisher::AudioNearEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioNearEndBuffer, bool bIsLiveStreamingRunning) :
		AudioNearEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, pAudioNearEndBuffer, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorPublisher::AudioNearEndProcessorPublisher()");
		m_pHeader = AudioPacketHeader::GetInstance(HEADER_COMMON);
		m_pAudioMixer.reset(new AudioMixer(BITS_USED_FOR_AUDIO_MIXING, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING));

		m_nTotalSentFrameSize = 0;

		m_pAudioDeviceInformation = new AudioDeviceInformation();
	}

	AudioNearEndProcessorPublisher::~AudioNearEndProcessorPublisher()
	{
		delete m_pAudioDeviceInformation;
	}

	//FILE* fp = fopen("/sdcard/out.pcm", "wb");

	void AudioNearEndProcessorPublisher::ProcessNearEndData()
	{
		//	MR_DEBUG("#nearEnd# AudioNearEndProcessorPublisher::ProcessNearEndData()");

		int version = 0;
		int nSendingFramePacketType = 0;
		long long llCapturedTime = 0, llRelativeTime = 0, llLasstTime = -1;
		

		if (m_pAudioCallSession->m_recordBuffer->PopData(m_saAudioRecorderFrame) == 0)
		//if (m_pAudioNearEndBuffer->GetQueueSize() == 0)
		{
			Tools::SOSleep(10);
		}
		else
		{
			//fwrite(m_saAudioRecorderFrame, 2, CHUNK_SIZE, fp);
			LOGT("##TT#18#NE#Publisher...");
			//m_pAudioNearEndBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);
			llCapturedTime = Tools::CurrentTimestamp();
			int nEchoStateFlags = m_pAudioCallSession->PreprocessAudioData(m_saAudioRecorderFrame, CHUNK_SIZE);
			DumpEncodingFrame();

			int nSendingDataSizeInByte = PCM_FRAME_SIZE_IN_BYTE;	//Or contain 18 bit data with mixed header.
			bool bIsMuxed = false;

			if (false == PreProcessAudioBeforeEncoding())
			{
				return;
			}
			

			m_nTotalSentFrameSize = m_iPacketNumber + 1;
			if (m_nTotalSentFrameSize % DEVICE_INFORMATION_PACKET_INTERVAL == 0)
			{
				UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

				DeviceInformation nowDeviceInformation;
				m_pAudioCallSession->GetDeviceInformation(nowDeviceInformation);
				m_pAudioCallSession->ResetDeviceInformation(1);

				if (m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER || m_pAudioCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE) nowDeviceInformation.llIsCallInLive = 1;
				else nowDeviceInformation.llIsCallInLive = 0;

				m_pAudioDeviceInformation->Reset();
				
				// Inforamation of Publisher End
				m_pAudioDeviceInformation->SetInformation(ByteSizeDelay, DEVICE_INFORMATION_DELAY_PUBLISHER, nowDeviceInformation.llDelay[0]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeDelayFraction, DEVICE_INFORMATION_DELAY_FRACTION_PUBLISHER, nowDeviceInformation.llDelayFraction[0]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeFarendSize, DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE_PUBLISHER, nowDeviceInformation.llStartUpFarEndBufferSize[0]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeFarendSize, DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX_PUBLISHER, nowDeviceInformation.llCurrentFarEndBufferSizeMax[0]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeFarendSize, DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN_PUBLISHER, nowDeviceInformation.llCurrentFarEndBufferSizeMin[0]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeAverageTimeDiff, DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF_PUBLISHER, nowDeviceInformation.llAverageTimeDiff[0] / DEVICE_INFORMATION_PACKET_INTERVAL);
				m_pAudioDeviceInformation->SetInformation(ByteSizeIsCalling, DEVICE_INFORMATION_IS_CALLING_PUBLISHER, nowDeviceInformation.llIsCallInLive);
				m_pAudioDeviceInformation->SetInformation(ByteSizeCountCall, DEVICE_INFORMATION_COUNT_CALL_PUBLISHER, nowDeviceInformation.llCallCount);
				m_pAudioDeviceInformation->SetInformation(ByteSizeTotalDataSize, DEVICE_INFORMATION_TOTAL_DATA_SZ, nowDeviceInformation.llTotalDataSize[0]);

				// Information of Callee End
				m_pAudioDeviceInformation->SetInformation(ByteSizeDelay, DEVICE_INFORMATION_DELAY_CALLEE, nowDeviceInformation.llDelay[1]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeDelayFraction, DEVICE_INFORMATION_DELAY_FRACTION_CALLEE, nowDeviceInformation.llDelayFraction[1]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeFarendSize, DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE_CALLEE, nowDeviceInformation.llStartUpFarEndBufferSize[1]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeFarendSize, DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX_CALLEE, nowDeviceInformation.llCurrentFarEndBufferSizeMax[1]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeFarendSize, DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN_CALLEE, nowDeviceInformation.llCurrentFarEndBufferSizeMin[1]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeAverageTimeDiff, DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF_CALLEE, nowDeviceInformation.llAverageTimeDiff[1]);
				m_pAudioDeviceInformation->SetInformation(ByteSizeTotalDataSize, DEVICE_INFORMATION_TOTAL_DATA_SZ_CALLEE, nowDeviceInformation.llTotalDataSize[1]);

				MediaLog(LOG_DEBUG, "[ANEPP][P] %lld %lld %lld %lld %lld %lld %lld %lld %lld", nowDeviceInformation.llDelay[0], nowDeviceInformation.llDelayFraction[0], nowDeviceInformation.llStartUpFarEndBufferSize[0], nowDeviceInformation.llCurrentFarEndBufferSizeMax[0], nowDeviceInformation.llCurrentFarEndBufferSizeMin[0], nowDeviceInformation.llAverageTimeDiff[0], nowDeviceInformation.llIsCallInLive, nowDeviceInformation.llCallCount, nowDeviceInformation.llTotalDataSize[0]);
				MediaLog(LOG_DEBUG, "[ANEPP][P] %lld %lld %lld %lld %lld %lld %lld", nowDeviceInformation.llDelay[1], nowDeviceInformation.llDelayFraction[1], nowDeviceInformation.llStartUpFarEndBufferSize[1], nowDeviceInformation.llCurrentFarEndBufferSizeMax[1], nowDeviceInformation.llCurrentFarEndBufferSizeMin[1], nowDeviceInformation.llAverageTimeDiff[1], nowDeviceInformation.llTotalDataSize[1]);

				m_ucaRawFrameForInformation[0] = 0;
				int nNowSendingDataSizeInByte = 1 + m_MyAudioHeadersize;

				BuildHeaderForLive(0, m_MyAudioHeadersize, version, m_iPacketNumber, nSendingDataSizeInByte, llRelativeTime, nEchoStateFlags, &m_ucaRawFrameForInformation[1]);

				int nSizeOfInformation = m_pAudioDeviceInformation->GetInformation( &(m_ucaRawFrameForInformation[nNowSendingDataSizeInByte]) );
				nNowSendingDataSizeInByte += nSizeOfInformation;

				StoreDataForChunkDeviceInformation(m_ucaRawFrameForInformation, llRelativeTime, nNowSendingDataSizeInByte);

				llCapturedTime = Tools::CurrentTimestamp();
			}

			UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

			//For opus livestreaming.
			if (m_pAudioCallSession->IsOpusEnable())
			{				
				nSendingDataSizeInByte = m_pAudioEncoder->EncodeAudio(m_saAudioRecorderFrame, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING, &m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize]);
				MediaLog(LOG_DEBUG, "[ANEPP] EncodedDataSize = %d", nSendingDataSizeInByte);
				nSendingFramePacketType = LIVE_PUBLISHER_PACKET_TYPE_OPUS;				
			}
			else	//For PCM livestreaming.
			{
				bIsMuxed = MuxIfNeeded(m_saAudioRecorderFrame, m_saSendingDataPublisher, nSendingDataSizeInByte, m_iPacketNumber);

				memcpy(&m_ucaRawFrameNonMuxed[1 + m_MyAudioHeadersize], m_saSendingDataPublisher, nSendingDataSizeInByte);
				nSendingFramePacketType = bIsMuxed ? AUDIO_LIVE_PUBLISHER_PACKET_TYPE_MUXED : AUDIO_LIVE_PUBLISHER_PACKET_TYPE_NONMUXED;
			}
		
			m_ucaRawFrameNonMuxed[0] = 0;	//Media packet type.

			//BuildAndGetHeaderInArray(nSendingFramePacketType, m_MyAudioHeadersize, 0,  m_iPacketNumber, nSendingDataSizeInByte,
			//	nChannel, version, llRelativeTime, m_pAudioCallSession->m_nEchoStateFlags, &m_ucaRawFrameNonMuxed[1]);

			BuildHeaderForLive(nSendingFramePacketType, m_MyAudioHeadersize, version, m_iPacketNumber, nSendingDataSizeInByte,
				llRelativeTime, nEchoStateFlags, &m_ucaRawFrameNonMuxed[1]);						

			MediaLog(LOG_CODE_TRACE, "[ANEPP] Publish#  FrameNo = %d, RT: %lld, SendingDataSizeInByte = %d HeaderLen = %d ESF: %d", m_iPacketNumber, llRelativeTime, nSendingDataSizeInByte, m_MyAudioHeadersize, nEchoStateFlags);

			++m_iPacketNumber;
			if (m_iPacketNumber == m_llMaxAudioPacketNumber)
			{
				m_iPacketNumber = 0;
			}
			
			int nSendingFrameSizeInByte = 1 + m_MyAudioHeadersize + nSendingDataSizeInByte;

			if (m_pAudioCallSession->IsOpusEnable())
			{
				int nCalleePacketLength = 0;
				if ( 0 < m_pAudioCallSession->m_FarEndBufferOpus->GetQueueSize())
				{
					nCalleePacketLength = m_pAudioCallSession->m_FarEndBufferOpus->DeQueue(m_uchFarEndFrame);
					m_pHeader->CopyHeaderToInformation(m_uchFarEndFrame + 1);

					MediaLog(LOG_DEBUG, "[NR][ANEPP] FarPacketType = %d", (int)m_uchFarEndFrame[1]);
				}
				
				MediaLog(LOG_DEBUG, "[ANEPP] NearFrameSize = %d, FarFrameSize = %d, RT = %lld", nSendingFrameSizeInByte, nCalleePacketLength, llRelativeTime);

				StoreDataForChunk(m_ucaRawFrameNonMuxed, nSendingFrameSizeInByte, m_uchFarEndFrame, nCalleePacketLength, llRelativeTime);
			}
			else {
				StoreDataForChunk(m_ucaRawFrameNonMuxed, llRelativeTime, nSendingFrameSizeInByte);
			}


			Tools::SOSleep(0);
		}
	}


	bool AudioNearEndProcessorPublisher::MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber)
	{
		long long nFrameNumber;
		nDataSizeInByte = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING;
		int nLastDecodedFrameSizeInByte = 0;
		bool bIsMuxed = false;
		int iDataStartIndex, iDataEndIndex;
		int iCallId = 0, nNumberOfBlocks;
		std::vector<std::pair<int, int> > vMissingBlocks;
		MuxHeader oCalleeMuxHeader;
		if (m_pAudioCallSession->m_PublisherBufferForMuxing->GetQueueSize() != 0)
		{
			bIsMuxed = true;
			nLastDecodedFrameSizeInByte = m_pAudioCallSession->m_PublisherBufferForMuxing->DeQueue(m_saAudioPrevDecodedFrame, nFrameNumber, oCalleeMuxHeader) * 2;	//#Magic
			LOG18("#18@# DEQUE data of size %d", nLastDecodedFrameSizeInByte);
			m_pAudioMixer->reset(18, AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING);

			iDataStartIndex = 0;
			iDataEndIndex = 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING - 1;
			iCallId = 0;	//Publisher
			nNumberOfBlocks = 16;
			int nMuxHeaderSize = 14;

			MuxHeader oPublisherMuxHeader(iCallId, nPacketNumber, vMissingBlocks);
			LOG18("#18@# -> PUB ID %lld CALLEE ID %lld", oPublisherMuxHeader.getCalleeId(), oCalleeMuxHeader.getCalleeId());
			m_pAudioMixer->addAudioData((unsigned char*)shPublisherData, oPublisherMuxHeader); // this data should contains only the mux header

			if (nLastDecodedFrameSizeInByte == 2 * AUDIO_FRAME_SAMPLE_SIZE_FOR_LIVE_STREAMING) //Both must be 800
			{
				m_pAudioMixer->addAudioData((unsigned char*)m_saAudioPrevDecodedFrame, oCalleeMuxHeader); // this data should contains only the mux header

				nDataSizeInByte = m_pAudioMixer->getAudioData((unsigned char*)shMuxedData);
			}
			else
			{
				LOG18("#18@# RETURN WITH FALSE and zeor");
				nDataSizeInByte = 0;
				return false;
			}
		}
		else
		{
			memcpy(shMuxedData, m_saAudioRecorderFrame, nDataSizeInByte);
		}
#ifdef DUMP_FILE
		AudioMixer* DumpAudioMixer = new AudioMixer(18, 800);
		unsigned char temp[2000];

		if (nDataSizeInByte == 1800)
		{
			DumpAudioMixer->Convert18BitTo16Bit((unsigned char*)shMuxedData, temp, 800);
			fwrite((short *)temp, sizeof(short), 800, m_pAudioCallSession->File18BitData);
		}
		else {
			fwrite(shMuxedData, sizeof(short), 800, m_pAudioCallSession->File18BitData);
		}

		short val = nDataSizeInByte;
		LOG18("#18#NE#DMP nDataSizeInByte = %d", nDataSizeInByte);
		fwrite(&val, 2, 1, m_pAudioCallSession->File18BitType);

#endif
		return bIsMuxed;
	}


} //namespace MediaSDK
