
#include "Controller.h"
#include "InterfaceOfAudioVideoEngine.h"
#include "LogPrinter.h"

CInterfaceOfAudioVideoEngine *G_pInterfaceOfAudioVideoEngine = NULL;

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine()
{
	G_pInterfaceOfAudioVideoEngine = this;
	m_pcController = new CController();
	m_llTimeOffset = -1;

	m_pcController->initializeEventHandler();
}

CInterfaceOfAudioVideoEngine::CInterfaceOfAudioVideoEngine(const char* szLoggerPath, int nLoggerPrintLevel)
{
	m_pcController = new CController(szLoggerPath, nLoggerPrintLevel);
	m_llTimeOffset = -1;

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

bool CInterfaceOfAudioVideoEngine::StartAudioCall(const IPVLongType llFriendID , int nServiceType)
{
	m_llTimeOffset = -1;

	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->StartAudioCall(llFriendID, nServiceType);

	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetVolume(const LongLong lFriendID, int iVolume, bool bRecorder)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetVolume(lFriendID, iVolume, bRecorder);
    return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetLoudSpeaker(const LongLong lFriendID, bool bOn)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetLoudSpeaker(lFriendID, bOn);
    return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::SetEchoCanceller(const LongLong lFriendID, bool bOn)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	bool bReturnedValue = m_pcController->SetEchoCanceller(lFriendID, bOn);
    
    return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nServiceType, int nEntityType, int packetSizeOfNetwork, int nNetworkType)
{
	m_llTimeOffset = -1;

	if (NULL == m_pcController)
	{
		return false;
	}

	m_pcController->m_pCommonElementsBucket->SetPacketSizeOfNetwork(packetSizeOfNetwork);

	bool bReturnedValue = m_pcController->StartVideoCall(llFriendID, nVideoHeight, nVideoWidth, nServiceType, nEntityType, nNetworkType);

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

int CInterfaceOfAudioVideoEngine::PushPacketForDecoding(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength)
{
	HITLER("#@#@26022017# RECEIVING DATA FOR BOKKOR %u", unLength);
	std::vector< std::pair<int, int> > vMissingFrames;

	return PushAudioForDecodingVector(llFriendID, mediaType, nEntityType, in_data, unLength, vMissingFrames);
}

int CInterfaceOfAudioVideoEngine::PushAudioForDecodingVector(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength, std::vector< std::pair<int, int> > vMissingFrames)
{
	HITLER("#@#@26022017# RECEIVING DATA FOR BOKKOR %u", unLength);
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
		//		if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[0])
		//        {
		//            iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data, unLength);
		//        }
		//		else if (AUDIO_PACKET_MEDIA_TYPE == (int)in_data[0])
		//        {
		//            iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, in_data, unLength);
		//        }
		//		else
		//			return 0;

		if (mediaType == MEDIA_TYPE_LIVE_STREAM && nEntityType != ENTITY_TYPE_PUBLISHER_CALLER)
		{
			//int lengthOfVideoData = m_Tools.UnsignedCharToIntConversion(in_data, 0);
			//int lengthOfAudioData = m_Tools.UnsignedCharToIntConversion(in_data, 4);

			/*int headerPosition;

			for (headerPosition = 0; numberOfMissingFrames > headerPosition && missingFrames[headerPosition] == headerPosition; headerPosition ++ )
			{
			if (headerPosition == NUMBER_OF_HEADER_FOR_STREAMING)
			return 5;
			}

			if(headerPosition >= NUMBER_OF_HEADER_FOR_STREAMING)
			return 6;*/

			int nValidHeaderOffset = 0;
						
			long long itIsNow = m_Tools.CurrentTimestamp();
			long long recvTimeOffset = m_Tools.GetMediaUnitTimestampInMediaChunck(in_data + nValidHeaderOffset);

			//LOGE("##DE#Interface## now %lld peertimestamp %lld timediff %lld relativediff %lld", itIsNow, recvTimeOffset, itIsNow - m_llTimeOffset, recvTimeOffset);

			if (m_llTimeOffset == -1)
			{
				m_llTimeOffset = itIsNow - recvTimeOffset;
				m_pcController->m_llLastTimeStamp = recvTimeOffset;
				HITLER("##DE#interface*first# timestamp:%lld recv:%lld", m_llTimeOffset, recvTimeOffset);
			}
			else
			{
				long long expectedTime = itIsNow - m_llTimeOffset;
				int tmp_headerLength = m_Tools.GetMediaUnitHeaderLengthFromMediaChunck(in_data + nValidHeaderOffset);
				int tmp_chunkDuration = m_Tools.GetMediaUnitChunkDurationFromMediaChunck(in_data + nValidHeaderOffset);
				HITLER("@#DE#Interface##now:%lld peertimestamp:%lld expected:%lld  [%lld] CHUNK_DURA = %d HEAD_LEN = %d ", itIsNow, recvTimeOffset, expectedTime, recvTimeOffset - expectedTime, tmp_chunkDuration , tmp_headerLength);
				
				if (recvTimeOffset < expectedTime - __CHUNK_DELAY_TOLERANCE__) {
					if (!m_pcController->IsCallInLiveEnabled())
					{
						//HITLER("##Discarding packet! | expected:%lld", expectedTime);
						//return -10;
					}
				}
				if (m_pcController->m_llLastTimeStamp >= recvTimeOffset) {
					// HITLER("#@#@26022017# RECEIVING DATA FOR BOKKOR %u", unLength);
					HITLER("#@#@26022017# ##Interface discarding duplicate packet.");
					return -10;
				}

				m_pcController->m_llLastTimeStamp = max(m_pcController->m_llLastTimeStamp, recvTimeOffset);
			}
			
			int version = m_Tools.GetMediaUnitVersionFromMediaChunck(in_data + nValidHeaderOffset);

			int headerLength = m_Tools.GetMediaUnitHeaderLengthFromMediaChunck(in_data + nValidHeaderOffset);
			int chunkDuration = m_Tools.GetMediaUnitChunkDurationFromMediaChunck(in_data + nValidHeaderOffset);

			LOGEF("headerLength %d chunkDuration %d\n", headerLength, chunkDuration);

			int lengthOfAudioData = m_Tools.GetAudioBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);
			int lengthOfVideoData = m_Tools.GetVideoBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);

			//LOGEF("THeKing--> interface:receive ############## lengthOfVideoData =  %d  Pos=%d   Offset= %d,  \n", lengthOfVideoData,headerPosition, nValidHeaderOffset);

			int audioFrameSizes[100];
			int videoFrameSizes[100];

			int blockInfoPosition = m_Tools.GetMediaUnitBlockInfoPositionFromMediaChunck(in_data + nValidHeaderOffset);

			int numberOfAudioFrames = m_Tools.GetNumberOfAudioFramesFromMediaChunck(blockInfoPosition, in_data + nValidHeaderOffset);

			int index = blockInfoPosition + LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE;

			for (int i = 0; i < numberOfAudioFrames; i++)
			{
				audioFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + nValidHeaderOffset);

				index += LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE;
			}

			int numberOfVideoFrames = m_Tools.GetNumberOfVideoFramesFromMediaChunck(index, in_data + nValidHeaderOffset);

			index += LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE;

			for (int i = 0; i < numberOfVideoFrames; i++)
			{
				videoFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + nValidHeaderOffset);

				index += LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE;
			}

			LOG_AAC("#aac#b4q# GotNumberOfAudioFrames: %d, numberOfVideoFrames: %d, missingVectorSize: %d, audioDataSize: %d", numberOfAudioFrames, numberOfVideoFrames, vMissingFrames.size(), lengthOfAudioData);

			int audioStartingPosition = m_Tools.GetAudioBlockStartingPositionFromMediaChunck(in_data + nValidHeaderOffset);
			int videoStartingPosition = m_Tools.GetVideoBlockStartingPositionFromMediaChunck(in_data + nValidHeaderOffset);

			int streamType = m_Tools.GetMediaUnitStreamTypeFromMediaChunck(in_data + nValidHeaderOffset);

			LOGE("audioStartingPosition %d videoStartingPosition %d streamType %d\n", audioStartingPosition, videoStartingPosition, streamType);

			iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, audioStartingPosition, in_data, lengthOfAudioData, numberOfAudioFrames, audioFrameSizes, vMissingFrames);

			//m_Tools.SOSleep(100); //Temporary Fix to Sync Audio And Video Data for LIVE STREAM SERVICE
