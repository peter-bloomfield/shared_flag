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
// copy constructor

TEST(shared_flag_reader, copyConstructorCopiesReferenceToExistingSharedStateInshared_flag)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    flag.set();
    ASSERT_TRUE(reader.get());
}

TEST(shared_flag_reader, copyConstructorCopiesReferenceToExistingSharedStateInshared_flag_reader)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ flag };
    flag.set();
    ASSERT_TRUE(reader2.get());
}

TEST(shared_flag_reader, copyConstructorThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(shared_flag_reader{ flag1 }, std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// copy assignment

TEST(shared_flag_reader, copyAssignmentCopiesReferenceToExistingSharedStateInshared_flag)
{
    shared_flag flag;
    shared_flag_reader reader{ shared_flag{} };
    reader = flag;
    flag.set();
    ASSERT_TRUE(reader.get());
}

TEST(shared_flag_reader, copyAssignmentCopiesReferenceToExistingSharedStateInshared_flag_reader)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ shared_flag{} };
    reader2 = reader1;
    flag.set();
    ASSERT_TRUE(reader2.get());
}

TEST(shared_flag_reader, copyAssignmentThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    shared_flag_reader reader{ shared_flag{} };
    ASSERT_THROW(reader = flag1, std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// move constructor

TEST(shared_flag_reader, moveConstructorTransfersExistingSharedStateReferenceToDestination)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    flag.set();
    ASSERT_TRUE(reader2.get());
}

TEST(shared_flag_reader, moveConstructorRemovesSharedStateReferenceFromSource)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    ASSERT_FALSE(reader1.valid());
}

TEST(shared_flag_reader, moveConstructorThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    ASSERT_THROW(shared_flag_reader{ std::move(flag1) }, std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// move assignment

TEST(shared_flag_reader, moveAssignmentTransfersExistingSharedStateReferenceToDestination)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    flag.set();
    shared_flag_reader reader2{ shared_flag{} };
    reader2 = std::move(reader1);
    ASSERT_TRUE(reader2.get());
}

TEST(shared_flag_reader, moveAssignmentRemovesSharedStateReferenceFromSource)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ shared_flag{} };
    reader2 = std::move(reader1);
    ASSERT_FALSE(reader1.valid());
}

TEST(shared_flag_reader, moveAssignmentThrowsLogicErrorIfSourceHasNoSharedState)
{
    shared_flag flag1;
    shared_flag flag2{ std::move(flag1) };
    shared_flag_reader reader{ shared_flag{} };
    ASSERT_THROW(reader = std::move(flag1), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// destructor

TEST(shared_flag_reader, destructorDoesNotAffectOtherInstancesReferringToTheSameSharedState)
{
    shared_flag flag;
    shared_flag_reader reader1{ shared_flag{} };
    {
        shared_flag_reader reader2{ flag };
        reader1 = reader2;
    }
    flag.set();
    ASSERT_TRUE(reader1.valid());
    ASSERT_TRUE(reader1.get());
}


//--------------------------------------------------------------------------------------------------
// valid()

TEST(shared_flag_reader, validReturnsTrueIfObjectHasSharedState)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    ASSERT_TRUE(reader.valid());
}

TEST(shared_flag_reader, validReturnsFalseIfSharedStateHasBeenMovedAway)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    ASSERT_FALSE(reader1.valid());
}


//--------------------------------------------------------------------------------------------------
// get()

TEST(shared_flag_reader, getReturnsFalseIfFlagHasNotBeenSet)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    ASSERT_FALSE(reader.get());
}

TEST(shared_flag_reader, getReturnsTrueIfFlagHasBeenSet)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    flag.set();
    ASSERT_TRUE(reader.get());
}

TEST(shared_flag_reader, getThrowsLogicErrorIfSharedStateHasBeenMovedAway)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    ASSERT_THROW(reader1.get(), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// operator bool

TEST(shared_flag_reader, operatorBoolReturnsFalseIfFlagHasNotBeenSet)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    ASSERT_FALSE(static_cast<bool>(reader));
}

TEST(shared_flag_reader, operatorBoolReturnsTrueIfFlagHasBeenSet)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    flag.set();
    ASSERT_TRUE(static_cast<bool>(reader));
}

// Prevent clang from warning us about the static_cast result being discarded in the next test.
#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-value"
#endif

TEST(shared_flag_reader, operatorBoolThrowsLogicErrorIfSharedStateHasBeenMovedAway)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    ASSERT_THROW(static_cast<bool>(reader1), std::logic_error);
}

#if defined(__clang__)
#   pragma clang diagnostic pop
#endif


//--------------------------------------------------------------------------------------------------
// wait()

TEST(shared_flag_reader, waitReturnsImmediatelyIfFlagWasAlreadySet)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    flag.set();
    reader.wait();
    SUCCEED();
}

