
#include "RetransmitVideoPacketQueue.h"

namespace MediaSDK
{

	CRetransmitVideoPacketQueue::CRetransmitVideoPacketQueue() :
		m_iPushIndex(0),
		m_iPopIndex(0),
		m_iQueueSize(0),
		m_iQueueCapacity(MAX_RETRANS_VIDEO_PACKET_QUEUE_SIZE)
	{
		m_pChannelMutex.reset(new CLockHandler);
	}

	CRetransmitVideoPacketQueue::~CRetransmitVideoPacketQueue()
	{

	}

	int CRetransmitVideoPacketQueue::Queue(unsigned char *frame, int length)
	{
		CRetransmitVideoPacketQueueLocker lock(*m_pChannelMutex);

		if (m_iQueueSize >= m_iQueueCapacity)
		{
			return -1;
		}
		else
		{
			memcpy(m_Buffer[m_iPushIndex], frame, length);

			m_BufferDataLength[m_iPushIndex] = length;

			IncreamentIndex(m_iPushIndex);

			m_iQueueSize++;

			return 1;
		}
	}

	int CRetransmitVideoPacketQueue::DeQueue(unsigned char *decodeBuffer)
	{
		CRetransmitVideoPacketQueueLocker lock(*m_pChannelMutex);

		if (m_iQueueSize <= 0)
		{
			return -1;
		}
		else
		{
			int length = m_BufferDataLength[m_iPopIndex];

			memcpy(decodeBuffer, m_Buffer[m_iPopIndex], length);

			IncreamentIndex(m_iPopIndex);

			m_iQueueSize--;

			return length;
		}
	}

	void CRetransmitVideoPacketQueue::IncreamentIndex(int &index)
	{
		index++;

		if (index >= m_iQueueCapacity)
			index = 0;
	}

	int CRetransmitVideoPacketQueue::GetQueueSize()
	{
		CRetransmitVideoPacketQueueLocker lock(*m_pChannelMutex);

		return m_iQueueSize;
	}

} //namespace MediaSDK
