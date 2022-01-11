#include "Fiber.h"
#include "Job.h"
#include "Dispatcher.h"
#include "WorkerThread.h"

#include <iostream>
#include <thread>
#include <vector>

namespace Hustle {

	WorkerThread::WorkerThread() :
		m_dwThreadID(0),
		m_hThread(nullptr),
		m_iCoreAffinity(-1),
		m_eState(State::None) {

	}

	WorkerThread::~WorkerThread() {
		if (m_hThread) {
			CloseHandle(m_hThread);
			m_hThread = nullptr;
		}
	}

	bool WorkerThread::Start(int iCoreAffinity) {

		// Worker thread needs to be in None or Done state in order to be started
		auto currentState = GetState();
		assert(currentState == WorkerThread::State::None || currentState == WorkerThread::State::Done);

		// Change state to starting
		m_eState.store(WorkerThread::State::Starting);

		// Create the thread here
		m_hThread = CreateThread(NULL,                   // default security attributes
								 0,                      // use default stack size  
								 Dispatcher::Scheduler,  // Thread entry point
								 this,                   // argument to thread function 
								 0,                      // use default creation flags 
								 &m_dwThreadID);         // returns the thread identifier 
		
		if (m_hThread == nullptr) {
			m_eState.store(WorkerThread::State::None);
			m_LastError = GetLastErrorAsStr(::GetLastError());
			return false;
		}

		// Set thread affinity
		if (iCoreAffinity != -1) {
			m_iCoreAffinity = iCoreAffinity;

			if (SetThreadAffinityMask(m_hThread, 1i64 << iCoreAffinity) == 0) {

				// If setting the thread affinity failed, the system is in an invalid state. 
				Stop();

				// Stop() should set the state to DONE, but we'll flip it back to None since technically we're failing here
				m_eState.store(WorkerThread::State::None);

				m_LastError = GetLastErrorAsStr(::GetLastError());
				return false;
			}
		}

		return true;
	}

	void WorkerThread::Stop() {
		// Set the state to stopping. Once the ThreadEntryPoint completes, the state will progress to Done
		m_eState.exchange(WorkerThread::State::Stopping);

		// Wait for the thread to complete
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}

	std::string WorkerThread::GetLastErrorAsStr(DWORD dwError) {

		assert(dwError != 0);

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, 
									 dwError,
									 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
									 (LPSTR)&messageBuffer, 0, NULL);

		//Copy the error message into a std::string.
		std::string message(messageBuffer, size);

		//Free the Win32's string's buffer.
		LocalFree(messageBuffer);

		return message;
	}
}