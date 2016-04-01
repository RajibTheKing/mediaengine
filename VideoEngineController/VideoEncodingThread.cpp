
#include "VideoEncodingThread.h"
#include "Globals.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

extern CFPSController g_FPSController;

#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8

int countFrame = 0;
int countFrameSize = 0;
long long encodeTimeStampFor15 = 0;

CVideoEncodingThread::CVideoEncodingThread(LongLong friendID, CEncodingBuffer *encodingBuffer, BitRateController *bitRateController, CColorConverter *colorConverter, CVideoEncoder *videoEncoder, CEncodedFramePacketizer *encodedFramePacketizer) :

m_iFrameNumber(0),
m_FriendID(friendID),
m_EncodingBuffer(encodingBuffer),
m_BitRateController(bitRateController),
m_pColorConverter(colorConverter),
m_pVideoEncoder(videoEncoder),
m_pEncodedFramePacketizer(encodedFramePacketizer)

{
	countFrame = 0;
	countFrameSize = 0;
	encodeTimeStampFor15 = 0;
}

CVideoEncodingThread::~CVideoEncodingThread()
{

}


void CVideoEncodingThread::StopEncodingThread()
{
	//if (pInternalThread.get())
	{

		bEncodingThreadRunning = false;

		while (!bEncodingThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}

	//pInternalThread.reset();
}

void CVideoEncodingThread::StartEncodingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::StartEncodingThread called");

	if (pEncodingThread.get())
	{
		pEncodingThread.reset();
		
		return;
	}
	
	bEncodingThreadRunning = true;
	bEncodingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(EncodeThreadQ, ^{
		this->EncodingThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoEncodingThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::StartEncodingThread Encoding Thread started");

	return;
}

void *CVideoEncodingThread::CreateVideoEncodingThread(void* param)
{
	CVideoEncodingThread *pThis = (CVideoEncodingThread*)param;
	pThis->EncodingThreadProcedure();

	return NULL;
}

void CVideoEncodingThread::EncodingThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() started EncodingThreadProcedure method");

	Tools toolsObject;
	int frameSize, encodedFrameSize;
	long long encodingTime, encodingTimeStamp, nMaxEncodingTime = 0, currentTimeStamp;
	double dbTotalEncodingTime = 0;
	int iEncodedFrameCounter = 0;
	encodingTimeStamp = toolsObject.CurrentTimestamp();
	long long encodingTimeFahadTest = 0;

	long long iterationtime = toolsObject.CurrentTimestamp();

	int m_iTimeStampDiff = 0;
	int m_FrameCounterbeforeEncoding = 0;	

	for(int i = 0; i < 200; i++)
	{
		if(m_BitRateController->m_iNetTypeMiniPktRcv)
		{
			CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure() m_BitRateController->m_iNetworkType after waiting = " + toolsObject.IntegertoStringConvert(m_BitRateController->m_iOpponentNetworkType));
			break;
		}
		toolsObject.SOSleep(10);
	}


	CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure() m_BitRateController->m_iNetworkType after waiting = own "+ toolsObject.IntegertoStringConvert(m_BitRateController->m_iOwnNetworkType) +
												  " opponent  " + toolsObject.IntegertoStringConvert(m_BitRateController->m_iOpponentNetworkType));

	if(m_BitRateController->m_iOpponentNetworkType == NETWORK_TYPE_2G || m_BitRateController->m_iOwnNetworkType == NETWORK_TYPE_2G)
	{

		m_pVideoEncoder->SetBitrate(BITRATE_MIN);
		m_pVideoEncoder->SetMaxBitrate(BITRATE_MIN);

	}
	else
	{
		m_pVideoEncoder->SetBitrate(BITRATE_BEGIN);
		m_pVideoEncoder->SetMaxBitrate(BITRATE_BEGIN);

	}

	while (bEncodingThreadRunning)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() RUNNING EncodingThreadProcedure method");

		if (m_EncodingBuffer->GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			int timeDiff;
			frameSize = m_EncodingBuffer->DeQueue(m_EncodingFrame, timeDiff, m_iTimeStampDiff);

//			CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure Deque packetSize " + Tools::IntegertoStringConvert(frameSize));

			CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG ," &*&*&* m_EncodingBuffer ->" + toolsObject.IntegertoStringConvert(timeDiff));
			//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "Before Processable");

			if (!g_FPSController.IsProcessableFrame())
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() NOTHING for encoding method");

				toolsObject.SOSleep(10);
				continue;
			}

			m_FrameCounterbeforeEncoding++;

			m_BitRateController->UpdateBitrate();

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);
			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertNV12ToI420 ", currentTimeStamp);

			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_EncodingFrame, frameSize, m_EncodedFrame);

			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Encode ", currentTimeStamp);

