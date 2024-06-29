/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "Pool.h"

#include "PoolOptions.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/enums/PoolLocation.h"
#include "coro/sync_wait.hpp"
#include "persistence/box/record/PackageRecord.h"
#include "utilities/Error.h"
#include "utilities/log/Logging.h"
#include "utilities/to_string.h"

#include <algorithm>
#include <expected>
#include <filesystem>
#include <fmt/core.h>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

std::expected<void, std::error_code>
    move_file(const std::filesystem::path& from,
              const std::filesystem::path& to) {
    std::error_code ec;

    std::filesystem::rename(from, to, ec);

    // try copy + remove original
    if (ec) {
        std::filesystem::copy_file(
            from.lexically_normal(), to.lexically_normal(),
            std::filesystem::copy_options::overwrite_existing, ec);
        if (!ec) { std::filesystem::remove(from, ec); }
    }
    if (ec.value() != 0) {
        return std::unexpected(ec);
    } else {
        return {};
    }
}

namespace bxt::Persistence::Box {

std::string
    Pool::format_target_path(Core::Domain::PoolLocation location,
                             const std::string& arch,
                             const std::optional<std::string>& filename) const {
    std::string template_string = "{location}/{arch}";

    if (m_options.templates.contains({arch})) {
        template_string = m_options.templates[arch];
    }

    const auto location_path = bxt::to_string(location);

    const auto prefix =
        fmt::format(fmt::runtime(template_string),
                    fmt::arg("location", std::string(location_path.data(),
                                                     location_path.size())),
                    fmt::arg("arch", arch));
    if (filename) {
        return std::filesystem::absolute(
            fmt::format("{}/{}/{}", m_pool_path.string(), prefix, *filename));
    } else {
        return std::filesystem::absolute(
            fmt::format("{}/{}", m_pool_path.string(), prefix));
    }
}

Pool::Pool(BoxOptions& box_options,
           PoolOptions& options,
           ReadOnlyRepositoryBase<Section>& section_repository,
           UnitOfWorkBaseFactory& uow_factory)
    : m_pool_path(box_options.box_path / "pool"),
      m_options(options),
      m_uow_factory(uow_factory) {
    auto uow = coro::sync_wait(m_uow_factory());
    const auto sections = coro::sync_wait(section_repository.all_async(uow));

    if (!sections.has_value()) {
        loge("Pool: Can't get available sections, the reason is \"{}\". "
             "Exiting.",
             sections.error().what());
        exit(1);
    }

    std::ranges::transform(
        *sections, std::inserter(m_architectures, m_architectures.end()),
        [](const auto& section) {
            return SectionDTOMapper::to_dto(section).architecture;
        });

    std::error_code ec;
    for (const auto& [location, _] : Core::Domain::pool_location_names) {
        for (const auto& architecture : m_architectures) {
            const auto target = format_target_path(location, architecture);

            std::filesystem::create_directories(target, ec);

            if (ec) {
                loge("Pool: Cannot create directories, the error is \"{}\". "
                     "Exiting.",
                     ec.message());
                exit(1);
            }
        }
    }
}
Pool::Result<PackageRecord> Pool::move_to(const PackageRecord& package) {
    PackageRecord result = package;
    for (auto& [location, description] : result.descriptions) {
        std::error_code ec;
        auto canonical_path =
            std::filesystem::weakly_canonical(description.filepath, ec);
        if (ec) { return bxt::make_error<FsError>(ec); }

        std::filesystem::path target = std::filesystem::weakly_canonical(
            format_target_path(location, package.id.section.architecture,
                               canonical_path.filename().string()));

        move_file(canonical_path, target);
        logd("Pool: Moving file from {} to {}", canonical_path.string(),
             target.string());

        description.filepath = target;

        m_pool_package_link_counts.lazy_emplace_l(
            target,
            [&](auto& p) {
                p.second += 1;
                logd("Pool: Incrementing link count for {}", target.string());
            },
            [&](const auto& ctor) {
                ctor(target, 1);
                logd("Pool: Adding new link count for {}", target.string());
            });

        if (!description.signature_path.has_value()) { continue; }

        std::filesystem::path signature_target = format_target_path(
            location, package.id.section.architecture,
            fmt::format("{}.sig", target.filename().string()));

        move_file(*description.signature_path, signature_target);
        logd("Pool: Moving signature file from {} to {}",
             description.signature_path->string(), signature_target.string());

        description.signature_path = signature_target;
    }
    return result;
}

PoolBase::Result<void> Pool::remove(const PackageRecord& package) {
    std::error_code ec;
    for (const auto& [location, description] : package.descriptions) {
        std::error_code ec;
        auto canonical_path =
            std::filesystem::weakly_canonical(description.filepath, ec);
        if (ec) { return bxt::make_error<FsError>(ec); }

        auto has_value = m_pool_package_link_counts.modify_if(
            canonical_path, [&](auto& count) {
                if (count.second > 0) {
                    count.second -= 1;
                    logd("Pool: Decrementing link count for {}",
                         canonical_path.string());
                } else {
                    logw("Pool: {} is being removed but the link count already "
                         "was 0. "
                         "Removing anyway.",
                         canonical_path.string());
                }

                if (count.second != 0) { return; }

                logd("Pool: No more links for {}, removing",
                     canonical_path.string());

                std::filesystem::remove(canonical_path, ec);
                if (ec) {
                    loge("Pool: Failed to remove file {}, error: {}",
                         canonical_path.string(), ec.message());
                    return;
                }
                logd("Pool: Removed file {}", canonical_path.string());

                if (!description.signature_path.has_value()) { return; }

                std::filesystem::remove(*description.signature_path, ec);
                if (ec) {
                    loge("Pool: Failed to remove signature file {}, error: {}",
                         description.signature_path->string(), ec.message());
                    return;
                }
                logd("Pool: Removed signature file {}",
                     description.signature_path->string());
            });

        if (has_value && m_pool_package_link_counts[canonical_path] == 0) {
            m_pool_package_link_counts.erase(canonical_path);
        }
    }

    return {};
}

PoolBase::Result<PackageRecord>
    Pool::path_for_package(const PackageRecord& package) const {
    PackageRecord result = package;
    for (auto& [location, description] : result.descriptions) {
        description.filepath = std::filesystem::weakly_canonical(
            format_target_path(location, package.id.section.architecture,
                               description.filepath.filename().string()));

        if (description.signature_path.has_value()) {
            description.signature_path =
                std::filesystem::weakly_canonical(format_target_path(
                    location, package.id.section.architecture,
                    fmt::format("{}.sig",
                                description.filepath.filename().string())));
        }
    }
    return result;
}

void Pool::count_links(PackageRepositoryBase& package_repository) {
    auto packages = coro::sync_wait(
        package_repository.all_async(coro::sync_wait(m_uow_factory())));

    if (!packages.has_value()) {
        loge("Pool: Can't get available packages, the reason is \"{}\". ",
             packages.error().what());
        exit(1);
    }

    for (const auto& package : *packages) {
        for (const auto& [location, desc] : package.pool_entries()) {
            std::error_code ec;
            auto canonical_path =
                std::filesystem::canonical(desc.file_path(), ec);
            if (ec) {
                loge("Pool: Can't get canonical path for {}, the reason is "
                     "\"{}\". ",
                     desc.file_path().string(), ec.message());
                exit(1);
            }
            m_pool_package_link_counts.lazy_emplace_l(
                canonical_path, [&](auto& p) { p.second += 1; },
                [&](const auto& ctor) { ctor(canonical_path, 1); });
        }
    }
    logd("Pool: Counted links for {} paths", m_pool_package_link_counts.size());
}

} // namespace bxt::Persistence::Box
