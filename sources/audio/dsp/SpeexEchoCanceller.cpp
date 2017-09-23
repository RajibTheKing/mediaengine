#include "SpeexEchoCanceller.h"
#include "LogPrinter.h"
#include "Tools.h"

#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "ALOG:" + a);


namespace MediaSDK
{
	SpeexEchoCanceller::SpeexEchoCanceller() : m_bFarendArrived(false), m_bReadingFarend(false), m_bWritingFarend(false)
	{
#ifdef USE_AECM
#ifdef __ANDROID__
		int sampleRate = AUDIO_SAMPLE_RATE;
		st = speex_echo_state_init(AECM_SAMPLES_IN_FRAME, 1024);
		int db = -60;
		den = speex_preprocess_state_init(AECM_SAMPLES_IN_FRAME, sampleRate);
		speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &db);
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &db);

		int i;
		float f;
		i = 1;
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DENOISE, &i);
		i = 1;
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB, &i);
		f = 0.9;
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
		f = 0.9;
		speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);


		// NOTE: Speex gain has been removed from here
#endif
#endif
	}


	SpeexEchoCanceller::~SpeexEchoCanceller()
	{
#ifdef USE_AECM
#ifdef __ANDROID__
		speex_echo_state_destroy(st);
		speex_preprocess_state_destroy(den);
#endif
#endif
	}

	
	int SpeexEchoCanceller::AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning)
	{
#ifdef USE_AECM
#ifdef __ANDROID__
		while (m_bReadingFarend)
		{
			Tools::SOSleep(1);
		}
		m_bWritingFarend = true;
		memcpy(m_sSpeexFarendBuf, farEndData, CURRENT_AUDIO_FRAME_SAMPLE_SIZE(isLiveStreamRunning) * sizeof(short));
		m_bWritingFarend = false;
#endif
#endif

		return true;
	}


	int SpeexEchoCanceller::CancelEcho(short *nearEndData, int nBufferSize, long long llDelay)
{
#ifdef USE_AECM
#ifdef __ANDROID__
		while (m_bWritingFarend)
		{
			Tools::SOSleep(1);
		}
		m_bReadingFarend = true;

		for (int i = 0; i < nBufferSize; i += AECM_SAMPLES_IN_FRAME)
		{
			speex_echo_playback(st, m_sSpeexFarendBuf + i);
			speex_echo_capture(st, nearEndData + i, nearEndData + i);
			speex_preprocess_run(den, nearEndData + i);
		}

		m_bReadingFarend = false;
		m_bFarendArrived = false;
#endif
#endif 

		return true;
	}

} //namespace MediaSDK