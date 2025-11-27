/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <functional>

#include <coro/io_scheduler.hpp>
#include <coro/mutex.hpp>
#include <coro/shared_mutex.hpp>

namespace bxt::utils {

template<typename T,
         typename TUnlocker = std::function<void(T&)>,
         typename TLock = coro::scoped_lock>
class Locked {
public:
    template<typename... Args>
    explicit Locked(TLock lock, TUnlocker deleter = TUnlocker {}, Args&&... args)
        : m_lock(std::move(lock))
        , m_value(std::forward<Args>(args)...)
        , m_unlocker(std::move(deleter)) {
    }

    ~Locked() {
        unlock();
    }

    Locked(Locked const&) = delete;
    Locked& operator=(Locked const&) = delete;

    Locked(Locked&&) = default;
    Locked& operator=(Locked&&) = default;

    T* operator->() {
        return &m_value;
    }
    T const* operator->() const {
        return &m_value;
    }

    T& operator*() {
        return m_value;
    }
    T const& operator*() const {
        return m_value;
    }

    void unlock() {
        if (m_unlocked) {
            return;
        }
        m_unlocked = true;

        m_lock.unlock();
        if constexpr (std::is_member_pointer_v<TUnlocker>) {
            if (m_unlocker) {
                (m_value.*m_unlocker)();
            }
        } else {
            if (m_unlocker) {
                m_unlocker(m_value);
            }
        }
    }

private:
    TLock m_lock;
    T m_value;
    TUnlocker m_unlocker;
    bool m_unlocked {false};
};

template<typename T, typename TUnlocker = void (T::*)()>
using SharedLocked = Locked<T, TUnlocker, coro::shared_scoped_lock<coro::io_scheduler>>;

} // namespace bxt::utils
