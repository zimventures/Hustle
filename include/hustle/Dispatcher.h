#pragma once

#include "Fiber.h"
#include "Job.h"
#include "LockedQueue.h"
#include "ResourcePool.h"
#include "SpinLock.h"
#include "WorkerThread.h"

#include <iostream>
#include <queue>
#include <map>

namespace Hustle {

	class Dispatcher {
	public:
		const static int FiberPoolSize = 100;
		const static int JobPoolSize = 10000;

		static Dispatcher& GetInstance() {
			static Dispatcher instance;
			return instance;
		}

		bool Init(int iWorkerThreadCount = -1);
		void Shutdown();

		int WorkerThreadCount() { return m_iWorkerThreadCount; }

		JobHandle AddJob(JobEntryPoint entryPoint, void* pUserData);
		void WaitForJob(JobHandle hJob);

		size_t GetJobQueueDepth() { return m_Jobs.Size(); }
		size_t GetFreeJobCount() { return m_JobPool.GetFreeCount(); }
		size_t GetFreeJobTotal() { return m_JobPool.GetTotalCount(); }

#ifdef _DEBUG
		int GetFreeJobHighWaterMark() { return m_JobPool.GetHighWaterMark(); }
		int GetFreeFiberHighWaterMark() { return m_FiberPool.GetHighWaterMark(); }
#endif

		size_t GetFiberPoolTotal() { return m_FiberPool.GetTotalCount(); }
		size_t GetFiberPoolFree() { return m_FiberPool.GetFreeCount(); }
		Fiber* GetCurrentFiber();

		std::string GetLastError() { return m_LastError; }

	private:
		Dispatcher();

		// Main loop for the threads (which will be converted to a fibers)
		static DWORD WINAPI Scheduler(LPVOID pData);

		ResourcePool<Fiber, FiberPoolSize>	m_FiberPool;

		// A map of the fibers, keyed on the pointer returned by CreateFiber()
		// This allows us to call the Win32 GetCurrentFiber() function and grab the Fiber object. 
		std::map<void*, Fiber*>	m_FiberMap;

		int m_iWorkerThreadCount;
		WorkerThread* m_pWorkerThreads;
		std::atomic<uint32_t> m_RunningThreads = { 0 };

		// Queue of jobs to run
		LockedQueue<Job*> m_Jobs;

		// The pool of Job objects to use for scheduling
		ResourcePool<Job, JobPoolSize> m_JobPool;

		// Holds any errors that occur by the scheduler or worker threads.
		std::string m_LastError;

		// Allow the WorkerThread class access to Scheduler()
		friend class WorkerThread;
	};
}