
#include "Controller.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "LogPrinter.h"

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine()
{
	m_pcController = new CController();

	m_pcController->initializeEventHandler();
}

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine(const char* szLoggerPath, int nLoggerPrintLevel)
{
	m_pcController = new CController(szLoggerPath, nLoggerPrintLevel);

	m_pcController->initializeEventHandler();
}

bool CInterfaceOfAudioVideoEngine::Init(const IPVLongType& llUserID, const char* szLoggerPath, int nLoggerPrintLevel)
{
    return true;
}

bool CInterfaceOfAudioVideoEngine::InitializeLibrary(const IPVLongType& llUserID)
{
    return true;
}

CInterfaceOfAudioVideoEngine::~CInterfaceOfAudioVideoEngine()
{
	if (NULL != m_pcController)
	{
		delete m_pcController;

		m_pcController = NULL;
	}
}

bool CInterfaceOfAudioVideoEngine::SetUserName(const IPVLongType llUserName)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	m_pcController->initializeEventHandler();

	bool Ret = m_pcController->SetUserName(llUserName);

	return Ret;
}

bool CInterfaceOfAudioVideoEngine::StartAudioCall(const IPVLongType llFriendID)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StartAudioCall(llFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetVolume(const LongLong lFriendID, int iVolume, bool bRecorder)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetVolume(lFriendID, iVolume, bRecorder);
}

bool CInterfaceOfAudioVideoEngine::SetLoudSpeaker(const LongLong lFriendID, bool bOn)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetLoudSpeaker(lFriendID, bOn);
}

bool CInterfaceOfAudioVideoEngine::SetEchoCanceller(const LongLong lFriendID, bool bOn)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetEchoCanceller(lFriendID, bOn);
}

bool CInterfaceOfAudioVideoEngine::StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nNetworkType)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StartVideoCall(llFriendID, nVideoHeight, nVideoWidth,nNetworkType);

	return bReturnedValue;
}

int CInterfaceOfAudioVideoEngine::EncodeAndTransfer(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->EncodeVideoFrame(llFriendID, in_data, unLength);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::PushPacketForDecoding(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength)
{   
	return -1;
}

int CInterfaceOfAudioVideoEngine::PushAudioForDecoding(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength)
{ 
    int iReturnedValue = 0;
    
	if (NULL == m_pcController)
    {
        return 0;
    }
	else if (nullptr == in_data)
    {
        CLogPrinter_Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding null data from connectivity");
        
        return 0;
    }
	else if (NULL == in_data)
    {
        CLogPrinter_Write(CLogPrinter::DEBUGS, "CInterfaceOfAudioVideoEngine::PushAudioForDecoding null data from connectivity");
        
        return 0;
    }
    else
    {
		if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[0])
        {
            iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data, unLength);
        }
		else if (AUDIO_PACKET_MEDIA_TYPE == (int)in_data[0])
        {
            iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, in_data, unLength);
        }
        else
            return 0;
    }

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SendAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SendAudioData(llFriendID, in_data, unLength);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::CancelAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->CancelAudioData(llFriendID, in_data, unLength);

	return iReturnedValue;
}


int CInterfaceOfAudioVideoEngine::SendVideoData(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength, unsigned int nOrientationType, int device_orientation)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SendVideoData(llFriendID, in_data, unLength, nOrientationType, device_orientation);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetEncoderHeightWidth(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SetEncoderHeightWidth(llFriendID, nVideoHeight, nVideoWidth);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetBitRate(const IPVLongType llFriendID, int nBitRate)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SetBitRate(llFriendID, nBitRate);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->CheckDeviceCapability(lFriendID, iHeightHigh, iWidthHigh, iHeightLow, iWidthLow);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow)
{
    if(NULL == m_pcController)
    {
        return false;
    }
    
    int iReturnedValue = m_pcController->SetDeviceCapabilityResults(iNotification, iHeightHigh, iWidthHigh, iHeightLow, iWidthLow);
    
    return iReturnedValue;
    
}

bool CInterfaceOfAudioVideoEngine::StopAudioCall(const IPVLongType llFriendID)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StopAudioCall(llFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StopVideoCall(const IPVLongType llFriendID)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StopVideoCall(llFriendID);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetLoggingState(bool bLoggingState, int nLogLevel)
{
	if (NULL == m_pcController)
    {
        return false;
    }
    
    bool bReturnedValue = m_pcController->SetLoggingState(bLoggingState, nLogLevel);
    
    return bReturnedValue;
}

void CInterfaceOfAudioVideoEngine::UninitializeLibrary()
{
	if (NULL != m_pcController)
	{
		m_pcController->UninitializeLibrary();
	}
}

void CInterfaceOfAudioVideoEngine::SetLoggerPath(std::string strLoggerPath)
{
	if (NULL != m_pcController)
	{
		m_pcController->SetLoggerPath(strLoggerPath);
	}
}

int CInterfaceOfAudioVideoEngine::StartAudioEncodeDecodeSession()
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StartAudioEncodeDecodeSession();
	}

	return nReturnedValue;
}

int CInterfaceOfAudioVideoEngine::EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer)
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->EncodeAudioFrame(psaEncodingDataBuffer, nAudioFrameSize, ucaEncodedDataBuffer);
	}

	return nReturnedValue;
}

int CInterfaceOfAudioVideoEngine::DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer)
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->DecodeAudioFrame(ucaDecodedDataBuffer, nAudioFrameSize, psaDecodingDataBuffer);
	}

	return nReturnedValue;
}

int CInterfaceOfAudioVideoEngine::StopAudioEncodeDecodeSession()
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StopAudioEncodeDecodeSession();
	}

	return nReturnedValue;
}


void CInterfaceOfAudioVideoEngine::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithPacketCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int, int, int, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithVideoDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(LongLong, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithVideoNotificationCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithNetworkStrengthNotificationCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithAudioDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithAudioPacketDataCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(LongLong, short*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithAudioAlarmCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int))
{
    if (NULL != m_pcController)
    {
        m_pcController->SetSendFunctionPointer(callBackFunctionPointer);
    }
}

