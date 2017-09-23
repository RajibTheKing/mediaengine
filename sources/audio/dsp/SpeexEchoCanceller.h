#ifndef SPEEX_ECHO_CANCELLER_H
#define SPEEX_ECHO_CANCELLER_H


#include "EchoCancellerInterface.h"
#include "AudioMacros.h"
#ifdef USE_AECM
#include "speex_echo.h"
#include "speex_preprocess.h"
#endif

namespace MediaSDK
{

	class SpeexEchoCanceller : public EchoCancellerInterface
	{
	private:
		bool m_bFarendArrived;
		bool m_bReadingFarend, m_bWritingFarend;

		short m_sSpeexFarendBuf[MAX_AUDIO_FRAME_SAMPLE_SIZE];
#ifdef USE_AECM
		SpeexEchoState *st;
		SpeexPreprocessState *den;
#endif


	public:

		SpeexEchoCanceller();

		~SpeexEchoCanceller();

		int AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning);
	
		int CancelEcho(short *nearEndData, int dataLen, long long llDelay);

	};

} //namespace MediaSDK

#endif  // !SPEEX_ECHO_CANCELLER_H