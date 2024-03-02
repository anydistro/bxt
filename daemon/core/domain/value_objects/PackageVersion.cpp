/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageVersion.h"

#include "core/domain/entities/Package.h"
#include "scn/tuple_return/tuple_return.h"
#include "utilities/Error.h"

#include <optional>
#include <scn/scn.h>
#include <scn/tuple_return.h>

namespace bxt::Core::Domain {

// naive rewrite of original rpmvercmp from alpm
std::strong_ordering rpmvercmp(const std::string& a_orig,
                               const std::string& b_orig) {
    if (a_orig == b_orig) return std::strong_ordering::equal;

    std::string a = a_orig, b = b_orig;

    auto one = a.begin(), it1 = a.begin();
    auto two = b.begin(), it2 = b.begin();

    bool isnum;

    while (one != a.end() && two != b.end()) {
        while (one != a.end() && !std::isalnum(*one))
            one++;
        while (two != b.end() && !std::isalnum(*two))
            two++;

        if (one == a.end() || two == b.end()) break;

        if ((one - it1) != (two - it2)) {
            return (one - it1) < (two - it2) ? std::strong_ordering::less
                                             : std::strong_ordering::greater;
        }

        it1 = one;
        it2 = two;

        if (std::isdigit(*it1)) {
            while (it1 != a.end() && std::isdigit(*it1))
                it1++;
            while (it2 != b.end() && std::isdigit(*it2))
                it2++;
            isnum = true;
        } else {
            while (it1 != a.end() && std::isalpha(*it1))
                it1++;
            while (it2 != b.end() && std::isalpha(*it2))
                it2++;
            isnum = false;
        }

        char oldch1 = *it1;
        *it1 = 0;
        char oldch2 = *it2;
        *it2 = 0;

        if (one == it1) { return std::strong_ordering::less; }

        if (two == it2) {
            return isnum ? std::strong_ordering::greater
                         : std::strong_ordering::less;
        }

        if (isnum) {
            while (*one == '0')
                one++;
            while (*two == '0')
                two++;

            if (std::distance(one, a.end()) > std::distance(two, b.end())) {
                return std::strong_ordering::greater;
            }
            if (std::distance(one, a.end()) < std::distance(two, b.end())) {
                return std::strong_ordering::less;
            }
        }
        auto substr1 = std::string(one, a.end());
        auto substr2 = std::string(two, b.end());
        auto rc = substr1.compare(substr2);

        if (rc) {
            return rc < 1 ? std::strong_ordering::less
                          : std::strong_ordering::greater;
        }

        *it1 = oldch1;
        one = it1;

        *it2 = oldch2;
        two = it2;
    }

    if (one == a.end() && two == b.end()) {
        return std::strong_ordering::equal;
    }

    if ((one == a.end() && std::isalpha(*two)) || std::isalpha(*one)) {
        return std::strong_ordering::less;
    } else {
        return std::strong_ordering::greater;
    }
}

std::strong_ordering PackageVersion::compare(const PackageVersion& lh,
                                             const PackageVersion& rh) {
    std::strong_ordering ret = std::strong_ordering::equal;

    if (!(lh.epoch || rh.epoch)) {
        ret = rpmvercmp(std::to_string(*lh.epoch), std::to_string(*rh.epoch));
    }

    if (ret == 0) {
        ret = rpmvercmp(lh.version, rh.version);

        if (ret == 0 && lh.release.length() > 0 && rh.release.length() > 0) {
            ret = rpmvercmp(lh.release, rh.release);
        }
    }

    return ret;
}

PackageVersion::ParseResult
    PackageVersion::from_string(std::string_view version_str) {
    auto [scan_ok, version, epoch_int, release] =
        scn::scan_tuple<std::string, int, std::string>(
            version_str, "{:[:alnum:.]}\:{}-{:[:alnum:.]}");
    std::optional<int> epoch = std::nullopt;

    if (!scan_ok) {
        if (!scn::scan(version_str, "{:[:alnum:.]}-{}", version, release)) {
            return bxt::make_error<ParsingError>(
                ParsingError::ErrorCode::InvalidFormat);
        }
    } else {
        epoch = epoch_int;
    }

    static constexpr auto version_validator = [](const char& ch) {
        return std::isalnum(ch) || ch == '.';
    };

    if (!std::ranges::all_of(version, version_validator)) {
        return bxt::make_error<ParsingError>(
            ParsingError::ErrorCode::InvalidVersion);
    }

    if (!std::ranges::all_of(release, version_validator)) {
        return bxt::make_error<ParsingError>(
            ParsingError::ErrorCode::InvalidReleaseTag);
    }

    return PackageVersion {version, epoch, release};
}

std::string PackageVersion::string() const {
    std::string format = "{1}";

    if (epoch) { format = "{0}:" + format; }

    if (release.length() > 0) { format = format + "-{2}"; }

    return fmt::format(fmt::runtime(format), epoch.value_or(0),
                       std::string(version), release);
}

} // namespace bxt::Core::Domain
