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

TEST(ResourcePool, IncreasePoolCount) {

	struct TestResource {
		int iSomething;
	};

	ResourcePool<TestResource> testPool;
	EXPECT_EQ(testPool.GetTotalCount(), 0);
	EXPECT_EQ(testPool.GetFreeCount(), 0);
	auto newSize = testPool.Grow(MaxPoolSize);
	EXPECT_EQ(newSize, MaxPoolSize);

	newSize = testPool.Grow(MaxPoolSize);
	EXPECT_EQ(newSize, MaxPoolSize * 2);
	EXPECT_EQ(testPool.GetFreeCount(), MaxPoolSize * 2);
	EXPECT_EQ(testPool.GetTotalCount(), MaxPoolSize * 2);
}

TEST(ResourcePool, PoolAutoGrow) {

	struct TestResource {
		int iSomething;
	};

	float fGrowthFactor = 1.5f;
	ResourcePool<TestResource> testPool;
	testPool.SetGrowthFactor(fGrowthFactor);
	EXPECT_EQ(testPool.GetGrowthFactor(), fGrowthFactor);

	// Start with 5 resources
	EXPECT_EQ(testPool.Grow(5), 5);

	TestResource* pResources[5];

	// Get the 5 resources, which should cause the pool to grow by 1.5 times.
	for (int i = 0; i < 5; i++) {
		pResources[i] = testPool.Get();
		EXPECT_EQ(testPool.GetFreeCount(), 5 - (i + 1));
	}
	
	// Grab one more resource that will trigger the resize (since there are none left)
	auto pResource = testPool.Get();
	assert(pResource != nullptr);

	// Initial size (5) + 1.5 times the initial size (7) = 12	
	EXPECT_EQ(testPool.GetTotalCount(), 12);
	EXPECT_EQ(testPool.GetFreeCount(), 6);

	// Release the (kraken?) resources
	testPool.Release(pResource);
	for (int i = 0; i < 5; i++)
		testPool.Release(pResources[i]);
	
	EXPECT_EQ(testPool.GetTotalCount(), 12);
	EXPECT_EQ(testPool.GetFreeCount(), 12);

}

TEST(ResourcePool, ArrayOperator) {

	struct TestResource {
		int iIndex;
	};

	ResourcePool<TestResource> testPool;
	testPool.Grow(5);

	// Grab 5 resources, set their index to match the loop counter
	for (int i = 0; i < 5; i++) {
		TestResource* pResource = testPool.Get();
		pResource->iIndex = i;
	}

	for (int i = 0; i < 5; i++) {
		TestResource* pResource = testPool[i];
		EXPECT_EQ(pResource->iIndex, i);
	}
	
}