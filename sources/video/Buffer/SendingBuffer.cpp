
#include "SendingBuffer.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	CSendingBuffer::CSendingBuffer() :

		m_iPushIndex(0),
		m_iPopIndex(0),
		m_nQueueSize(0),
		m_nMaxQueueSizeTillNow(0),
		m_nQueueCapacity(MAX_VIDEO_PACKET_SENDING_BUFFER_SIZE)

	{
		m_pSendingBufferMutex.reset(new CLockHandler);
	}

	CSendingBuffer::~CSendingBuffer()
	{

	}

	void CSendingBuffer::ResetBuffer()
	{
		SendingBufferLocker lock(*m_pSendingBufferMutex);

		m_iPushIndex = 0;
		m_iPopIndex = 0;
		m_nQueueSize = 0;
	}

	int CSendingBuffer::Queue(long long llFriendID, unsigned char *ucaSendingVideoPacketData, int nLength, int iFrameNumber, int iPacketNumber)
	{
		SendingBufferLocker lock(*m_pSendingBufferMutex);

		if (m_nQueueSize > m_nMaxQueueSizeTillNow)
			m_nMaxQueueSizeTillNow = m_nQueueSize;

		CLogPrinter_LOG(BUFFER_SIZE_LOG, "CSendingBuffer::Queue SENDING Buffer size %d m_nMaxQueueSizeTillNow %d m_nQueueCapacity %d", m_nQueueSize, m_nMaxQueueSizeTillNow, m_nQueueCapacity);

		memcpy(m_uc2aSendingVideoPacketBuffer[m_iPushIndex], ucaSendingVideoPacketData, nLength);

		m_naBufferDataLengths[m_iPushIndex] = nLength;
		m_llaBufferFriendIDs[m_iPushIndex] = llFriendID;
		m_naBufferFrameNumbers[m_iPushIndex] = iFrameNumber;
		m_naBufferPacketNumbers[m_iPushIndex] = iPacketNumber;

		m_llaBufferInsertionTimes[m_iPushIndex] = m_Tools.CurrentTimestamp();

		if (m_nQueueSize == m_nQueueCapacity)
		{
			IncreamentIndex(m_iPopIndex);

			CLogPrinter_LOG(QUEUE_OVERFLOW_LOG, "CSendingBuffer::Queue SENDING Buffer OVERFLOW m_nQueueSize %d m_nQueueCapacity %d", m_nQueueSize, m_nQueueCapacity);
		}
		else
		{
			m_nQueueSize++;
		}

		IncreamentIndex(m_iPushIndex);

		return 1;
	}

	int CSendingBuffer::DeQueue(long long &rllFriendID, unsigned char *ucaSendingVideoPacketData, int &rnFrameNumber, int &rnPacketNumber, int &rnTimeDifferenceInQueue)
	{
		SendingBufferLocker lock(*m_pSendingBufferMutex);

		if (m_nQueueSize <= 0)
		{
			return -1;
		}
		else
		{
			int nLength;

			nLength = m_naBufferDataLengths[m_iPopIndex];
			rllFriendID = m_llaBufferFriendIDs[m_iPopIndex];
			rnFrameNumber = m_naBufferFrameNumbers[m_iPopIndex];
			rnPacketNumber = m_naBufferPacketNumbers[m_iPopIndex];

			memcpy(ucaSendingVideoPacketData, m_uc2aSendingVideoPacketBuffer[m_iPopIndex], nLength);

			rnTimeDifferenceInQueue = (int)(m_Tools.CurrentTimestamp() - m_llaBufferInsertionTimes[m_iPopIndex]);

			IncreamentIndex(m_iPopIndex);
			m_nQueueSize--;

			return nLength;
		}
	}

	void CSendingBuffer::IncreamentIndex(int &irIndex)
	{
		irIndex++;

		if (irIndex >= m_nQueueCapacity)
			irIndex = 0;
	}

	int CSendingBuffer::GetQueueSize()
	{
		SendingBufferLocker lock(*m_pSendingBufferMutex);

		return m_nQueueSize;
	}

} //namespace MediaSDK
