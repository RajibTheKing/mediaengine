//
// Created by ipvision on 10/23/2016.
//

#ifndef LIVESTREAMING_LIVERECEIVER_H
#define LIVESTREAMING_LIVERECEIVER_H

#include "SmartPointer.h"
#include "LockHandler.h"
#include "AudioDecoderBuffer.h"
#include "LiveVideoDecodingQueue.h"

class LiveReceiver {
public:
    LiveReceiver(CAudioDecoderBuffer *pAudioDecoderBuffer, LiveVideoDecodingQueue *pLiveVideoDecodingQueue);
    ~LiveReceiver();
	void PushAudioData(unsigned char* uchAudioData, int iLen, int numberOfFrames = 0, int *frameSizes = NULL);
    void PushVideoData(unsigned char* uchVideoData,int iLen);
    bool GetVideoFrame(unsigned char* uchVideoFrame,int iLen);

private:
    SmartPointer<CLockHandler> m_pLiveReceiverMutex;
    CAudioDecoderBuffer *m_pAudioDecoderBuffer;
    LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
};


#endif //LIVESTREAMING_LIVERECEIVER_H
