
#ifndef _AUDIO_BUFFER_H_
#define _AUDIO_BUFFER_H_

#include "SmartPointer.h"
#include "LockHandler.h"

#define MAX_AUDIO_ENCODER_BUFFER_SIZE 30
#define MAX_AUDIO_ENCODER_FRAME_SIZE 4096

class CAudioEncoderBuffer
{

public:

	CAudioEncoderBuffer();
	~CAudioEncoderBuffer();

	int Queue(short *frame, int length);
	int DeQueue(short *decodeBuffer);
	void IncreamentIndex(int &index);
	int GetQueueSize();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_iDecodingIndex;
	int m_iQueueCapacity;
	int m_iQueueSize;

	short m_Buffer[MAX_AUDIO_ENCODER_BUFFER_SIZE][MAX_AUDIO_ENCODER_FRAME_SIZE];
	int m_BufferDataLength[MAX_AUDIO_ENCODER_BUFFER_SIZE];
	int m_BufferIndexState[MAX_AUDIO_ENCODER_BUFFER_SIZE];

	SmartPointer<CLockHandler> m_pChannelMutex;
};

#endif 
