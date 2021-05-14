/**
 * @file shared_flag_reader.hpp
 * @brief Declares a class which can read the state of a shared flag.
 * @author Peter Bloomfield (https://peter.bloomfield.online)
 * @copyright MIT License
 */

#ifndef PRB_SHARED_FLAG_READER_HPP_INCLUDED
#define PRB_SHARED_FLAG_READER_HPP_INCLUDED

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

namespace prb
{
    /**
     * A synchronisation structure which can read and wait on the state of a shared boolean flag.
     * This is useful for receiving a one-off signal from another thread, such as a signal to shut
     *  down.
     * 
     * Note that this class cannot be used in isolation as it can only read the state of a flag.
     * There must be a separate instance of shared_flag which can write to the same flag. The shared
     *  state stored in the shared_flag instance can be copied to shared_flag_reader via copy
     *  construction or assignment. Any number of instances can refer to the same flag.
     * 
     * It is not possible to construct or assign a shared_flag from a shared_flag_reader. This is to
     *  prevent "promoting" a flag from read-only to writeable.
     * 
     * Example of using the flag to terminate a worker thread:
     * 
     * @code
     *      auto backgroundTask = [](shared_flag_reader flagReader)
     *      {
     *          // Keep looping until signalled to stop.
     *          while (!flagReader.waitFor(1s))
     *          {
     *              // Do regular work in the background here.
     *          }
     *      };
     *      
     *      // Start the thread, and give it read-only access to the flag.
     *      shared_flag flag;
     *      std::thread t{ backgroundTask, flag };
     * 
     *      // Do some other long-running task here.
     * 
     *      // Signal the thread to stop.
     *      flag.set();
     *      t.join();
     * @endcode
     * 
     * @note This class is thread safe, meaning multiple threads can query and wait on the same flag
     *  at the same time via the same instance. However, for simplicity, it's generally recommended
     *  that each thread gets its own copy.
     */
    class shared_flag_reader
    {
    public:
        //------------------------------------------------------------------------------------------
        // Construction / destruction.

        /**
         * Copy constructor -- copies a reference to the shared state of an existing instance.
         * Afterwards, this instance and the other instance will both have a reference to the same
         *  shared state. That means both can query and wait on the same flag.
         * 
         * @param other An existing instance to copy a shared state reference from. This can be an
         *  instance of shared_flag or shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has been moved away.
         * 
         * @note This will not block if another thread is waiting on the other instance.
         */
        shared_flag_reader(const shared_flag_reader & other);

        /**
         * Copy assignment -- copies a reference to the shared state of an existing instance.
         * Afterwards, this instance and the other instance will both have a reference to the same
         *  shared state. That means both can query and wait on the same flag.
         * If this instance previously had a reference to a shared state then it will have been
         *  released first.
         * 
         * @param other An existing instance to copy a shared state reference from. This can be an
         *  instance of shared_flag or shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @return Returns a reference to this instance.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has been moved away.
         * 
         * @warning If another thread is waiting on this instance (by calling one of the wait*()
         *  functions) then this function will block until the wait has finished.
         * @note This will not block if another thread is waiting on the other instance.
         */
        shared_flag_reader & operator=(const shared_flag_reader & other);

        /**
         * Move constructor -- acquires the shared state reference from another instance.
         * Afterwards, the other instance will no longer have a reference to the shared state. It
         *  will have been transferred to this instance. The other instance cannot be used after
         *  that unless another reference is copied or assigned into it.
         * 
         * @param other An existing instance to move a shared state reference from. This can be an
         *  instance of shared_flag or shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has already been moved away.
         * 
         * @warning If another thread is waiting on the existing instance (by calling one of the
         *  wait*() functions) then this function will block until the wait has finished.
         */
        shared_flag_reader(shared_flag_reader && other);

        /**
         * Move assignment -- acquires the shared state reference from another instance.
         * Afterwards, the other instance will no longer have a reference to the shared state. It
         *  will have been transferred to this instance. The other instance cannot be used after
         *  that unless another reference is copied or assigned into it.
         * If this instance previously had a reference to a shared state then it will have been
         *  released first.
         * 
         * @param other An existing instance to move a shared state reference from. This can be an
         *  instance of shared_flag or shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @return Returns a reference to this instance.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has already been moved away.
         * 
         * @warning If another thread is waiting on the existing instance (by calling one of the
         *  wait*() functions) then this function will block until the wait has finished.
         */
        shared_flag_reader & operator=(shared_flag_reader && other);

        /**
         * The destructor releases this instance's reference to the shared state, if it has one.
         * If it was the last reference to the shared state then the state is deleted.
         */
        virtual ~shared_flag_reader();


        //------------------------------------------------------------------------------------------
        // Accessors / operations.

        /**
         * Check if this instance contains a reference to a shared state.
         * Calls to isSet(), wait(), waitFor(), and waitUntil() will fail if there is no reference
         *  to a shared state. This will happen if the contents of this object have been moved away.
         * 
         * @return Returns true if this object contains a reference to a shared state. Returns false
         *  if the reference has been moved away.
         * 
         * @note It's possible to assign a new reference into an object after the contents have been
         *  moved away.
         */
        bool valid() const noexcept;

        /**
         * Check if the flag has been set.
         * 
         * @return Returns true if the flag has been set. Returns false otherwise.
         * @throw std::logic_error This instance does not contain a reference to a shared state.
         *  This happens if the contents of this object have been moved away.
         * 
         * @note This will not block if another thread is currently waiting on this instance.
         */
        bool get() const;

