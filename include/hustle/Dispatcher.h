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

		static Dispatcher& GetInstance() {
			static Dispatcher instance;
			return instance;
		}

		/**
		 * @brief Initialize the job system. Only to be called once.
		 * @param iFiberPoolSize - The number of fibers to allocate in fiber pool
		 * @param iJobPoolSize - The number of items to allocate in the job pool
		 * @param iWorkerThreadCount - The number of worker threads to allocate. -1 for one thread per logical core.
		 * @return
		*/
		bool Init(int iFiberPoolSize, int iJobPoolSize, int iWorkerThreadCount = -1);

		/**
		 * @brief Stop the job system. All pools will be destroyed.
		*/
		void Shutdown();

		/**
		 * @brief The number of worker threads (one per logical core) that are running.
		 * @return Thread count
		*/
		int WorkerThreadCount() { return m_iWorkerThreadCount; }
		
		/**
		 * @brief Put a new job onto the global queue
		 * @param entryPoint - Function to invoke for the job
		 * @param pUserData - A pointer to data that will be passed into the entry point function
		 * @return - Handle to the queued job
		*/
		JobHandle AddJob(JobEntryPoint entryPoint, void* pUserData);

		/**
		 * @brief - Poll for the job to reach completion. Will not return until the specified job is complete.
		 * @param hJob - Handle for the job to poll for completion
		*/
		void WaitForJob(JobHandle hJob);

		/**
		 * @brief If in a job, give execution control back to the scheduler. If called outside a job, invoke the system Yield().
		*/
		void YieldToScheduler();

		/**
		 * @brief Query the current number of items in the job queue
		 * @return The current number of items in the job queue 
		*/
		size_t GetJobQueueDepth() { return m_Jobs.Size(); }

		/**
		 * @brief Query the current number of free jobs in the job pool
		 * @return Free job count
		*/
		size_t GetFreeJobCount() { return m_JobPool.GetFreeCount(); }

		/**
		 * @brief Query the total number of jobs in the job pool
		 * @return Total job pool item count
		*/
		size_t GetFreeJobTotal() { return m_JobPool.GetTotalCount(); }

#ifdef _DEBUG
		int GetFreeJobHighWaterMark() { return m_JobPool.GetHighWaterMark(); }
		int GetFreeFiberHighWaterMark() { return m_FiberPool.GetHighWaterMark(); }
#endif

		size_t GetFiberPoolTotal() { return m_FiberPool.GetTotalCount(); }
		size_t GetFiberPoolFree() { return m_FiberPool.GetFreeCount(); }

		std::string GetLastError() { return m_LastError; }

	private:
		Dispatcher();
		static DWORD WINAPI Scheduler(LPVOID pData);
		
		int m_iWorkerThreadCount;
		WorkerThread* m_pWorkerThreads;
		std::atomic<uint32_t> m_RunningThreads;

		// Pool of available fibers
		ResourcePool<Fiber>	m_FiberPool;

		// Queue of jobs to run
		LockedQueue<Job*> m_Jobs;

		// The pool of Job objects to use for scheduling
		ResourcePool<Job> m_JobPool;

		// Holds any errors that occur by the scheduler or worker threads.
		std::string m_LastError;

		// Allow the WorkerThread class access to Scheduler()
		friend class WorkerThread;
	};
}