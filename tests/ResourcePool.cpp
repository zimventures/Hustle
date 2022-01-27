#include "gtest/gtest.h"
#include "hustle/ResourcePool.h"

#include <queue>

using namespace Hustle;

const int MaxPoolSize = 100;

TEST(ResourcePool, FreeCount) {

	struct TestResource {
		int iSomething;
	};


	ResourcePool<TestResource> testPool;
	testPool.Grow(MaxPoolSize);
	EXPECT_EQ(testPool.GetFreeCount(), MaxPoolSize);
	EXPECT_EQ(testPool.GetTotalCount(), MaxPoolSize);

	std::queue<TestResource*> pulledResources;

	// Pull half of the test resources out
	for (int i = 0; i < MaxPoolSize / 2; i++) {
		pulledResources.push(testPool.Get());
	}

	// Verify counts
	EXPECT_EQ(testPool.GetTotalCount(), MaxPoolSize);
	EXPECT_EQ(testPool.GetFreeCount(), MaxPoolSize / 2);
	EXPECT_EQ(pulledResources.size(), MaxPoolSize / 2);

	// Add the resources back into the pool
	for (int i = 0; i < MaxPoolSize / 2; i++) {
		testPool.Release(pulledResources.front());
		pulledResources.pop();
	}

	// Verify counts
	EXPECT_EQ(testPool.GetFreeCount(), MaxPoolSize);
	EXPECT_EQ(testPool.GetTotalCount(), MaxPoolSize);
}