#elif defined(_DESKTOP_C_SHARP_)

			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			int iCurWidth = this->m_pColorConverter->GetWidth();
			int iCurHeight = this->m_pColorConverter->GetHeight();

			long long icc = m_Tools.CurrentTimestamp();
			if(frameSize == iCurWidth * iCurHeight * 2) //That Means.... Desktop is Sending YUY2 Data
			{
				frameSize = this->m_pColorConverter->ConvertYUY2ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			else if (frameSize == iCurWidth * iCurHeight * 3) //That Means.... Desktop is Sending RGB24 Data
			{
				frameSize = this->m_pColorConverter->ConvertRGB24ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			}

			//printf("WinD--> Convertion Time --> %d\n", m_Tools.CurrentTimestamp() - icc);

			//m_Tools.WriteToFile(m_ConvertedEncodingFrame, frameSize);

            //printf("WinD--> CVideoCallSession::EncodingThreadProcedure frameSIze: %d\n", frameSize);


			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
            //printf("WinD--> CVideoCallSession::EncodingThreadProcedure encodedFrameSize: %d\n", encodedFrameSize);
            CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Encode ", currentTimeStamp);

#elif defined(TARGET_OS_WINDOWS_PHONE)

			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			if (orientation_type == ORIENTATION_90_MIRRORED)
			{
				this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			else if (orientation_type == ORIENTATION_0_MIRRORED)
			{
//				CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type) + " ORIENTATION_0_MIRRORED ");
				this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420ForBackCam(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertNV12ToI420 ", currentTimeStamp);



			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);


			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Encode ", currentTimeStamp);

			//printf("enctime = %lld\n", m_Tools.CurrentTimestamp() - enctime);

//			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() video data encoded");
#else

//			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type));

			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			
			if (orientation_type == ORIENTATION_90_MIRRORED)
			{
//				CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type) + "  ORIENTATION_90_MIRRORED");
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			else if (orientation_type == ORIENTATION_0_MIRRORED)
			{
//				CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type) + " ORIENTATION_0_MIRRORED ");
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420ForBackCam(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " ConvertNV12ToI420 ", currentTimeStamp);

//			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() Converted to 420");
			encodingTimeStamp = toolsObject.CurrentTimestamp();
			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			long long enctime = m_Tools.CurrentTimestamp();
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
			long long timediff = (m_Tools.CurrentTimestamp() - enctime);
			CLogPrinter_WriteSpecific5(CLogPrinter::INFO, " OOOO encode  " + m_Tools.LongLongToString(timediff));
			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Encode ", currentTimeStamp);
			encodingTime = toolsObject.CurrentTimestamp() - encodingTimeStamp;

			encodeTimeStampFor15 += encodingTime;


			long long sleepTimeStamp3 = toolsObject.CurrentTimestamp(); //packetization time

			countFrameSize = countFrameSize + encodedFrameSize;
			if (countFrame >= 15)
			{
				encodingTimeFahadTest = toolsObject.CurrentTimestamp() - encodingTimeFahadTest;
				CLogPrinter_WriteSpecific3(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() Encoded " + Tools::IntegertoStringConvert(countFrame) + " frames Size: " + Tools::IntegertoStringConvert(countFrameSize * 8) + " encodeTimeStampFor15 : " + Tools::IntegertoStringConvert(encodeTimeStampFor15) + " Full_Lop: " + Tools::IntegertoStringConvert(encodingTimeFahadTest));
				encodingTimeFahadTest = toolsObject.CurrentTimestamp();
				countFrame = 0;
				countFrameSize = 0;
				encodeTimeStampFor15 = 0;
			}
			countFrame++;

			dbTotalEncodingTime += encodingTime;
			++iEncodedFrameCounter;
			nMaxEncodingTime = max(nMaxEncodingTime, encodingTime);

//			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() before notify function");


#endif
			m_BitRateController->NotifyEncodedFrame(encodedFrameSize);

			//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure() m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));

			//m_pVideoEncoder->GetEncodedFramePacketizer()->Packetize(m_FriendID,m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
			currentTimeStamp = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);
			//- (void)WriteToFile:(const char *)path withData:(unsigned char *)data dataLength:(int)datalen

			/*
			if(m_iFrameNumber<200)
			{
			string str ="/Encode/"+m_Tools.IntegertoStringConvert(m_iFrameNumber) + "_" + m_Tools.IntegertoStringConvert(encodedFrameSize);
			str+=".dump";
			[[Helper_IOS GetInstance] WriteToFile:str.c_str() withData:m_EncodedFrame dataLength:encodedFrameSize];
			}
			*/

//			CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure() packetized lFriendID " + Tools::IntegertoStringConvert(m_FriendID) + " packetSize " + Tools::IntegertoStringConvert(encodedFrameSize));
			m_pEncodedFramePacketizer->Packetize(m_FriendID, m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Packetize ", currentTimeStamp);
			++m_iFrameNumber;
			//CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure2 m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(CVideoEncodingThread::m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));

			toolsObject.SOSleep(1);

		}
	}

	bEncodingThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() stopped EncodingThreadProcedure method.");
}