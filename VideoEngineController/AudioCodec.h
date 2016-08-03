#ifndef _AUDIO_ENCODER_H_
#define _AUDIO_ENCODER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>

#include "SmartPointer.h"
#include "LockHandler.h"
#include "ThreadTools.h"
#include "opus.h"
#include "Tools.h"
#include "size.h"

//#define FRAME_SIZE 960






namespace IPV
{
	class thread;
}

class CCommonElementsBucket;

class CAudioCodec
{

public:

	CAudioCodec(CCommonElementsBucket* sharedObject);
	~CAudioCodec();

	int CreateAudioEncoder();
	int Encode(short *in_data, unsigned int in_size, unsigned char *out_buffer);
	int Decode(unsigned char *in_data, unsigned int in_size, short *out_buffer);

	int decodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer);
	int encodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer);
	int encodeDecodeTest();
	bool SetBitrateOpus(int nBitrate);
	void DecideToChangeBitrate(int iNumPacketRecvd);

private:

	CCommonElementsBucket* m_pCommonElementsBucket;
	int m_iCurrentBitRate;
	int m_inoLOssSlot;
	int m_iComplexity;

	OpusEncoder	*encoder;
	OpusDecoder	*decoder;
	opus_int32	length;
	int 		err;

	opus_int16 in[AUDIO_FRAME_SIZE * AUDIO_CHANNELS];
	opus_int16 out[AUDIO_MAX_FRAME_SIZE * AUDIO_CHANNELS];
	opus_int16 *dummyData;
	unsigned char cbits[AUDIO_MAX_PACKET_SIZE];

	int nbBytes;

	FILE *m_fin;

	Tools m_Tools;

protected:


	SmartPointer<CLockHandler> m_pMediaSocketMutex;

};

#endif