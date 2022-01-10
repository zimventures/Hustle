// FiberSandbox.cpp : Defines the entry point for the application.
//
// References: 
// https://graphitemaster.github.io/fibers/#fibers-oh-my
// https://github.com/RichieSams/FiberTaskingLib


#include <iostream>
#include <string>
#include <windows.h>

#include "Job.h"
#include "Dispatcher.h"

using namespace std;
using namespace JobSystem;

void ThirdLevelJob(void* pUserData) {
    
}

void SubJob(void* pUserData) {

   auto hJob = Dispatcher::GetInstance().AddJob(ThirdLevelJob, nullptr);
   Dispatcher::GetInstance().WaitForJob(hJob);
}

void JobFunction(void *pUserData) {

    auto hJob = Dispatcher::GetInstance().AddJob(SubJob, nullptr);
    Dispatcher::GetInstance().WaitForJob(hJob);

}

int main()
{
    Job job(JobFunction, nullptr);

    std::cout << "Starting Scheduler" << std::endl;
    if (Dispatcher::GetInstance().Init() == false) {
        std::cout << "Failed: " << Dispatcher::GetInstance().GetLastError() << std::endl;
        return -1;
    }

    std::cout << "Scheduler started" << std::endl;

    std::string input;
    bool bRunning = true;
    while(bRunning) {
        std::cout << "0|1|2|3> ";
        std::cin >> input;
        std::cout << std::endl;

        switch (std::stoi(input)) {
        case 1:
            std::cout << "Adding 1 jobs to queue";

            for (auto x = 0; x < 1; x++)
                Dispatcher::GetInstance().AddJob(JobFunction, nullptr);

            break;
        case 2:
            std::cout << "Adding 300 jobs to queue";

            for (auto x = 0; x < 300; x++)
                Dispatcher::GetInstance().AddJob(JobFunction, nullptr);

            break;
        case 3:
            std::cout << "jobs to run: " << Dispatcher::GetInstance().GetJobQueueDepth() << std::endl;
            std::cout << "available jobs: " << Dispatcher::GetInstance().GetFreeJobCount() << std::endl;
            std::cout << "total job pool size: " << Dispatcher::GetInstance().GetFreeJobTotal() << std::endl;
            
            std::cout << "available fibers: " << Dispatcher::GetInstance().GetFiberPoolFree() << std::endl;
            std::cout << "total fiber pool size: " << Dispatcher::GetInstance().GetFiberPoolTotal() << std::endl;           

            // Print out the high water mark for the various queues
#ifdef _DEBUG
            std::cout << "Free Job High Water Mark: " << Dispatcher::GetInstance().GetFreeJobHighWaterMark() << std::endl;
            std::cout << "Free Fiber High Water Mark: " << Dispatcher::GetInstance().GetFreeFiberHighWaterMark() << std::endl;            
#endif
            break;
        case 0:
            bRunning = false;
        }
    }

    // TODO: Test/handle when there are jobs in flight and we try to shut things down
    std::cout << "Shutting down Scheduler" << std::endl;
    Dispatcher::GetInstance().Shutdown();
    
    std::cout << "Done." << endl;
    return 0;
}