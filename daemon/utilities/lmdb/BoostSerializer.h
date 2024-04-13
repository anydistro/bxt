/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageLogEntryDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/dtos/UserDTO.h"
#include "core/domain/entities/PackageLogEntry.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"
#include "utilities/lmdb/Error.h"

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>

template<class Archive>
void boost::serialization::serialize(Archive &ar,
                                     std::filesystem::path &path,
                                     const unsigned int version) {
    std::string path_string = path.string();
    ar & path_string;
    if (Archive::is_loading::value) {
        path = std::filesystem::path(path_string);
    }
}

template<class Archive>
void boost::serialization::serialize(
    Archive &ar,
    bxt::Core::Application::PackageSectionDTO &section,
    const unsigned int version) {
    ar & section.branch;
    ar & section.repository;
    ar & section.architecture;
}

template<class Archive>
void boost::serialization::serialize(Archive &ar,
                                     bxt::Core::Application::UserDTO &user,
                                     const unsigned int version) {
    ar & user.name;
    ar & user.password;
    ar & user.permissions;
}

template<class Archive>
void boost::serialization::serialize(Archive &ar,
                                     bxt::Core::Application::PackageDTO &pkg,
                                     const unsigned int version) {
    ar & pkg.name;
    ar & pkg.filepath;
    ar & pkg.section;
}

template<class Archive>
void boost::serialization::serialize(
    Archive &ar,
    bxt::Core::Application::PackageLogEntryDTO &entry,
    const unsigned int version) {
    ar & entry.id;
    ar & entry.package;
    ar & entry.time;
    ar & entry.type;
}

namespace bxt::Utilities::LMDB {

struct BoostSerializationError : public bxt::Error {
    BoostSerializationError(boost::archive::archive_exception &&exception)
        : exception(exception) {
        message = exception.what();
    };

    boost::archive::archive_exception exception;
};

template<typename TSerializable> struct BoostSerializer {
    BXT_DECLARE_RESULT(SerializationError);

    static Result<std::string> serialize(const TSerializable &value) {
        try {
            std::stringstream entity_stream;
            boost::archive::text_oarchive entity_archive(entity_stream);

            entity_archive << value;

            return entity_stream.str();
        } catch (boost::archive::archive_exception &e) {
            return bxt::make_error_with_source<SerializationError>(
                BoostSerializationError(std::move(e)));
        }
    }

    static Result<TSerializable> deserialize(std::string_view value) {
        try {
            std::stringstream entity_stream((std::string(value)));
            boost::archive::text_iarchive entity_archive(entity_stream);

            TSerializable result;
            entity_archive >> result;

            return result;
        } catch (boost::archive::archive_exception &e) {
            return bxt::make_error_with_source<SerializationError>(
                BoostSerializationError(std::move(e)));
        }
    }
};
} // namespace bxt::Utilities::LMDB
