
#include "AudioDecoderBuffer.h"

#include <string.h>

CAudioDecoderBuffer::CAudioDecoderBuffer() :
m_iPushIndex(0),
m_iPopIndex(0),
m_iQueueSize(0),
m_iQueueCapacity(5)
{
	m_pChannelMutex.reset(new CLockHandler);
    m_lPrevOverFlowTime = -1;
    m_dAvgOverFlowTime = 0;
    m_iOverFlowCount = 0;
    m_lSumOverFlowTime = 0;
    
}

CAudioDecoderBuffer::~CAudioDecoderBuffer()
{

}

int CAudioDecoderBuffer::Queue(unsigned char *frame, int length)
{
	Locker lock(*m_pChannelMutex);

	memcpy(m_Buffer[m_iPushIndex], frame, length);

	m_BufferDataLength[m_iPushIndex] = length;
	m_BufferInsertionTime[m_iPushIndex] =  m_Tools.CurrentTimestamp();

	if (m_iQueueSize == m_iQueueCapacity)
	{
        /*
        if(m_lPrevOverFlowTime == -1)
        {
            m_lPrevOverFlowTime = m_Tools.CurrentTimestamp();
        }
        else
        {
            long long lOverFlowTime = m_Tools.CurrentTimestamp() - m_lPrevOverFlowTime;
            m_lSumOverFlowTime += lOverFlowTime;
            m_iOverFlowCount ++;
            m_dAvgOverFlowTime = m_lSumOverFlowTime * 1.0 / m_iOverFlowCount;
            printf("TheVampire--> OverFlow Difftime Decode= %lld, m_dAvgOverFlowTimeDif = %lf\n", lOverFlowTime, m_dAvgOverFlowTime);
            m_lPrevOverFlowTime = m_Tools.CurrentTimestamp();
        }
        */
        
		IncreamentIndex(m_iPopIndex);
	}
	else
	{
		m_iQueueSize++;
	}

	IncreamentIndex(m_iPushIndex);

	return 1;
}

int CAudioDecoderBuffer::DeQueue(unsigned char *decodeBuffer)
{
	Locker lock(*m_pChannelMutex);

	if (m_iQueueSize == 0)
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

void CAudioDecoderBuffer::IncreamentIndex(int &index)
{
	index++;

	if (index >= m_iQueueCapacity)
		index = 0;
}

int CAudioDecoderBuffer::GetQueueSize()
{
	Locker lock(*m_pChannelMutex);

	return m_iQueueSize;
}
