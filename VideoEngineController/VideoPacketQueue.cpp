
#include "VideoPacketQueue.h"
#include "ThreadTools.h"
#include "LogPrinter.h"


CVideoPacketQueue::CVideoPacketQueue() :

m_iPushIndex(0),
m_iPopIndex(0),
m_nQueueSize(0),
m_nQueueCapacity(MAX_VIDEO_PACKET_QUEUE_SIZE)

{
	m_pVideoPacketQueueMutex.reset(new CLockHandler);
}

CVideoPacketQueue::~CVideoPacketQueue()
{
	SHARED_PTR_DELETE(m_pVideoPacketQueueMutex);
}

void CVideoPacketQueue::ResetBuffer()
{
	Locker lock(*m_pVideoPacketQueueMutex);

	m_iPushIndex = 0;
	m_iPopIndex = 0;
	m_nQueueSize = 0;
}

int CVideoPacketQueue::Queue(unsigned char *ucaVideoPacketData, int nLength)
{
	Locker lock(*m_pVideoPacketQueueMutex);

	if (m_nQueueSize >= m_nQueueCapacity)
	{
		CLogPrinter_WriteLog(CLogPrinter::DEBUGS, QUEUE_OVERFLOW_LOG ,"Video Buffer OverFlow ( VideoPacketQueue ) --> OverFlow " );
		return -1;
	}
	else
	{
		if (nLength >= MAX_VIDEO_PACKET_SIZE)
			return -1;

		memcpy(m_uc2aVideoPacketBuffer[m_iPushIndex], ucaVideoPacketData, nLength);

		m_naBufferDataLengths[m_iPushIndex] = nLength;

		IncreamentIndex(m_iPushIndex);
		m_nQueueSize++;

		return 1;
	}
}

int CVideoPacketQueue::DeQueue(unsigned char *ucaVideoPacketData)
{
	Locker lock(*m_pVideoPacketQueueMutex);

	if (m_nQueueSize <= 0)
	{
		return -1;
	}
	else
	{
		int nLength;
			
		nLength = m_naBufferDataLengths[m_iPopIndex];

		memcpy(ucaVideoPacketData, m_uc2aVideoPacketBuffer[m_iPopIndex], nLength);

		IncreamentIndex(m_iPopIndex);
		m_nQueueSize--;

		return nLength;
	}
}

void CVideoPacketQueue::IncreamentIndex(int &riIndex)
{
	riIndex++;

	if (riIndex >= m_nQueueCapacity)
		riIndex = 0;
}

int CVideoPacketQueue::GetQueueSize()
{
	Locker lock(*m_pVideoPacketQueueMutex);

	return m_nQueueSize;
}