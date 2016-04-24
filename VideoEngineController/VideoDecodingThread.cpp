
#include "VideoDecodingThread.h"
#include "VideoCallSession.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

extern map<int,long long>g_ArribalTime;

CVideoDecodingThread::CVideoDecodingThread(CEncodedFrameDepacketizer *encodedFrameDepacketizer, CRenderingBuffer *renderingBuffer, CVideoDecoder *videoDecoder, CColorConverter *colorConverter, CVideoCallSession* pVideoCallSession) :
m_pEncodedFrameDepacketizer(encodedFrameDepacketizer),
m_RenderingBuffer(renderingBuffer),
m_pVideoDecoder(videoDecoder),
m_pColorConverter(colorConverter),
m_pVideoCallSession(pVideoCallSession)
{

}

CVideoDecodingThread::~CVideoDecodingThread()
{

}

void CVideoDecodingThread::StopDecodingThread()
{
	//if (pDepacketizationThread.get())
	{
		bDecodingThreadRunning = false;

		while (!bDecodingThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}
	//pDepacketizationThread.reset();
}

void CVideoDecodingThread::StartDecodingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDecodingThread::StartDecodingThread called");

	if (pDecodingThread.get())
	{
		pDecodingThread.reset();

		return;
	}

	bDecodingThreadRunning = true;
	bDecodingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t PacketizationThreadQ = dispatch_queue_create("PacketizationThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(PacketizationThreadQ, ^{
		this->DecodingThreadProcedure();
	});

#else

	std::thread myThread(CreateDecodingThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDecodingThread::StartDecodingThread Decoding Thread started");

	return;
}

void *CVideoDecodingThread::CreateDecodingThread(void* param)
{
	CVideoDecodingThread *pThis = (CVideoDecodingThread*)param;
	pThis->DecodingThreadProcedure();

	return NULL;
}

