#ifndef AUDIO_NEAREND_DATA_PROCESSOR_H
#define AUDIO_NEAREND_DATA_PROCESSOR_H

#include "AudioTypes.h"
#include "Size.h"
#include "LockHandler.h"
#include "Tools.h"
#include "SmartPointer.h"
#include "LogPrinter.h"
#include <vector>

class CAudioCallSession;
class CCommonElementsBucket;
class AudioPacketHeader;
class CAudioShortBuffer;
class CAudioCodec;
class AudioMixer;

class AudioEncoderInterface;
class NoiseReducerInterface;

class AudioNearEndProcessorThread;

class AudioNearEndDataProcessor
{
public:

	AudioNearEndDataProcessor(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
	virtual ~AudioNearEndDataProcessor();

	//static void *CreateAudioEncodingThread(void* param);
	//void EncodingThreadProcedure();
	void GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);

	void StartCallInLive(int nEntityType);
	void StopCallInLive(int nEntityType);

	long long GetBaseOfRelativeTime();

	void SetDataReadyCallback(OnDataReadyToSendCB cbDataReady)
	{
		m_cbOnDataReady = cbDataReady;
		MR_DEBUG("#ptt# SetDataReadyCallback: %x", m_cbOnDataReady);
	}

	void SetEventCallback(OnFirePacketEventCB cbOnEvent)
	{
		m_cbOnPacketEvent = cbOnEvent;
		MR_DEBUG("#ptt# SetEventCallback: %x", m_cbOnPacketEvent);
	}

	virtual void ProcessNearEndData() = 0;


protected:

	//void LiveStreamNearendProcedureViewer();
	//void LiveStreamNearendProcedurePublisher();
	//void AudioCallNearendProcedure();

	//void StartEncodingThread();
	//void StopEncodingThread();	
	bool MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber);
	void DumpEncodingFrame();
	void UpdateRelativeTimeAndFrame(long long &llLasstTime, long long & llRelativeTime, long long & llCapturedTime);
	bool PreProcessAudioBeforeEncoding();
	void BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);
	void StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nDataLengthInByte);
	void SentToNetwork(long long llRelativeTime);

	void DecideToChangeComplexity(int iEncodingTime);
	

private:

	std::vector<std::pair<int, int>> m_vFrameMissingBlocks;

	long long m_llFriendID;
	bool m_bIsReady;
	int m_nServiceType;
	int m_nEntityType;
	LongLong m_llEncodingTimeStampOffset;

//	SmartPointer<NoiseReducerInterface> m_pNoise;

	CAudioCallSession *m_pAudioCallSession = nullptr;
	CAudioShortBuffer *m_pAudioEncodingBuffer = nullptr;
	SmartPointer<AudioPacketHeader> m_pAudioPacketHeader = nullptr;
	AudioMixer *m_pAudioMixer = nullptr;


	bool m_bAudioEncodingThreadRunning;
	bool m_bAudioEncodingThreadClosed;

	int m_iRawDataSendIndexViewer;

	short m_saAudioPrevDecodedFrame[MAX_AUDIO_FRAME_Length];
	unsigned char m_ucaRawDataToSendViewer[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	std::vector<int> m_vRawFrameLengthViewer;
	long long m_llLastChunkLastFrameRT;
	long long m_llLastFrameRT;
	
	//SmartPointer<std::thread> m_pAudioEncodingThread;
	SmartPointer<CLockHandler> m_pAudioEncodingMutex;

	OnDataReadyToSendCB m_cbOnDataReady;
	OnFirePacketEventCB m_cbOnPacketEvent;

	AudioNearEndProcessorThread *m_cNearEndProcessorThread;

protected:

	bool m_bIsLiveStreamingRunning;

	int m_MyAudioHeadersize;
	int m_iPacketNumber;
	int m_nRawFrameSize;
	int m_nEncodedFrameSize;
	LongLong m_llMaxAudioPacketNumber;

	short m_saAudioRecorderFrame[MAX_AUDIO_FRAME_Length];//Always contains UnMuxed Data
	short m_saSendingDataPublisher[MAX_AUDIO_FRAME_Length];//Always contains data for VIEWER_NOT_IN_CALL, MUXED data if m_saAudioPrevDecodedFrame is available
	unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_Length];
	unsigned char m_ucaRawFrameNonMuxed[MAX_AUDIO_FRAME_Length];

	SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
};

#endif //AUDIO_NEAREND_DATA_PROCESSOR_H