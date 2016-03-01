
#include "VideoEncodingThread.h"
#include "Globals.cpp"

#define ORIENTATION_0_MIRRORED 1
#define ORIENTATION_90_MIRRORED 2
#define ORIENTATION_180_MIRRORED 3
#define ORIENTATION_270_MIRRORED 4
#define ORIENTATION_0_NOT_MIRRORED 5
#define ORIENTATION_90_NOT_MIRRORED 6
#define ORIENTATION_180_NOT_MIRRORED 7
#define ORIENTATION_270_NOT_MIRRORED 8

int countFrame = 0;
int countFrameFor15 = 0;
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
	countFrameFor15 = 0;
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
	if (pEncodingThread.get())
	{
		CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::StartedInternalThread 2");
		pEncodingThread.reset();
		CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::StartDepacketizationThread 3");
		return;
	}
	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::StartedInternalThread 4");
	bEncodingThreadRunning = true;
	bEncodingThreadClosed = false;
	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::StartedInternalThread 5");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(EncodeThreadQ, ^{
		this->EncodingThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoEncodingThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::StartedInternalThread Encoding Thread started");

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
	CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() Started EncodingThreadProcedure.");
	Tools toolsObject;
	int frameSize, encodedFrameSize;
	long long encodingTime, encodingTimeStamp, nMaxEncodingTime = 0, currentTimeStamp;
	double dbTotalEncodingTime = 0;
	int iEncodedFrameCounter = 0;
	encodingTimeStamp = toolsObject.CurrentTimestamp();
	long long encodingTimeFahadTest = 0;

	long long iterationtime = toolsObject.CurrentTimestamp();

	bool m_bFirstFrame = true;				// added
	long long m_ll1stFrameTimeStamp = 0;	// added
	unsigned  int m_iTimeStampDiff = 0;		// added
	int m_FrameCounterbeforeEncoding = 0;	// added

	while (bEncodingThreadRunning)
	{
		CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure");

		if (m_EncodingBuffer->GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			long long sleepTimeStamp1 = toolsObject.CurrentTimestamp();//total time

			int timeDiff;
			frameSize = m_EncodingBuffer->DeQueue(m_EncodingFrame, timeDiff);

			CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure Deque packetSize " + Tools::IntegertoStringConvert(frameSize));

			CLogPrinter_WriteForQueueTime(CLogPrinter::INFO, " &*&*&* m_EncodingBuffer ->" + toolsObject.IntegertoStringConvert(timeDiff));
			//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "Before Processable");

			if (!g_FPSController.IsProcessableFrame())
			{
				toolsObject.SOSleep(10);
				continue;
			}

			if (m_bFirstFrame)
			{
				m_ll1stFrameTimeStamp = toolsObject.CurrentTimestamp();
				m_bFirstFrame = false;
			}

			m_iTimeStampDiff = toolsObject.CurrentTimestamp() - m_ll1stFrameTimeStamp;


			m_FrameCounterbeforeEncoding++;
			m_BitRateController->UpdateBitrate();

			//			if(m_FrameCounterbeforeEncoding%FRAME_RATE == 0 && g_OppNotifiedByterate>0 && m_bsetBitrateCalled == false)
			//			{
			//
			//                int iRet = -1, iRet2 = -1;
			//                int iCurrentBitRate = g_OppNotifiedByterate* 8 - nFirstTimeDecrease;
			//				nFirstTimeDecrease = 0;
			//
			//				CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, " $$$*( SET BITRATE :"+ m_Tools.IntegertoStringConvert(iCurrentBitRate)+"  Pre: "+ m_Tools.IntegertoStringConvert(m_iPreviousByterate));
			//                //printf("VampireEngg--> iCurrentBitRate = %d, g_OppNotifiedByteRate = %d\n", iCurrentBitRate, g_OppNotifiedByterate);
			//
			//                if(iCurrentBitRate < m_pVideoEncoder->GetBitrate())
			//                {
			//                    iRet = m_pVideoEncoder->SetBitrate(iCurrentBitRate);
			//
			//                    if(iRet == 0) //First Initialization Successful
			//                        iRet2 = m_pVideoEncoder->SetMaxBitrate(iCurrentBitRate);
			//
			//                }
			//                else
			//                {
			//                    iRet = m_pVideoEncoder->SetMaxBitrate(iCurrentBitRate);
			//
			//                    if(iRet == 0) //First Initialization Successful
			//                        iRet2 = m_pVideoEncoder->SetBitrate(iCurrentBitRate);
			//                }
			//
			//                if(iRet == 0 && iRet2 ==0) //We are intentionally skipping status of setbitrate operation success
			//                {
			//                    m_iPreviousByterate = iCurrentBitRate/8;
			//
			//                    m_bMegSlotCounterShouldStop = false;
			//                }
			//
			//                m_bsetBitrateCalled = true;
			//
			//            }


			//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "$ENCODEING$");
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			this->m_pColorConverter->ConvertNV12ToI420(m_EncodingFrame);
			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertNV12ToI420 ", currentTimeStamp);

			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_EncodingFrame, frameSize, m_EncodedFrame);

			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);

			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure video data encoded");
#elif defined(_DESKTOP_C_SHARP_)


			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			frameSize = this->m_pColorConverter->ConvertYUY2ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			//printf("WinD--> CVideoEncodingThread::EncodingThreadProcedure frameSIze: %d\n", frameSize);
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
			//printf("WinD--> CVideoEncodingThread::EncodingThreadProcedure encodedFrameSize: %d\n", encodedFrameSize);
			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);

