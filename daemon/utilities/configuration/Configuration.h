/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <any>
#include <filesystem>
#include <optional>
#include <string>
#include <toml++/toml.h>

namespace bxt::Utilities {

class Configuration {
public:
    Configuration(const toml::table& table) : m_table(table) {}
    ~Configuration() = default;

    template<typename T> std::optional<T> get(const std::string& key) const {
        if (!m_table.contains(key)) { return {}; }

        return std::make_optional<T>(*m_table.get_as<T>(key));
    };

    template<typename T> void set(const std::string& key, const T& value) {
        m_table.insert_or_assign(key, value);
    }

    template<typename T> void clear(const std::string& key) {
        m_table.erase(key);
    }

private:
    toml::table m_table;
    Configuration() = default;
};

struct Configurable {
    virtual void serialize(Configuration& config) = 0;
    virtual void deserialize(const Configuration& config) = 0;
};

} // namespace bxt::Utilities
