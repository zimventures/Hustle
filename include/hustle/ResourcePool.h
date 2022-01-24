#pragma once

#include "LockedQueue.h"

#include <assert.h>
#include <atomic>
#include <queue>

namespace Hustle {

	template<class T, int iCount>
	class ResourcePool {
	public:

		/**
		 * @brief Default constructor for the ResourcePool. 
		*/
		ResourcePool() :
			m_fGrowthFactor(0.0f),
			m_iInUseCounter(0),
			m_pPool(nullptr),
			m_iResourcCount(0) {

			m_iResourcCount = iCount;

			// Add all of the newly allocated items to the queue
			for (int i = 0; i < m_iResourcCount; i++) {
				T* pResource = new T();
				m_Pool.push_back(pResource);
				m_FreeResources.Push(pResource);
			}
		}
		
		~ResourcePool() {

			// Delete all of the resources
			for (auto pResource : m_Pool)
				delete pResource;
			
			// Empty out the pool
			m_Pool.clear();

		}

		/**
		 * @brief Obtain a single resorce from the pool
		 * @return A pointer to the resource (T)
		*/
		T* Get() {
			
			T* pResource = nullptr;
			pResource = m_FreeResources.Pop();

			if (pResource) {
				m_iInUseCounter++;

				// Only called in DEBUG mode
				HighWaterCheck();
			} else {

				// Are we allowing dynamic pool resizing?
				if (m_fGrowthFactor > 0.0f) {		

					// Take the resize lock
					m_ResizeLock.Lock();

					// What if...the resize alredy occured somewhere else?
					pResource = m_FreeResources.Pop();

					// Got one - great!
					if (pResource) {
						m_iInUseCounter++;

						// Only called in DEBUG mode
						HighWaterCheck();

						m_ResizeLock.Unlock();
						return pResource;
					}

					// Still nothing available - let's grow the pool
					int iItemsToGrow = m_iResourcCount * m_fGrowthFactor;
					for (auto i = 0; i < iItemsToGrow; i++) {
						T* pResource = new T();
						m_Pool.push_back(pResource);
						m_FreeResources.Push(pResource);
						m_iResourcCount++;
					}

					m_ResizeLock.Unlock();

					// Alright...get one for real this time
					pResource = m_FreeResources.Pop();

					// Got one - great!
					if (pResource) {
						m_iInUseCounter++;

						// Only called in DEBUG mode
						HighWaterCheck();

						return pResource;
					}
				}
			}
			

			return pResource;
		}

		void Release(T* pResource) {

			m_iInUseCounter--;
			m_FreeResources.Push(pResource);
		}

		int GetTotalCount() { return m_iResourcCount; }

		size_t GetFreeCount() {
			return m_FreeResources.Size();
		}

		float GetGrowthFactor() {
			return m_fGrowthFactor;
		}

		void SetGrowthFactor(float fFactor) {
			m_fGrowthFactor = fFactor;
		}

		/**
		 * @brief Array operator to allow for direct access to the entire pool (use at your own risk)
		 * @param i Index of the pool item to access
		 * @return A pointer to the pool item (T)
		*/
		T* operator [](int i) { 
			return m_Pool[i];
		}

#ifdef _DEBUG 

		/**
		 * @brief Fetch the highest number of outstanding items requested from the pool. Only available in debug builds.
		 * @return Highest number of items resuqested by the application
		*/
		int GetHighWaterMark() {
			int iHighWaterMark = 0;
			m_StatsLock.Lock();
			iHighWaterMark = m_iHighWaterMark;
			m_StatsLock.Unlock();
			return iHighWaterMark;
		}
#else 
		/**
		 * @brief Not availbale in release mode.
		 * @return N/A
		*/
		int GetHighWaterMark() { return 0; }
#endif

	private:

		T* m_pPool;
		int m_iResourcCount;
		float m_fGrowthFactor;

		std::vector<T*> m_Pool;				// Vector of every allocated resource
		LockedQueue<T*> m_FreeResources;	// Holds all available resources
		SpinLock m_ResizeLock;				// Taken when a pool resize is underway

		// Performance metrics
		SpinLock m_StatsLock;
		int m_iHighWaterMark = { 0 };
		std::atomic<int> m_iInUseCounter;

		// Performance methods that are only available/called if running in DEBUG mode
#ifdef _DEBUG
		void HighWaterCheck() {
			// Only increment the lock if there was an available resource

			// TODO: Is there a way to do this without locking?
			m_StatsLock.Lock();
			if (m_iInUseCounter > m_iHighWaterMark)
				m_iHighWaterMark = m_iInUseCounter;
			m_StatsLock.Unlock();			
		}
#else
		void HighWaterCheck(void *pItem) {}
#endif
	};
}