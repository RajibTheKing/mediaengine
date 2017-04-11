#include "GomGomGain.h"
#include "math.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

#include <limits.h>

#endif

#define MAX_GAIN 2

CGomGomGain::CGomGomGain(int id)
{
	m_ID = id;
	mFilter = new Filter(BPF, 51, 8.0, 0.5, 2.0);
	memset(m_daMovingAvg, 0, MAX_AUDIO_FRAME_SAMPLE_SIZE * sizeof(double));
	memset(m_naMultFactor, 0, MAX_AUDIO_FRAME_SAMPLE_SIZE * sizeof(unsigned int));
	memset(m_sLastFilteredFrame, 0, MAX_AUDIO_FRAME_SAMPLE_SIZE * sizeof(short));
	b1stFrame = true;
	m_nMovingSum = 0;
}

CGomGomGain::~CGomGomGain()
{
	delete mFilter;
}

int CGomGomGain::AddGain(short *sInBuf, int sBufferSize)
{
	for (int i = 0; i < sBufferSize; i++)
	{
		//m_sFilteredFrame[i] = (short)(mFilter->do_sample((double)sInBuf[i]));
		m_sFilteredFrame[i] = sInBuf[i];
	}
	
	for (int i = 0; i < sBufferSize; i++)
	{
		m_nMovingSum += abs(m_sFilteredFrame[i]) - abs(m_sLastFilteredFrame[i]);

		if (b1stFrame)
		{
			m_daMovingAvg[i] = ((double)m_nMovingSum) /(i+1);
		}
		else 
		{
			m_daMovingAvg[i] = m_nMovingSum / 800.0;
		}
	}
	for (int i = 0; i < sBufferSize; i++)
	{
		for (int j = MAX_GAIN; j >= 1; j--)
		{
			if (abs(m_daMovingAvg[i]) * j > SHRT_MAX / 2)
			{
				continue;
			}
			m_naMultFactor[i] = j;
			break;
		}
	}
	for (int i = 0; i < sBufferSize; i++)
	{
		int iTemp = m_sFilteredFrame[i] * m_naMultFactor[i];
		if (iTemp > SHRT_MAX)
		{
			iTemp = SHRT_MAX;
		}
		else if (iTemp < SHRT_MIN)
		{
			iTemp = SHRT_MIN;
		}
		sInBuf[i] = iTemp;
	}
	memcpy(m_sLastFilteredFrame, m_sFilteredFrame, sBufferSize * sizeof(short));
	b1stFrame = false;
	return true;
}