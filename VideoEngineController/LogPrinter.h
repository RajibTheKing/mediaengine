#ifndef _LOG_PRINTER_H_
#define _LOG_PRINTER_H_

#define _CRT_SECURE_NO_WARNINGS

//#define __PRINT_LOG__
//#define __EXACT_LOG__
//#define __SPECIFIC_LOG__
//#define __SPECIFIC_LOG4__
//#define __SPECIFIC_LOG5__
//#define __SPECIFIC_LOG3__
//#define __INSTENT_TEST_LOG__
//#define __OPERATION_TIME_LOG__
//#define __QUEUE_TIME_LOG__
//#define __PACKET_LOSS_INFO_LOG__
//#define __THREAD_LOG__
//#define __BITRATE_CHNANGE_LOG__

#define ON 1
#define OFF 0

#define LOG_ENABLED

#define INSTENT_TEST_LOG		OFF
#define OPERATION_TIME_LOG		OFF
#define QUEUE_TIME_LOG			OFF
#define PACKET_LOSS_INFO_LOG	OFF
#define THREAD_LOG				OFF
#define BITRATE_CHNANGE_LOG		OFF
#define DEPACKETIZATION_LOG		OFF


#define FILE_NAME "VideoEngineTrack.log"
#define PRIORITY CLogPrinter::DEBUGS

#ifdef TARGET_OS_WINDOWS_PHONE
typedef __int64 IPVLongType;
#else
typedef long long IPVLongType;
#endif

#include <stdio.h>

#include <string>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <stdarg.h>

#else
#include <sys/time.h>
#endif 

#if defined(_DESKTOP_C_SHARP_) || defined(TARGET_OS_WINDOWS_PHONE)
#define printg(X,...) _RPT1(0,X,__VA_ARGS__)
#define printf(...) printg(__VA_ARGS__,"")
#define printf(...)
#endif

#define printf(...)

using namespace std;

class CLogPrinter
{

public:

	enum Priority
	{
		NONE,
		DEBUGS,
		CONFIG,
		INFO,
		WARNING,
		ERRORS
	};

	CLogPrinter();

	static void Start(Priority maxPriority, const char* logFile);
	static void Stop();
	static void SetPriority(Priority maxPriority);
	static void SetExactness(bool exact);
	static std::string GetDateTime();
	static void Write(Priority priority, const std::string message);
	static void SetLoggerPath(std::string location);
    static void WriteSpecific(Priority priority, const std::string message);
    static void WriteSpecific2(Priority priority, const std::string message);
    static bool SetLoggingState(bool loggingState, int logLevel);
	static long long WriteForOperationTime(Priority priority, const std::string message, long long prevTime = 0);
	static void WriteForQueueTime(Priority priority, const std::string message);
	static void WriteForPacketLossInfo(Priority priority, const std::string message);

	static long long WriteLog(Priority priority, int isLogEnabled, const std::string message = "", bool calculatedTime = false, long long prevTime = 0);

	static long long GetTimeDifference(long long prevTime);

private:

	ofstream    fileStream;
	Priority    maxPriority;
	std::string		logFile;

	static const std::string PRIORITY_NAMES[];
	static CLogPrinter instance;
    static bool isLogEnable;

};



#ifdef LOG_ENABLED
#define CLogPrinter_WriteLog(...) CLogPrinter::WriteLog(__VA_ARGS__)
#else
#define CLogPrinter_WriteLog(...) 0
#endif




#ifdef __PRINT_LOG__
#define CLogPrinter_Write(...) CLogPrinter::Write(__VA_ARGS__)
#else
#define CLogPrinter_Write(...)
#endif

#ifdef __SPECIFIC_LOG__
#define CLogPrinter_WriteSpecific(...) CLogPrinter::WriteSpecific(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific(...)
#endif

#ifdef __SPECIFIC_LOG2__
#define CLogPrinter_WriteSpecific2(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific2(...)
#endif

#ifdef __SPECIFIC_LOG3__
#define CLogPrinter_WriteSpecific3(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific3(...)
#endif

#ifdef __INSTENT_TEST_LOG__
#define CLogPrinter_WriteInstentTestLog(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteInstentTestLog(...)
#endif

#ifdef __SPECIFIC_LOG4__
#define CLogPrinter_WriteSpecific4(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific4(...)
#endif

#ifdef __SPECIFIC_LOG5__
#define CLogPrinter_WriteSpecific5(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific5(...)
#endif

#ifdef __THREAD_LOG__
#define CLogPrinter_WriteThreadLog(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteThreadLog(...)
#endif

#ifdef __OPERATION_TIME_LOG__
#define CLogPrinter_WriteForOperationTime(...) CLogPrinter::WriteForOperationTime(__VA_ARGS__)
#else
#define CLogPrinter_WriteForOperationTime(...) 0
#endif

#ifdef __QUEUE_TIME_LOG__
#define CLogPrinter_WriteForQueueTime(...) CLogPrinter::WriteForQueueTime(__VA_ARGS__)
#else
#define CLogPrinter_WriteForQueueTime(...)
#endif

#ifdef __PACKET_LOSS_INFO_LOG__
#define CLogPrinter_WriteForPacketLossInfo(...) CLogPrinter::WriteForPacketLossInfo(__VA_ARGS__)
#else
#define CLogPrinter_WriteForPacketLossInfo(...)
#endif



#ifdef __BITRATE_CHNANGE_LOG__
#define CLogPrinter_WriteBitrateChangeInfo(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteBitrateChangeInfo(...)
#endif

#endif
