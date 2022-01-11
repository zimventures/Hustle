#pragma once

#include <functional>
#include "SpinLock.h"

namespace Hustle {
	typedef std::function<void(void* pArg)> JobEntryPoint;

	class Job : public SpinLock {
	public:
		Job() = default;
		Job(JobEntryPoint entryPoint, void* pUserData = nullptr) :
			m_JobEntrypoint(entryPoint),
			m_pUserData(pUserData) {

		}

		void SetEntryPoint(JobEntryPoint entryPoint) { m_JobEntrypoint = entryPoint; }
		JobEntryPoint& GetEntryPoint() { return m_JobEntrypoint; }

		void SetUserData(void* pUserData) { m_pUserData = pUserData; }
		void* GetUserData() { return m_pUserData; }

	private:

		void* m_pUserData;		// User data to be passed into the entrypoint function
		JobEntryPoint	m_JobEntrypoint;	// Entrypoint function to be called for the job

	};

	typedef Job* JobHandle;
}