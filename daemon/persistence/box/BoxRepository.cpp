/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "BoxRepository.h"

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Package.h"
#include "core/domain/entities/Section.h"
#include "core/domain/events/PackageEvents.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "core/domain/value_objects/PackagePoolEntry.h"
#include "persistence/box/BoxOptions.h"
#include "persistence/box/record/PackageRecord.h"
#include "persistence/box/record/RecordMapper.h"
#include "utilities/Error.h"
#include "utilities/StaticDTOMapper.h"
#include "utilities/alpmdb/Database.h"
#include "utilities/alpmdb/Desc.h"

#include <coro/sync_wait.hpp>
#include <expected>
#include <fmt/core.h>
#include <fmt/format.h>
#include <functional>
#include <kangaru/operator.hpp>
#include <ranges>
#include <string>
#include <vector>
namespace bxt::Persistence::Box {

BoxRepository::BoxRepository(BoxOptions options,
                             PackageStoreBase &package_store,
                             WritebackScheduler &writeback_sceduler,
                             ExporterBase &exporter)
    : m_options(std::move(options)),
      m_package_store(package_store),
      m_scheduler(writeback_sceduler),
      m_exporter(exporter) {};
coro::task<BoxRepository::TResult>
    BoxRepository::find_by_id_async(TId id,
                                    std::shared_ptr<UnitOfWorkBase> uow) {
}

coro::task<BoxRepository::TResult>
    BoxRepository::find_first_async(std::function<bool(const Package &)>,
                                    std::shared_ptr<UnitOfWorkBase> uow) {
}

coro::task<BoxRepository::TResults>
    BoxRepository::find_async(std::function<bool(const Package &)> condition,
                              std::shared_ptr<UnitOfWorkBase> uow) {
}

coro::task<BoxRepository::TResults>
    BoxRepository::all_async(std::shared_ptr<UnitOfWorkBase> uow) {
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::add_async(const std::vector<Package> entity,
                             std::shared_ptr<UnitOfWorkBase> uow) {
    auto tasks = entity | std::views::transform(RecordMapper::to_record)
                 | std::views::transform([&](auto &&record) {
                       return m_package_store.add(record, uow);
                   })
                 | std::ranges::to<std::vector>();

    auto added_list = co_await coro::when_all(std::move(tasks));
    for (const auto &added : added_list) {
        auto &added_result = added.return_value();
        if (!added_result.has_value()) {
            co_return bxt::make_error_with_source<WriteError>(
                std::move(added_result.error()), WriteError::OperationError);
        }
    }
    co_return {};
}
coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::add_async(const Package entity,
                             std::shared_ptr<UnitOfWorkBase> uow) {
    auto result =
        co_await m_package_store.add(RecordMapper::to_record(entity), uow);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<WriteError>(
            std::move(result.error()), WriteError::OperationError);
    }

    co_return {};
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::delete_async(const std::vector<TId> ids,
                                std::shared_ptr<UnitOfWorkBase> uow) {
    auto tasks = ids | std::views::transform([&](const auto &id) {
                     return m_package_store.delete_by_id(
                         PackageRecord::Id {
                             .section = SectionDTOMapper::to_dto(id.section),
                             .name = id.package_name},
                         uow);
                 })
                 | std::ranges::to<std::vector>();

    auto deleted_list = co_await coro::when_all(std::move(tasks));
    for (const auto &deleted : deleted_list) {
        auto &deleted_result = deleted.return_value();
        if (!deleted_result.has_value()) {
            co_return bxt::make_error_with_source<WriteError>(
                std::move(deleted_result.error()), WriteError::OperationError);
        }
    }
    co_return {};
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::delete_async(const TId id,
                                std::shared_ptr<UnitOfWorkBase> uow) {
    auto result = co_await m_package_store.delete_by_id(
        PackageRecord::Id {.section = SectionDTOMapper::to_dto(id.section),
                           .name = id.package_name},
        uow);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<WriteError>(
            std::move(result.error()), WriteError::OperationError);
    }
    co_return {};
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::update_async(const std::vector<Package> entity,
                                std::shared_ptr<UnitOfWorkBase> uow) {
    auto tasks = entity | std::views::transform(RecordMapper::to_record)
                 | std::views::transform([&](auto &&record) {
                       return m_package_store.update(record, uow);
                   })
                 | std::ranges::to<std::vector>();

    auto updated_list = co_await coro::when_all(std::move(tasks));
    for (const auto &updated : updated_list) {
        auto &updated_result = updated.return_value();
        if (!updated_result.has_value()) {
            co_return bxt::make_error_with_source<WriteError>(
                std::move(updated_result.error()), WriteError::OperationError);
        }
    }
    co_return {};
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::update_async(const Package entity,
                                std::shared_ptr<UnitOfWorkBase> uow) {
    auto result =
        co_await m_package_store.update(RecordMapper::to_record(entity), uow);
    if (!result.has_value()) {
        co_return bxt::make_error_with_source<WriteError>(
            std::move(result.error()), WriteError::OperationError);
    }
    co_return {};
}

coro::task<BoxRepository::TResults>
    BoxRepository::find_by_section_async(const Section section,
                                         std::shared_ptr<UnitOfWorkBase> uow) {
    auto packages = co_await m_package_store.find_by_section(
        SectionDTOMapper::to_dto(section), uow);

    if (!packages.has_value()) {
        co_return bxt::make_error_with_source<ReadError>(
            std::move(packages.error()), ReadError::InvalidArgument);
    };

    std::vector<Core::Domain::Package> result;
    result.reserve(packages->size());

    for (const auto &package : *packages) {
        auto entity = RecordMapper::to_entity(package);

        result.emplace_back(entity);
    }

    co_return result;
}

coro::task<BoxRepository::TResults> BoxRepository::find_by_section_async(
    const Section section,
    const std::function<bool(const Package &)> predicate,
    std::shared_ptr<UnitOfWorkBase> uow) {
    auto packages = co_await m_package_store.find_by_section(
        SectionDTOMapper::to_dto(section), uow);

    if (!packages.has_value()) {
        co_return bxt::make_error_with_source<ReadError>(
            std::move(packages.error()), ReadError::InvalidArgument);
    };

    std::vector<Core::Domain::Package> result;
    result.reserve(packages->size());

    for (const auto &package : *packages) {
        auto entity = RecordMapper::to_entity(package);

        if (predicate(entity)) { result.emplace_back(entity); }
    }

    co_return result;
}

coro::task<BoxRepository::TResult>
    BoxRepository::find_by_section_async(const Section section,
                                         const Name name,
                                         std::shared_ptr<UnitOfWorkBase> uow) {
    std::optional<Core::Domain::Package> result;

    co_await m_package_store.accept(
        [&result](std::string_view key, const auto &record) {
            result = RecordMapper::to_entity(record);

            return Utilities::NavigationAction::Stop;
        },
        fmt::format("{}/{}", section.string(), name), uow);

    if (result.has_value()) {
        co_return *result;
    } else {
        co_return bxt::make_error<ReadError>(ReadError::EntityNotFound);
    }
}

} // namespace bxt::Persistence::Box
