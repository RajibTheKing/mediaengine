
#include "DepacketizationThread.h"
#include "Size.h"
#include "CommonElementsBucket.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

//extern unsigned char g_uchSendPacketVersion;						// bring check

CVideoDepacketizationThread::CVideoDepacketizationThread(LongLong friendID, CVideoPacketQueue *VideoPacketQueue, CVideoPacketQueue *RetransVideoPacketQueue, CVideoPacketQueue *MiniPacketQueue, BitRateController *BitRateController, CEncodedFrameDepacketizer *EncodedFrameDepacketizer, CCommonElementsBucket* CommonElementsBucket, unsigned int *miniPacketBandCounter, CVersionController *pVersionController, CVideoCallSession* pVideoCallSession) :

m_FriendID(friendID),
m_pVideoPacketQueue(VideoPacketQueue),
m_pRetransVideoPacketQueue(RetransVideoPacketQueue),
m_pMiniPacketQueue(MiniPacketQueue),
m_BitRateController(BitRateController),
m_pEncodedFrameDepacketizer(EncodedFrameDepacketizer),
m_pCommonElementsBucket(CommonElementsBucket),
m_miniPacketBandCounter(miniPacketBandCounter),
m_pVideoCallSession(pVideoCallSession)
{
	ExpectedFramePacketPair.first = 0;
	ExpectedFramePacketPair.second = 0;

	iNumberOfPacketsInCurrentFrame = 0;
    m_pVersionController = pVersionController;
    
    
}

CVideoDepacketizationThread::~CVideoDepacketizationThread()
{

}

