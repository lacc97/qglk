#ifndef LOG_HPP
#define LOG_HPP

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <type_traits>

#include <QString>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "format.hpp"


namespace fmt {
    template <>
    struct formatter<QString> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const QString& str, FormatContext &ctx) {
            return format_to(ctx.out(), "\"{0}\"", str.toStdString());
        }
    };

    template<>
    struct formatter<std::filesystem::path> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const std::filesystem::path& p, FormatContext &ctx) {
            return format_to(ctx.out(), "\"{0}\"", p.generic_string());
        }
    };
}

inline std::ostream& operator<<(std::ostream& os, const QString& str) {
    return os << fmt::format("{}", str);
}

#endif
