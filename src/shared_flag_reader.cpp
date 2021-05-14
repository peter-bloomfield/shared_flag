/**
 * @file shared_flag_reader.cpp
 * @brief Defines a class which can read the state of a shared flag.
 * @author Peter Bloomfield (https://peter.bloomfield.online)
 * @copyright MIT License
 */

#include "shared_flag/shared_flag_reader.hpp"
#include <utility>

namespace prb
{
    //----------------------------------------------------------------------------------------------
    // Construction / destruction.

    shared_flag_reader::shared_flag_reader(const shared_flag_reader & other)
    {
        *this = other;
    }

    shared_flag_reader & shared_flag_reader::operator=(const shared_flag_reader & other)
    {
        std::unique_lock thisLock{ m_state_ptr_mtx, std::defer_lock };
        std::shared_lock otherLock{ other.m_state_ptr_mtx, std::defer_lock };
        std::lock(thisLock, otherLock);

        if (!other.m_state)
            throw std::logic_error{ "Shared state has been moved away." };

        m_state = other.m_state;
        return *this;
    }

    shared_flag_reader::shared_flag_reader(shared_flag_reader && other)
    {
        *this = std::move(other);
    }

    shared_flag_reader & shared_flag_reader::operator=(shared_flag_reader && other)
    {
        if (this == &other)
            return *this;

        std::unique_lock thisLock{ m_state_ptr_mtx, std::defer_lock };
        std::unique_lock otherLock{ other.m_state_ptr_mtx, std::defer_lock };
        std::lock(thisLock, otherLock);

        if (!other.m_state)
            throw std::logic_error{ "Shared state has been moved away." };

        m_state = std::move(other.m_state);
        return *this;
    }
    
    shared_flag_reader::~shared_flag_reader()
    {
    }


    //----------------------------------------------------------------------------------------------
    // Accessors / operations.

    bool shared_flag_reader::valid() const noexcept
    {
        std::shared_lock lock{ m_state_ptr_mtx };
        return m_state != nullptr;
    }

    bool shared_flag_reader::get() const
    {
        std::shared_lock outerLock{ m_state_ptr_mtx };
        if (!m_state)
            throw std::logic_error{ "Shared state has been moved away." };

        std::lock_guard innerLock{ m_state->m_state_data_mtx };
        return m_state->m_flag;
    }

    shared_flag_reader::operator bool() const
    {
        return get();
    }

    void shared_flag_reader::wait() const
    {
        std::shared_lock outerLock{ m_state_ptr_mtx };
        if (!m_state)
            throw std::logic_error{ "Shared state has been moved away." };

        std::unique_lock innerLock{ m_state->m_state_data_mtx };
        m_state->m_cond_var.wait(innerLock, [this]{ return m_state->m_flag; });
    }
}
