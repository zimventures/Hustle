#include "gtest/gtest.h"
#include "../SpinLock.h"

using namespace JobSystem;

TEST(SpinLock, TryLockFail) {

	SpinLock lock;
	ASSERT_EQ(lock.Status(), false);

	lock.Lock();

	ASSERT_EQ(lock.TryLock(), false);

	lock.Unlock();
	ASSERT_EQ(lock.Status(), false);
}

TEST(SpinLock, TryLockPass) {
	SpinLock lock;
	ASSERT_EQ(lock.Status(), false);

	ASSERT_EQ(lock.TryLock(), true);

	// Second attempt to grab the lock should fail
	ASSERT_EQ(lock.TryLock(), false);

	// Lock should be taken
	ASSERT_EQ(lock.Status(), true);

	lock.Unlock();

	ASSERT_EQ(lock.Status(), false);
}