
#include "VideoDecodingThread.h"

int iValuableFrameUsedCounter = 0;

CVideoDecodingThread::CVideoDecodingThread(CEncodedFrameDepacketizer *encodedFrameDepacketizer, CRenderingBuffer *renderingBuffer, CVideoDecoder *videoDecoder, CColorConverter *colorConverter, CFPSController *FPSController) :

m_pEncodedFrameDepacketizer(encodedFrameDepacketizer),
m_RenderingBuffer(renderingBuffer),
m_pVideoDecoder(videoDecoder),
m_pColorConverter(colorConverter),
g_FPSController(FPSController)

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
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 1");

	if (pDecodingThread.get())
	{
		//		CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 2");
		pDecodingThread.reset();
		//		CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 3");
		return;
	}

	//	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 4");

	bDecodingThreadRunning = true;
	bDecodingThreadClosed = false;

	//	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread 5");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t PacketizationThreadQ = dispatch_queue_create("PacketizationThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(PacketizationThreadQ, ^{
		this->DecodingThreadProcedure();
	});

#else

	std::thread myThread(CreateDecodingThread, this);
	myThread.detach();

#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::StartDepacketizationThread Decoding Thread started");

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
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method.");
	Tools toolsObject;

	int frameSize, nFrameNumber, intervalTime, nFrameLength, nEncodingTime;
	unsigned int nTimeStampDiff = 0;
	long long nTimeStampBeforeDecoding, nFirstFrameDecodingTime, nFirstFrameEncodingTime, currentTime, nShiftedTime;
	long long nMaxDecodingTime = 0;
	int RenderFaildCounter = 0;
	int nExpectedTime;

	int nDecodingStatus, fps = -1;
	double dbAverageDecodingTime = 0, dbTotalDecodingTime = 0;
	int nOponnentFPS, nMaxProcessableByMine;
	int m_iDecodedFrameCounter = 0;

	nFirstFrameDecodingTime = -1;
	nExpectedTime = -1;
	long long maxDecodingTime = 0, framCounter = 0, decodingTime, nBeforeDecodingTime;
	double decodingTimeAverage = 0;

	while (bDecodingThreadRunning)
	{


		currentTime = toolsObject.CurrentTimestamp();
		if (-1 != nFirstFrameDecodingTime)
			nExpectedTime = currentTime - nShiftedTime;


		nFrameLength = m_pEncodedFrameDepacketizer->GetReceivedFrame(m_PacketizedFrame, nFrameNumber, nEncodingTime, nExpectedTime, 0);
		//printf("FrameLength:  %d\n", nFrameLength);

		decodingTime = toolsObject.CurrentTimestamp() - currentTime;

		if (nFrameLength>-1)
			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " GetReceivedFrame # Get Time: " + m_Tools.IntegertoStringConvert(decodingTime) + "  Len: " + m_Tools.IntegertoStringConvert(nFrameLength) + "  FrameNo: " + m_Tools.IntegertoStringConvert(nFrameNumber));


		if (-1 == nFrameLength) {
			toolsObject.SOSleep(10);
		}
		else
		{
			nBeforeDecodingTime = toolsObject.CurrentTimestamp();
			if (-1 == nFirstFrameDecodingTime)
				nTimeStampBeforeDecoding = nBeforeDecodingTime;

			nOponnentFPS = g_FPSController->GetOpponentFPS();
			nMaxProcessableByMine = g_FPSController->GetMaxOwnProcessableFPS();

			if (nOponnentFPS > 1 + nMaxProcessableByMine && (nFrameNumber & 7) > 3) {
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "Force:: Frame: " + m_Tools.IntegertoStringConvert(nFrameNumber) + "  FPS: " + m_Tools.IntegertoStringConvert(nOponnentFPS) + " ~" + toolsObject.IntegertoStringConvert(nMaxProcessableByMine));
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


			nDecodingStatus = DecodeAndSendToClient(m_PacketizedFrame, nFrameLength, nFrameNumber, nTimeStampDiff);
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
					fps = 1000 / dbAverageDecodingTime;

					if (fps<FPS_MAXIMUM)
						g_FPSController->SetMaxOwnProcessableFPS(fps);
				}
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "Force:: AVG Decoding Time:" + m_Tools.DoubleToString(dbAverageDecodingTime) + "  Max Decoding-time: " + m_Tools.IntegertoStringConvert(nMaxDecodingTime) + "  MaxOwnProcessable: " + m_Tools.IntegertoStringConvert(fps));
			}

			if (-1 == nFirstFrameDecodingTime)
			{
				nFirstFrameDecodingTime = nTimeStampBeforeDecoding;
				nFirstFrameEncodingTime = nEncodingTime;
				nShiftedTime = nFirstFrameDecodingTime - nEncodingTime;
			}

			toolsObject.SOSleep(5);
		}
	}

	bDecodingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoCallSession::DepacketizationThreadProcedure() Stopped DepacketizationThreadProcedure method.");
}

int CVideoDecodingThread::DecodeAndSendToClient(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int nTimeStampDiff)
{
	//printf("Wind--> DecodeAndSendToClient 0\n");
#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
	if (g_TraceRetransmittedFrame[nFramNumber] == 1)
	{
		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "Very valuable frame used " + m_Tools.IntegertoStringConvert(nFramNumber) + ", counter =  " + m_Tools.IntegertoStringConvert(iValuableFrameUsedCounter));
		iValuableFrameUsedCounter++;
	}
	CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "$$$Very Valuable Retransmission packet used counter =  " + m_Tools.IntegertoStringConvert(iValuableFrameUsedCounter));

#endif

	long long currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
	m_decodedFrameSize = m_pVideoDecoder->Decode(in_data, frameSize, m_DecodedFrame, m_decodingHeight, m_decodingWidth);

	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Decode ", currentTimeStamp);

	if (1 > m_decodedFrameSize)
		return -1;

	currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertI420ToNV21 ");
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
	CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertI420ToNV21 ", currentTimeStamp);
#if defined(_DESKTOP_C_SHARP_)
	m_RenderingBuffer->Queue(nFramNumber, m_RenderingRGBFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth);
	return m_decodedFrameSize;
#else

	m_RenderingBuffer->Queue(nFramNumber, m_DecodedFrame, m_decodedFrameSize, nTimeStampDiff, m_decodingHeight, m_decodingWidth);
	return m_decodedFrameSize;
#endif
}