void CVideoDepacketizationThread::StopDepacketizationThread()
{

	//if (pDepacketizationThread.get())
	{
		bDepacketizationThreadRunning = false;
		Tools toolsObject;

		while (!bDepacketizationThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}

	//pDepacketizationThread.reset();
}

void CVideoDepacketizationThread::StartDepacketizationThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDepacketizationThread::StartDepacketizationThread() called");

	if (pDepacketizationThread.get())
	{
		pDepacketizationThread.reset();

		return;
	}

	bDepacketizationThreadRunning = true;
	bDepacketizationThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t DecodeThreadQ = dispatch_queue_create("DecodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(DecodeThreadQ, ^{
		this->DepacketizationThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoDepacketizationThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDepacketizationThread::StartDepacketizationThread() Depacketization Thread started");

	return;
}

void *CVideoDepacketizationThread::CreateVideoDepacketizationThread(void* param)
{
	CVideoDepacketizationThread *pThis = (CVideoDepacketizationThread*)param;
	pThis->DepacketizationThreadProcedure();

	return NULL;
}

void CVideoDepacketizationThread::DepacketizationThreadProcedure()		//Merging Thread
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDepacketizationThread::DepacketizationThreadProcedure() Started DepacketizationThreadProcedure method");

	Tools toolsObject;
	int frameSize, queSize = 0, miniPacketQueueSize = 0;
    long long llDepacitazationThreadStartTime = m_Tools.CurrentTimestamp();
	while (bDepacketizationThreadRunning)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDepacketizationThread::DepacketizationThreadProcedure() RUNNING DepacketizationThreadProcedure method");

		queSize = m_pVideoPacketQueue->GetQueueSize();

		miniPacketQueueSize = m_pMiniPacketQueue->GetQueueSize();
        //printf("TheVersion--> Depcackatization Thread retQueueSize = %d, minQueueSize  =  %d\n", queSize, miniPacketQueueSize);

		//		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "SIZE "+ m_Tools.IntegertoStringConvert(retQueuSize)+"  "+ m_Tools.IntegertoStringConvert(queSize));
        

		if (0 == queSize && 0 == miniPacketQueueSize)
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDepacketizationThread::DepacketizationThreadProcedure() NOTHING for depacketization method");
            
            //printf("Rajib: OwnCallVersion = %d, oppCallVersion = %d\n", m_pVersionController->GetOwnVersion(), m_pVersionController->GetOpponentVersion());
            if(m_pVersionController->GetOpponentVersion() == -1)
            {
                if(m_Tools.CurrentTimestamp()-llDepacitazationThreadStartTime >= TIMEOUT_START_SENDING_VIDEO_DATA)
                {
                    
                    m_pVersionController->SetOpponentVersion(0); //version code
                    
                    m_pVersionController->SetCurrentCallVersion(0); //version code
                    
                    if (VIDEO_CALL_TYPE_UNKNOWN == m_pVideoCallSession->GetOpponentVideoCallQualityLevel())
                    {		//Not necessary
                        m_pVideoCallSession->SetOpponentVideoCallQualityLevel(SUPPORTED_RESOLUTION_FPS_352_15);
                        
                        m_pVideoCallSession->SetCurrentVideoCallQualityLevel(min(m_pVideoCallSession->GetOwnVideoCallQualityLevel(),m_pVideoCallSession->GetOpponentVideoCallQualityLevel()));
                        
                        m_pVideoCallSession->GetBitRateController()->SetOpponentNetworkType(NETWORK_TYPE_NOT_2G);
                    }
                    
                    
                    
                }
            }
            
			toolsObject.SOSleep(10);

			continue;
		}

		if (miniPacketQueueSize != 0) {
			frameSize = m_pMiniPacketQueue->DeQueue(m_PacketToBeMerged);
		}
		else if (queSize > 0) {
			frameSize = m_pVideoPacketQueue->DeQueue(m_PacketToBeMerged);
		}
        

		m_RcvdPacketHeader.setPacketHeader(m_PacketToBeMerged);
        
        
		CLogPrinter_WriteSpecific4(CLogPrinter::DEBUGS,
								   "CVideoDepacketizationThread::StartDepacketizationThread() !@# Versions: " +
								   m_Tools.IntegertoStringConvert(g_uchSendPacketVersion));
		//			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "VC..>>>  FN: "+ m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getFrameNumber()) + "  pk: "+ m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getPacketNumber())
		//														  + " tmDiff : " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getTimeStamp()));

		bool bRetransmitted = (m_PacketToBeMerged[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_RETRANS_PACKET) & 1;
		bool bMiniPacket = (m_PacketToBeMerged[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] >> BIT_INDEX_MINI_PACKET) & 1;
		m_PacketToBeMerged[RETRANSMISSION_SIG_BYTE_INDEX_WITHOUT_MEDIA] = 0;

		if (bMiniPacket) {
			if (m_RcvdPacketHeader.getPacketNumber() == BITRATE_TYPE_MINIPACKET) {                    /* Opponent response of data receive. */
				m_BitRateController->HandleBitrateMiniPacket(m_RcvdPacketHeader);
			}
			else if (m_RcvdPacketHeader.getPacketNumber() == NETWORK_TYPE_MINIPACKET) {        /* Opponent Network type */
				m_BitRateController->HandleNetworkTypeMiniPacket(m_RcvdPacketHeader);
				CLogPrinter_WriteSpecific5(CLogPrinter::DEBUGS,
										   "CVideoDepacketizationThread::StartDepacketizationThread() rcv minipkt PACKET FOR NETWORK_TYPE");
			}

			toolsObject.SOSleep(1);

			continue;
		}
		else if(bRetransmitted)
		{
			continue;
		}
        
        
        if(m_RcvdPacketHeader.getNumberOfPacket() == 0)
        {
            if(m_pVersionController->GetOpponentVersion() == -1)
            {
                
                m_pVersionController->SetOpponentVersion((int)m_PacketToBeMerged[PACKET_HEADER_LENGTH_NO_VERSION]);
                //printf("TheVersion --> setOpponentVersion %d, because m_RcvdPacketHeader.getNumberOfPacket() == 0\n", m_pVersionController->GetOpponentVersion());
                
                m_pVersionController->SetCurrentCallVersion(min ((int)m_pVersionController->GetOwnVersion(), m_pVersionController->GetOpponentVersion()));
                
                
                //printf("TheVersion --> Setting current Call Version to %d\n", m_pVersionController->GetCurrentCallVersion());
				if(1 == m_pVersionController->GetOpponentVersion()){
					m_pVideoCallSession->SetOpponentVideoCallQualityLevel(0);
					m_pVideoCallSession->SetCurrentVideoCallQualityLevel(0);
					continue;
				}

            }

			bool bIs2GOpponentNetwork = (m_PacketToBeMerged[PACKET_HEADER_LENGTH_NO_VERSION + 1] & 0x01);
			int nOpponentVideoCallQualityLevel = (m_PacketToBeMerged[PACKET_HEADER_LENGTH_NO_VERSION + 1] & 0x06) >> 1;

			if (VIDEO_CALL_TYPE_UNKNOWN == m_pVideoCallSession->GetOpponentVideoCallQualityLevel())
			{
				m_pVideoCallSession->SetOpponentVideoCallQualityLevel(nOpponentVideoCallQualityLevel);
				m_pVideoCallSession->SetCurrentVideoCallQualityLevel( min( m_pVideoCallSession->GetOwnVideoCallQualityLevel(), m_pVideoCallSession->GetOpponentVideoCallQualityLevel() ) );
			}

			if (bIs2GOpponentNetwork) {
				m_pVideoCallSession->GetVideoEncoder()->SetNetworkType(NETWORK_TYPE_2G);
				m_pVideoCallSession->GetBitRateController()->SetOpponentNetworkType(NETWORK_TYPE_2G);
			}

            continue;
        }
        
        
        if(m_pVersionController->GetOpponentVersionCompatibleFlag() == true)
        {
            //if (m_pVersionController->GetOpponentVersion() == -1 && m_RcvdPacketHeader.getVersionCode()>0)
            
            unsigned char uchOpponentVersion = max(1, (int)m_RcvdPacketHeader.getVersionCode());
            //printf("TheVersion--> RcvPacketHeader.getVersionCode = %d, packetSize = %d, frameNumber = %d\n", m_RcvdPacketHeader.getVersionCode(), m_RcvdPacketHeader.getPacketLength(), m_RcvdPacketHeader.getFrameNumber());
            
            if(uchOpponentVersion > m_pVersionController->GetOpponentVersion())
            {
                m_pVersionController->SetOpponentVersion(uchOpponentVersion);
                //printf("TheVersion --> setOpponentVersion %d, because GetOpponentVersionCompatibleFlag() == true\n", m_pVersionController->GetOpponentVersion());
                
                
                m_pVersionController->SetCurrentCallVersion(min ((int)m_pVersionController->GetOwnVersion(), m_pVersionController->GetOpponentVersion()));

				if (VIDEO_CALL_TYPE_UNKNOWN == m_pVideoCallSession->GetOpponentVideoCallQualityLevel()) {		//Not necessary
					m_pVideoCallSession->SetOpponentVideoCallQualityLevel(m_RcvdPacketHeader.GetOpponentResolution(m_PacketToBeMerged) );

					m_pVideoCallSession->SetCurrentVideoCallQualityLevel(min(m_pVideoCallSession->GetOwnVideoCallQualityLevel(),m_pVideoCallSession->GetOpponentVideoCallQualityLevel()));

					m_pVideoCallSession->GetBitRateController()->SetOpponentNetworkType( m_RcvdPacketHeader.GetOpponentNetworkType(m_PacketToBeMerged) );
				}
            }      
        }
        else if(m_pVersionController->GetOpponentVersion() == -1)
        {
            m_pVersionController->SetOpponentVersion(0);
            m_pVersionController->SetCurrentCallVersion(min ((int)m_pVersionController->GetOwnVersion(), m_pVersionController->GetOpponentVersion()));

			if (VIDEO_CALL_TYPE_UNKNOWN == m_pVideoCallSession->GetOpponentVideoCallQualityLevel()) {	//Not necessary
				m_pVideoCallSession->SetOpponentVideoCallQualityLevel(0);
				m_pVideoCallSession->SetCurrentVideoCallQualityLevel(min(m_pVideoCallSession->GetOwnVideoCallQualityLevel(),m_pVideoCallSession->GetOpponentVideoCallQualityLevel()));
			}
        }
        
        
        CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,
                             "TheKing--> Finally, CurrentCallVersion = "+ m_Tools.IntegertoStringConvert(m_pVersionController->GetCurrentCallVersion()) +
                             ", CurrentVideoQuality = "+ m_Tools.IntegertoStringConvert(m_pVideoCallSession->GetCurrentVideoCallQualityLevel()) +
                             ", OppVideoQuality = " + m_Tools.IntegertoStringConvert(m_pVideoCallSession->GetOpponentVideoCallQualityLevel()) +
                             ", OwnVideoQuality = " + m_Tools.IntegertoStringConvert(m_pVideoCallSession->GetOwnVideoCallQualityLevel()) );

