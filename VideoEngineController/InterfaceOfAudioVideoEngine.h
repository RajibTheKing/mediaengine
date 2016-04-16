
#ifndef _INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H_
#define _INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H_

#include <string>

class CController;

class CInterfaceOfAudioVideoEngine
{
    
public:
    
    CInterfaceOfAudioVideoEngine();
    CInterfaceOfAudioVideoEngine(const char* szLoggerPath, int nLoggerPrintLevel);
     ~CInterfaceOfAudioVideoEngine();
    
     bool Init(const IPVLongType& llUserID, const char* szLoggerPath, int nLoggerPrintLevel);
     bool InitializeLibrary(const IPVLongType& llUserID);
     bool SetUserName(const IPVLongType llUserName);
     bool StartAudioCall(const IPVLongType llFriendID);
     bool StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nNetworkType = 0);
     int EncodeAndTransfer(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength);
     int PushPacketForDecoding(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength);
     int PushAudioForDecoding(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength);
     int SendAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength);
     int SendVideoData(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength, unsigned int nOrientationType, int iHeight, int iWidth);
     int SetHeightWidth(const IPVLongType llFriendID, int nVideoWidth, int nVideoHeight);
     int SetBitRate(const IPVLongType llFriendID, int nBitRate);
     bool StopAudioCall(const IPVLongType llFriendID);
     bool StopVideoCall(const IPVLongType llFriendID);
     void SetLoggerPath(std::string strLoggerPath);
     bool SetLoggingState(bool bLoggingState, int nLogLevel);
     void UninitializeLibrary();
    
     void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
     void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int, int, int));
	 void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int));
     void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(IPVLongType, short*, int));
     void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));

     void SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int));
    
private:
    
    CController* m_pcController;
};

#endif