//
// Created by ipvision on 10/23/2016.
//

#ifndef LIVESTREAMING_LIVERECEIVER_H
#define LIVESTREAMING_LIVERECEIVER_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "AudioDecoderBuffer.h"
#include "LiveVideoDecodingQueue.h"
#include "LiveAudioDecodingQueue.h"


#include<vector>
class CCommonElementsBucket;
class CAudioPacketHeader;
class CAudioCallSession;

class LiveReceiver {
public:
	LiveReceiver(CCommonElementsBucket* sharedObject, CAudioCallSession* pAudioCallSession);
    ~LiveReceiver();
    void SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue);
    void SetAudioDecodingQueue(LiveAudioDecodingQueue *pQueue);

	void PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
	void PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
    bool GetVideoFrame(unsigned char* uchVideoFrame,int iLen);
	void ProcessAudioStream(int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int,int> > vMissingBlocks);

	bool m_bIsCurrentlyParsingAudioData;
	bool m_bIsRoleChanging;
private:

	Tools m_Tools;

	CAudioPacketHeader *m_pAudioPacketHeader;

    SmartPointer<CLockHandler> m_pLiveReceiverMutex;
    CAudioByteBuffer *m_pAudioDecoderBuffer;
    LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
    LiveAudioDecodingQueue *m_pLiveAudioReceivedQueue;
	CCommonElementsBucket* m_pCommonElementsBucket;

	CAudioCallSession* m_pAudioCallSession;
	// FILE* logFile;
};


#endif //LIVESTREAMING_LIVERECEIVER_H