
#ifndef IPV_VideoFrameBuffer_H
#define IPV_VideoFrameBuffer_H

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Size.h"

namespace MediaSDK
{

class VideoFrameBuffer
{

public:

	VideoFrameBuffer();
	~VideoFrameBuffer();

	int Queue(unsigned char *ucaDecodedVideoFrameData, int nLength);
	int DeQueue(unsigned char *ucaDecodedVideoFrameData);
	void IncreamentIndex(int &riIndex);
	int GetQueueSize();
	void ResetBuffer();

private:

	int m_iPushIndex;
	int m_iPopIndex;
	int m_nQueueCapacity;
	int m_nQueueSize;

	unsigned char m_uc2aDecodedVideoDataBuffer[MAX_VIDEO_RENDERER_BUFFER_SIZE][MAX_VIDEO_RENDERER_FRAME_SIZE];

	int m_naBufferDataLengths[MAX_VIDEO_RENDERER_BUFFER_SIZE];

	SharedPointer<CLockHandler> m_pVideoFrameBufferMutex;
};

} //namespace MediaSDK

#endif 
