#ifndef IPV_EVENT_NOTIFIER_H
#define IPV_EVENT_NOTIFIER_H

#include <stdio.h>

#include "AudioVideoEngineDefinitions.h"
#include "LogPrinter.h"

class CController;
class CEventNotifier
{
	
public:
	CEventNotifier(CController *pController);

	void firePacketEvent(int eventType, int frameNumber, int numberOfPackets, int packetNumber, int packetSize, int dataLenth, unsigned char data[]);

#if defined(DESKTOP_C_SHARP)

	void fireVideoEvent(long long friendID, int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth, int nInsetHeight, int nInsetWidth, int iOrientation);

#else

	void fireVideoEvent(long long friendID, int eventType, int frameNumber, int dataLenth, unsigned char data[], int iVideoHeight, int iVideoWidth, int iOrientation);

#endif

	void fireAudioPacketEvent(int eventType, int dataLenth, unsigned char data[]);
	void fireAudioEvent(long long friendID, int eventType, int dataLenth, short data[]);
	void fireAudioAlarm(int eventType, int dataLenth, short data[]);
    void fireVideoNotificationEvent(long long callID, int eventType);
	void fireNetworkStrengthNotificationEvent(long long callID, int eventType);
    
    void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int));

#if defined(DESKTOP_C_SHARP)

	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int, int, int));

#else

	void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int));

#endif

	void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
	void SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(LongLong, int));
    void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int));
	void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
	void SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int));
	bool IsVideoCallRunning();

	
    static const int NETWORK_STRENTH_GOOD = 202;
	static const int NETWORK_STRENTH_EXCELLENT = 204;
    static const int NETWORK_STRENTH_BAD = 203;
	static const int VIDEO_SHOULD_STOP = 201;

	static const int SET_CAMERA_RESOLUTION_640x480_25FPS = 205;
    static const int SET_CAMERA_RESOLUTION_640x480_25FPS_NOT_SUPPORTED = 206;
    static const int SET_CAMERA_RESOLUTION_352x288_25FPS = 207;
	static const int SET_CAMERA_RESOLUTION_352x288_25FPS_NOT_SUPPORTED = 208;
    
    static const int SET_CAMERA_RESOLUTION_352x288_15FPS = 209;

	static const int SET_CAMERA_RESOLUTION_352x288 = 210;
	static const int SET_CAMERA_RESOLUTION_640x480 = 211;
    static const int SET_CAMERA_RESOLUTION_FAILED= 221;

	static const int VIDEO_SESSION_START_FAILED = 121;

	static const int LIVE_CALL_INSET_ON = 231;
	static const int LIVE_CALL_INSET_OFF = 232;

	static const int RESOLUTION_NOT_SUPPORTED = 127;

private:
	CController *m_pController;
};

#endif
