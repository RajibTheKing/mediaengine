#include "WebRTCGain.h"

#include "LogPrinter.h"
#ifdef USE_AGC
#include "gain_control.h"
#endif
#include "AudioMacros.h"

#ifndef ALOG
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "ALOG:" + a);
#endif

WebRTCGain::WebRTCGain() : m_bGainEnabled(false)
{
#ifdef USE_AGC
	LOG_AAC("#gain# WebRTCGain::WebRTCGain()");

	m_iVolume = DEFAULT_GAIN;
	m_sTempBuf = new short[MAX_AUDIO_FRAME_SAMPLE_SIZE];
	int agcret = -1;

	if ((agcret = WebRtcAgc_Create(&AGC_instance)))
	{
		ALOG("WebRtcAgc_Create failed with error code = " + m_Tools.IntegertoStringConvert(agcret));
	}
	else
	{
		ALOG("WebRtcAgc_Create successful");
	}
	if ((agcret = WebRtcAgc_Init(AGC_instance, WEBRTC_AGC_MIN_LEVEL, SHRT_MAX, MODE_ADAPTIVE_DIGITAL, AUDIO_SAMPLE_RATE)))
	{
		ALOG("WebRtcAgc_Init failed with error code= " + m_Tools.IntegertoStringConvert(agcret));
	}
	else
	{
		ALOG("WebRtcAgc_Init successful");
	}

	WebRtcAgc_config_t gain_config;
	gain_config.targetLevelDbfs = 3;
	gain_config.compressionGaindB = m_iVolume * 10;
	gain_config.limiterEnable = 0;

	if ((agcret = WebRtcAgc_set_config(AGC_instance, gain_config)))
	{
		ALOG("WebRtcAgc_set_config failed with error code= " + m_Tools.IntegertoStringConvert(agcret));
	}
	else
	{
		ALOG("WebRtcAgc_Create successful");
	}
#endif
}


WebRTCGain::~WebRTCGain()
{
#ifdef USE_AGC
	LOG_AAC("#gain# WebRTCGain::~WebRTCGain()");

	WebRtcAgc_Free(AGC_instance);
#endif
}


int WebRTCGain::SetGain(int iGain)
{
#ifdef USE_AGC
	LOG_AAC("#gain#Set# WebRTCGain::SetGain(), %d", iGain);

	if (iGain < 0)
	{
		m_bGainEnabled = false;
		return false;
	}
	else if (iGain >= 0)
	{
		m_bGainEnabled = true;
	}

	WebRtcAgc_config_t gain_config;

	m_iVolume = iGain;
	gain_config.targetLevelDbfs = 3;
	gain_config.compressionGaindB = m_iVolume * 10;
	gain_config.limiterEnable = 0;

	if (WebRtcAgc_set_config(AGC_instance, gain_config))
	{
		ALOG("WebRtcAgc_set_config failed  ");
	}
	else
	{
		ALOG("WebRtcAgc_set_config successful");
	}
#endif

	return true;
}


int WebRTCGain::AddFarEnd(short *sInBuf, int nBufferSize)
{
#ifdef USE_AGC
	LOG_AAC("#gain# WebRTCGain::AddFarEnd(), %d", nBufferSize);

	if (!m_bGainEnabled)
	{
		return false;
	}

	for (int i = 0; i < nBufferSize; i += AGC_SAMPLES_IN_FRAME)
	{
		if (0 != WebRtcAgc_AddFarend(AGC_instance, sInBuf + i, AGC_SAMPLES_IN_FRAME))
		{
			ALOG("WebRtcAgc_AddFarend failed");
		}
	}
#endif
	return true;
}


int WebRTCGain::AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning)
{
#ifdef USE_AGC
	LOG_AAC("#gain# WebRTCGain::AddGain(), %d", nBufferSize);

	if (!m_bGainEnabled)
	{
		return false;
	}

	uint8_t saturationWarning;
	int32_t inMicLevel = 1;
	int32_t outMicLevel;
	bool bSucceeded = true;

	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i += AGC_ANALYSIS_SAMPLES_IN_FRAME)
	{
		if (0 != WebRtcAgc_AddMic(AGC_instance, sInBuf + i, 0, AGC_SAMPLES_IN_FRAME))
		{
			ALOG("WebRtcAgc_AddMic failed");
			bSucceeded = false;
		}
		if (0 != WebRtcAgc_VirtualMic(AGC_instance, sInBuf + i, 0, AGC_SAMPLES_IN_FRAME, inMicLevel, &outMicLevel))
		{
			ALOG("WebRtcAgc_AddMic failed");
			bSucceeded = false;
		}
		if (0 != WebRtcAgc_Process(AGC_instance, sInBuf + i, 0, AGC_SAMPLES_IN_FRAME,
			m_sTempBuf + i, 0,
			inMicLevel, &outMicLevel, 0, &saturationWarning))
		{
			ALOG("WebRtcAgc_Process failed");
			bSucceeded = false;
		}
	}

#if 0
	if (memcmp(sInBuf, m_sTempBuf, nBufferSize * sizeof(short)) == 0)
	{
		ALOG("WebRtcAgc_Process did nothing");
		return false;
	}
	else
	{
		ALOG("WebRtcAgc_Process tried to do something, believe me :-( . Outputmic =  "
			+ m_Tools.IntegertoStringConvert(outMicLevel) + " saturationWarning = " + m_Tools.IntegertoStringConvert(saturationWarning));
	}

	int k = 1;
	double iRatio = 0;
	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i++)
	{
		if (sInBuf[i])
		{
			//ALOG("ratio = " + m_Tools.IntegertoStringConvert(m_saAudioDecodedFrameTemp[i] / m_saAudioRecorderFrame[i]));
			iRatio += m_sTempBuf[i] * 1.0 / sInBuf[i];
			k++;
		}
	}
	ALOG("ratio = " + m_Tools.DoubleToString(iRatio / k));
#endif

	memcpy(sInBuf, m_sTempBuf, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning) * sizeof(short));

	return bSucceeded;;
#endif

	return true;
}
