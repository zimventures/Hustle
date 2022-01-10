#pragma once
/**********************************************************************
* spinlock implementation modified from https://rigtorp.se/spinlock/
**********************************************************************/

#include <atomic>

namespace JobSystem {

	class SpinLock {
	public:
		
		void Lock() noexcept {
			for (;;) {
				// Optimistically assume the lock is free on the first try
				if (!m_Lock.exchange(true, std::memory_order_acquire)) {
					return;
				}
				// Wait for lock to be released without generating cache misses
				while (m_Lock.load(std::memory_order_relaxed)) {
					// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
					// hyper-threads
					// __builtin_ia32_pause(); on GCC or clang

					// WARNING: It's noted here (https://graphitemaster.github.io/fibers/#avoid-the-pause-instruction) that 
					// this may be a much larger CPU stall than we're hoping for due to recent Intel architecture changes
					_mm_pause();
				}
			}
		}

		bool TryLock() noexcept {
			// First do a relaxed load to check if lock is free in order to prevent
			// unnecessary cache misses if someone does while(!try_lock())
			return !m_Lock.load(std::memory_order_relaxed) &&
				!m_Lock.exchange(true, std::memory_order_acquire);
		}

		void Unlock() noexcept {
			m_Lock.store(false, std::memory_order_release);
		}

		bool Status() { return m_Lock.load(std::memory_order_relaxed); }

	private:
		std::atomic<bool> m_Lock = { 0 };
	};
}