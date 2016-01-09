#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    #include <dispatch/dispatch.h>
#endif
//#include <android/log.h>

//#define LOG_TAG "NewTest"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

CAudioCallSession::CAudioCallSession(LongLong fname, CCommonElementsBucket* sharedObject) :

m_pCommonElementsBucket(sharedObject)

{
	m_pSessionMutex.reset(new CLockHandler);
	friendID = fname;
    
    StartEncodingThread();
    StartDecodingThread();

	CLogPrinter::Write(CLogPrinter::INFO, "CController::StartAudioCall Session empty");
}

CAudioCallSession::~CAudioCallSession()
{
    StopDecodingThread();
    StopEncodingThread();


    delete m_pG729CodecNative;

    
	/*if (NULL != m_pAudioDecoder)
	{
		delete m_pAudioDecoder;

		m_pAudioDecoder = NULL;
	}

	if (NULL != m_pAudioEncoder)
	{
		delete m_pAudioEncoder;

		m_pAudioEncoder = NULL;
	}*/

	friendID = -1;

	SHARED_PTR_DELETE(m_pSessionMutex);
}

LongLong CAudioCallSession::GetFriendID()
{
	return friendID;
}

void CAudioCallSession::InitializeAudioCallSession(LongLong lFriendID)
{
	CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession");

	//this->m_pAudioEncoder = new CAudioEncoder(m_pCommonElementsBucket);

	//m_pAudioEncoder->CreateAudioEncoder();

	//this->m_pAudioDecoder = new CAudioDecoder(m_pCommonElementsBucket);

	//m_pAudioDecoder->CreateAudioDecoder();

	m_pG729CodecNative = new G729CodecNative();
	m_pG729CodecNative->Open();

	CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSession::InitializeAudioCallSession session initialized");
}

int CAudioCallSession::EncodeAudioData(short *in_data, unsigned int in_size)
{
    /*CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData 1");
	int size = m_pG729CodecNative->Encode(in_data, in_size, &m_EncodedFrame[1]);
    m_EncodingFrame[0] = 0;
    CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData encoded");

	//m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodingFrame);
    
    m_pCommonElementsBucket->SendFunctionPointer(friendID,1,m_EncodedFrame,size);
     
    return 1;*/
    
    CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData");
    
    int returnedValue = m_EncodingBuffer.Queue(in_data, in_size);
    
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodeAudioData pushed to encoder queue");

    return returnedValue;
}

int CAudioCallSession::DecodeAudioData(unsigned char *in_data, unsigned int in_size)
{
    if(in_size > 200)
    {
        CLogPrinter::Write(CLogPrinter::DEBUGS, "CController::DecodeAudioData BIG AUDIO !!!");
        return 0;
    }
    
    /*CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CController::DecodeAudioData in_size " + m_Tools.IntegertoStringConvert(in_size));
    
	int size = m_pG729CodecNative->Decode(&in_data[1], in_size-1, m_DecodedFrame);
    
    CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CController::DecodeAudioData size " + m_Tools.IntegertoStringConvert(size));
    
	m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(friendID, size, m_DecodedFrame);
    
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CController::DecodeAudioData 3");
     
    return 1;*/
    
    int returnedValue = m_DecodingBuffer.Queue(&in_data[1], in_size-1);

	return returnedValue;
}

CAudioEncoder* CAudioCallSession::GetAudioEncoder()
{
	//	return sessionMediaList.GetFromAudioEncoderList(mediaName);

	return m_pAudioEncoder;
}

CAudioDecoder* CAudioCallSession::GetAudioDecoder()
{
	//	return sessionMediaList.GetFromAudioEncoderList(mediaName);

	return m_pAudioDecoder;
}


