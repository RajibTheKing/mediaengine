#include "Gain.h"
#include "AudioCallSession.h"

#ifdef USE_WEBRTC_AGC
#define AGC_SAMPLES_IN_FRAME 80
#define AGC_ANALYSIS_SAMPLES_IN_FRAME 80
#define AGCMODE_UNCHANGED 0
#define AGCMODE_ADAPTIVE_ANALOG 1
#define AGNMODE_ADAPTIVE_DIGITAL 2
#define AGCMODE_FIXED_DIGITAL 3
#define MINLEVEL 1
#define MAXLEVEL 255
#endif


#define MAX_GAIN 10
#define DEF_GAIN 3
#define LS_RATIO 1


CGain::CGain()
{
	m_iVolume = DEF_GAIN;
	m_bGainEnabled = false;

#ifdef USE_WEBRTC_AGC
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
	if ((agcret = WebRtcAgc_Init(AGC_instance, MINLEVEL, SHRT_MAX, AGNMODE_ADAPTIVE_DIGITAL, AUDIO_SAMPLE_RATE)))
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


CGain::~CGain()
{
#ifdef USE_WEBRTC_AGC
	WebRtcAgc_Free(AGC_instance);
#endif
}

int CGain::SetGain(int iGain)
{
	if(iGain < 0)
	{
		m_bGainEnabled = false;
		return false;
	}
	else if (iGain >= 0)
	{
		m_bGainEnabled = true;
	}
#ifdef USE_NAIVE_AGC
	if (iGain >= 0 && iGain <= MAX_GAIN)
	{
		m_iVolume = iGain;
		ALOG("SetVolume called with: " + Tools::IntegertoStringConvert(iVolume));
	}
	else
	{
		m_iVolume = DEF_GAIN;
	}
#elif defined(USE_WEBRTC_AGC)
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


int CGain::AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning)
{
	if (!m_bGainEnabled)
	{
		return false;
	}
#ifdef USE_WEBRTC_AGC
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
#elif defined(USE_NAIVE_AGC)

	for (int i = 0; i < CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning); i++)
	{
		int temp = (int)sInBuf[i] * m_iVolume;
		if (temp > SHRT_MAX)
		{
			temp = SHRT_MAX;
		}
		if (temp < SHRT_MIN)
		{
			temp = SHRT_MIN;
		}
		sInBuf[i] = temp;
	}
	return true;
#endif
}

int CGain::AddFarEnd(short *sBuffer, int nBufferSize)
{
	if (!m_bGainEnabled)
	{
		return false;
	}
#ifdef USE_WEBRTC_AGC
	for (int i = 0; i < nBufferSize; i += AGC_SAMPLES_IN_FRAME)
	{
		if (0 != WebRtcAgc_AddFarend(AGC_instance, sBuffer + i, AGC_SAMPLES_IN_FRAME))
		{
			ALOG("WebRtcAgc_AddFarend failed");
		}
	}
#endif
	return true;
}