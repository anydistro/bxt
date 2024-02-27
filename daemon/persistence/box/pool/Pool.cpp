/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
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
#include <filesystem>
#include <fmt/core.h>
#include <iterator>
#include <nonstd/expected.hpp>
#include <string>
#include <system_error>
#include <vector>

nonstd::expected<void, std::error_code>
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
        return nonstd::make_unexpected(ec);
    } else {
        return {};
    }
}

namespace bxt::Persistence::Box {

std::string
    Pool::format_target_path(Core::Domain::PoolLocation location,
                             const std::string& arch,
                             const std::optional<std::string>& filename) {
    std::string template_string = "{location}/{arch}";

    if (m_options.templates.contains(arch)) {
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

Pool::Pool(PoolOptions options,
           ReadOnlyRepositoryBase<Section>& section_repository,
           const std::filesystem::path pool_path)
    : m_pool_path(std::move(pool_path)), m_options(std::move(options)) {
    const auto& sections = coro::sync_wait(section_repository.all_async());

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
        std::filesystem::path target =
            format_target_path(location, package.id.section.architecture,
                               description.filepath.filename().string());

        move_file(description.filepath, target);

        description.filepath = target;

        if (!description.signature_path.has_value()) { continue; }

        std::filesystem::path signature_target = format_target_path(
            location, package.id.section.architecture,
            fmt::format("{}.sig", description.filepath.filename().string()));

        move_file(*description.signature_path, signature_target);

        description.signature_path = signature_target;
    }
    return result;
}
} // namespace bxt::Persistence::Box