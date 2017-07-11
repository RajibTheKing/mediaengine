#include "AudioNearEndProcessorCall.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"
#include "AudioEncoderBuffer.h"

namespace MediaSDK
{

	AudioNearEndProcessorCall::AudioNearEndProcessorCall(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioNearEndBuffer, bool bIsLiveStreamingRunning) :
		AudioNearEndDataProcessor(nServiceType, nEntityType, pAudioCallSession, pAudioNearEndBuffer, bIsLiveStreamingRunning)
	{
		MR_DEBUG("#nearEnd# AudioNearEndProcessorCall::AudioNearEndProcessorCall()");

		m_pAudioNearEndBuffer = pAudioNearEndBuffer;
	}


	void AudioNearEndProcessorCall::ProcessNearEndData()
	{
		//	MR_DEBUG("#nearEnd# AudioNearEndProcessorCall::ProcessNearEndData()");

		int version = 0;
		long long llCapturedTime, llRelativeTime = 0, llLasstTime = -1;;
		if (m_pAudioCallSession->m_recordBuffer->PopData(m_saAudioRecorderFrame) == 0)
		{
			Tools::SOSleep(10);
		}
		else
		{
			//LOGT("##TT dequed #18#NE#AudioCall...");
			m_pAudioCallSession->PreprocessAudioData(m_saAudioRecorderFrame, CHUNK_SIZE);
			//m_pAudioNearEndBuffer->DeQueue(m_saAudioRecorderFrame, llCapturedTime);

			DumpEncodingFrame();
			UpdateRelativeTimeAndFrame(llLasstTime, llRelativeTime, llCapturedTime);

			if (false == PreProcessAudioBeforeEncoding())
			{
				return;
			}

			long long llEncodingTime, llTimeBeforeEncoding = Tools::CurrentTimestamp();
			m_nEncodedFrameSize = m_pAudioEncoder->EncodeAudio(m_saAudioRecorderFrame, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(m_bIsLiveStreamingRunning), &m_ucaEncodedFrame[1 + m_MyAudioHeadersize]);

			//ALOG("#A#EN#--->> nEncodingFrameSize = " + m_Tools.IntegertoStringConvert(nEncodingFrameSize) + " PacketNumber = " + m_Tools.IntegertoStringConvert(m_iPacketNumber));
			llEncodingTime = Tools::CurrentTimestamp() - llTimeBeforeEncoding;
			this->DecideToChangeComplexity(llEncodingTime);


			int iSlotID = 0;
			int iPrevRecvdSlotID = 0;
			int iReceivedPacketsInPrevSlot = 0;
			int nChannel = 0;

			BuildAndGetHeaderInArray(AUDIO_NORMAL_PACKET_TYPE, m_MyAudioHeadersize, 0, iSlotID, m_iPacketNumber, m_nEncodedFrameSize, iPrevRecvdSlotID, iReceivedPacketsInPrevSlot, nChannel, version, llRelativeTime, &m_ucaEncodedFrame[1]);

			++m_iPacketNumber;
			if (m_iPacketNumber == m_llMaxAudioPacketNumber)
			{
				m_iPacketNumber = 0;
			}

			SentToNetwork(llRelativeTime);
			LOG18("#18#NE#AudioCall Sent");
			Tools::SOSleep(0);
		}
	}

} //namespace MediaSDK