TEST(shared_flag_reader, waitReturnsIfFlagWasSetWhileWaiting)
{
    shared_flag flag;
    auto function{ [&](shared_flag_reader reader) { reader.wait(); } };
    auto task{ std::async(std::launch::async, function, flag) };

    std::this_thread::sleep_for(150ms);
    flag.set();
    task.wait();
    SUCCEED();
}

TEST(shared_flag_reader, waitSupportsMultipleThreadsWaitingOnTheSameFlagViaTheSameInstance)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    auto function{ [&]() { reader.wait(); } };

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

TEST(shared_flag_reader, waitSupportsMultipleThreadsWaitingOnTheSameFlagViaDifferentInstances)
{
    auto function{ [](shared_flag_reader reader) { reader.wait(); } };

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

TEST(shared_flag_reader, waitThrowsLogicErrorIfSharedStateWasMovedAway)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    ASSERT_THROW(reader1.wait(), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// wait_for()

TEST(shared_flag_reader, waitForReturnsFalseIfFlagHasNotBeenSetBeforeTimeout)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    ASSERT_FALSE(reader.wait_for(10ms));
}

TEST(shared_flag_reader, waitForReturnsTrueIfFlagWasAlreadySet)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    flag.set();
    ASSERT_TRUE(reader.wait_for(10ms));
}

TEST(shared_flag_reader, waitForReturnsTrueIfFlagWasSetWhileWaiting)
{
    shared_flag flag;
    auto function{ [](shared_flag_reader reader) { return reader.wait_for(2s); } };
    auto task{ std::async(std::launch::async, function, flag) };

    std::this_thread::sleep_for(150ms);
    flag.set();
    ASSERT_TRUE(task.get());
}


TEST(shared_flag_reader, waitForSupportsMultipleThreadsWaitingOnTheSameFlagViaTheSameInstance)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    auto function{ [&]() { return reader.wait_for(2s); } };

    auto task1{ std::async(std::launch::async, function) };
    auto task2{ std::async(std::launch::async, function) };
    auto task3{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    ASSERT_TRUE(task1.get());
    ASSERT_TRUE(task2.get());
    ASSERT_TRUE(task3.get());
}

TEST(shared_flag_reader, waitForSupportsMultipleThreadsWaitingOnTheSameFlagViaDifferentInstances)
{
    auto function{ [](shared_flag_reader reader) { return reader.wait_for(2s); } };

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

TEST(shared_flag_reader, waitForThrowsLogicErrorIfSharedStateWasMovedAway)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    ASSERT_THROW(reader1.wait_for(10ms), std::logic_error);
}


//--------------------------------------------------------------------------------------------------
// wait_until()

TEST(shared_flag_reader, waitUntilReturnsFalseIfFlagHasNotBeenSetBeforeTimeout)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    ASSERT_FALSE(reader.wait_until(now() + 10ms));
}

TEST(shared_flag_reader, waitUntilReturnsTrueIfFlagWasAlreadySet)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    flag.set();
    ASSERT_TRUE(reader.wait_until(now() + 10ms));
}

TEST(shared_flag_reader, waitUntilReturnsTrueIfFlagWasSetWhileWaiting)
{
    shared_flag flag;
    auto function{ [](shared_flag_reader reader) { return reader.wait_until(now() + 2s); } };
    auto task{ std::async(std::launch::async, function, flag) };

    std::this_thread::sleep_for(150ms);
    flag.set();
    ASSERT_TRUE(task.get());
}

TEST(shared_flag_reader, waitUntilSupportsMultipleThreadsWaitingOnTheSameFlagViaTheSameInstance)
{
    shared_flag flag;
    shared_flag_reader reader{ flag };
    auto function{ [&]() { return reader.wait_until(now() + 2s); } };

    auto task1{ std::async(std::launch::async, function) };
    auto task2{ std::async(std::launch::async, function) };
    auto task3{ std::async(std::launch::async, function) };

    std::this_thread::sleep_for(150ms);
    flag.set();

    ASSERT_TRUE(task1.get());
    ASSERT_TRUE(task2.get());
    ASSERT_TRUE(task3.get());
}

TEST(shared_flag_reader, waitUntilSupportsMultipleThreadsWaitingOnTheSameFlagViaDifferentInstances)
{
    auto function{ [](shared_flag_reader reader) { return reader.wait_until(now() + 2s); } };

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

TEST(shared_flag_reader, waitUntilThrowsLogicErrorIfSharedStateWasMovedAway)
{
    shared_flag flag;
    shared_flag_reader reader1{ flag };
    shared_flag_reader reader2{ std::move(reader1) };
    ASSERT_THROW(reader1.wait_until(now() + 10ms), std::logic_error);
}

