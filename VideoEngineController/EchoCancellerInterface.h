#ifndef ECHO_CANCELLER_INTERFACE_H
#define ECHO_CANCELLER_INTERFACE_H


#define AECM_SAMPLES_IN_FRAME 80


class EchoCancellerInterface
{

public:

	virtual int AddFarEndData(short *farEndData, int dataLen, bool isLiveStreamRunning);

	virtual int CancelEcho(short *nearEndData, int dataLen, bool isLiveStreamRunning);

};


#endif // !ECHO_CANCELLER_INTERFACE_H
