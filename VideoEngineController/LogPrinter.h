#ifndef _LOG_PRINTER_H_
#define _LOG_PRINTER_H_

#define _CRT_SECURE_NO_WARNINGS

//#define __PRINT_LOG__
//#define __EXACT_LOG__
#define __SPECIFIC_LOG__
#define __SPECIFIC_LOG2__
#define __OPERATION_TIME_LOG__
#define __QUEUE_TIME_LOG__
#define __PACKET_LOSS_INFO_LOG__
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
#endif

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
	static long long GetTimeDifference(long long prevTime);

private:

	ofstream    fileStream;
	Priority    maxPriority;
	std::string		logFile;

	static const std::string PRIORITY_NAMES[];
	static CLogPrinter instance;
    static bool isLogEnable;

};

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


#endif
