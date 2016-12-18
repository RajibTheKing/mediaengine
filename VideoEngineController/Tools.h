
#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>

#include "Size.h"

#ifdef WIN32
typedef __int64 LongLong;
#else 
typedef long long LongLong;
#endif

#if defined(TARGET_OS_WINDOWS_PHONE) || defined (_DESKTOP_C_SHARP_) || defined (_WIN32)
#include <windows.h>
#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)
#include <unistd.h>
#endif

class Tools;

using namespace std;

class Tools
{

public:

	Tools();
	~Tools();

	void SOSleep(int nSleepTimeout);
	static LongLong CurrentTimestamp();
    static std::string IntegertoStringConvert(int nConvertingNumber);
	static std::string LongLongtoStringConvert(LongLong number);
	static std::string DoubleToString(double dConvertingValue);
	
	static std::string getText(double dConvertingValue);
	static std::string getText(int nConvertingNumber);
	static std::string getText(LongLong number);
    
    static int StringToIntegerConvert(std::string strConvertingString);
	static std::string LongLongToString(LongLong llConvertingValue);

	void WriteToFile(short* saDataToWriteToFile, int nLength);
	void WriteToFile(unsigned char* ucaDataToWriteToFile, int nLength);
	static unsigned long long GetTotalSystemMemory();
	unsigned long long GetAvailableSystemMemory();



	void IntToUnsignedCharConversion(int number, unsigned char convertedArray[], int index);
	int UnsignedCharToIntConversion(unsigned char convertedArray[], int index);



	void SetMediaUnitVersionInMediaChunck(int number, unsigned char convertedArray[]);
	int GetMediaUnitVersionFromMediaChunck(unsigned char convertedArray[]);

	void SetMediaUnitTimestampInMediaChunck(int number, unsigned char data[]);
	int GetMediaUnitTimestampInMediaChunck(unsigned char data[]);

	void SetAudioBlockSizeInMediaChunck(int number, unsigned char convertedArray[]);
	int GetAudioBlockSizeFromMediaChunck(unsigned char convertedArray[]);

	void SetVideoBlockSizeInMediaChunck(int number, unsigned char convertedArray[]);
	int GetVideoBlockSizeFromMediaChunck(unsigned char convertedArray[]);

	void SetNumberOfAudioFramesInMediaChunck(int index, int number, unsigned char convertedArray[]);
	int GetNumberOfAudioFramesFromMediaChunck(int index, unsigned char convertedArray[]);

	void SetNumberOfVideoFramesInMediaChunck(int index, int number, unsigned char convertedArray[]);
	int GetNumberOfVideoFramesFromMediaChunck(int index, unsigned char convertedArray[]);

	void SetNextAudioFramePositionInMediaChunck(int index, int number, unsigned char convertedArray[]);
	int GetNextAudioFramePositionFromMediaChunck(int index, unsigned char convertedArray[]);

	void SetNextVideoFramePositionInMediaChunck(int index, int number, unsigned char convertedArray[]);
	int GetNextVideoFramePositionFromMediaChunck(int index, unsigned char convertedArray[]);




	static int GetIntegerFromUnsignedChar(unsigned char *packetData, int index, int nLenght);
	static void SetIntegerIntoUnsignedChar(unsigned char *packetData, int index, int nLenght, int value);


private:

	FILE *m_filePointerToWriteShortData, *m_filePointerToWriteByteData;
};

#define funcline(...) CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, std::string(__func__) + " : " + Tools::IntegertoStringConvert(__LINE__) + ":" + std::string(__VA_ARGS__));

#endif
