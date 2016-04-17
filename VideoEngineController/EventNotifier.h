#ifndef _EVENT_NOTIFIER_H_
#define _EVENT_NOTIFIER_H_

#include <stdio.h>

#include "AudioVideoEngineDefinitions.h"
#include "LogPrinter.h"

class CEventNotifier
{
    
public:

	void firePacketEvent(int eventType, int frameNumber, int numberOfPackets, int packetNumber, int packetSize, int dataLenth, unsigned char data[]);
	void fireVideoEvent(int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth);
	void fireAudioPacketEvent(int eventType, int dataLenth, unsigned char data[]);
	void fireAudioEvent(int eventType, int dataLenth, short data[]);
    void fireVideoNotificationEvent(long long callID, int eventType);
    
    void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int));
    void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int));
	void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
    void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int));
    void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
	
    static const int VIDEO_QUALITY_LOW = 202;
	static const int VIDEO_QUALITY_HIGH = 204;
    static const int VIDEO_SHOULD_STOP = 203;

	static const int SET_CAMERA_RESOLUTION_640x480 = 205;
	static const int SET_CAMERA_RESOLUTION_352x288_OR_320x240 = 206;
};

#endif
