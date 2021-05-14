/**
 * @file shared_flag.cpp
 * @brief Defines a class which can read and write the state of a shared flag.
 * @author Peter Bloomfield (https://peter.bloomfield.online)
 * @copyright MIT License
 */

#include "shared_flag/shared_flag.hpp"
#include <utility>

namespace prb
{
    //----------------------------------------------------------------------------------------------
    // Construction / destruction.

    shared_flag::shared_flag()
    {
        m_state = std::make_shared<state>();
    }

    shared_flag::shared_flag(const shared_flag & other) : shared_flag_reader(other)
    {
    }

    shared_flag & shared_flag::operator=(const shared_flag & other)
    {
        shared_flag_reader::operator=(other);
        return *this;
    }

    shared_flag::shared_flag(shared_flag && other) : shared_flag_reader(std::move(other))
    {
    }

    shared_flag & shared_flag::operator=(shared_flag && other)
    {
        shared_flag_reader::operator=(std::move(other));
        return *this;
    }

    shared_flag::~shared_flag()
    {
    }


    //----------------------------------------------------------------------------------------------
    // Accessors / operations.

    void shared_flag::set()
    {
        std::shared_lock<decltype(m_statePointerMutex)> outerLock{ m_statePointerMutex };
        if (!m_state)
            throw std::logic_error{ "Shared state has been moved away." };

        std::unique_lock<decltype(state::m_stateContentMutex)> innerLock{ m_state->m_stateContentMutex };
        if (!m_state->m_flag)
        {
            m_state->m_flag = true;
            innerLock.unlock();
            m_state->m_conditionVariable.notify_all();
        }
    }
}
