/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "utilities/log/Logging.h"

#include <chrono>
#include <coro/io_scheduler.hpp>
#include <coro/mutex.hpp>

namespace bxt::Persistence::Box {
class WritebackScheduler {
public:
    explicit WritebackScheduler(std::shared_ptr<coro::io_scheduler> scheduler)
        : m_scheduler(std::move(scheduler)) {}

    coro::task<void> schedule(coro::task<void> task) {
        const auto lock = co_await m_mutex.lock();

        if (m_scheduled) { co_return; }

        this->m_scheduled = true;

        auto scheduled_task = [](auto* self,
                                 coro::task<void> task) -> coro::task<void> {
            using namespace std::chrono_literals;

            co_await self->m_scheduler->schedule_after(5s);

            const auto lock = co_await self->m_mutex.lock();

            co_await task;

            self->m_scheduled = false;

            co_return;
        }(this, std::move(task));

        m_scheduler->schedule(std::move(scheduled_task));

        co_return;
    }

    bool scheduled() { return m_scheduled; }

private:
    std::shared_ptr<coro::io_scheduler> m_scheduler;
    coro::mutex m_mutex;
    std::atomic<bool> m_scheduled = false;
};
} // namespace bxt::Persistence::Box
