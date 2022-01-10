#pragma once
#include <queue>

#include "SpinLock.h"

namespace JobSystem {
	template<class T>
	class LockedQueue : private SpinLock {
	public:

		void Push(T val) {
			Lock();
			m_Queue.push(val);
			Unlock();
		}

		T Pop() {
			T val;
			Lock();
			if (m_Queue.size() == 0) {
				val = nullptr;
			}
			else {
				val = m_Queue.front();
				m_Queue.pop();
			}
			Unlock();
			return val;
		}

		size_t Size() {
			size_t size;
			Lock();
			size = m_Queue.size();
			Unlock();
			return size;
		}

	private:
		std::queue<T>	m_Queue;
	};
}