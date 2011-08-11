/* Copyright (c) 2011 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

// This program contains a collection of low-level performance measurements
// for RAMCloud, which can be run either individually or altogether.  These
// tests measure performance in a single stand-alone process, not in a cluster
// with multiple servers.  Invoke the program like this:
//
//     Perf test1 test2 ...
//
// test1 and test2 are the names of individual performance measurements to
// run.  If no test names are provided then all of the performance tests
// are run.
//
// To add a new test:
// * Write a function that implements the test.  Use existing test functions
//   as a guideline, and be sure to generate output in the same form as
//   other tests.
// * Create a new entry for the test in the #tests table.

#include <cstdatomic>
#include <vector>

#include "Common.h"
#include "AtomicInt.h"
#include "Cycles.h"
#include "Dispatch.h"
#include "Fence.h"
#include "Object.h"
#include "Segment.h"
#include "SegmentIterator.h"
#include "SpinLock.h"

using namespace RAMCloud;

/**
 * Ask the operating system to pin the current thread to a given CPU.
 *
 * \param cpu
 *      Indicates the desired CPU and hyperthread; low order 2 bits
 *      specify CPU, next bit specifies hyperthread.
 */
void bindThreadToCpu(int cpu)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    sched_setaffinity((pid_t)syscall(SYS_gettid), sizeof(set), &set);
}

//----------------------------------------------------------------------
// Test functions start here
//----------------------------------------------------------------------


