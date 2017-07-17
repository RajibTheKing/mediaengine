
#ifndef _MEDIA_LOGGER_H_
#define _MEDIA_LOGGER_H_

#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>
#include <thread>

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "CommonMacros.h" //Need for LOG_ENABLED

#define MEDIA_LOGGING_FOLDER_NAME "MediaLogs"
#define MEDIA_LOGGER_TAG "MediaSDK"

#if defined(__ANDROID__)
#define MEDIA_LOGGING_PATH "/sdcard/" MEDIA_LOGGING_FOLDER_NAME "/"
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#define MEDIA_LOGGING_PATH std::string(getenv("HOME")) + "/Documents/" MEDIA_LOGGING_FOLDER_NAME "/"
#elif defined(DESKTOP_C_SHARP)
#define MEDIA_LOGGING_PATH "C:\\" MEDIA_LOGGING_FOLDER_NAME "\\"
#endif

#define MIN_BUFFERED_LOG 5
#define MAX_LOG_WRITE 100
#define THREAD_SLEEP_TIME 250
#define MEDIA_LOG_MAX_SIZE	512

#define MEDIA_LOGGING_FILE_NAME "MediaLog.log"
#define MEDIA_FULL_LOGGING_PATH MEDIA_LOGGING_PATH MEDIA_LOGGING_FILE_NAME

namespace MediaSDK
{

	enum LogLevel
	{
		ERRORLOG, //Changed to avoid conflict with wingdi
		WARNING,
		INFO,
		DEBUGLOG,
		CONFIG,
		CODE_TRACE,
		PACKET_DUMP
	};

	#define LOG_ERROR LogLevel::ERRORLOG
	#define LOG_WARNING LogLevel::WARNING
	#define LOG_INFO LogLevel::INFO
	#define LOG_DEBUG LogLevel::DEBUGLOG
	#define LOG_CONFIG LogLevel::CONFIG
	#define LOG_CODE_TRACE LogLevel::CODE_TRACE
	#define LOG_PACKET_DUMP LogLevel::PACKET_DUMP

	class MediaLogger
	{

	public:
		MediaLogger();
		~MediaLogger();

		void Init(LogLevel logLevel);
		void Release();
		
		void Log(LogLevel loglevel, const char *format, ...);
		
		std::string GetFilePath();
		LogLevel GetLogLevel();

	private:
		
		bool CreateLogDirectory();
		void WriteLogToFile();

		void InternalLog(const char *format, ...);
		
		size_t GetThreadID(char* buffer);
		size_t GetDateTime(char* buffer);

		void StartMediaLoggingThread();
		void StopMediaLoggingThread();
		static void* CreateLoggingThread(void* param);
		
	private:
		
		std::unique_ptr<CLockHandler> m_pMediaLoggerMutex;
		bool m_bMediaLoggingThreadRunning;
		std::thread m_threadInstance;

		std::vector<std::string> m_vLogVector;
		std::ofstream   m_pLoggerFileStream;

		LogLevel m_elogLevel;
		std::string m_sFilePath;

		char m_sMessage[MEDIA_LOG_MAX_SIZE];

		/// File System Error
		bool m_bFSError; 
	};

#ifdef LOG_ENABLED

	static MediaLogger g_Logger;

	#define MediaLogInit(level) g_Logger.Init(level);
	#define MediaLogRelease() g_Logger.Release();
	#define MediaLog(a, ...) g_Logger.Log( (a), __VA_ARGS__);

	class ScopeLog
	{
	public:
		ScopeLog(char* scopeName):
			m_sName(scopeName)
		{
			MediaLog(LOG_INFO, ">>> %s Entered >>>", scopeName);
		}

		~ScopeLog()
		{
			MediaLog(LOG_INFO, "<<< %s Exited <<<", m_sName.c_str());
		}
	private:
		std::string m_sName;
	};

#else

/// If log not enabled then these macros will work as a placeholder
#define MediaLogInit(level);
#define MediaLogRelease();
#define MediaLog(a, ...);

#define ScopeLog(...)

#endif
}

#endif
