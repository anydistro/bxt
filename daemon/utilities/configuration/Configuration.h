/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/log/Logging.h"

#include <optional>
#include <string>
#include <toml++/toml.h>

namespace bxt::Utilities {

class Configuration {
public:
    Configuration() = default;
    Configuration(toml::table const table)
        : m_table(std::move(table)) {
    }

    template<typename T> std::optional<T> get(std::string const& key) const {
        if (!m_table.contains(key)) {
            return {};
        }
        auto const result_node = m_table.get(key);
        if (!result_node->is<T>()) {
            loge(R"(Wrong configuration value type for "{}". Using default one.)", key);
            return {};
        }

        return std::make_optional<T>(*result_node->as<T>());
    };

    template<typename T> void set(std::string const& key, T const& value) {
        m_table.insert_or_assign(key, value);
    }

    template<typename T> void clear(std::string const& key) {
        m_table.erase(key);
    }

private:
    toml::table m_table;
};

} // namespace bxt::Utilities
