
#ifndef IPV_LOCK_HANDLER_H
#define IPV_LOCK_HANDLER_H

#include <stdio.h>
#include <mutex>

#ifdef __APPLE__
#include <pthread.h>
#endif

namespace MediaSDK
{
	class Locker;

	class CLockHandler
	{

	public:

		CLockHandler();
		~CLockHandler();

		std::mutex* GetMutex();
		void Lock();
		void UnLock();

	private:

		std::mutex *m_pMutex;
	};

	class Locker
	{

	public:

		Locker(CLockHandler& m) :

			mutex(m)

		{
			mutex.Lock();
		}

		~Locker()
		{
			mutex.UnLock();
		}

	private:

		CLockHandler& mutex;
	};

} //namespace MediaSDK

#endif

