#include "MediaLogger.h"

#if defined(TARGET_OS_WINDOWS_PHONE) || defined (DESKTOP_C_SHARP) 

#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "Tools.h"
#include "LogPrinter.h"

namespace MediaSDK
{
	MediaLogger::MediaLogger(LogLevel logLevel) :
		m_elogLevel(logLevel),
		m_bFSError(false)
	{		
		m_sFilePath = MEDIA_FULL_LOGGING_PATH;
	}

	MediaLogger::~MediaLogger()
	{
		Release();
	}

	void MediaLogger::Init()
	{
		m_pMediaLoggerMutex.reset(new CLockHandler());

		m_bFSError = !CreateLogDirectory(); //if TRUE then NO ERROR, else we have FS error

		if (!m_bFSError) //no error then
		{
			CLogPrinter::Log("MANSUR----------directory creation successful >> %s\n", m_sFilePath.c_str());

			if (!m_pLoggerFileStream.is_open())
			{
				m_pLoggerFileStream.open(m_sFilePath.c_str(), ofstream::out | ofstream::app);
			}
		}
		else
		{
			CLogPrinter::Log("MANSUR----------directory creation failed >> %s\n", m_sFilePath.c_str());
		}

		
		StartMediaLoggingThread();
	}
	
	void MediaLogger::Release()
	{
		StopMediaLoggingThread();

		if (m_pLoggerFileStream.is_open())
		{
			m_pLoggerFileStream.close();
		}
	}

	std::string MediaLogger::GetFilePath()
	{
		return m_sFilePath;
	}

	LogLevel MediaLogger::GetLogLevel()
	{
		return m_elogLevel;
	}

	void MediaLogger::Log(LogLevel logLevel, const char *format, ...)
	{
		MediaLocker lock(m_pMediaLoggerMutex.get());

		if (logLevel > m_elogLevel) return;

		int len = GetDateTime(m_sMessage);
		len += GetThreadID(m_sMessage + len);
		m_sMessage[len++] = ' ';

		va_list vargs;
		//argument to string start
		va_start(vargs, format);
		vsnprintf(m_sMessage + len, MEDIA_LOG_MAX_SIZE - len, format, vargs);
		va_end(vargs);
		//argument to string end
		
		m_vLogVector.push_back(m_sMessage);
		CLogPrinter::Log("MANSUR----------pushing >> %s\n", m_sMessage);
	}

	bool MediaLogger::CreateLogDirectory()
	{
#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)

		if (0 == CreateDirectory(MEDIA_LOGGING_PATH, NULL) )
		{
			int retCode = GetLastError();
			if (ERROR_ALREADY_EXISTS != retCode)
			{
				printf("Log folder creation FAILED, ret code %d\n", retCode);
				return false;
			}
		}

#else
		struct stat st = { 0 };

		if (stat(MEDIA_LOGGING_PATH, &st) == -1)
		{
			if(0 != mkdir(MEDIA_LOGGING_PATH, 0700))
			{
#if __ANDROID__
				__android_log_print(ANDROID_LOG_ERROR, MEDIA_LOGCAT_TAG, "Log folder creation FAILED, returned %d\n", errno);

#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

				printf("Log folder creation FAILED, returned %d\n", errno );
#endif

				return false;
			}
		}
#endif

		return true;
	}

	void MediaLogger::WriteLogToFile()
	{
		MediaLocker lock(m_pMediaLoggerMutex.get());

		size_t vSize = min(m_vLogVector.size(), (size_t)MAX_LOG_WRITE);

		for(int  i=0; i < vSize; i++)
		{
			CLogPrinter::Log("MANSUR----------writing to file stream >> %s\n", m_vLogVector[i].c_str());

			if (!m_bFSError)
			{
				m_pLoggerFileStream << m_vLogVector[i] << std::endl;
			}
			else
			{
#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

				std::cout << m_vLogVector[i] << std::endl;

#elif __ANDROID__

				__android_log_write(ANDROID_LOG_ERROR, MEDIA_LOGCAT_TAG, m_vLogVector[i].c_str());

#endif
			}
		}

		m_pLoggerFileStream.flush();

		m_vLogVector.erase(m_vLogVector.begin(), m_vLogVector.begin() + vSize);
	}

	size_t MediaLogger::GetThreadID(char* buffer)
	{
		//For All Platforms
		stringstream ss;
		ss << std::this_thread::get_id();		
		ss >> buffer;
		return strlen(buffer);
	}

	size_t MediaLogger::GetDateTime(char* buffer)
	{
		stringstream ss;
		ss<<std::time(nullptr);

#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)

		SYSTEMTIME st;

		GetLocalTime(&st);

		return _snprintf(buffer, 40, "[%02d-%02d-%04d %02d:%02d:%02d: %03s] ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, ss.str().c_str());
#else

		timeval curTime;
		gettimeofday(&curTime, NULL);
		int milli = curTime.tv_usec / 1000, pos;

		pos = strftime(buffer, 22, "[%d-%m-%Y %H:%M:%S", localtime(&curTime.tv_sec));
		CLogPrinter::Log("MANSUR---------position value = %d\n", pos);
		return snprintf(buffer+pos, 20, " %s] ", ss.str().c_str());
#endif 
	}

	void MediaLogger::StartMediaLoggingThread()
	{
		m_threadInstance = std::thread(CreateLoggingThread, this);
	}
	
	void MediaLogger::StopMediaLoggingThread()
	{
		CLogPrinter::Log("MANSUR----------stopping logger thread\n");
		m_bMediaLoggingThreadRunning = false;
		
		m_threadInstance.join();
	}

	void *MediaLogger::CreateLoggingThread(void* param)
	{
		MediaLogger *pThis = (MediaLogger*)param;

		pThis->m_bMediaLoggingThreadRunning = true;

		CLogPrinter::Log("MANSUR----------starting logger thread\n");

		while (pThis->m_bMediaLoggingThreadRunning)
		{
			if (pThis->m_vLogVector.size() >= MIN_BUFFERED_LOG)
			{
				pThis->WriteLogToFile();
			}
			else
			{ 
				CLogPrinter::Log("MANSUR----------sleeping logger thread , vector size = %d\n", pThis->m_vLogVector.size());
				Tools::SOSleep(THREAD_SLEEP_TIME);
			}
		}

		return NULL;
	}

}