void CVideoDecodingThread::DecodingThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDecodingThread::DecodingThreadProcedure() started DecodingThreadProcedure method");

	Tools toolsObject;

	int frameSize, nFrameNumber, intervalTime, nFrameLength, nEncodingTime, nOrientation;
	unsigned int nTimeStampDiff = 0;
	long long nTimeStampBeforeDecoding, currentTime;
	long long nMaxDecodingTime = 0;
	int nExpectedTime;

	int nDecodingStatus, fps = -1;
	double dbAverageDecodingTime = 0, dbTotalDecodingTime = 0;
	int nOponnentFPS, nMaxProcessableByMine;
	int m_iDecodedFrameCounter = 0;

	nExpectedTime = -1;
	long long maxDecodingTime = 0, framCounter = 0, decodingTime, nBeforeDecodingTime;
	double decodingTimeAverage = 0;

	while (bDecodingThreadRunning)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDecodingThread::DecodingThreadProcedure() RUNNING DecodingThreadProcedure method");

		if( -1 == m_pVideoCallSession->GetShiftedTime())
		{
			toolsObject.SOSleep(10);
			continue;
		}

		currentTime = toolsObject.CurrentTimestamp();
		nExpectedTime = currentTime - m_pVideoCallSession->GetShiftedTime();

		nFrameLength = m_pEncodedFrameDepacketizer->GetReceivedFrame(m_PacketizedFrame, nFrameNumber, nEncodingTime, nExpectedTime, 0, nOrientation);

		if (nFrameLength>-1) {
			CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG ,"#$Dec# FN: " +
																 m_Tools.IntegertoStringConvert(
																		 nFrameNumber) + "  Len: " +
																 m_Tools.IntegertoStringConvert(
																		 nFrameLength) +
																 "  E.Time: " +
																 m_Tools.IntegertoStringConvert(
																		 nEncodingTime)
																 + "  Exp E.Time: " +
																 m_Tools.IntegertoStringConvert(
																		 nExpectedTime) + " -> " +
																 m_Tools.IntegertoStringConvert(
																		 nExpectedTime -
																		 nEncodingTime) + "Orientation = " +
																 m_Tools.IntegertoStringConvert(nOrientation));
			CLogPrinter_WriteLog(CLogPrinter::DEBUGS, DEPACKETIZATION_LOG ,"#$ Cur: " +m_Tools.LongLongToString(currentTime) +" diff: "+m_Tools.LongLongToString(currentTime - g_ArribalTime[nFrameNumber]));
		}
		if (-1 == nFrameLength) 
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDecodingThread::DecodingThreadProcedure() NOTHING for decoding method");

			toolsObject.SOSleep(1);
		}
		else
		{
			nBeforeDecodingTime = toolsObject.CurrentTimestamp();

			nOponnentFPS = m_pVideoCallSession->GetFPSController()->GetOpponentFPS();
			nMaxProcessableByMine = m_pVideoCallSession->GetFPSController()->GetMaxOwnProcessableFPS();

			if (nOponnentFPS > 1 + nMaxProcessableByMine && (nFrameNumber & 7) > 3) {
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDecodingThread::DecodingThreadProcedure() Force:: Frame: " + m_Tools.IntegertoStringConvert(nFrameNumber) + "  FPS: " + m_Tools.IntegertoStringConvert(nOponnentFPS) + " ~" + toolsObject.IntegertoStringConvert(nMaxProcessableByMine));
				toolsObject.SOSleep(5);
				continue;
			}

			/*
			if(nFrameNumber<200)
			{
			string str = "/Decode/" + m_Tools.IntegertoStringConvert(nFrameNumber) + "_" + m_Tools.IntegertoStringConvert(nFrameLength);
			str+=".dump";
			[[Helper_IOS GetInstance] WriteToFile:str.c_str() withData:m_PacketizedFrame dataLength:nFrameLength];
			}
			*/


			nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame, nFrameLength, nFrameNumber, nEncodingTime, nOrientation);
			//printf("decode:  %d, nDecodingStatus %d\n", nFrameNumber, nDecodingStatus);
			//			toolsObject.SOSleep(100);

			if (nDecodingStatus > 0) {
				decodingTime = toolsObject.CurrentTimestamp() - nBeforeDecodingTime;
				dbTotalDecodingTime += decodingTime;
				++m_iDecodedFrameCounter;
				nMaxDecodingTime = max(nMaxDecodingTime, decodingTime);
				if (0 == (m_iDecodedFrameCounter & 3))
				{
					dbAverageDecodingTime = dbTotalDecodingTime / m_iDecodedFrameCounter;
					dbAverageDecodingTime *= 1.5;

					if (dbAverageDecodingTime > 30)
					{
						fps = 1000 / dbAverageDecodingTime;
					//	printf("WinD--> Error Case Average Decoding time = %lf, fps = %d\n", dbAverageDecodingTime, fps);
						if (fps < FPS_MAXIMUM)
							m_pVideoCallSession->GetFPSController()->SetMaxOwnProcessableFPS(fps);
					}
				}
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDecodingThread::DecodingThreadProcedure() Force:: AVG Decoding Time:" + m_Tools.DoubleToString(dbAverageDecodingTime) + "  Max Decoding-time: " + m_Tools.IntegertoStringConvert(nMaxDecodingTime) + "  MaxOwnProcessable: " + m_Tools.IntegertoStringConvert(fps));
			}

			toolsObject.SOSleep(1);
		}
	}

	bDecodingThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDecodingThread::DecodingThreadProcedure() stopped DecodingThreadProcedure method.");
}

int CVideoDecodingThread::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff, int nOrientation)
{
	long long currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
	long long dectime = m_Tools.CurrentTimestamp();
	m_decodedFrameSize = m_pVideoDecoder->DecodeVideoFrame(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

	long long  timediff = (m_Tools.CurrentTimestamp() - dectime);
	CLogPrinter_WriteSpecific5(CLogPrinter::INFO, " OOOO decode  " + m_Tools.LongLongToString( timediff));
	CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Decode ", currentTimeStamp);

	if (1 > m_decodedFrameSize)
		return -1;

	currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertI420ToNV21 ");
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	this->m_pColorConverter->ConvertI420ToNV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
#elif defined(_DESKTOP_C_SHARP_)
	//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "DepacketizationThreadProcedure() For Desktop");
	m_decodedFrameSize = this->m_pColorConverter->ConverterYUV420ToRGB24(m_DecodedFrame, m_RenderingRGBFrame, m_decodingHeight, m_decodingWidth);
#elif defined(TARGET_OS_WINDOWS_PHONE)
	this->m_pColorConverter->ConvertI420ToYV12(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
#else

	this->m_pColorConverter->ConvertI420ToNV21(m_DecodedFrame, m_decodingHeight, m_decodingWidth);
#endif
	CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertI420ToNV21 ", currentTimeStamp);

	long long currentTimeStampForBrust = m_Tools.CurrentTimestamp();

#if defined(_DESKTOP_C_SHARP_)
	m_RenderingBuffer->Queue(nFramNumber, m_RenderingRGBFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth, nOrientation);
	return m_decodedFrameSize;
#else

	m_RenderingBuffer->Queue(nFramNumber, m_DecodedFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth, nOrientation);
	return m_decodedFrameSize;
#endif
}
