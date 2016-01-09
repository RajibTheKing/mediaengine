#ifndef _SHARED_BUCKET_H_
#define _SHARED_BUCKET_H_

#include <stdio.h>

#include "EventNotifier.h"
#include "VideoEncoderListHandler.h"
#include "VideoCallSessionListHandler.h"
#include "AudioCallSessionListHandler.h"

class CCommonElementsBucket
{

public:

	CCommonElementsBucket();
	~CCommonElementsBucket();
    
    void(*SendFunctionPointer)(LongLong, int, unsigned char*, int) = NULL;
    
    void SetUserName(const LongLong& username);
    LongLong GetUsername();
   
	CEventNotifier *m_pEventNotifier;
	CVideoCallSessionListHandler *m_pVideoCallSessionList;
	CAudioCallSessionListHandler *m_pAudioCallSessionList;
	CVideoEncoderListHandler *m_pVideoEncoderList;
	CLockHandler* GetSharedMutex();
    
    void SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int));
    
private:

    void InstantiateSharedMutex();

    LongLong m_friendID;
	LongLong userName;
	CLockHandler* sharedMutex;
};

#endif
