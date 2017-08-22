#ifndef AUDIO_NEAREND_DATA_PROCESSOR_H
#define AUDIO_NEAREND_DATA_PROCESSOR_H

#include "Size.h"
#include "CommonTypes.h"
#include "AudioTypes.h"
#include "SmartPointer.h"
#include <vector>

namespace MediaSDK
{
	class AudioEncoderInterface;
	class NoiseReducerInterface;

	class AudioMixer;
	class CAudioCallSession;
	class CAudioShortBuffer;
	class AudioPacketHeader;


	class AudioNearEndDataProcessor
	{

	public:

		AudioNearEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		virtual ~AudioNearEndDataProcessor();
		virtual void ProcessNearEndData() = 0;

		void StartCallInLive(int nEntityType);
		void StopCallInLive(int nEntityType);

		void GetAudioDataToSend(unsigned char *pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector, int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);
		long long GetBaseOfRelativeTime();

		void SetDataReadyCallback(DataReadyListenerInterface* pDataReady) { m_pDataReadyListener = pDataReady; }
		void SetEventCallback(PacketEventListener* pEventListener) { m_pPacketEventListener = pEventListener; }

		
	protected:

		void DumpEncodingFrame();
		void UpdateRelativeTimeAndFrame(long long &llLasstTime, long long & llRelativeTime, long long & llCapturedTime);
		bool PreProcessAudioBeforeEncoding();
		void BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber, int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);
		void StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nDataLengthInByte);


	public:
		
		CAudioCallSession *m_pAudioCallSession = nullptr;


	protected:

		DataReadyListenerInterface* m_pDataReadyListener;

		bool m_bIsLiveStreamingRunning;

		int m_MyAudioHeadersize;
		int m_iPacketNumber;
		long long m_llMaxAudioPacketNumber;

		short m_saAudioRecorderFrame[MAX_AUDIO_FRAME_Length];    //Always contains UnMuxed Data
		unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_Length];
		unsigned char m_ucaRawFrameNonMuxed[MAX_AUDIO_FRAME_Length];

		SharedPointer<CAudioShortBuffer> m_pAudioNearEndBuffer;
		SharedPointer<AudioEncoderInterface> m_pAudioEncoder;


	private:

		PacketEventListener* m_pPacketEventListener;

		bool m_bIsReady;
		bool m_bAudioEncodingThreadClosed;
		bool m_bAudioEncodingThreadRunning;

		int m_nServiceType;
		int m_nEntityType;
		int m_iRawDataSendIndexViewer;

		long long m_llFriendID;
		long long m_llLastChunkLastFrameRT;
		long long m_llLastFrameRT;
		long long m_llEncodingTimeStampOffset;

		unsigned char m_ucaRawDataToSendViewer[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];

		std::vector<int> m_vRawFrameLengthViewer;
		std::vector<std::pair<int, int>> m_vFrameMissingBlocks;

		//SharedPointer<NoiseReducerInterface> m_pNoise;
		//SharedPointer<std::thread> m_pAudioEncodingThread;
		SharedPointer<AudioPacketHeader> m_pAudioNearEndPacketHeader = nullptr;
		SharedPointer<CLockHandler> m_pAudioEncodingMutex;

	};

} //namespace MediaSDK

#endif //AUDIO_NEAREND_DATA_PROCESSOR_H