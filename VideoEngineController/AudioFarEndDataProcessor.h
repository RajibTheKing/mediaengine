#ifndef AUDIO_FAREND_DATA_PROCESSOR_H
#define AUDIO_FAREND_DATA_PROCESSOR_H

#include "AudioCallSession.h"
#include "AudioDecoderBuffer.h"
#include "AudioMacros.h"
#include "AudioPacketHeader.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "AudioDePacketizer.h"
#include "AudioCodec.h"

class AudioGainInterface;
class CAudioCallSession;
class AudioPacketizer;
class CCommonElementsBucket;
class AudioPacketHeader;
class CAudioShortBuffer;
class Tools;
class CAudioByteBuffer;
class GomGomGain;
class ILiveAudioParser;
class AudioMixer;
class AudioDecoderInterface;

class CAudioFarEndDataProcessor
{
public:
	CAudioFarEndDataProcessor(long long llFriendID, int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, bool bIsLiveStreamingRunning);
	~CAudioFarEndDataProcessor();
	int DecodeAudioData(int nOffset, unsigned char *pucaDecodingAudioData, unsigned int unLength, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > &vMissingFrames);
	void StartCallInLive(int nEntityType);
	void StopCallInLive(int nEntityType);
	void DumpDecodedFrame(short * psDecodedFrame, int nDecodedFrameSize);
	void SendToPlayer(short* pshSentFrame, int nSentFrameSize, long long &llLastTime, int iCurrentPacketNumber);

	void SetEventCallback(OnFireDataEventCB dataCB, OnFireNetworkChangeCB networkCB, OnFireAudioAlarmCB alarmEvent)
	{
		m_cbOnDataEvent = dataCB;
		m_cbOnNetworkChange = networkCB;
		m_cbOnAudioAlarm = alarmEvent;
	}

	//LiveReceiver *m_pLiveReceiverAudio = nullptr;
	ILiveAudioParser* m_pLiveAudioParser;
	long long m_llDecodingTimeStampOffset = -1;
	AudioDePacketizer* m_pAudioDePacketizer = nullptr;
	CAudioByteBuffer m_AudioReceivedBuffer;

	short tmpBuffer[2048];
private:
	int m_nServiceType;
	int m_nEntityType;
	long long m_llLastTime;
	long long m_llFriendID = -1;
	bool m_bIsLiveStreamingRunning = false;
	int m_iLastDecodedPacketNumber = -1;
	int m_nDecodedFrameSize = 0;
	int m_nDecodingFrameSize = 0;
	int m_iAudioVersionFriend = -1;
	//int m_iPrevRecvdSlotID;
	int m_iCurrentRecvdSlotID = -1;
	int m_iReceivedPacketsInCurrentSlot = AUDIO_SLOT_SIZE;
	int m_iOpponentReceivedPackets = AUDIO_SLOT_SIZE;

	std::vector<std::pair<int, int>> m_vFrameMissingBlocks;

	OnFireDataEventCB m_cbOnDataEvent;
	OnFireNetworkChangeCB m_cbOnNetworkChange;
	OnFireAudioAlarmCB m_cbOnAudioAlarm;

	AudioMixer* m_pAudioMixer;
	CAudioCallSession *m_pAudioCallSession = nullptr;
	CCommonElementsBucket *m_pCommonElementsBucket = nullptr;
	//AudioPacketHeader *m_pAudioPacketHeader = nullptr;
	SmartPointer<AudioPacketHeader> m_ReceivingHeader = nullptr;
	SmartPointer<GomGomGain> m_pGomGomGain;

	std::vector<LiveAudioDecodingQueue*> m_vAudioFarEndBufferVector;


	bool m_bAudioDecodingThreadRunning;
	bool m_bAudioDecodingThreadClosed;

	unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_Length];
	short m_saDecodedFrame[MAX_AUDIO_FRAME_Length];
	short m_saCalleeSentData[MAX_AUDIO_FRAME_Length];

	SmartPointer<AudioEncoderInterface> m_pAudioEncoder;
	SmartPointer<AudioDecoderInterface> m_cAacDecoder;

	int m_inoLossSlot;
	int m_ihugeLossSlot;

	bool m_bAudioQualityLowNotified;
	bool m_bAudioQualityHighNotified;
	bool m_bAudioShouldStopNotified;

	void StartDecodingThread();
	static void* CreateAudioDecodingThread(void* param);
	void StopDecodingThread();
	void DecodingThreadProcedure();

	void FarEndProcedureAudioCall();
	void FarEndProcedureLiveStreamViewer();
	void FarEndProcedureLiveStreamPublisher();
	void FarEndProcedureChannel();
	bool IsQueueEmpty();
	void DequeueData(int &m_nDecodingFrameSize);
	void DecodeAndPostProcessIfNeeded(int &iPacketNumber, int &nCurrentPacketHeaderLength, int &nCurrentAudioPacketType);
	void ParseHeaderAndGetValues(int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &packetNumber, int &packetLength, int &recvSlotNumber,
		int &numPacketRecv, int &channel, int &version, long long &timestamp, unsigned char* header, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength);
	bool IsPacketTypeSupported(int &nCurrentAudioPacketType);
	bool IsPacketProcessableBasedOnRole(int &nCurrentAudioPacketType);
	bool IsPacketProcessableInNormalCall(int &nCurrentAudioPacketType, int &nVersion);
	bool IsPacketProcessableBasedOnRelativeTime(long long &llCurrentFrameRelativeTime, int &iPacketNumber, int &nPacketType);
	void SetSlotStatesAndDecideToChangeBitRate(int &nSlotNumber);
	void PrintDecodingTimeStats(long long &llNow, long long &llTimeStamp, int &iDataSentInCurrentSec,
		 long long &nDecodingTime, double &dbTotalTime, long long &llCapturedTime);

	void DecideToChangeBitrate(int iNumPacketRecvd);
	
};

#endif //AUDIO_FAREND_DATA_PROCESSOR_H