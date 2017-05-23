#include "WebRTCEchoCanceller.h"

#include "Tools.h"
#include "LogPrinter.h"

//It is strongly recommended you don't remove this commented out code
//#include "Filt.h"
//#define USE_LOW_PASS

#define ECHO_ANALYSIS

#ifdef ECHO_ANALYSIS
FILE *EchoFile;
#define HEADER_SIZE 1
#define WEBRTC_FAREND 1
#define SPEEX_FAREND 2
#define NEAREND 3
#endif

#ifndef ALOG
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "ALOG:" + a);
#endif


WebRTCEchoCanceller::WebRTCEchoCanceller() : m_bAecmCreated(false), m_bAecmInited(false)
{

#ifdef USE_AECM
#ifdef ECHO_ANALYSIS
	m_bNearEndingOrFarEnding = false;
	EchoFile = fopen("/sdcard/endSignal.pcma3", "wb");
#endif


	int iAECERR = -1;

	AECM_instance = WebRtcAecm_Create();
	m_bAecmCreated = true;

	iAECERR = WebRtcAecm_Init(AECM_instance, AUDIO_SAMPLE_RATE);
	if (iAECERR)
	{
		ALOG("WebRtcAecm_Init failed");
	}
	else
	{
		ALOG("WebRtcAecm_Init successful");
		m_bAecmInited = true;
	}

	AecmConfig aecConfig;
	aecConfig.cngMode = AecmFalse;
	aecConfig.echoMode = 4;
	if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
	{
		ALOG("WebRtcAecm_set_config unsuccessful");
	}
	else
	{
		ALOG("WebRtcAecm_set_config successful");
	}
	memset(m_sZeroBuf, 0, AECM_SAMPLES_IN_FRAME * sizeof(short));

	m_llLastFarendTime = 0;
	iCounter = 0;
	iCounter2 = 0;
#endif
}


WebRTCEchoCanceller::~WebRTCEchoCanceller()
{
#ifdef USE_AECM
	ALOG("WebRtcAec_destructor called");
	WebRtcAecm_Free(AECM_instance);
#endif

#ifdef ECHO_ANALYSIS
	fclose(EchoFile);
#endif

}


int WebRTCEchoCanceller::CancelEcho(short *sInBuf, int sBufferSize, bool isLiveStreamRunning, long long llDelay)
{
#ifdef USE_AECM
	if (sBufferSize != CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning))
	{
		ALOG("aec nearend Invalid size");
		return false;
	}
	LOG18("Nearending2");


	iCounter++;

	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i += AECM_SAMPLES_IN_FRAME)
	{

		while (m_bNearEndingOrFarEnding)
		{
			Tools::SOSleep(1);
		}
		m_bNearEndingOrFarEnding = true;
#ifdef ECHO_ANALYSIS

		short temp = NEAREND;
		fwrite(&temp, sizeof(short), 1, EchoFile);
		fwrite(&llDelay, sizeof(short), 1, EchoFile);
		fwrite(sInBuf + i, sizeof(short), AECM_SAMPLES_IN_FRAME, EchoFile);
#endif
		bool bFailed = false, bZeroed = false;
		if (0 != WebRtcAecm_Process(AECM_instance, sInBuf + i, NULL, sInBuf + i, AECM_SAMPLES_IN_FRAME, llDelay))
		{
			ALOG("WebRtcAec_Process failed bAecmCreated = " + m_Tools.IntegertoStringConvert((int)bAecmCreated) + " delay = " + m_Tools.IntegertoStringConvert((int)llDelay)
				+ " err = " + m_Tools.IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance)) + " id = " + m_Tools.IntegertoStringConvert(m_ID)
				+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));
			bFailed = true;
		}
		else
		{
			/*ALOG("WebRtcAec_Process successful Delay = " + m_Tools.IntegertoStringConvert((int)delay) + " id = " + m_Tools.IntegertoStringConvert(m_ID)
			+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
			+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));*/
		}
		m_bNearEndingOrFarEnding = false;

	}


#ifdef USE_LOW_PASS
	if (isLoudspeaker)
	{
		LowPass(sInBuf, sBufferSize);
		//DeAmplitude(sInBuf, sBufferSize);
		/ *for (int i = 0; i < sBufferSize; i++)
		{
			if (sInBuf[i] > 10922)
			{
				LOGE("###CE### %d", (int)sInBuf[i]);
			}
			sInBuf[i] *= 2;
		}*/
	}
#endif
#endif
	return true;
}

int WebRTCEchoCanceller::AddFarEndData(short *sBuffer, int sBufferSize, bool isLiveStreamRunning)
{
#ifdef USE_AECM

	if (sBufferSize != CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning))
	{
		ALOG("aec farend Invalid size");
		return false;
	}

	LOG18("Farending2");

	for (int i = 0; i < sBufferSize; i += AECM_SAMPLES_IN_FRAME)
	{
		while (m_bNearEndingOrFarEnding)
		{
			Tools::SOSleep(1);
		}
		m_bNearEndingOrFarEnding = true;

#ifdef ECHO_ANALYSIS

		short temp = WEBRTC_FAREND;
		short iDelay = 0;
		fwrite(&temp, sizeof(short), 1, EchoFile);
		fwrite(&iDelay, sizeof(short), 1, EchoFile);
		fwrite(sBuffer + i, sizeof(short), AECM_SAMPLES_IN_FRAME, EchoFile);

#endif
		if (0 != WebRtcAecm_BufferFarend(AECM_instance, sBuffer + i, AECM_SAMPLES_IN_FRAME))
		{
			ALOG("WebRtcAec_BufferFarend failed id = " + m_Tools.IntegertoStringConvert(m_ID) + " err = " + m_Tools.IntegertoStringConvert(WebRtcAecm_get_error_code(AECM_instance))
				+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
				+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));
		}
		else
		{
			m_llLastFarendTime = Tools::CurrentTimestamp();
			/*ALOG("WebRtcAec_BufferFarend successful id = " + m_Tools.IntegertoStringConvert(m_ID)
			+ " iCounter = " + m_Tools.IntegertoStringConvert(iCounter)
			+ " iCounter2 = " + m_Tools.IntegertoStringConvert(iCounter2));*/
		}
		m_bNearEndingOrFarEnding = false;
	}
#endif

	return true;
}

//It is strongly recommended you don't remove this commented out code
/*
int CEcho::DeAmplitude(short *sInBuf, int sBufferSize)
{
int iAmplitudeThreshold = 50;
int iAmplitudeSum = 0;

for (int i = 0; i < sBufferSize; i++)
{
iAmplitudeSum += abs(sInBuf[i]);
}
short iAvgAmplitude = iAmplitudeSum / sBufferSize;

if (iAvgAmplitude <= iAmplitudeThreshold)
{
for (int i = 0; i < sBufferSize; i++)
{
sInBuf[i] = 0;
}
}
return true;
}
int CEcho::LowPass(short *sInBuf, int sBufferSize)
{
Filter *my_filter = new Filter(LPF, 51, 8, 2.0);
for (int i = 0; i < sBufferSize; i++)
{
sInBuf[i] = (short)(my_filter->do_sample((double)sInBuf[i]));
}
return true;
}
*/