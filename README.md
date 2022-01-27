# Hustle: A C++ Job Scheduler

In 2015, [Christian Gyrling](https://twitter.com/cgyrling) gave a [fantastic presentation](https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine) at the annual 
[Game Developer Conference](https://gdconf.com/). He outlined how the Naughty Dog Engine utilized fibers on the PS4 to create a high performance
job scheduler. This project is a slight modification on Christian's architecture. 

# Settings
The `Dispatcher` needs to be tuned for the application needs. As such, there are two constants available: 
- `FiberPoolSize`: The number of fibers to provision in the global pool. 
- `JobPoolSize`: The size of the global job pool. This will need to handle the maximum job size per cycle for your application.

__WARNING__ Currently, there is no special logic in the scheduler to detect and mitigate when all fibers are in a state where they're waiting for jobs 
that are stuck on the queue. To avoid this scenario, ensure there are enough fibers in the pool to handle the high water mark for fibers. 

# Supportetd Systems
This demo currently runs on Windows using its Fiber API. It could (should?) be expanded to support the 
[POSIX fiber constructs](https://linux.die.net/man/2/setcontext) or even the 
[Boost implementation](https://www.boost.org/doc/libs/1_78_0/libs/fiber/doc/html/fiber/overview.html). Noted below is an interesting article by
Dale Weiler which uses assembly to perform the fiber (context switching) functionality, to minimize impacts by the OS. 

# Components
Hustle is a job scheduling system comprised of a few key components. A dispatcher manages the queuing, distribution, and execution of jobs. Worker threads 
manage the execution of fibers, which in turn execute jobs. 

## Job
The job is the unit of work in Hustle. A job is comprised of an entrypoint (function) and an optional pointer to user data. When jobs are queued, using
`Dispatch::AddJob()`, they are added to a global queue of jobs, to be processed at some point later by the worker fibers.

## Dispatcher
The `Dispatch` class is what manages the entire job system. It is a [singleton](https://en.wikipedia.org/wiki/Singleton_pattern) with methods 
to perform the following operations:
- Initialize the system
- Shutdown the system
- Add jobs to be executed
- Wait for a previously created job to complete

You'll note there is no method to get or remove jobs. That's because the `WorkerThread` class is a `friend` of the `Dispatch` class, thus giving it
access to all of the resource pools. 

## Fiber
The `Fiber` class is a wrapper around the [Windows Fiber API](https://docs.microsoft.com/en-us/windows/win32/procthread/fibers). In the future, if 
Hustle is ever updated to work with the Posix or Boost equivallent functionality, it would happen here. 

tldr; Fibers are user-space threads. The application (Hustle in this case) must manage everything about the scheduling and execution of the fibers. 
The `Dispatcher` class manages a pool of available fibers and handles switching between them on each CPU core. 

## Worker Threads (Fibers)
When the `Dispatcher::Init()` method is invoked, a worker thread is started on each available CPU core - except core 0. The `WorkerThread` class
provides a simple wrapper around the threading mechanism. The entry point for all worker threads is actually the `Dispatcher::Scheduler()` method. 
`WorkerThread` classes are friends of the `Dispatcher` and as such, have access to the private `Scheduler` method. 

## Distributed Computing Primitives
When writing applications for paralell computing, care must be taken to ensure that data is written to or read from in an orchestrated manner. 
Take the case where you have a queue of network packet buffers. The application will run packet processing logic on multiple CPU cores. 
Access to that queue (read or write) can only happen from a single CPU at a time. For Hustle, we'll implement some utility classes 
to address all of our concurrency needs. 

### SpinLock class
The `SpinLock` class is a simple wrapper around the [`std::atomic`](https://en.cppreference.com/w/cpp/atomic/atomic) primitive. Only one process can
take and hold the class in a `locked` state. 

Example use:
```c++

// Defined somewhere global
SpinLock g_lock;

// From thread 1
g_lock.Lock();
auto aThing = sharedQueue.front();
sharedQueue.pop();
g_lock.Unlock();

// From thread 2
g_lock.Lock();	// Execution stops here until the lock is released in thread 1
auto anotherThing = sharedQueue.front();
sharedQueue.pop();
g_lock.Unlock();
```

### LockedQueue class
The `Dispatcher` manages a queue of `Job` objects that are to be executed by one of the worker threads (via a fiber). Since multiple worker threads
will pull jobs off of this queue, it must be locked using the `SpinLock` class. We'll implement a templatized locked queue class which is a simple wrapper
around the `std::queue` class. 

```c++
LockedQueue<Job*> m_Jobs;

// From the Dispatcher job queuing function
Job *pJob = GetAFreeJob();
pJob->SetEntryPoint(SomeFunction);
pJob->SetUserData(SomeData);
m_Jobs.Push(pJob);

// From the worker thread jobs, poll the jobs queue. It will return nullptr if there are no jobs. 
if(pJob = m_Jobs.Pop()) {
	// Process the job
}
```

### ResourcePool class

The last data structure we need for Hustle is a protected resource pool. This templatized class provides a heap allocated block of memory to store
the specified number of resources. Available resources are placed on a `LockedQueue` that can be accessed via the `Get()` and `Release()` methods. 

The `Dispatcher` manages two resource pools: one for available `Fiber`s and another for available `Job`s. 

# Future work/enhancements
- Add `Dispatcher` methods for starting and waiting on multiple jobs
- OSX & Linux support
- Scheduler algorightm to detect & mitigate fiber exhaustion
- Performance profiling
- Valgrind
- Add an optional timeout vailute to `Dispatcher::WaitForJob()`
- Add a new IMGui-based demo, visualizing queues and adding buttons to manage running jobs
- Full documentation
- Full test coverage



# Additional Reading
Special thanks go to the following authors:

[Fibers, Oh My!](https://graphitemaster.github.io/fibers/#fibers-oh-my) by [Dale Weiler](https://twitter.com/actualGraphite)

[C++ atomics: from basic to
advanced. What do they do?](https://www.cs.sfu.ca/~ashriram/Courses/CS431/assets/lectures/Part5/Atomics.pdf) by Fedor Pikus

[Parallelizing the Naughty Dog
engine using fibers](https://ubm-twvideo01.s3.amazonaws.com/o1/vault/gdc2015/presentations/Gyrling_Christian_Parallelizing_The_Naughty.pdf) by 
[Christian Gyrling](https://twitter.com/cgyrling)

[Correctly implementing a spinlock in C++](https://rigtorp.se/spinlock/) by [Erik Rigtorp](https://mobile.twitter.com/rigtorp)
Erik also has a [great list of modern C++ resources](https://github.com/rigtorp/awesome-modern-cpp)

[Fibers: the Most Elegant Windows API](https://nullprogram.com/blog/2019/03/28/) by [Chris Wellons](https://github.com/skeeto)
