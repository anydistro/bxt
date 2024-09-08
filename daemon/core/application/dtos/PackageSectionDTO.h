/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/Package.h"
#include "core/domain/value_objects/Name.h"
#include "utilities/StaticDTOMapper.h"

#include <boost/functional/hash.hpp>
#include <string>

namespace bxt::Core::Application {
struct PackageSectionDTO {
    std::string branch;
    std::string repository;
    std::string architecture;

    auto operator<=>(PackageSectionDTO const& other) const = default;

    operator std::string() const {
        return fmt::format("{}/{}/{}", branch, repository, architecture);
    }

    template<class Archive> void serialize(Archive& ar) {
        ar(branch, repository, architecture);
    }
};

using SectionDTOMapper =
    bxt::Utilities::StaticDTOMapper<bxt::Core::Domain::Section, PackageSectionDTO>;

} // namespace bxt::Core::Application

namespace {
using namespace bxt::Core::Domain;
using namespace bxt::Core::Application;
} // namespace

template<> struct bxt::Utilities::StaticDTOMapper<Section, PackageSectionDTO> {
    static PackageSectionDTO to_dto(Section const& from) {
        return Core::Application::PackageSectionDTO {.branch = from.branch(),
                                                     .repository = from.repository(),
                                                     .architecture = from.architecture()};
    }
    static Section to_entity(PackageSectionDTO const& from) {
        return Core::Domain::Section(from.branch, from.repository, from.architecture);
    }
};

template<> struct std::hash<PackageSectionDTO> {
    std::size_t operator()(PackageSectionDTO const& p) const {
        std::size_t seed = 0;

        boost::hash_combine(seed, p.branch);
        boost::hash_combine(seed, p.repository);
        boost::hash_combine(seed, p.architecture);

        return seed;
    }
};

template<> inline std::string bxt::to_string(PackageSectionDTO const& dto) {
    return fmt::format("{}/{}/{}", dto.branch, dto.repository, dto.architecture);
}
