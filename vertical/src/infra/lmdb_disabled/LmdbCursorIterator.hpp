/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <iterator>
#include <memory>
#include <string_view>
#include <utility>

#include <lmdbxx/lmdb++.h>
#include <rfl.hpp>

#include "internal/Formats.hpp"
#include "shared/common.hpp"
#include "core/error/kinds.hpp"
#include "core/error/kinds.hpp"

namespace bxt::adapters::lmdb {

namespace lmdbxx = ::lmdb;

enum class CursorPosition { First, Last, Range };

template<typename T> class LmdbCursorIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<std::string_view, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using CrudError = bxt::err::CrudError;
    using Result = bxt::err::Result<value_type, CrudError>;

    LmdbCursorIterator() = default;

    LmdbCursorIterator(std::weak_ptr<lmdbxx::cursor> cursor,
                       CursorPosition position = CursorPosition::First)
        : m_cursor(std::move(cursor)) {
        if (auto c = m_cursor.lock()) {
            if (position == CursorPosition::First) {
                move_cursor(MDB_FIRST);
            } else if (position == CursorPosition::Last) {
                move_cursor(MDB_LAST);
            }
        }
    }

    LmdbCursorIterator(std::weak_ptr<lmdbxx::cursor> cursor, std::string_view key)
        : m_cursor(std::move(cursor)) {
        if (auto c = m_cursor.lock()) {
            m_key_view = key;
            move_cursor(MDB_SET_RANGE);
        }
    }

    // Static factory methods for easier creation
    static LmdbCursorIterator begin(std::weak_ptr<lmdbxx::cursor> cursor) {
        return LmdbCursorIterator(cursor, CursorPosition::First);
    }

    static LmdbCursorIterator end() {
        return LmdbCursorIterator();
    }

    static LmdbCursorIterator last(std::weak_ptr<lmdbxx::cursor> cursor) {
        return LmdbCursorIterator(cursor, CursorPosition::Last);
    }

    static LmdbCursorIterator from_key(std::weak_ptr<lmdbxx::cursor> cursor, std::string_view key) {
        return LmdbCursorIterator(cursor, key);
    }

    Result operator*() const {
        if (!m_valid) {
            return make_error<CrudError>(earg(CrudError::Kind::DatabaseError),
                                         "Cursor is not valid");
        }

        auto deserialized_value =
            internal::deserialize<T>(m_value_view.data(), m_value_view.size());

        if (!deserialized_value.has_value()) {
            return make_error<CrudError>(earg(CrudError::Kind::DatabaseError),
                                         "Deserialization failed");
        }

        return std::make_pair(m_key_view, deserialized_value.value());
    }

    LmdbCursorIterator& operator++() {
        move_cursor(MDB_NEXT);
        return *this;
    }

    LmdbCursorIterator operator++(int) {
        LmdbCursorIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    LmdbCursorIterator& operator--() {
        move_cursor(MDB_PREV);
        return *this;
    }

    LmdbCursorIterator operator--(int) {
        LmdbCursorIterator tmp = *this;
        --(*this);
        return tmp;
    }

    bool operator==(LmdbCursorIterator const& other) const {
        if (!m_valid && !other.m_valid) {
            return true;
        }

        if (m_valid != other.m_valid) {
            return false;
        }

        return m_key_view == other.m_key_view;
    }

    bool operator!=(LmdbCursorIterator const& other) const {
        return !(*this == other);
    }

private:
    std::weak_ptr<lmdbxx::cursor> m_cursor;
    std::string_view m_key_view;
    std::string_view m_value_view;
    bool m_valid = false;

    VoidResult<CrudError> move_cursor(MDB_cursor_op op) {
        auto cursor = m_cursor.lock();
        if (!cursor) {
            m_valid = false;
            return {};
        }

        try {
            if (op == MDB_SET_RANGE) {
                m_valid = cursor->get(m_key_view, m_value_view, op);
            } else {
                m_valid = cursor->get(m_key_view, m_value_view, op);
            }
        } catch (lmdbxx::error const& e) {
            m_valid = false;
            return make_error<CrudError>(earg(CrudError::Kind::DatabaseError), e.what());
        }

        if (!m_valid) {
            m_key_view = std::string_view();
            m_value_view = std::string_view();
        }
        return {};
    }
};

} // namespace bxt::adapters::lmdb
