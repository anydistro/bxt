/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>

#include <lmdb.h>

namespace bxt::adapters::lmdb {

class LmdbError {
public:
    explicit LmdbError(int errorCode)
        : m_errorCode(errorCode) {
    }

    std::string what() const noexcept {
        return mdb_strerror(m_errorCode);
    }

private:
    int m_errorCode;
};

} // namespace bxt::adapters::lmdb
