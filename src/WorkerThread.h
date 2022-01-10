#pragma once

#include <atomic>
#include <windows.h>

namespace JobSystem {

	class WorkerThread {
	public:

		WorkerThread();
		~WorkerThread();

		// States that this thread can be in. Modified & queried with accessor methods below.
		enum class State {
			None,		// Occurs between class constructor and invocation of Start()
			Starting,	// Until the main loop is reached, this is the current state
			Running,	// In the main loop
			Stopping,	// Occurs when Stop() is called, and we're waiting for Done to be reached.
			Done		// ThreadEntryPoint is complete. Thread is no longer running
		};

		State GetState() { return m_eState.load(); }
		void SetState(State eState) { m_eState.store(eState); }

		bool Start(int iCoreAffinity = -1);
		void Stop();

		std::string GetLastError() { return m_LastError; }
	private:

		std::string GetLastErrorAsStr(DWORD dwError);

		// What core to run on. -1 means we don't care.
		int m_iCoreAffinity;

		// State of the thread
		std::atomic<State>	m_eState;

		// Thread handle & ID
		HANDLE m_hThread;
		DWORD  m_dwThreadID;

		std::string m_LastError;
	};
}