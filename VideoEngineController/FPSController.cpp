//
// Created by ipvision on 1/2/2016.
//

#include "FPSController.h"
#include "Size.h"
#include <math.h>
#include <map>

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
extern map<int,int> g_TraceRetransmittedFrame;
#endif

CFPSController::CFPSController(){
    m_pMutex.reset(new CLockHandler);
    m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
    m_ClientFPS = m_nOwnFPS = m_nOpponentFPS = FPS_BEGINNING;
    m_iFrameDropIntervalCounter=0;
    m_EncodingFrameCounter = 0;
    m_DropSum = 0;
    m_nForceFPSFlag = 0;
    m_nMaxOwnProcessableFPS = m_nMaxOpponentProcessableFPS = FPS_MAXIMUM;
    m_nFPSForceSignalCounter = 0;
}

CFPSController::~CFPSController(){
    while(!m_SignalQue.empty())
        m_SignalQue.pop();
    SHARED_PTR_DELETE(m_pMutex);
}

void CFPSController::Reset(){
    m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
    m_ClientFPS = m_nOwnFPS = m_nOpponentFPS = FPS_BEGINNING;
    m_iFrameDropIntervalCounter=0;
    m_EncodingFrameCounter = 0;
    m_DropSum = 0;
}

int CFPSController::GetOpponentFPS() const {
    return m_nOpponentFPS;
}

void CFPSController::SetOpponentFPS(int OpponentFPS) {
    Locker lock(*m_pMutex);
    m_nOpponentFPS = OpponentFPS;
}

int CFPSController::GetOwnFPS() const {
    return m_nOwnFPS;
}

void CFPSController::SetOwnFPS(int nOwnFPS){
    Locker lock(*m_pMutex);
    m_nOwnFPS = nOwnFPS;
}

void CFPSController::SetMaxOwnProcessableFPS(int fps){
    Locker lock(*m_pMutex);
    m_nFPSForceSignalCounter = 2;
    m_nMaxOwnProcessableFPS = fps;
}

int CFPSController::GetMaxOwnProcessableFPS(){
    return m_nMaxOwnProcessableFPS;
}


void CFPSController::SetClientFPS(double fps){
    Locker lock(*m_pMutex);
    m_ClientFPS = fps;
}

unsigned char CFPSController::GetFPSSignalByte()
{
    unsigned char ret = m_nOwnFPS;
    unsigned char changeSignal = 0;

    if(m_nFPSForceSignalCounter > 0)
    {
//        Locker lock(*m_pMutex);
        --m_nFPSForceSignalCounter;
        ret =  m_nMaxOwnProcessableFPS;
        ret |= (1<<5);
        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "# Force: ------------------------Sent------------------------------------->   ");
    }
    else if(m_ClientFPS < FPS_MAXIMUM)
    {
        //Locker lock(*m_pMutex);
        int tmp = m_ClientFPS+0.5;
        if(m_nOwnFPS > tmp)
            ret = tmp;
    }

//    if(!m_SignalQue.empty()) {
//        changeSignal = m_SignalQue.front();
//        m_SignalQue.pop();
//    }
//    ret |= (changeSignal << 6);

#ifdef FIRST_BUILD_COMPATIBLE
    if(changeSignal == 0)
        ret |= 0xC0;
#endif

    return ret;
}


void CFPSController::SetFPSSignalByte(unsigned char signalByte)
{
    signalByte &= 0xEF;
    if(0==signalByte)   return;
    int bIsForceFPS =  (signalByte & (1 << 5) ) >> 5;
    int FPSChangeSignal = (signalByte >> 6);
    int opponentFPS = 15 & signalByte;

    if(bIsForceFPS)
    {
        Locker lock(*m_pMutex);

        if(FPS_MAXIMUM >= opponentFPS)
            m_nMaxOpponentProcessableFPS = opponentFPS;
        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "# Force: -----------------GOT-------------------------------------->   "+ m_Tools.IntegertoStringConvert(m_nMaxOpponentProcessableFPS));
        if(m_nOwnFPS > m_nMaxOpponentProcessableFPS) {
            m_nOwnFPS = m_nMaxOpponentProcessableFPS;
//            m_pVideoEncoder->SetBitrate(m_nOwnFPS);
//            m_pVideoEncoder->SetMaxBitrate(m_nOwnFPS);
        }
    }
    else if (opponentFPS != m_nOpponentFPS) {
            Locker lock(*m_pMutex);
            m_nOpponentFPS = opponentFPS;
        }

    if(FPSChangeSignal && FPSChangeSignal < 3)
        CLogPrinter_WriteSpecific2(CLogPrinter::DEBUGS, "$$$*(# SIGNAL: -------------------------------------------------------SET------>   "+ m_Tools.IntegertoStringConvert(FPSChangeSignal));

    if (FPSChangeSignal == 1) {
        if (m_nOwnFPS > FPS_MINIMUM) {
//            Locker lock(*m_pMutex);
//            m_nOwnFPS--;
            int nCurrentBitRate = m_pVideoEncoder->GetBitrate();
            int nNewBitRate = nCurrentBitRate *=0.85;

            m_pVideoEncoder->SetBitrate( nNewBitRate );
            m_pVideoEncoder->SetMaxBitrate(nNewBitRate);
        }
    }
    else if (FPSChangeSignal == 2) {
        if (m_nOwnFPS < FPS_MAXIMUM  && m_nOwnFPS < m_nMaxOpponentProcessableFPS && m_nOwnFPS < m_ClientFPS + 0.1) {
//            Locker lock(*m_pMutex);
//            m_nOwnFPS++;
            int nCurrentBitRate = m_pVideoEncoder->GetBitrate();
            int nNewBitRate = nCurrentBitRate *=1.15;
            m_pVideoEncoder->SetMaxBitrate(nNewBitRate);
            m_pVideoEncoder->SetBitrate(nNewBitRate);
        }
    }

    if (m_nOwnFPS > m_ClientFPS)
        m_nOwnFPS = m_ClientFPS;

}

