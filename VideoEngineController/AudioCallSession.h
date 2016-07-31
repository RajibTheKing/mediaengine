
#ifndef _AUDIO_CALL_SESSION_H_
#define _AUDIO_CALL_SESSION_H_

#include "AudioEncoderBuffer.h"
#include "AudioDecoderBuffer.h"
#include "G729CodecNative.h"
#include "AudioEncoder.h"
#include "AudioDecoder.h"
#include "LockHandler.h"
#include "Tools.h"
#include "AudioPacketHeader.h"
#include <stdio.h>
#include <string>
#include <map>

class CCommonElementsBucket;
class CVideoEncoder;
class CAudioPacketHeader;

class CAudioCallSession
{

public:

    CAudioCallSession(LongLong llFriendID, CCommonElementsBucket* pSharedObject, bool bIsCheckCall=false);
    ~CAudioCallSession();

    CAudioEncoder* GetAudioEncoder();
    CAudioDecoder* GetAudioDecoder();



    void InitializeAudioCallSession(LongLong llFriendID);
    int EncodeAudioData(short *psaEncodingAudioData, unsigned int unLength);
    int DecodeAudioData(unsigned char *pucaDecodingAudioData, unsigned int unLength);

    void EncodingThreadProcedure();
    void StopEncodingThread();
    void StartEncodingThread();

    void DecodingThreadProcedure();
    void StopDecodingThread();
    void StartDecodingThread();

    static void *CreateAudioEncodingThread(void* param);
    static void *CreateAudioDecodingThread(void* param);

private:

    Tools m_Tools;
    LongLong m_FriendID;

    CAudioPacketHeader *SendingHeader;
    CAudioPacketHeader *ReceivingHeader;
    int m_AudioHeadersize;

    CCommonElementsBucket* m_pCommonElementsBucket;
    CAudioEncoderBuffer m_AudioEncodingBuffer;
    CAudioDecoderBuffer m_AudioDecodingBuffer;
    CAudioEncoder *m_pAudioEncoder;
    CAudioDecoder *m_pAudioDecoder;

    int m_iPacketNumber;

    bool m_bIsCheckCall;

    short m_saAudioEncodingFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_LENGHT];
    unsigned char m_ucaDecodingFrame[MAX_AUDIO_FRAME_LENGHT];
    short m_saDecodedFrame[MAX_AUDIO_FRAME_LENGHT];

    bool m_bAudioEncodingThreadRunning;
    bool m_bAudioEncodingThreadClosed;

    bool m_bAudioDecodingThreadRunning;
    bool m_bAudioDecodingThreadClosed;

    G729CodecNative *m_pG729CodecNative;

protected:

    SmartPointer<CLockHandler> m_pAudioCallSessionMutex;
    SmartPointer<std::thread> m_pAudioEncodingThread;
    SmartPointer<std::thread> m_pAudioDecodingThread;
};


#endif