void CAudioCallSession::StopEncodingThread()
{
    //if (pInternalThread.get())
    {
        bEncodingThreadRunning = false;
        
        while (!bEncodingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //pInternalThread.reset();
}

void CAudioCallSession::StartEncodingThread()
{
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 1");
    
    if (pEncodingThread.get())
    {
        CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 2");
        
        pEncodingThread.reset();
        
        CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDecodingThread 3");
        
        return;
    }
    
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 4");
    
    bEncodingThreadRunning = true;
    bEncodingThreadClosed = false;
    
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread 5");
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(EncodeThreadQ, ^{
        this->EncodingThreadProcedure();
    });
    
#else
    
    std::thread myThread(CreateAudioEncodingThread, this);
    myThread.detach();
    
#endif
    
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartedInternalThread Encoding Thread started");
    
    return;
}

void *CAudioCallSession::CreateAudioEncodingThread(void* param)
{
    CAudioCallSession *pThis = (CAudioCallSession*)param;
    pThis->EncodingThreadProcedure();
    
    return NULL;
}

void CAudioCallSession::EncodingThreadProcedure()
{
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::EncodingThreadProcedure() Started EncodingThreadProcedure.");
    Tools toolsObject;
    int frameSize, encodedFrameSize;
    
    while (bEncodingThreadRunning)
    {
        CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::InternalThreadImpl");
        
        if (m_EncodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
            frameSize = m_EncodingBuffer.DeQueue(m_EncodingFrame);

            CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData 1");
            int size;


            size = m_pG729CodecNative->Encode(m_EncodingFrame, frameSize, &m_EncodedFrame[1]);

            m_EncodedFrame[0] = 0;
            
            CLogPrinter::Write(CLogPrinter::INFO, "CAudioCallSession::EncodeAudioData encoded");
            
            //m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(1, size, m_EncodedFrame);
            
            m_pCommonElementsBucket->SendFunctionPointer(friendID,1,m_EncodedFrame,size);

            toolsObject.SOSleep(1);
            
        }
    }
    
    bEncodingThreadClosed = true;
    
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CAudioCallSession::EncodingThreadProcedure() Stopped EncodingThreadProcedure");
}

void CAudioCallSession::StopDecodingThread()
{
    //if (pDecodingThread.get())
    {
        bDecodingThreadRunning = false;
        
        while (!bDecodingThreadClosed)
            m_Tools.SOSleep(5);
    }
    
    //pDecodingThread.reset();
}

void CAudioCallSession::StartDecodingThread()
{
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDecodingThread 1");
    
    if (pDecodingThread.get())
    {
        CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDecodingThread 2");
        
        pDecodingThread.reset();
        
        CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDecodingThread 3");
        
        return;
    }
    
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDecodingThread 4");
    
    bDecodingThreadRunning = true;
    bDecodingThreadClosed = false;
    
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDecodingThread 5");
    
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    
    dispatch_queue_t DecodeThreadQ = dispatch_queue_create("DecodeThreadQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(DecodeThreadQ, ^{
        this->DecodingThreadProcedure();
    });
    
#else
    
    std::thread myThread(CreateAudioDecodingThread, this);
    myThread.detach();
    
#endif
    
    CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::StartDecodingThread Decoding Thread started");
    
    return;
}

void *CAudioCallSession::CreateAudioDecodingThread(void* param)
{
    CAudioCallSession *pThis = (CAudioCallSession*)param;
    pThis->DecodingThreadProcedure();
    
    return NULL;
}

void CAudioCallSession::DecodingThreadProcedure()
{
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::DecodingThreadProcedure() Started DecodingThreadProcedure method.");
    Tools toolsObject;
    int frameSize;
    
    while (bDecodingThreadRunning)
    {
        //CLogPrinter::Write(CLogPrinter::INFO, "CVideoCallSession::DecodingThreadProcedure");
        
        if (m_DecodingBuffer.GetQueueSize() == 0)
            toolsObject.SOSleep(10);
        else
        {
            frameSize = m_DecodingBuffer.DeQueue(m_DecodingFrame);
            
            CLogPrinter::Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure frameSize " + m_Tools.IntegertoStringConvert(frameSize));

            int size;

            size = m_pG729CodecNative->Decode(m_DecodingFrame, frameSize, m_DecodedFrame);

            CLogPrinter::Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure size " + m_Tools.IntegertoStringConvert(size));
            
            m_pCommonElementsBucket->m_pEventNotifier->fireAudioEvent(friendID, size, m_DecodedFrame);
            
            CLogPrinter::Write(CLogPrinter::DEBUGS, "CAudioCallSession::DecodingThreadProcedure 3");

            toolsObject.SOSleep(1);
        }
    }
    
    bDecodingThreadClosed = true;
    
    CLogPrinter::Write(CLogPrinter::DEBUGS, "CVideoCallSession::DecodingThreadProcedure() Stopped DecodingThreadProcedure method.");
}