#if 0
int CFPSController::NotifyFrameComplete(int framNumber)
{
//    m_iFrameCompletedIntervalCounter++;
    LongLong diffTimeStamp = m_Tools.CurrentTimestamp() - m_LastIntervalStartingTime;

    bool bIsIntervalOver = diffTimeStamp >= 3*1000;

    if( bIsIntervalOver && 0==m_iFrameDropIntervalCounter)
    {
        {
            Locker lock(*m_pMutex);
            m_nFPSChangeSignal = 2;
//            m_SignalQue.push(2);
        }
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
//        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: @@@@@@@@@@@@@@@@@@@@@@@@@   FPS INCREASE  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    }
    else if(bIsIntervalOver)
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
    
    return 1;
}

#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
int counterDroppedValuableframe = 0;
int counterUsedValuableframe = 0;
#endif
#ifdef FRAME_USAGE_STATISTICS_ENABLED
int counterDroppedframe = 0;
#endif
int CFPSController::NotifyFrameDropped(int framNumber)
{
    
#ifdef RETRANSMITTED_FRAME_USAGE_STATISTICS_ENABLED
    if(g_TraceRetransmittedFrame[framNumber] == 1)
    {
        CLogPrinter_WriteSpecific2(CLogPrinter::INFO,"Very Valuable frame dropped "+m_Tools.IntegertoStringConvert(framNumber)  +", counterDroppedframe =  "+m_Tools.IntegertoStringConvert(counterDroppedValuableframe) );
        counterDroppedValuableframe++;
        
    }
#endif
    
#ifdef FRAME_USAGE_STATISTICS_ENABLED
    
    CLogPrinter_WriteSpecific2(CLogPrinter::INFO,"Frame Dropped "+m_Tools.IntegertoStringConvert(framNumber)  +", counter =  "+m_Tools.IntegertoStringConvert(counterDroppedframe) );
    counterDroppedframe++;
#endif
    m_iFrameDropIntervalCounter++;

//    CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: FRAME DROP------------> "+m_Tools.IntegertoStringConvert(framNumber)+" CNT: "+m_Tools.IntegertoStringConvert(m_iFrameDropIntervalCounter));

    LongLong diffTimeStamp = m_Tools.CurrentTimestamp() - m_LastIntervalStartingTime;
    bool bIsIntervalOver = diffTimeStamp <= 3*1000;

    if(bIsIntervalOver && m_iFrameDropIntervalCounter>3)
    {
        {
            Locker lock(*m_pMutex);
            m_nFPSChangeSignal = 1;
//            m_SignalQue.push(1);
        }
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();

        m_iFrameDropIntervalCounter=0;
//        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: *************************   FPS DECREASING  ***********************************************************");
    }
    else if(!bIsIntervalOver)
    {
        m_LastIntervalStartingTime = m_Tools.CurrentTimestamp();
        m_iFrameDropIntervalCounter=0;
    }
    
    return 1;
}

#endif

bool CFPSController::IsProcessableFrame()
{
    Tools tools;

//	CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "PushPacketForDecoding:: ClientFPS "+tools.DoubleToString(m_ClientFPS));

    if(m_nOwnFPS + FPS_COMPARISON_EPS > m_ClientFPS) return true;

    double diff = m_ClientFPS - m_nOwnFPS;

    double ratio = (double)(m_ClientFPS*1.0)/(diff*1.0);

    if(m_EncodingFrameCounter == 0)
    {
        m_DropSum+=ratio;
    }

    int indx = floor(m_DropSum + 0.5);
    m_EncodingFrameCounter++;

    if(m_EncodingFrameCounter == indx)
    {
        m_DropSum+=ratio;
//		CLogPrinter_WriteSpecific(CLogPrinter::INFO, "PushPacketForDecoding -> Indx = "+m_Tools.IntegertoStringConvert(indx) + "  ClientFPS: " +m_Tools.IntegertoStringConvert((int)m_ClientFPS)
//                                                      +"m_nOwnFPS = " + m_Tools.IntegertoStringConvert(m_nOwnFPS)+ ",  m_DropSum =" + m_Tools.DoubleToString(m_DropSum));
        return false;
    }

    return true;
}


void CFPSController::SetEncoder(CVideoEncoder *videoEncoder)
{
    m_pVideoEncoder = videoEncoder;
}