        /**
         * Check if the flag has been set.
         * This is a convenience wrapper around get(). It allows this object to be used as part of a
         *  boolean condition.
         * 
         * Example usage:
         * 
         *      shared_flag_reader flag{ ... };
         *      if (flag)
         *          doSomething();
         * 
         * @return Returns true if the flag has been set. Returns false otherwise.
         * @throw std::logic_error This instance does not contain a reference to a shared state.
         *  This happens if the contents of this object have been moved away.
         * 
         * @note This will not block if another thread is currently waiting on this instance.
         */
        operator bool() const;

        /**
         * Block the current thread until the flag has been set.
         * This will return immediately if the flag was already set.
         * 
         * @throw std::logic_error This instance does not contain a reference to a shared state.
         *  This happens if the contents of this object have been moved away.
         * 
         * @warning If the flag is not set, and the only remaining objects referencing it are
         *  shared_flag_reader instances, then the flag can never be set. That means this function
         *  will block indefinitely. It is the application's responsibility to avoid this.
         * @note It is safe to have multiple theads waiting on the same instance at the same time.
         */
        void wait() const;

        /**
         * Block the current thread until the flag has been set or the specified time has elapsed.
         * This will return immediately if the flag was already set.
         * 
         * @param timeoutDuration The maximum period of time to block for. If this time elapses
         *  before the flag has been set then the function will return false.
         * @return Returns true if the flag has been set. Returns false if the flag had not been set
         *  when the timeout expired.
         * @throw std::logic_error This instance does not contain a reference to a shared state.
         *  This happens if the contents of this object have been moved away.
         * 
         * @note It is safe to have multiple theads waiting on the same instance at the same time.
         */
        template <class Rep, class Period>
        bool wait_for(const std::chrono::duration<Rep, Period> & timeoutDuration) const;

        /**
         * Block the current thread until the flag has been set or the specified time is reached.
         * This will return immediately if the flag was already set.
         * 
         * @param timeoutTime The maximum time point to block until. If this time point is reached
         *  before the flag has been set then the function will return false.
         * @return Returns true if the flag has been set. Returns false if the flag had not been set
         *  when the time point was reached.
         * @throw std::logic_error This instance does not contain a reference to a shared state.
         *  This happens if the contents of this object have been moved away.
         * 
         * @note It is safe to have multiple theads waiting on the same instance at the same time.
         */
        template <class Clock, class Duration>
        bool wait_until(const std::chrono::time_point<Clock,Duration> & timeoutTime) const;

    protected:
        //------------------------------------------------------------------------------------------
        // Internal operations.
    
        /**
         * Default construction of shared_flag_reader is not permitted.
         * It must be initialised from an existing instance of shared_flag_reader or shared_flag.
         * This constructor is only defined here so that the shared_flag sub-class can be
         *  default-constructed.
         */
        shared_flag_reader() = default;


        //------------------------------------------------------------------------------------------
        // Data.

        /**
         * This mutex protects access to the m_state pointer.
         * It must be locked whenever anything uses or changes the pointer.
         * To avoid deadlock, instances must lock this mutex before locking the mutex within the
         *  shared state.
         * If an instance is waiting on the flag then it must retain a shared lock on this mutex
         *  until it has finished waiting. This ensures the state is not destroyed during the wait.
         */
        mutable std::shared_mutex m_statePointerMutex;

        // Forward declaration to the shared state structure.
        struct state;

        /**
         * A pointer to the shared state referenced by this instance.
         * This will be null if this instance has no shared state. This can happen if a
         *  shared_flag_reader was default-constructed, or the shared state was moved away.
         * 
         * Access to this variable is protected by m_statePointerMutex.
         * 
         * @todo Manage this manually in future so that we can count the number of remaining writers
         */
        std::shared_ptr<state> m_state;
    };

    /**
     * Contains the shared state referenced by shared_flag_reader and shared_flag instances.
     * This contains the flag value and the synchronisation primitives which are waited-upon.
     */
    struct shared_flag_reader::state
    {
        /**
         * Protects access to m_conditionVariable and m_flag.
         * To avoid deadlock, instances of shared_flag_reader and shared_flag must always lock
         *  their own m_statePointerMutex before locking m_stateContentMutex.
         */
        mutable std::mutex m_stateContentMutex;

        /**
         * Allows threads to wait on the flag value and be notified when it changes.
         * 
         * This is protected by m_stateContentMutex. Threads waiting on this must lock that
         *  mutex.
         */
        std::condition_variable m_conditionVariable;

        /**
         * Indicates if the flag is has been set.
         * When this has been set to true, it should never return to false.
         * 
         * This is protected by m_stateContentMutex.
         */
        bool m_flag{ false };
    };


    //----------------------------------------------------------------------------------------------
    // Template implementations.

    template <class Rep, class Period>
    bool shared_flag_reader::wait_for(const std::chrono::duration<Rep, Period> & timeoutDuration) const
    {
        std::shared_lock<decltype(m_statePointerMutex)> outerLock{ m_statePointerMutex };
        if (!m_state)
            throw std::logic_error{ "Shared state has been moved away." };

        std::unique_lock<decltype(state::m_stateContentMutex)> innerLock{ m_state->m_stateContentMutex };
        m_state->m_conditionVariable.wait_for(innerLock, timeoutDuration, [this]{ return m_state->m_flag; });
        return m_state->m_flag;
    }

    template <class Clock, class Duration>
    bool shared_flag_reader::wait_until(const std::chrono::time_point<Clock,Duration> & timeoutTime) const
    {
        std::shared_lock<decltype(m_statePointerMutex)> outerLock{ m_statePointerMutex };
        if (!m_state)
            throw std::logic_error{ "Shared state has been moved away." };

        std::unique_lock<decltype(state::m_stateContentMutex)> innerLock{ m_state->m_stateContentMutex };
        m_state->m_conditionVariable.wait_until(innerLock, timeoutTime, [this]{ return m_state->m_flag; });
        return m_state->m_flag;
    }
}

#endif
