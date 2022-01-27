#include "hustle/Dispatcher.h"
#include "hustle/Fiber.h"

#include <assert.h>
#include <iostream>
#include <thread>
#include <Windows.h>

namespace Hustle {
	
	bool Dispatcher::Init(int iFiberPoolSize, int iJobPoolSize, int iWorkerThreadCount) {

		// Set a growth factor on the fiber pool
		m_FiberPool.Grow(iFiberPoolSize);
		m_FiberPool.SetGrowthFactor(10);

		m_JobPool.Grow(iJobPoolSize);
		m_JobPool.SetGrowthFactor(10);

		bool bReturn = true;		

		// Create a thread for each core, except on core 0
		if (iWorkerThreadCount == -1)
			m_iWorkerThreadCount = std::thread::hardware_concurrency() - 1;
		else
			m_iWorkerThreadCount = iWorkerThreadCount;

		m_pWorkerThreads = new WorkerThread[m_iWorkerThreadCount];

		// Start up each thread, setting CPU affinity for each one.
		for (int i = 0; i < m_iWorkerThreadCount; i++) {

			// Creating this temp variable to avoid C6385
			WorkerThread* pThread = &m_pWorkerThreads[i];

			// m_iWorkerThreadCount is always the number of CPUs, less one. 
			// We don't want any worker threads running on CPU 0, so always add 1 to i. 
			if (pThread->Start(i + 1) == false) {
				
				m_LastError = pThread->GetLastError();
				bReturn = false;
			}
		}

		// If all of the threads started correctly, wait for them to reach their running state
		if (bReturn == true) {

			bool bAllThreadsReady = false;
			while (bAllThreadsReady != true) {

				bAllThreadsReady = true;

				// If any of the threads aren't in the ready state, set the flag to false
				for (auto i = 0; i < m_iWorkerThreadCount; i++) {
					// Temp variable to void C6385
					WorkerThread* pWorkerThread = &m_pWorkerThreads[i];

					if (pWorkerThread->GetState() != WorkerThread::State::Running) {
						bAllThreadsReady = false;
						Yield();
						break;
					}
				}
			}
		}

		return bReturn;
	}
	
	void Dispatcher::Shutdown() {

		// Stop all of the threads
		for (int i = 0; i < m_iWorkerThreadCount; i++)
			m_pWorkerThreads[i].Stop();

		if (m_pWorkerThreads)
			delete[] m_pWorkerThreads;
	}
	
	JobHandle Dispatcher::AddJob(JobEntryPoint entryPoint, void* pUserData) {

		Job* pJob;

		// Poll until we get something from the pool		
		pJob = m_JobPool.Get();

		// There are no available jobs! 
		assert(pJob != nullptr);
		

		// When the job finishes, it will be unlocked. Until then, calls to WaitForJob() will wait.
		pJob->Lock();

		// TODO: handle this better
		assert(pJob != nullptr);

		pJob->SetEntryPoint(entryPoint);
		pJob->SetUserData(pUserData);

		m_Jobs.Push(pJob);
		return pJob;
	}
	
	void Dispatcher::WaitForJob(JobHandle hJob) {

		// Poll on the job until it is done
		while (hJob->TryLock() != true) {

			// In case this is being called from outside of the fiber system, just yield back to the os
			auto currentFiber = Fiber::GetCurrentFiber();
			if (currentFiber) {
				currentFiber->GetParent()->SwitchTo();
			} else {
				Yield();
			}			
		}

		// We got the lock, go ahead and unlock it...
		hJob->Unlock();
	}
	
	void Dispatcher::YieldToScheduler() {

		// In case this is being called from outside of the fiber system, just yield back to the os
		auto currentFiber = Fiber::GetCurrentFiber();
		if (currentFiber) {
			currentFiber->GetParent()->SwitchTo();
		}
		else {
			Yield();
		}
	}

	/**
	 * @brief Entrypoint for each worker thread. 
	 * @param pData Pointer to a WorkerThread object
	 * @return - Should always be 0
	*/
	DWORD WINAPI Dispatcher::Scheduler(LPVOID pData) {

		// Convert the thread to a fiber
		void* pFiber = ConvertThreadToFiber(nullptr);
		Fiber* pJobFiber = nullptr;

		// Snag a pointer to the worker thread that started this scheduler
		WorkerThread* pWorkerThread = (WorkerThread*)pData;

		Job* pJob;

		// Create a dummy Fiber object for ourselves to pass into child fibers
		Fiber thisFiber(pFiber);
		auto &dispatcher = Dispatcher::GetInstance();

		// Vector for fibers that have yieled back and are still in the running state
		// NOTE: This does not need to be read/write protected since it will only be used by this thread/fiber
		std::vector<Fiber*>	pendingFibers;

		// TODO: Add barrier to not start until all threads are spun up

		// Set the state to running
		pWorkerThread->SetState(WorkerThread::State::Running);

		while (pWorkerThread->GetState() == WorkerThread::State::Running) {
			
			bool bDidWork = false;

			// Go through the wait queue and process those jobs			
			if (pendingFibers.size()) {
				auto fiberIt = pendingFibers.begin();
				while (fiberIt != pendingFibers.end()) {

					bDidWork = true;

					Fiber* pPendingFiber = *fiberIt;
					pPendingFiber->SwitchTo();

					switch (pPendingFiber->GetState()) {
					case Fiber::State::Running:
						// Don't do anything...we'll get to it later
						fiberIt++;
						break;
					case Fiber::State::Idle:
						// Remove the fiber from the pending list
						fiberIt = pendingFibers.erase(fiberIt);

						// Free the job's spinlock, in case there is a pending job on the wait queue
						pPendingFiber->CurrentJob()->Unlock();

						// Put the fiber and job back into their respective free queues
						dispatcher.m_FiberPool.Release(pPendingFiber);
						dispatcher.m_JobPool.Release(pPendingFiber->CurrentJob());
					}
				}
			}
			
			if (pJob = dispatcher.m_Jobs.Pop()) {

				bDidWork = true;
				// Grab a new fiber
				// TODO: Handle the case where there are no more fibers and every worker is waiting for one...
				pJobFiber = nullptr;
				do {

					pJobFiber = dispatcher.m_FiberPool.Get();
				} while (pJobFiber == nullptr);

				// Start running the fiber
				pJobFiber->Activate(pJob, &thisFiber);

				switch (pJobFiber->GetState()) {

				case Fiber::State::Running:
					// Toss the job onto the pending queue
					pendingFibers.push_back(pJobFiber);
					break;
				case Fiber::State::Idle:

					// Free the job's spinlock, in case there is a pending job on the wait queue
					pJobFiber->CurrentJob()->Unlock();

					// Put the fiber and job back into their respective free queues
					dispatcher.m_FiberPool.Release(pJobFiber);
					dispatcher.m_JobPool.Release(pJob);
					break;
				}
			}

			// We didn't do anything, take a breather
			if (bDidWork == false)
				_mm_pause();
		}

		// Set the current state to done so callers know we're...done. 
		pWorkerThread->SetState(WorkerThread::State::Done);

		return 0;
	}

	/**
	 * @brief Default constructor, hidden behind the singleton pattern.
	*/
	Dispatcher::Dispatcher() :
		m_RunningThreads(0),
		m_iWorkerThreadCount(0),
		m_pWorkerThreads(nullptr) {

	}
}