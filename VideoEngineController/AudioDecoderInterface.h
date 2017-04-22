#ifndef AUDIO_DECODER_INTERFACE_H
#define AUDIO_DECODER_INTERFACE_H


class AudioDecoderInterface
{

public:

	virtual bool SetParameters(int sampleRate, int numberOfChannels) = 0;

	virtual int DecodeAudio(unsigned char *in_data, unsigned int in_size, short *out_buffer) = 0;

	virtual ~AudioDecoderInterface() { }
};


#endif  // !AUDIO_DECODER_INTERFACE_H 


// AudioDecoderInterface --> AudioAacDecoder/AudioOpusDecoder/AudioPCMDecoder -->  AudioDecoderProvider



