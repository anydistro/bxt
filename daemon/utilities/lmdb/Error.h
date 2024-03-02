/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "lmdbxx/lmdb++.h"
#include "utilities/errors/DatabaseError.h"

#include <string>
namespace bxt::Utilities::LMDB {

struct Error : public bxt::Error {
    Error(const lmdb::error&& error) : error(std::move(error)) {
        message = error.what();
    }

private:
    lmdb::error error;
};

struct SerializationError : public bxt::Error {
    SerializationError() { message = "Serialization/deserialization error"; }
};

} // namespace bxt::Utilities::LMDB