#if 0
		ExpectedPacket();	//Calculate Expected Video Packet For Debugging.
		int iNumberOfPackets = m_RcvdPacketHeader.getNumberOfPacket();
		CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDepacketizationThread::StartDepacketizationThread() ::Minipacket: FrameNumber: " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getFrameNumber())
													   + " PacketNumber. : " + m_Tools.IntegertoStringConvert(m_RcvdPacketHeader.getPacketNumber()));
#endif

		m_pEncodedFrameDepacketizer->Depacketize(m_PacketToBeMerged, frameSize,
												 NORMAL_PACKET_TYPE, m_RcvdPacketHeader);
		toolsObject.SOSleep(0);
	}

	bDepacketizationThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoDepacketizationThread::DepacketizationThreadProcedure() Stopped DepacketizationThreadProcedure method");
}


void CVideoDepacketizationThread::UpdateExpectedFramePacketPair(pair<int, int> currentFramePacketPair, int iNumberOfPackets)
{
	int iFrameNumber = currentFramePacketPair.first;
	int iPackeNumber = currentFramePacketPair.second;
	if (iPackeNumber == iNumberOfPackets - 1)//Last Packet In a Frame
	{
		iNumberOfPacketsInCurrentFrame = 1;//next frame has at least 1 packet, it will be updated when a packet is received
		ExpectedFramePacketPair.first = iFrameNumber + 1;
		ExpectedFramePacketPair.second = 0;
	}
	else
	{
		iNumberOfPacketsInCurrentFrame = iNumberOfPackets;
		ExpectedFramePacketPair.first = iFrameNumber;
		ExpectedFramePacketPair.second = iPackeNumber + 1;
	}

	//CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDepacketizationThread::StartDepacketizationThread() ExFrameNumber: "+ m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first) + " ExPacketNo. : "+  m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)+ " ExNumberOfPacket : "+  m_Tools.IntegertoStringConvert(iNumberOfPacketsInCurrentFrame));
}


