#include "shared_flag/shared_flag.hpp"
#include <future>
#include <gtest/gtest.h>
#include <thread>

using namespace std::literals;
using namespace prb;

namespace
{
    // Short-hand alias to get the current steady clock time point.
    constexpr auto now = std::chrono::steady_clock::now;
}

//--------------------------------------------------------------------------------------------------
// default constructor

TEST(shared_flag, defaultConstructorCreatesAnIndependentInstance)
{
    shared_flag flag1;
    shared_flag flag2;
    flag1.set();
    ASSERT_FALSE(flag2.get());
}


//--------------------------------------------------------------------------------------------------
// copy constructor

TEST(shared_flag, copyConstructorCopiesReferenceToExistingSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ flag1 };
    flag1.set();
    ASSERT_TRUE(flag2.get());
}


TEST(shared_flag, copyConstructorThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(shared_flag{ flag1 }, std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// copy assignment

TEST(shared_flag, copyAssignmentCopiesReferenceToExistingSharedState)
{
    shared_flag flag1;
    shared_flag flag2;
    flag2 = flag1;
    flag1.set();
    ASSERT_TRUE(flag2.get());
}


TEST(shared_flag, copyAssignmentThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    shared_flag flag3;
    ASSERT_THROW(flag3 = flag1, std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// move constructor

TEST(shared_flag, moveConstructorTransfersExistingSharedStateReferenceToDestination)
{
    shared_flag flag1;
    flag1.set();
    shared_flag flag2{ std::move(flag1) };
    ASSERT_TRUE(flag2.get());
}

TEST(shared_flag, moveConstructorRemovesSharedStateReferenceFromSource)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_FALSE(flag1.valid());
}

TEST(shared_flag, moveConstructorThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(shared_flag{ std::move(flag1) }, std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// move assignment

TEST(shared_flag, moveAssignmentTransfersExistingSharedStateReferenceToDestination)
{
    shared_flag flag1;
    flag1.set();
    shared_flag flag2;
    flag2 = std::move(flag1);
    ASSERT_TRUE(flag2.get());
}

TEST(shared_flag, moveAssignmentRemovesSharedStateReferenceFromSource)
{
    shared_flag flag1;
    shared_flag flag2;
    flag2 = std::move(flag1);
    ASSERT_FALSE(flag1.valid());
}

TEST(shared_flag, moveAssignmentThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    shared_flag flag3;
    ASSERT_THROW(flag3 = std::move(flag1), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// destructor

TEST(shared_flag, destructorDoesNotAffectOtherInstancesReferringToTheSameSharedState)
{
    shared_flag flag1;
    {
        shared_flag flag2;
        flag1 = flag2;
    }
    flag1.set();
    ASSERT_TRUE(flag1.valid());
    ASSERT_TRUE(flag1.get());
}


//--------------------------------------------------------------------------------------------------
// set()

TEST(shared_flag, setUpdatesFlagInSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ flag1 };
    flag1.set();
    ASSERT_TRUE(flag2.get());
}

TEST(shared_flag, setHasNoEffectIfFlagWasAlreadySet)
{
    shared_flag flag1;
    shared_flag flag2{ flag1 };
    flag1.set();
    ASSERT_TRUE(flag2.get());
    ASSERT_NO_THROW(flag1.set());
    ASSERT_TRUE(flag2.get());
}

TEST(shared_flag, setThrowsLogicErrorIfSharedStateHasBeenMovedAway)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(flag1.set(), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// valid()

TEST(shared_flag, validReturnsTrueIfObjectHasSharedState)
{
    shared_flag flag;
    ASSERT_TRUE(flag.valid());
}

TEST(shared_flag, validReturnsFalseIfSharedStateHasBeenMovedAway)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_FALSE(flag1.valid());
}


//--------------------------------------------------------------------------------------------------
// get()

TEST(shared_flag, getReturnsFalseIfFlagHasNotBeenSet)
{
    shared_flag flag;
    ASSERT_FALSE(flag.get());
}

TEST(shared_flag, getReturnsTrueIfFlagHasBeenSet)
{
    shared_flag flag;
    flag.set();
    ASSERT_TRUE(flag.get());
}

TEST(shared_flag, getThrowsLogicErrorIfSharedStateHasBeenMovedAway)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(flag1.get(), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// operator bool

TEST(shared_flag, operatorBoolReturnsFalseIfFlagHasNotBeenSet)
{
    shared_flag flag;
    ASSERT_FALSE(static_cast<bool>(flag));
}

TEST(shared_flag, operatorBoolReturnsTrueIfFlagHasBeenSet)
{
    shared_flag flag;
    flag.set();
    ASSERT_TRUE(static_cast<bool>(flag));
}

// Prevent clang from warning us about the static_cast result being discarded in the next test.
#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-value"
#endif

TEST(shared_flag, operatorBoolThrowsLogicErrorIfSharedStateHasBeenMovedAway)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(static_cast<bool>(flag1), std::logic_error);
}

#if defined(__clang__)
#   pragma clang diagnostic pop
#endif


//--------------------------------------------------------------------------------------------------
// wait()

TEST(shared_flag, waitReturnsImmediatelyIfFlagWasAlreadySet)
{
    shared_flag flag;
    flag.set();
    flag.wait();
    SUCCEED();
}

TEST(shared_flag, waitReturnsIfFlagWasSetViaTheSameInstanceWhileWaiting)
{
    shared_flag flag;
    auto function{ [&]() { flag.wait(); } };
    auto task{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();
    task.wait();
    SUCCEED();
}

TEST(shared_flag, waitReturnsIfFlagWasSetViaAnotherInstanceWhileWaiting)
{
    auto function{ [](shared_flag flagCopy) { flagCopy.wait(); } };

    shared_flag flag;
    auto task{ std::async(std::launch::async, function, flag) };
    std::this_thread::sleep_for(150ms);
    flag.set();
    task.wait();
    SUCCEED();
}

TEST(shared_flag, waitSupportsMultipleThreadsWaitingOnTheSameFlagViaTheSameInstance)
{
    shared_flag flag;
    auto function{ [&]() { flag.wait(); } };

    auto task1{ std::async(std::launch::async, function) };
    auto task2{ std::async(std::launch::async, function) };
    auto task3{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    task1.wait();
    task2.wait();
    task3.wait();
    SUCCEED();
}

TEST(shared_flag, waitSupportsMultipleThreadsWaitingOnTheSameFlagViaDifferentInstances)
{
    auto function{ [](shared_flag flagCopy) { flagCopy.wait(); } };

    shared_flag flag;
    auto task1{ std::async(std::launch::async, function, flag) };
    auto task2{ std::async(std::launch::async, function, flag) };
    auto task3{ std::async(std::launch::async, function, flag) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    task1.wait();
    task2.wait();
    task3.wait();
    SUCCEED();
}

TEST(shared_flag, waitThrowsLogicErrorIfSharedStateWasMovedAway)
{
    shared_flag flag1;
    const shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(flag1.wait(), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// wait_for()

TEST(shared_flag, waitForReturnsFalseIfFlagHasNotBeenSetBeforeTimeout)
{
    shared_flag flag;
    ASSERT_FALSE(flag.wait_for(10ms));
}

TEST(shared_flag, waitForReturnsTrueIfFlagWasAlreadySet)
{
    shared_flag flag;
    flag.set();
    ASSERT_TRUE(flag.wait_for(10ms));
}

TEST(shared_flag, waitForReturnsTrueIfFlagWasSetViaTheSameInstanceWhileWaiting)
{
    shared_flag flag;
    auto function{ [&]() { return flag.wait_for(2s); } };
    auto task{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();
    ASSERT_TRUE(task.get());
}

TEST(shared_flag, waitForReturnsTrueIfFlagWasSetViaAnotherInstanceWhileWaiting)
{
    auto function{ [](shared_flag flagCopy) { return flagCopy.wait_for(2s); } };

    shared_flag flag;
    auto task{ std::async(std::launch::async, function, flag) };
    std::this_thread::sleep_for(150ms);
    flag.set();
    ASSERT_TRUE(task.get());
}

TEST(shared_flag, waitForSupportsMultipleThreadsWaitingOnTheSameFlagViaTheSameInstance)
{
    shared_flag flag;
    auto function{ [&]() { return flag.wait_for(2s); } };

    auto task1{ std::async(std::launch::async, function) };
    auto task2{ std::async(std::launch::async, function) };
    auto task3{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    ASSERT_TRUE(task1.get());
    ASSERT_TRUE(task2.get());
    ASSERT_TRUE(task3.get());
}

TEST(shared_flag, waitForSupportsMultipleThreadsWaitingOnTheSameFlagViaDifferentInstances)
{
    auto function{ [](shared_flag flagCopy) { return flagCopy.wait_for(2s); } };

    shared_flag flag;
    auto task1{ std::async(std::launch::async, function, flag) };
    auto task2{ std::async(std::launch::async, function, flag) };
    auto task3{ std::async(std::launch::async, function, flag) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    ASSERT_TRUE(task1.get());
    ASSERT_TRUE(task2.get());
    ASSERT_TRUE(task3.get());
}

TEST(shared_flag, waitForThrowsLogicErrorIfSharedStateWasMovedAway)
{
    shared_flag flag1;
    const shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(flag1.wait_for(10ms), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// wait_until()

TEST(shared_flag, waitUntilReturnsFalseIfFlagHasNotBeenSetBeforeTimeout)
{
    shared_flag flag;
    ASSERT_FALSE(flag.wait_until(now() + 10ms));
}

TEST(shared_flag, waitUntilReturnsTrueIfFlagWasAlreadySet)
{
    shared_flag flag;
    flag.set();
    ASSERT_TRUE(flag.wait_until(now() + 10ms));
}

TEST(shared_flag, waitUntilReturnsTrueIfFlagWasSetViaTheSameInstanceWhileWaiting)
{
    shared_flag flag;
    auto function{ [&]() { return flag.wait_until(now() + 2s); } };
    auto task{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();
    ASSERT_TRUE(task.get());
}

TEST(shared_flag, waitUntilReturnsTrueIfFlagWasSetViaAnotherInstanceWhileWaiting)
{
    auto function{ [](shared_flag flagCopy) { return flagCopy.wait_until(now() + 2s); } };

    shared_flag flag;
    auto task{ std::async(std::launch::async, function, flag) };
    std::this_thread::sleep_for(150ms);
    flag.set();
    ASSERT_TRUE(task.get());
}

TEST(shared_flag, waitUntilSupportsMultipleThreadsWaitingOnTheSameFlagViaTheSameInstance)
{
    shared_flag flag;
    auto function{ [&]() { return flag.wait_until(now() + 2s); } };

    auto task1{ std::async(std::launch::async, function) };
    auto task2{ std::async(std::launch::async, function) };
    auto task3{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    ASSERT_TRUE(task1.get());
    ASSERT_TRUE(task2.get());
    ASSERT_TRUE(task3.get());
}

TEST(shared_flag, waitUntilSupportsMultipleThreadsWaitingOnTheSameFlagViaDifferentInstances)
{
    auto function{ [](shared_flag flagCopy) { return flagCopy.wait_until(now() + 2s); } };

    shared_flag flag;
    auto task1{ std::async(std::launch::async, function, flag) };
    auto task2{ std::async(std::launch::async, function, flag) };
    auto task3{ std::async(std::launch::async, function, flag) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    ASSERT_TRUE(task1.get());
    ASSERT_TRUE(task2.get());
    ASSERT_TRUE(task3.get());
}

TEST(shared_flag, waitUntilThrowsLogicErrorIfSharedStateWasMovedAway)
{
    shared_flag flag1;
    const shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(flag1.wait_until(now() + 10ms), std::logic_error);
}

