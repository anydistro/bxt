/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "DomainEvent.hpp"

#include <random>
#include <sstream>
#include <iomanip>

namespace percpptency {

std::string DomainEvent::generate_id() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch())
                         .count();
    auto random = dis(gen);

    // Hex width for 64-bit values
    constexpr int hex_width = 16;
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(hex_width) << timestamp << "-"
        << std::setw(hex_width) << random;
    return oss.str();
}

} // namespace percpptency
