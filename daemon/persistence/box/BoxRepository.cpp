/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "BoxRepository.h"

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Package.h"
#include "core/domain/entities/Section.h"
#include "core/domain/events/PackageEvents.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/value_objects/PackagePoolEntry.h"
#include "persistence/box/BoxOptions.h"
#include "persistence/box/record/PackageRecord.h"
#include "persistence/box/record/RecordMapper.h"
#include "utilities/Error.h"
#include "utilities/StaticDTOMapper.h"
#include "utilities/alpmdb/Database.h"
#include "utilities/alpmdb/Desc.h"

#include <coro/sync_wait.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <functional>
#include <kangaru/operator.hpp>
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

coro::task<BoxRepository::TResult> BoxRepository::find_by_id_async(TId id) {
}

coro::task<BoxRepository::TResult>
    BoxRepository::find_first_async(std::function<bool(const Package &)>) {
}

coro::task<BoxRepository::TResults>
    BoxRepository::find_async(std::function<bool(const Package &)> condition) {
}

coro::task<BoxRepository::TResults> BoxRepository::all_async() {
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::add_async(const Package entity) {
    m_to_add.emplace_back(entity);
    co_return {};
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::remove_async(const TId entity) {
    m_to_remove.emplace_back(entity);
    co_return {};
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::add_async(const std::vector<Package> entities) {
    m_to_add.insert(m_to_add.end(), entities.begin(), entities.end());
    co_return {};
}

coro::task<BoxRepository::WriteResult<void>>
    BoxRepository::update_async(const Package entity) {
    m_to_update.emplace_back(entity);
    co_return {};
}

coro::task<BoxRepository::TResults>
    BoxRepository::find_by_section_async(const Core::Domain::Section section) {
    auto packages = co_await m_package_store.find_by_section(
        SectionDTOMapper::to_dto(section));

    if (!packages.has_value()) {
        co_return bxt::make_error_with_source<ReadError>(
            std::move(packages.error()),
            ReadError::ErrorTypes::InvalidArgument);
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
    const std::function<bool(const Package &)> predicate) {
    auto packages = co_await m_package_store.find_by_section(
        SectionDTOMapper::to_dto(section));

    if (!packages.has_value()) {
        co_return bxt::make_error_with_source<ReadError>(
            std::move(packages.error()),
            ReadError::ErrorTypes::InvalidArgument);
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
                                         const Name name) {
    std::optional<Core::Domain::Package> result;

    co_await m_package_store.accept(
        [&result](std::string_view key, const auto &record) {
            result = RecordMapper::to_entity(record);

            return Utilities::NavigationAction::Stop;
        },
        fmt::format("{}/{}", section.string(), name));

    if (result.has_value()) {
        co_return *result;
    } else {
        co_return bxt::make_error<ReadError>(
            ReadError::ErrorTypes::EntityNotFound);
    }
}

coro::task<UnitOfWorkBase::Result<void>> BoxRepository::commit_async() {
    std::vector<coro::task<nonstd::expected<void, DatabaseError>>> tasks;

    std::set<PackageSectionDTO> added_sections;

    for (const auto &entity : m_to_add) {
        auto record = RecordMapper::to_record(entity);
        tasks.emplace_back(m_package_store.add(record));

        added_sections.emplace(record.id.section);

        auto event = std::make_shared<Events::PackageAdded>(entity);

        m_event_store.emplace_back(event);
    }

    phmap::node_hash_map<PackageSectionDTO, std::set<std::string>>
        names_to_remove;

    /// TODO: Implement update and remove

    co_await coro::when_all(std::move(tasks));

    m_exporter.add_dirty_sections(std::move(added_sections));

    co_await m_scheduler.schedule([this]() -> coro::task<void> {
        co_await m_exporter.export_to_disk();
        co_return;
    }());

    m_to_add.clear();
    m_to_remove.clear();
    m_to_update.clear();

    co_return {};
}

coro::task<UnitOfWorkBase::Result<void>> BoxRepository::rollback_async() {
    m_to_add.clear();
    m_to_remove.clear();
    m_to_update.clear();

    co_return {};
}

} // namespace bxt::Persistence::Box