#elif defined(TARGET_OS_WINDOWS_PHONE)

			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			if (orientation_type == ORIENTATION_90_MIRRORED)
			{
				this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			else if (orientation_type == ORIENTATION_0_MIRRORED)
			{
				CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type) + " ORIENTATION_0_MIRRORED ");
				this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420ForBackCam(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertNV12ToI420 ", currentTimeStamp);

			long long enctime = m_Tools.CurrentTimestamp();

			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);

			//printf("enctime = %lld\n", m_Tools.CurrentTimestamp() - enctime);

			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure video data encoded");
#else

			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type));

			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			if (orientation_type == ORIENTATION_90_MIRRORED)
			{
				CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type) + "  ORIENTATION_90_MIRRORED");
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			else if (orientation_type == ORIENTATION_0_MIRRORED)
			{
				CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "orientation_type : " + m_Tools.IntegertoStringConvert(orientation_type) + " ORIENTATION_0_MIRRORED ");
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420ForBackCam(m_EncodingFrame, m_ConvertedEncodingFrame);
			}
			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " ConvertNV12ToI420 ", currentTimeStamp);

			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure Converted to 420");
			encodingTimeStamp = toolsObject.CurrentTimestamp();
			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			encodedFrameSize = m_pVideoEncoder->EncodeAndTransfer(m_ConvertedEncodingFrame, frameSize, m_EncodedFrame);
			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Encode ", currentTimeStamp);
			encodingTime = toolsObject.CurrentTimestamp() - encodingTimeStamp;

			encodeTimeStampFor15 += encodingTime;


			long long sleepTimeStamp3 = toolsObject.CurrentTimestamp(); //packetization time

			countFrameSize = countFrameSize + encodedFrameSize;
			if (countFrame >= 15)
			{
				encodingTimeFahadTest = toolsObject.CurrentTimestamp() - encodingTimeFahadTest;
				CLogPrinter_WriteSpecific3(CLogPrinter::DEBUGS, "Encoded " + Tools::IntegertoStringConvert(countFrame) + " frames Size: " + Tools::IntegertoStringConvert(countFrameSize * 8) + " encodeTimeStampFor15 : " + Tools::IntegertoStringConvert(encodeTimeStampFor15) + " Full_Lop: " + Tools::IntegertoStringConvert(encodingTimeFahadTest));
				encodingTimeFahadTest = toolsObject.CurrentTimestamp();
				countFrame = 0;
				countFrameSize = 0;
				encodeTimeStampFor15 = 0;
			}
			countFrame++;

			dbTotalEncodingTime += encodingTime;
			++iEncodedFrameCounter;
			nMaxEncodingTime = max(nMaxEncodingTime, encodingTime);

			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure before notify function");


#endif
			m_BitRateController->NotifyEncodedFrame(encodedFrameSize);

			//            m_ByteSendInSlotInverval+=encodedFrameSize;
			//            if(m_FrameCounterbeforeEncoding % FRAME_RATE == 0)
			//            {
			//
			//
			//                int ratioHelperIndex = (m_FrameCounterbeforeEncoding - FRAME_RATE) / FRAME_RATE;
			//                if(m_bMegSlotCounterShouldStop == false)
			//                {
			//                    //printf("VampireEngg--> ***************m_ByteSendInSlotInverval = (%d, %d)\n", ratioHelperIndex, m_ByteSendInSlotInverval);
			//					m_LastSendingSlot = ratioHelperIndex;
			//                    m_BandWidthRatioHelper.insert(ratioHelperIndex, m_ByteSendInSlotInverval);
			//                }
			//
			//                m_ByteSendInSlotInverval = 0;
			//            }


			//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));
			//			CLogPrinter_WriteSpecific(CLogPrinter::INFO, "$ENCODEING$ To Parser");
			//m_pVideoEncoder->GetEncodedFramePacketizer()->Packetize(m_FriendID,m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
			currentTimeStamp = CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, "");
			//- (void)WriteToFile:(const char *)path withData:(unsigned char *)data dataLength:(int)datalen

			/*
			if(m_iFrameNumber<200)
			{
			string str ="/Encode/"+m_Tools.IntegertoStringConvert(m_iFrameNumber) + "_" + m_Tools.IntegertoStringConvert(encodedFrameSize);
			str+=".dump";
			[[Helper_IOS GetInstance] WriteToFile:str.c_str() withData:m_EncodedFrame dataLength:encodedFrameSize];
			}
			*/

			CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure packetized lFriendID " + Tools::IntegertoStringConvert(m_FriendID) + " packetSize " + Tools::IntegertoStringConvert(encodedFrameSize));
			m_pEncodedFramePacketizer->Packetize(m_FriendID, m_EncodedFrame, encodedFrameSize, m_iFrameNumber, m_iTimeStampDiff);
			CLogPrinter_WriteForOperationTime(CLogPrinter::DEBUGS, " Packetize ", currentTimeStamp);
			++m_iFrameNumber;
			//CLogPrinter_WriteSpecific(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure2 m_iFrameNumber : "+ m_Tools.IntegertoStringConvert(CVideoEncodingThread::m_iFrameNumber) + " :: encodedFrameSize: " + m_Tools.IntegertoStringConvert(encodedFrameSize));
			CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure before sleep");
			toolsObject.SOSleep(1);

		}
	}

	bEncodingThreadClosed = true;

	CLogPrinter_WriteInstentTestLog(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}