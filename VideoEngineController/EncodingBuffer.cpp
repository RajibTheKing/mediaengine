
#include "EncodingBuffer.h"

#include <string.h>

CEncodingBuffer::CEncodingBuffer() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_ENCODER_BUFFER_SIZE)

{
	m_pEncodingBufferMutex.reset(new CLockHandler);
}

CEncodingBuffer::~CEncodingBuffer()
{

}

int CEncodingBuffer::Queue(unsigned char *ucaCapturedVideoFrameData, int nLength, int nCaptureTimeDifference)
{
	Locker lock(*m_pEncodingBufferMutex);
    
	memcpy(m_uc2aCapturedVideoDataBuffer[m_iPushIndex], ucaCapturedVideoFrameData, nLength);

	m_naBufferDataLengths[m_iPushIndex] = nLength;

	m_naBufferCaptureTimeDifferences[m_iPushIndex] = nCaptureTimeDifference;
	m_llBufferInsertionTimes[m_iPushIndex] = m_Tools.CurrentTimestamp();
    
	if (m_nQueueSize == m_nQueueCapacity)
    {
        IncreamentIndex(m_iPopIndex);
    }
    else
    {  
		m_nQueueSize++;      
    }
    
    IncreamentIndex(m_iPushIndex);
    
    return 1;   
}

int CEncodingBuffer::DeQueue(unsigned char *ucaCapturedVideoFrameData, int &nrTimeDifferenceInQueue, int &nrCaptureTimeDifference)
{
	Locker lock(*m_pEncodingBufferMutex);

	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nlength;
		
		nlength = m_naBufferDataLengths[m_iPopIndex];

		memcpy(ucaCapturedVideoFrameData, m_uc2aCapturedVideoDataBuffer[m_iPopIndex], nlength);

		nrCaptureTimeDifference = m_naBufferCaptureTimeDifferences[m_iPopIndex];
		nrTimeDifferenceInQueue = m_Tools.CurrentTimestamp() - m_llBufferInsertionTimes[m_iPopIndex];

		IncreamentIndex(m_iPopIndex);
		m_nQueueSize--;

		return nlength;
	}
}

void CEncodingBuffer::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_nQueueCapacity)
		index = 0;
}

int CEncodingBuffer::GetQueueSize()
{
	Locker lock(*m_pEncodingBufferMutex);

	return m_nQueueSize;
}