#include "gtest/gtest.h"
#include "LockedQueue.h"

using namespace Hustle;

const int MaxQueueSize = 100;
TEST(LockedQueue, SizeCheck) {
	LockedQueue<int *> intQueue;
	
	int intData[MaxQueueSize];

	for (auto i = 0; i < MaxQueueSize; i++)
		intData[i] = i;

	for (auto i = 0; i < MaxQueueSize; i++) {
		intQueue.Push(&intData[i]);
	}

	EXPECT_EQ(intQueue.Size(), MaxQueueSize);

	for (auto i = 0; i < MaxQueueSize / 2; i++) {
		int* pInt = intQueue.Pop();
		EXPECT_EQ(*pInt, i);
	}

	EXPECT_EQ(intQueue.Size(), MaxQueueSize / 2);
}

TEST(LockedQueue, EmptyQueue) {

	LockedQueue<int*> intQueue;
	EXPECT_EQ(intQueue.Pop(), nullptr);
	EXPECT_EQ(intQueue.Size(), 0);
}