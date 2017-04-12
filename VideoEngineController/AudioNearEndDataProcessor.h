#ifndef AUDIO_NEAREND_DATA_PROCESSOR_H
#define AUDIO_NEAREND_DATA_PROCESSOR_H

#include "Size.h"
#include "LockHandler.h"
#include "Tools.h"
#include "SmartPointer.h"
#include <vector>

class CAudioCallSession;
class AudioPacketizer;
class CCommonElementsBucket;
class CAudioPacketHeader;
class CAudioShortBuffer;
class CAudioCodec;
class AudioMixer;

class CAudioNearEndDataProcessor
{
public:

	CAudioNearEndDataProcessor(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
	~CAudioNearEndDataProcessor();

	static void *CreateAudioEncodingThread(void* param);
	void EncodingThreadProcedure();
	void GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthPeer, long long &llAudioChunkDuration, long long &llAudioChunkRelativeTime);

	void StartCallInLive();

private:
	void LiveStreamNearendProcedureViewer();
	void LiveStreamNearendProcedurePublisher();
	void AudioCallNearendProcedure();
	void StartEncodingThread();
	void StopEncodingThread();	
	bool MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber);
	void DumpEncodingFrame();
	void UpdateRelativeTimeAndFrame(long long &llLasstTime, long long & llRelativeTime, long long & llCapturedTime);
	bool PreProcessAudioBeforeEncoding();
	void EncodeIfNeeded();
	void SetAudioIdentifierAndNextPacketType();	
	void AddHeader(int &version, long long &llRelativeTime);
	void BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);
	void StoreDataForChunk(unsigned char *uchDataToChunk, long long llRelativeTime, int nDataLengthInByte);
	void SentToNetwork(long long llRelativeTime);
	
	std::vector<std::pair<int, int>> m_vFrameMissingBlocks;

	long long m_llFriendID;
	bool m_bIsLiveStreamingRunning;
	bool m_bIsReady;
	int m_nEncodedFrameSize;
	int m_nRawFrameSize;
	int m_MyAudioHeadersize;
	int m_iPacketNumber;
	int m_iNextPacketType;
	int m_iSlotID;
	int m_nServiceType;
	int m_nEntityType;
	LongLong m_llMaxAudioPacketNumber;
	LongLong m_llEncodingTimeStampOffset;

	CAudioCodec *m_pAudioCodec;

	CAudioCallSession *m_pAudioCallSession = nullptr;
	CCommonElementsBucket *m_pCommonElementsBucket = nullptr;	
	CAudioShortBuffer *m_pAudioEncodingBuffer = nullptr;
	CAudioPacketHeader *m_pAudioPacketHeader = nullptr;
	AudioMixer *m_pAudioMixer = nullptr;

	short m_saAudioRecorderFrame[MAX_AUDIO_FRAME_Length];//Always contains UnMuxed Data
	unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_Length];
	short m_saSendingDataPublisher[MAX_AUDIO_FRAME_Length];//Always contains data for VIEWER_NOT_IN_CALL, MUXED data if m_saAudioPrevDecodedFrame is available

	bool m_bAudioEncodingThreadRunning;
	bool m_bAudioEncodingThreadClosed;

	unsigned char m_ucaRawFrameNonMuxed[MAX_AUDIO_FRAME_Length];
	int m_iRawDataSendIndexViewer;

	short m_saAudioPrevDecodedFrame[MAX_AUDIO_FRAME_Length];
	unsigned char m_ucaRawDataToSendViewer[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	std::vector<int> m_vRawFrameLengthViewer;
	AudioPacketizer* m_pAudioPacketizer = nullptr;
	long long m_llLastChunkLastFrameRT;
	long long m_llLastFrameRT;
	
	//SmartPointer<std::thread> m_pAudioEncodingThread;
	SmartPointer<CLockHandler> m_pAudioEncodingMutex;
};

#endif //AUDIO_NEAREND_DATA_PROCESSOR_H