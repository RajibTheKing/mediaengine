
#include "MediaLogger.h"
#include "AudioMacros.h"
#include <vector>

namespace MediaSDK
{
	// Byte size of different value
	#define ByteSizeDelay 2
	#define ByteSizeDelayFraction 1
	#define ByteSizeFarendSize 1
	#define ByteSizeAverageTimeDiff 1
	#define ByteSizeIsCalling 1
	#define ByteSizeCountCall 1
	#define ByteSizeTotalDataSize 4

	// Types of different value
	#define DEVICE_INFORMATION_DELAY_PUBLISHER 1
	#define DEVICE_INFORMATION_DELAY_FRACTION_PUBLISHER 3
	#define DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE_PUBLISHER 5
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX_PUBLISHER 7
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN_PUBLISHER 9
	#define DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF_PUBLISHER 11
	#define DEVICE_INFORMATION_IS_CALLING_PUBLISHER 13
	#define DEVICE_INFORMATION_COUNT_CALL_PUBLISHER 15
	#define DEVICE_INFORMATION_TOTAL_DATA_SZ 17

	#define DEVICE_INFORMATION_DELAY_CALLEE 2
	#define DEVICE_INFORMATION_DELAY_FRACTION_CALLEE 4
	#define DEVICE_INFORMATION_STARTUP_FAREND_BUFFER_SIZE_CALLEE 6
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MAX_CALLEE 8
	#define DEVICE_INFORMATION_CURRENT_FAREND_BUFFER_SIZE_MIN_CALLEE 10
	#define DEVICE_INFORMATION_AVERAGE_RECORDER_TIME_DIFF_CALLEE 12
	#define DEVICE_INFORMATION_TOTAL_DATA_SZ_CALLEE 14

	class AudioDeviceInformation
	{

	public:
		AudioDeviceInformation();
		~AudioDeviceInformation();
		void Reset();
		void SetInformation(int nInfoSize, int nInfoType, unsigned long long ullInfo);
		int GetInformation(unsigned char* ucaInfo);
		std::vector < std::pair < int, long long > > ParseInformation(unsigned char *ucaInfo, int len);
		

	private:
		unsigned char m_ucaBuffer[400];
		int m_nBufferSize;
		SharedPointer<CLockHandler> m_pAudioDeviceInformationLocker;
	};

};