void CVideoDepacketizationThread::ExpectedPacket()
{
	int iPacketType = NORMAL_PACKET;

	int iNumberOfPackets = m_RcvdPacketHeader.getNumberOfPacket();
	pair<int, int> currentFramePacketPair = make_pair(m_RcvdPacketHeader.getFrameNumber(), m_RcvdPacketHeader.getPacketNumber());

	if (currentFramePacketPair != ExpectedFramePacketPair /*&& !m_pVideoPacketQueue->PacketExists(ExpectedFramePacketPair.first, ExpectedFramePacketPair.second)*/) //Out of order frame found, need to retransmit
	{
		/*
        string sMsg = "CVideoDepacketizationThread::Current(FN,PN) = ("
        + m_Tools.IntegertoStringConvert(currentFramePacketPair.first)
        + ","
        + m_Tools.IntegertoStringConvert(currentFramePacketPair.second)
        + ") and Expected(FN,PN) = ("
        + m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.first)
        + ","
        + m_Tools.IntegertoStringConvert(ExpectedFramePacketPair.second)
        + ")" ;
        //printf("%s\n", sMsg.c_str());
        */

		if (currentFramePacketPair.first != ExpectedFramePacketPair.first) //different frame received
		{
			if (currentFramePacketPair.first - ExpectedFramePacketPair.first == 2) //one complete frame missed, maybe it was a mini frame containing only 1 packet
			{
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDepacketizationThread::StartDepacketizationThread() ExpectedFramePacketPair case 1");
			}
			else if (currentFramePacketPair.first - ExpectedFramePacketPair.first == 1) //last packets from last frame and some packets from current misssed
			{
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDepacketizationThread::StartDepacketizationThread() ExpectedFramePacketPair case 2");
			}
			else//we dont handle burst frame miss, but 1st packets of the current frame should come, only if it is an iFrame
			{
				CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDepacketizationThread::StartDepacketizationThread() ExpectedFramePacketPair case 3-- killed previous frames");
			}

		}
		else //packet missed from same frame
		{
			CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDepacketizationThread::StartDepacketizationThread() ExpectedFramePacketPair case 4");
		}
	}

	UpdateExpectedFramePacketPair(currentFramePacketPair, iNumberOfPackets);
}