#ifndef DISABLE_VIDEO_FOR_LIVE
			iReturnedValue = m_pcController->PushPacketForDecodingVector(llFriendID, videoStartingPosition, in_data + videoStartingPosition, lengthOfVideoData, numberOfVideoFrames, videoFrameSizes, vMissingFrames);
#endif
			
		}
		else if (mediaType == MEDIA_TYPE_AUDIO || mediaType == MEDIA_TYPE_VIDEO)
		{
			if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[0])
			{
				iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data + 1, unLength - 1); //Skip First byte for Video Data
			}
			else if (AUDIO_PACKET_MEDIA_TYPE == (int)in_data[0])
			{
				iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, 0, in_data + 1, unLength - 1, 0, NULL, vMissingFrames); //Skip First byte for Audio Data
			}
			else
				return 0;
		}
		else if (mediaType == MEDIA_TYPE_LIVE_CALL_AUDIO || mediaType == MEDIA_TYPE_LIVE_CALL_VIDEO)
		{
			if (VIDEO_PACKET_MEDIA_TYPE == (int)in_data[1])
			{
				iReturnedValue = m_pcController->PushPacketForDecoding(llFriendID, in_data + 2, unLength - 2); //Skip First byte for Video Data
			}
			else
			{
				iReturnedValue = m_pcController->PushAudioForDecoding(llFriendID, 0, in_data + 1, unLength - 1, 0, NULL, vMissingFrames); //Skip First byte for Audio Data
			}
		}
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

