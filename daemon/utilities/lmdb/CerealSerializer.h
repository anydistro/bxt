/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/Error.h"
#include "utilities/errors/Macro.h"
#include "utilities/lmdb/Error.h"

#include <cereal/archives/binary.hpp>
#include <cereal/details/helpers.hpp>
#include <cereal/types/string.hpp>
#include <filesystem>
#include <sstream>

namespace bxt::Utilities::LMDB {

struct CerealSerializationError : public bxt::Error {
    CerealSerializationError(cereal::Exception&& exception) {
        message = exception.what();
    };
};

template<typename TSerializable> struct CerealSerializer {
    BXT_DECLARE_RESULT(SerializationError);

    static Result<std::string> serialize(TSerializable const& value) {
        try {
            std::stringstream entity_stream;
            {
                cereal::BinaryOutputArchive entity_archive(entity_stream);
                entity_archive(value);
            }
            return entity_stream.str();
        } catch (cereal::Exception& e) {
            return bxt::make_error_with_source<SerializationError>(
                CerealSerializationError(std::move(e)));
        }
    }

    static Result<TSerializable> deserialize(std::string const& value) {
        try {
            std::stringstream entity_stream(value);
            TSerializable result;
            {
                cereal::BinaryInputArchive entity_archive(entity_stream);
                entity_archive(result);
            }
            return result;
        } catch (cereal::Exception& e) {
            return bxt::make_error_with_source<SerializationError>(
                CerealSerializationError(std::move(e)));
        }
    }
};
}; // namespace bxt::Utilities::LMDB
