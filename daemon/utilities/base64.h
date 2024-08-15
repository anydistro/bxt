/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once
#include <openssl/evp.h>
#include <string>
#include <vector>

namespace bxt::Utilities {

inline std::string b64_encode(const std::string& input) {
    std::vector<unsigned char> output((input.size() + 2) / 3 * 4);
    int encoded_length = EVP_EncodeBlock(
        output.data(), reinterpret_cast<const unsigned char*>(input.data()),
        static_cast<int>(input.size()));
    return {output.begin(), output.begin() + encoded_length};
}

inline std::string b64_decode(std::string_view input) {
    std::vector<unsigned char> output((input.size() + 3) / 4 * 3);
    int decoded_length = EVP_DecodeBlock(
        output.data(), reinterpret_cast<const unsigned char*>(input.data()),
        static_cast<int>(input.size()));
    if (decoded_length < 0) {
        throw std::runtime_error("Base64 decoding failed");
    }
    if (input.size() >= 2 && input[input.size() - 1] == '=') {
        decoded_length--;
        if (input[input.size() - 2] == '=') { decoded_length--; }
    }
    return {output.begin(), output.begin() + decoded_length};
}

} // namespace bxt::Utilities