int CInterfaceOfAudioVideoEngine::SetDeviceDisplayHeightWidth(int nVideoHeight, int nVideoWidth)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SetDeviceDisplayHeightWidth(nVideoHeight, nVideoWidth);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::SetVideoEffect(const IPVLongType llFriendID, int nEffectStatus)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->SetVideoEffect(llFriendID, nEffectStatus);

	return iReturnedValue;
}

int CInterfaceOfAudioVideoEngine::TestVideoEffect(const IPVLongType llFriendID, int *param, int size)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	int iReturnedValue = m_pcController->TestVideoEffect(llFriendID, param, size);

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


int CInterfaceOfAudioVideoEngine::StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data,int iLen, int nVideoHeight, int nVideoWidth)
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StartVideoMuxingAndEncodeSession(pBMP32Data, iLen, nVideoHeight, nVideoWidth);
	}

	return nReturnedValue;

}

int CInterfaceOfAudioVideoEngine::FrameMuxAndEncode( unsigned char *pVideoYuv, int iHeight, int iWidth)
{

	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->FrameMuxAndEncode(pVideoYuv, iHeight, iWidth);
	}

	return nReturnedValue;

}

int CInterfaceOfAudioVideoEngine::StopVideoMuxingAndEncodeSession(unsigned char *finalData)
{
	int nReturnedValue = 0;

	if (NULL != m_pcController)
	{
		nReturnedValue = m_pcController->StopVideoMuxingAndEncodeSession(finalData);
	}

	return nReturnedValue;

}

void CInterfaceOfAudioVideoEngine::InterruptOccured(const LongLong lFriendID)
{
	if (NULL != m_pcController)
	{
		m_pcController->InterruptOccured(lFriendID);
	}
}

void CInterfaceOfAudioVideoEngine::InterruptOver(const LongLong lFriendID)
{
	if (NULL != m_pcController)
	{
		m_pcController->InterruptOver(lFriendID);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(LongLong, unsigned char*, int))
{
	if (NULL != m_pcController)
	{
		m_pcController->SetNotifyClientWithPacketCallback(callBackFunctionPointer);
	}
}

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(LongLong, int, unsigned char*, int, int, int, int))
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

void CInterfaceOfAudioVideoEngine::SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(LongLong, int, short*, int))
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

void CInterfaceOfAudioVideoEngine::SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int, int))
{
    if (NULL != m_pcController)
    {
        m_pcController->SetSendFunctionPointer(callBackFunctionPointer);
    }
}

bool CInterfaceOfAudioVideoEngine::StartCallInLive(const IPVLongType llFriendID, int iRole)
{
	if (NULL == m_pcController)
	{
		return false;
	}
	m_llTimeOffset = -1;
	bool bReturnedValue = m_pcController->StartAudioCallInLive(llFriendID, iRole);
	
	m_pcController->SetCallInLiveEnabled(true);
	
	if (bReturnedValue)
	{
		bReturnedValue = m_pcController->StartVideoCallInLive(llFriendID);
	}
	
	return bReturnedValue;
}

bool CInterfaceOfAudioVideoEngine::EndCallInLive(const IPVLongType llFriendID)
{
	if (NULL == m_pcController)
	{
		return false;
	}

	m_llTimeOffset = -1;
	bool bReturnedValue = m_pcController->EndAudioCallInLive(llFriendID);
	
	m_pcController->SetCallInLiveEnabled(false);
	

	if (bReturnedValue)
	{
		bReturnedValue = m_pcController->EndVideoCallInLive(llFriendID);
	}
	
	return bReturnedValue;
}



