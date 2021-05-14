/**
 * @file shared_flag.hpp
 * @brief Declares a class which can read and write the state of a shared flag.
 * @author Peter Bloomfield (https://peter.bloomfield.online)
 * @copyright MIT License
 */

#ifndef PRB_SHARED_FLAG_HPP_INCLUDED
#define PRB_SHARED_FLAG_HPP_INCLUDED

#include "shared_flag_reader.hpp"

namespace prb
{
    /**
     * A synchronisation structure which can set, query, and wait on the state of a shared boolean flag.
     * This is useful for sending a one-off signal between threads, such as a signal to shut down.
     * 
     * The flag itself is stored in a shared state which can be referenced by multiple instances.
     * The reference can be shared with other instances via copy construction or assignment. Any
     *  number of instances can refer to the same flag.
     * 
     * The flag can never be reset. However, existing instances of shared_flag (and shared_flag_reader)
     *  can be reassigned so that they refer to a different shared state. This allows them to be
     *  reused, but the reassignment is not propagated to other instances referencing the same
     *  shared state.
     * 
     * Example of using the flag to terminate a worker thread:
     * 
     * @code
     *      auto task = [](shared_flag_reader flag)
     *      {
     *          // Keep looping until signalled to stop.
     *          while (!flag.wait_for(1s))
     *          {
     *              // Do regular work in the background here.
     *          }
     *      };
     *      
     *      // Start the thread, and give it read-only access to the flag.
     *      shared_flag flag;
     *      std::thread task_thread{ task, flag };
     * 
     *      // Do some other long-running task here.
     * 
     *      // Signal the thread to stop.
     *      flag.set();
     *      task_thread.join();
     * @endcode
     * 
     * @note This class is thread-safe, meaning multiple threads can safely set, query, and wait on
     *  the flag at the same time via the same instance of shared_flag. However, for simplicity, it's
     *  generally recommended that each thread or component receives its own copy.
     */
    class shared_flag final : public shared_flag_reader
    {
    public:
        //------------------------------------------------------------------------------------------
        // Construction / destruction.

        /**
         * Default constructor -- generates and stores a reference to a new shared state.
         * Initially, no other objects will be have a reference to the new shared state. In order to
         *  set, query, or wait on the same flag from other instances, they will have to be
         *  constructed or assigned from this instance.
         */
        shared_flag();

        /**
         * Copy constructor -- copies a reference to the shared state of an existing instance.
         * Afterwards, this instance and the other instance will both have a reference to the same
         *  shared state. That means both can set, query, and wait on the same flag.
         * 
         * @param other An existing instance to copy a shared state reference from. This must be an
         *  instance of shared_flag, not shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has been moved away.
         * 
         * @note This will not block if another thread is waiting on the other instance.
         */
        shared_flag(const shared_flag & other);

        /**
         * Copy assignment -- copies a reference to the shared state of an existing instance.
         * Afterwards, this instance and the other instance will both have a reference to the same
         *  shared state. That means both can set, query, and wait on the same flag.
         * If this instance previously had a reference to a shared state then it will have been
         *  released first.
         * 
         * @param other An existing instance to copy a shared state reference from. This must be an
         *  instance of shared_flag, not shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @return Returns a reference to this instance.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has been moved away.
         * 
         * @warning If another thread is waiting on this instance (by calling one of the wait*()
         *  functions) then this function will block until the wait has finished.
         * @note This will not block if another thread is waiting on the other instance.
         */
        shared_flag & operator=(const shared_flag & other);

        /**
         * Move constructor -- acquires the shared state reference from another instance.
         * Afterwards, the other instance will no longer have a reference to the shared state. It
         *  will have been transferred to this instance. The other instance cannot be used after
         *  that unless another reference is copied or assigned into it.
         * 
         * @param other An existing instance to move a shared state reference from. This must be an
         *  instance of shared_flag, not shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has already been moved away.
         * 
         * @warning If another thread is waiting on the existing instance (by calling one of the
         *  wait*() functions) then this function will block until the wait has finished.
         */
        shared_flag(shared_flag && other);

        /**
         * Move assignment -- acquires the shared state reference from another instance.
         * Afterwards, the other instance will no longer have a reference to the shared state. It
         *  will have been transferred to this instance. The other instance cannot be used after
         *  that unless another reference is copied or assigned into it.
         * If this instance previously had a reference to a shared state then it will have been
         *  released first.
         * 
         * @param other An existing instance to move a shared state reference from. This must be an
         *  instance of shared_flag, not shared_flag_reader. It must contain a reference to a shared
         *  state; i.e. it must not have been moved away.
         * @return Returns a reference to this instance.
         * @throw std::logic_error The other instance does not have a reference to a shared state.
         *  This happens if it has already been moved away.
         * 
         * @warning If another thread is waiting on the existing instance (by calling one of the
         *  wait*() functions) then this function will block until the wait has finished.
         */
        shared_flag & operator=(shared_flag && other);

        /// Promoting a shared_flag_reader to a shared_flag is not permitted.
        shared_flag(const shared_flag_reader &) = delete;

        /// Promoting a shared_flag_reader to a shared_flag is not permitted.
        shared_flag & operator=(const shared_flag_reader &) = delete;

        /// Promoting a shared_flag_reader to a shared_flag is not permitted.
        shared_flag(shared_flag_reader &&) = delete;

        /// Promoting a shared_flag_reader to a shared_flag is not permitted.
        shared_flag & operator=(shared_flag_reader &&) = delete;

        /**
         * The destructor releases this instance's reference to the shared state, if it has one.
         * If it was the last reference to the shared state then the state is deleted.
         */
        ~shared_flag() override;


        //------------------------------------------------------------------------------------------
        // Accessors / operations.

        /**
         * Set the flag and wake any threads which are waiting on it.
         * This does nothing if the flag was already set.
         * 
         * @throw std::logic_error This instance does not have a reference to a shared state. This
         *  happens if it has been moved away.
         * 
         * @note This can only wake threads which are waiting on the same flag; i.e. they must have
         *  a reference to the same shared state.
         */
        void set();
    };
}

#endif