// Measure the cost of AtomicInt::compareExchange.
double atomicIntCmpX()
{
    int count = 1000000;
    AtomicInt value(11);
    int test = 11;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
         value.compareExchange(test, test+2);
         test += 2;
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Final value: %d\n", value.load());
    return Cycles::toSeconds(stop - start)/count;
}
// Measure the cost of AtomicInt::inc.
double atomicIntInc()
{
    int count = 1000000;
    AtomicInt value(11);
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
         value.inc();
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Final value: %d\n", value.load());
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of reading an AtomicInt.
double atomicIntLoad()
{
    int count = 1000000;
    AtomicInt value(11);
    int total = 0;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
         total += value.load();
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Total: %d\n", total);
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of AtomicInt::exchange.
double atomicIntXchg()
{
    int count = 1000000;
    AtomicInt value(11);
    int total = 0;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
         total += value.exchange(i);
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Total: %d\n", total);
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of storing a new value in a AtomicInt.
double atomicIntStore()
{
    int count = 1000000;
    AtomicInt value(11);
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        value.store(88);
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of acquiring and releasing a boost mutex in the
// fast case where the mutex is free.
double bMutexNoBlock()
{
    int count = 1000000;
    boost::mutex m;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        m.lock();
        m.unlock();
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of the exchange method on a C++ atomic_int.
double cppAtomicExchange()
{
    int count = 100000;
    std::atomic_int value(11);
    int other = 22;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
         other = value.exchange(other);
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of the load method on a C++ atomic_int (seems to have
// 2 mfence operations!).
double cppAtomicLoad()
{
    int count = 100000;
    std::atomic_int value(11);
    int total = 0;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        total += value.load();
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Total: %d\n", total);
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the minimum cost of Dispatch::poll, when there are no
// Pollers and no Timers.
double dispatchPoll()
{
    int count = 1000000;
    Dispatch dispatch;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        dispatch.poll();
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of calling ThreadId::get.
double getThreadId()
{
    int count = 1000000;
    int64_t result = 0;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        result += ThreadId::get();
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Result: %d\n", downCast<int>(result));
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of an lfence instruction.
double lfence()
{
    int count = 1000000;
    Dispatch dispatch;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        Fence::lfence();
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of creating and deleting a Dispatch::Lock from within
// the dispatch thread.
double lockInDispThrd()
{
    int count = 1000000;
    Dispatch dispatch;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        Dispatch::Lock lock(&dispatch);
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of creating and deleting a Dispatch::Lock from a thread
// other than the dispatch thread (best case: the dispatch thread has no
// pollers).
void dispatchThread(Dispatch **d, volatile int* flag)
{
    bindThreadToCpu(2);
    Dispatch dispatch;
    *d = &dispatch;
    dispatch.poll();
    *flag = 1;
    while (*flag == 1)
        dispatch.poll();
}

double lockNonDispThrd()
{
    int count = 100000;
    volatile int flag = 0;

    // Start a new thread and wait for it to create a dispatcher.
    Dispatch* dispatch;
    boost::thread thread(dispatchThread, &dispatch, &flag);
    while (flag == 0) {
        usleep(100);
    }

    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        Dispatch::Lock lock(dispatch);
    }
    uint64_t stop = Cycles::rdtsc();
    flag = 0;
    thread.join();
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of the Cylcles::toNanoseconds method.
double perfCyclesToNanoseconds()
{
    int count = 1000000;
    std::atomic_int value(11);
    uint64_t total = 0;
    uint64_t cycles = 994261;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        total += Cycles::toNanoseconds(cycles);
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Result: %lu\n", total/count);
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of the Cycles::toSeconds method.
double perfCyclesToSeconds()
{
    int count = 1000000;
    std::atomic_int value(11);
    double total = 0;
    uint64_t cycles = 994261;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        total += Cycles::toSeconds(cycles);
    }
    uint64_t stop = Cycles::rdtsc();
    // printf("Result: %.4f\n", total/count);
    return Cycles::toSeconds(stop - start)/count;
}

// Measure the cost of an sfence instruction.
double sfence()
{
    int count = 1000000;
    Dispatch dispatch;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        Fence::sfence();
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// Sorting functor for #segmentEntrySort.
struct SegmentEntryLessThan {
  public:
    bool
    operator()(const SegmentEntryHandle a, const SegmentEntryHandle b)
    {
        return a->userData<Object>()->timestamp <
               b->userData<Object>()->timestamp;
    }
};

// Measure the time it takes to walk a Segment full of small objects,
// populate a vector with their entries, and sort it by age.
double segmentEntrySort()
{
    void *block = xmemalign(Segment::SEGMENT_SIZE, Segment::SEGMENT_SIZE);
    Segment s(0, 0, block, Segment::SEGMENT_SIZE, NULL);
    const int avgObjectSize = 100;

    DECLARE_OBJECT(obj, 2 * avgObjectSize);
    vector<SegmentEntryHandle> entries;

    int count;
    for (count = 0; ; count++) {
        obj->timestamp = static_cast<uint32_t>(generateRandom());
        uint32_t size = obj->timestamp % (2 * avgObjectSize);
        if (s.append(LOG_ENTRY_TYPE_OBJ, obj, obj->objectLength(size)) == NULL)
            break;
    }

    // doesn't appear to help
    entries.reserve(count);

    uint64_t start = Cycles::rdtsc();

    // appears to take about 1/8th the time
    for (SegmentIterator i(&s); !i.isDone(); i.next()) {
        if (i.getType() == LOG_ENTRY_TYPE_OBJ)
            entries.push_back(i.getHandle());
    }

    // the rest is, unsurprisingly, here
    std::sort(entries.begin(), entries.end(), SegmentEntryLessThan());

    uint64_t stop = Cycles::rdtsc();

    free(block);

    return Cycles::toSeconds(stop - start);
}

// Measure the cost of acquiring and releasing a SpinLock (assuming the
// lock is initially free).
double spinLock()
{
    int count = 1000000;
    SpinLock lock;
    uint64_t start = Cycles::rdtsc();
    for (int i = 0; i < count; i++) {
        lock.lock();
        lock.unlock();
    }
    uint64_t stop = Cycles::rdtsc();
    return Cycles::toSeconds(stop - start)/count;
}

// The following struct and table define each performance test in terms of
// a string name and a function that implements the test.
struct TestInfo {
    const char* name;             // Name of the performance test; this is
                                  // what gets typed on the command line to
                                  // run the test.
    double (*func)();             // Function that implements the test;
                                  // returns the time (in seconds) for each
                                  // iteration of that test.
    const char *description;      // Short description of this test (not more
                                  // than about 40 characters, so the entire
                                  // test output fits on a single line).
};
TestInfo tests[] = {
    {"atomicIntCmpX", atomicIntCmpX,
     "AtomicInt::compareExchange"},
    {"atomicIntInc", atomicIntInc,
     "AtomicInt::inc"},
    {"atomicIntLoad", atomicIntLoad,
     "AtomicInt::load"},
    {"atomicIntStore", atomicIntStore,
     "AtomicInt::store"},
    {"atomicIntXchg", atomicIntXchg,
     "AtomicInt::exchange"},
    {"bMutexNoBlock", bMutexNoBlock,
     "Boost mutex lock/unlock (no blocking)"},
    {"cppAtomicExchg", cppAtomicExchange,
     "Exchange method on a C++ atomic_int"},
    {"cppAtomicLoad", cppAtomicLoad,
     "Read a C++ atomic_int"},
    {"cyclesToSeconds", perfCyclesToSeconds,
     "Convert a rdtsc result to (double) seconds"},
    {"cyclesToNanos", perfCyclesToNanoseconds,
     "Convert a rdtsc result to (uint64_t) nanoseconds"},
    {"dispatchPoll", dispatchPoll,
     "Dispatch::poll (no timers or pollers)"},
    {"getThreadId", getThreadId,
     "Retrieve thread id via ThreadId::get"},
    {"lfence", lfence,
     "Lfence instruction"},
    {"lockInDispThrd", lockInDispThrd,
     "Acquire/release Dispatch::Lock (in dispatch thread)"},
    {"lockNonDispThrd", lockNonDispThrd,
     "Acquire/release Dispatch::Lock (non-dispatch thread)"},
    {"segmentEntrySort", segmentEntrySort,
     "Sort a Segment full of avg. 100-byte Objects by age"},
    {"sfence", sfence,
     "Sfence instruction"},
    {"spinLock", spinLock,
     "Acquire/release SpinLock"},
};

/**
 * Runs a particular test and prints a one-line result message.
 *
 * \param info
 *      Describes the test to run.
 */
void runTest(TestInfo& info)
{
    double secs = info.func();
    int width = printf("%-16s ", info.name);
    if (secs < 1.0e-06) {
        width += printf("%.2fns", 1e09*secs);
    } else if (secs < 1.0e-03) {
        width += printf("%.2fus", 1e06*secs);
    } else if (secs < 1.0) {
        width += printf("%.2fms", 1e03*secs);
    } else {
        width += printf("%.2fs", secs);
    }
    printf("%*s %s\n", 26-width, "", info.description);
}

int
main(int argc, char *argv[])
{
    bindThreadToCpu(3);
    if (argc == 1) {
        // No test names specified; run all tests.
        foreach (TestInfo& info, tests) {
            runTest(info);
        }
    } else {
        // Run only the tests that were specified on the command line.
        for (int i = 1; i < argc; i++) {
            bool foundTest = false;
            foreach (TestInfo& info, tests) {
                if (strcmp(argv[i], info.name) == 0) {
                    foundTest = true;
                    runTest(info);
                    break;
                }
            }
            if (!foundTest) {
                int width = printf("%-16s ??", argv[i]);
                printf("%*s No such test\n", 26-width, "");
            }
        }
    }
}