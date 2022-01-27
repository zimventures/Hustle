#include "hustle/Fiber.h"
#include "hustle/Job.h"
#include "hustle/Dispatcher.h"

#include <iostream>
#include <Windows.h>

namespace Hustle {

	
	Fiber::Fiber() :
		m_pJob(nullptr),
		m_hFiber(nullptr),
		m_pParent(nullptr),
		m_eState(State::None) {

		// TODO: Allow the stack size to be configured
		m_hFiber = CreateFiber(0, Run, this);

		// Add to the static fiber map
		s_FiberMap[m_hFiber] = this;
	}

	Fiber::Fiber(const Fiber& fiber) :
		m_eState(fiber.m_eState),
		m_pJob(fiber.m_pJob),
		m_pParent(fiber.m_pParent),
		m_hFiber(fiber.m_hFiber) {

		// Remove from the static fiber map
		s_FiberMap.erase(m_hFiber);
		
	}

	Fiber::Fiber(void* pFiberHandle) :
		m_eState(State::None),
		m_pJob(nullptr),
		m_pParent(nullptr),
		m_hFiber(pFiberHandle) {
	}

	Fiber::~Fiber() {
		if (m_hFiber)
			DeleteFiber(m_hFiber);
	}
	
	
	void Fiber::Activate(Job* pJob, Fiber* pParent) {

		// The thread that we'll switch back to when the job is complete
		m_pParent = pParent;

		// The job that we'll run
		m_pJob = pJob;

		// Enable the fiber
		SwitchToFiber(m_hFiber);
	}

	Fiber* Fiber::GetCurrentFiber() {

		// OS handle of the current fiber
		void* pCurrentFiber = ::GetCurrentFiber();

		// Reach into the global map of Fiber objects, searching by the OS handle
		auto fiberIt = s_FiberMap.find(pCurrentFiber);
		if (fiberIt == s_FiberMap.end())
			return nullptr;
		else
			return fiberIt->second;

	}

	void __stdcall Fiber::Run(void* pData) {

		JobEntryPoint fn;

		Fiber* pThis = (Fiber*)pData;

		// Start the processing loop
		while (true) {

			// Set our state to running
			pThis->m_eState = State::Running;

			// Execute the entrypoint
			fn = pThis->m_pJob->GetEntryPoint();
			fn(pThis->m_pJob->GetUserData());

			// The entrypoint has completed. Setting our state to IDLE so that the scheduler
			// knows to put the fiber back onto the available pool
			pThis->m_eState = State::Idle;

			// Switch back to the scheduler
			SwitchToFiber(pThis->m_pParent->m_hFiber);
		}

		// We should never get here
		assert(false);
	}

	std::map<void*, Fiber*>	Fiber::s_FiberMap;
}