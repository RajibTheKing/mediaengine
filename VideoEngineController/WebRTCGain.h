#ifndef WEBRTC_GAIN_H
#define WEBRTC_GAIN_H

#include "AudioGainInterface.h"

#define AGC_SAMPLES_IN_FRAME 80
#define AGC_ANALYSIS_SAMPLES_IN_FRAME 80

#define WEBRTC_AGC_MIN_LEVEL 1
#define WEBRTC_AGC_MAX_LEVEL 255

enum AGCMode
{
	MODE_UNCHANGED = 0,
	MODE_ADAPTIVE_ANALOG = 1,
	MODE_ADAPTIVE_DIGITAL = 2,
	MODE_FIXED_DIGITAL = 3
};

class WebRTCGain : public AudioGainInterface
{
	bool m_bGainEnabled;
	short *m_sTempBuf;
	int m_iVolume;

	void* AGC_instance;

public:

	WebRTCGain();

	virtual ~WebRTCGain();

	int SetGain(int iGain);

	int AddFarEnd(short *sInBuf, int nBufferSize);

	int AddGain(short *sInBuf, int nBufferSize, bool isLiveStreamRunning);

};


#endif  // !WEBRTC_GAIN_H
