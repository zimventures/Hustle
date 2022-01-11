#pragma once

#include <windows.h>

namespace Hustle {
	class Job;

	class Fiber {
	public:
		Fiber();
		Fiber(const Fiber& fiber);
		Fiber(void* pFiberHandle);
		~Fiber();

		void Activate(Job* pJob, Fiber* pParent);

		enum class State {
			None,		// Created, but never activated
			Running,	// Running a job
			Idle,		// Finished running a job, 
			Waiting,	// Sitting on the wait queue
		};

		State GetState() { return m_eState; }
		Fiber* GetParent() { return m_pParent; }
		Job* CurrentJob() { return m_pJob; }
		void* GetFiberHandle() { return m_hFiber; }

		void SwitchTo() { ::SwitchToFiber(m_hFiber); }

	private:

		// The while(1) loop to process the jobs
		static void __stdcall Run(void* pData);

		State	m_eState;

		// Handle returned by CreateFiber()
		void* m_hFiber;

		// Parent fiber to switch to when the current job is complete
		Fiber* m_pParent;

		// Current job being executed
		Job* m_pJob;
	};
}