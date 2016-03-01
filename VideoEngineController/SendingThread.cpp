
#include "SendingThread.h"
#include "Size.h"
#include "PacketHeader.h"
//#include "Globals.cpp"
#include "CommonElementsBucket.h"

CSendingThread::CSendingThread(CCommonElementsBucket* commonElementsBucket, CSendingBuffer *sendingBuffer, CFPSController *FPSController) :

m_pCommonElementsBucket(commonElementsBucket),
m_SendingBuffer(sendingBuffer),
g_FPSController(FPSController)

{

}

CSendingThread::~CSendingThread()
{

}

void CSendingThread::StopSendingThread()
{
	//if (pInternalThread.get())
	{
		bSendingThreadRunning = false;

		while (!bSendingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();
}

void CSendingThread::StartSendingThread()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 1");

	if (pSendingThread.get())
	{
		CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 2");
		pSendingThread.reset();
		CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartDecodingThread 3");
		return;
	}
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 4");
	bSendingThreadRunning = true;
	bSendingThreadClosed = false;
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread 5");

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t SendingThreadQ = dispatch_queue_create("SendingThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(SendingThreadQ, ^{
		this->SendingThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoSendingThread, this);
	myThread.detach();

#endif

	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::StartedInternalThread Encoding Thread started");

	return;
}

void *CSendingThread::CreateVideoSendingThread(void* param)
{
	CSendingThread *pThis = (CSendingThread*)param;
	pThis->SendingThreadProcedure();

	return NULL;
}

#ifdef PACKET_SEND_STATISTICS_ENABLED
int iPacketCounter = 0;
int iPrevFrameNumer = 0;
int iNumberOfPacketsInLastFrame = 0;
int iNumberOfPacketsActuallySentFromLastFrame = 0;
#endif

#ifdef  BANDWIDTH_CONTROLLING_TEST
std::vector<int>g_TimePeriodInterval;
std::vector<int>g_BandWidthList;
#endif

void CSendingThread::SendingThreadProcedure()
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::EncodingThreadProcedure() Started EncodingThreadProcedure.");

	Tools toolsObject;
	int packetSize;
	LongLong lFriendID;
	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int fpsSignal, frameNumber, packetNumber;
	CPacketHeader packetHeader;

#ifdef  BANDWIDTH_CONTROLLING_TEST
	g_BandWidthList.push_back(500 * 1024);    g_TimePeriodInterval.push_back(20 * 1000);
	g_BandWidthList.push_back(8 * 1024);    g_TimePeriodInterval.push_back(20 * 1000);
	g_BandWidthList.push_back(3 * 1024);    g_TimePeriodInterval.push_back(100 * 1000);
	/*g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);
	g_BandWidthList.push_back(500*1024);    g_TimePeriodInterval.push_back(20*1000);
	g_BandWidthList.push_back(5*1024);    g_TimePeriodInterval.push_back(2*1000);*/
	g_BandWidthController.SetTimeInterval(g_BandWidthList, g_TimePeriodInterval);
#endif

	while (bSendingThreadRunning)
	{
		//CLogPrinter_Write(CLogPrinter::INFO, "CVideoCallSession::InternalThreadImpl");

		if (m_SendingBuffer->GetQueueSize() == 0)
			toolsObject.SOSleep(10);
		else
		{
			int timeDiffForQueue;
			packetSize = m_SendingBuffer->DeQueue(lFriendID, m_EncodedFrame, frameNumber, packetNumber, timeDiffForQueue);
			CLogPrinter_WriteForQueueTime(CLogPrinter::INFO, " m_SendingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));

			int startPoint = RESEND_INFO_START_BYTE_WITH_MEDIA_TYPE;
			pair<int, int> FramePacketToSend = { -1, -1 };


			packetHeader.setPacketHeader(m_EncodedFrame + 1);

			/*if(ExpectedFramePacketDeQueue.size() > 0)
			{
			FramePacketToSend = ExpectedFramePacketDeQueue.front();
			ExpectedFramePacketDeQueue.pop_front();
			}*/
#ifdef	RETRANSMISSION_ENABLED
			/*for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendFrameNumber
			{
			m_EncodedFrame[startPoint ++] = (FramePacketToSend.first >> f) & 0xFF;
			}
			for (int f = startFraction; f >= 0; f -= fractionInterval)//ResendPacketNumber
			{
			m_EncodedFrame[startPoint ++] = (FramePacketToSend.second >> f) & 0xFF;
			}*/
#endif
			//			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " Before Bye SIGBYTE: ");

			unsigned char signal = g_FPSController->GetFPSSignalByte();
			m_EncodedFrame[1 + SIGNAL_BYTE_INDEX_WITHOUT_MEDIA] = signal;

			//			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, " Bye SIGBYTE: "+ m_Tools.IntegertoStringConvert(signal));



#ifdef PACKET_SEND_STATISTICS_ENABLED

			int iNumberOfPackets = -1;

			iNumberOfPackets = packetHeader.getNumberOfPacket();

			pair<int, int> FramePacketPair = /*toolsObject.GetFramePacketFromHeader(m_EncodedFrame + 1, iNumberOfPackets);*/make_pair(packetHeader.getFrameNumber(), packetHeader.getPacketNumber());

			if (FramePacketPair.first != iPrevFrameNumer)
			{
				//CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,"iNumberOfPacketsActuallySentFromLastFrame = %d, iNumberOfPacketsInLastFrame = %d, currentframenumber = %d\n",
				//	iNumberOfPacketsActuallySentFromLastFrame, iNumberOfPacketsInLastFrame, FramePacketPair.first);

				if (iNumberOfPacketsActuallySentFromLastFrame != iNumberOfPacketsInLastFrame)
				{
					CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "$$-->******* iNumberOfPacketsActuallySentFromLastFrame = "
						+ m_Tools.IntegertoStringConvert(iNumberOfPacketsActuallySentFromLastFrame)
						+ " iNumberOfPacketsInLastFrame = "
						+ m_Tools.IntegertoStringConvert(iNumberOfPacketsInLastFrame)
						+ " currentframenumber = "
						+ m_Tools.IntegertoStringConvert(FramePacketPair.first)
						+ " m_SendingBuffersize = "
						+ m_Tools.IntegertoStringConvert(m_SendingBuffer->GetQueueSize()));

				}


				iNumberOfPacketsInLastFrame = iNumberOfPackets;
				iNumberOfPacketsActuallySentFromLastFrame = 1;
				iPrevFrameNumer = FramePacketPair.first;
			}
			else
			{
				iNumberOfPacketsActuallySentFromLastFrame++;
			}
#endif


			//			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "Parsing..>>>  FN: "+ m_Tools.IntegertoStringConvert(packetHeader.getFrameNumber())
			//														  + "  pNo : "+ m_Tools.IntegertoStringConvert(packetHeader.getPacketNumber())
			//														  + "  Npkt : "+ m_Tools.IntegertoStringConvert(packetHeader.getNumberOfPacket())
			//														  + "  FPS : "+ m_Tools.IntegertoStringConvert(packetHeader.getFPS())
			//														  + "  Rt : "+ m_Tools.IntegertoStringConvert(packetHeader.getRetransSignal())
			//														  + "  Len : "+ m_Tools.IntegertoStringConvert(packetHeader.getPacketLength())
			//														  + " tmDiff : " + m_Tools.IntegertoStringConvert(packetHeader.getTimeStamp()));


#ifdef  BANDWIDTH_CONTROLLING_TEST
			if (g_BandWidthController.IsSendeablePacket(packetSize)) {
#endif

				//printf("WIND--> SendFunctionPointer with size  = %d\n", packetSize);
				m_pCommonElementsBucket->SendFunctionPointer(lFriendID, 2, m_EncodedFrame, packetSize);

				CLogPrinter_WriteForPacketLossInfo(CLogPrinter::DEBUGS, " &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));

				//toolsObject.SOSleep((int)(SENDING_INTERVAL_FOR_15_FPS * MAX_FPS * 1.0) / (g_FPSController->GetOwnFPS()  * 1.0));

				toolsObject.SOSleep(GetSleepTime());

#ifdef  BANDWIDTH_CONTROLLING_TEST
			}
#endif

		}
	}

	bSendingThreadClosed = true;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

int CSendingThread::GetSleepTime()
{
	int SleepTimeDependingOnFPS = (SENDING_INTERVAL_FOR_15_FPS * FPS_MAXIMUM * 1.0) / (g_FPSController->GetOwnFPS()  * 1.0);
	int SleepTimeDependingOnQueueSize = 1000 * 1.0 / (m_SendingBuffer->GetQueueSize() + 1.0);

	if (SleepTimeDependingOnFPS < SleepTimeDependingOnQueueSize)
	{
		return SleepTimeDependingOnFPS;
	}
	else
	{
		return SleepTimeDependingOnQueueSize;
	}
}