/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageDTO.h"
#include "core/domain/entities/PackageLogEntry.h"
#include "core/domain/enums/LogEntryType.h"

#include <cstdint>
#include <string>

namespace bxt::Core::Application {
struct PackageLogEntryDTO {
    std::string id;
    uint64_t time;
    LogEntryType type;
    PackageDTO package;

    template<class Archive> void serialize(Archive &ar) {
        ar(id, time, type, package);
    }
};

using PackageLogEntryDTOMapper =
    bxt::Utilities::StaticDTOMapper<bxt::Core::Domain::PackageLogEntry,
                                    PackageLogEntryDTO>;

} // namespace bxt::Core::Application

template<>
struct bxt::Utilities::StaticDTOMapper<
    bxt::Core::Domain::PackageLogEntry,
    bxt::Core::Application::PackageLogEntryDTO> {
    static PackageLogEntryDTO to_dto(const PackageLogEntry &from) {
        PackageLogEntryDTO dto;
        dto.id = from.id();
        dto.time = std::chrono::duration_cast<std::chrono::milliseconds>(
                       from.time().time_since_epoch())
                       .count();
        dto.type = from.type();
        dto.package =
            StaticDTOMapper<Package, PackageDTO>::to_dto(from.package());
        return dto;
    }

    static PackageLogEntry to_entity(const PackageLogEntryDTO &from) {
        PackageLogEntry entity(
            StaticDTOMapper<Package, PackageDTO>::to_entity(from.package),
            from.type, boost::uuids::string_generator()(from.id),
            std::chrono::system_clock::time_point(
                std::chrono::milliseconds(from.time)));
        return entity;
    }
};
