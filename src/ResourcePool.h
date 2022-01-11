#pragma once

#include <assert.h>
#include <atomic>
#include <queue>
#include "LockedQueue.h"

namespace Hustle {
	template<class T, int iCount>
	class ResourcePool {
	public:
		ResourcePool() :
			m_pPool(nullptr),
			m_iResourcCount(0) {

			m_iResourcCount = iCount;
			m_pPool = new T[iCount];

			// Add all of the newly allocated items to the queue
			for (int i = 0; i < m_iResourcCount; i++)
				m_FreeResources.Push(&m_pPool[i]);
		}

		~ResourcePool() {
			delete[] m_pPool;
		}

		T* Get() {
			
			T* pResource = nullptr;
			pResource = m_FreeResources.Pop();

#ifdef _DEBUG 
			// Only increment the lock if there was an available resource
			if (pResource) {
				m_StatsLock.Lock();
				m_iInUseCounter++;
				if (m_iInUseCounter > m_iHighWaterMark)
					m_iHighWaterMark = m_iInUseCounter;
				m_StatsLock.Unlock();
			}
#endif

			return pResource;
		}

		void Release(T* pResource) {

#ifdef _DEBUG 
			m_iInUseCounter--;
#endif

			m_FreeResources.Push(pResource);
		}

		int GetTotalCount() { return m_iResourcCount; }

		size_t GetFreeCount() {
			return m_FreeResources.Size();
		}

		// Allow caller to access the resource array by index
		T* operator [](int i) { return &(m_pPool[i]); }

#ifdef _DEBUG 
		int GetHighWaterMark() {
			int iHighWaterMark = 0;
			m_StatsLock.Lock();
			iHighWaterMark = m_iHighWaterMark;
			m_StatsLock.Unlock();
			return iHighWaterMark;
		}
#endif

	private:

		T* m_pPool;
		int m_iResourcCount;

		// Performance metrics (only available in debug builds)
#ifdef _DEBUG 
		SpinLock m_StatsLock;
		int m_iHighWaterMark = { 0 };
		int m_iInUseCounter = { 0 };
#endif
		LockedQueue<T*> m_FreeResources;

	};
}