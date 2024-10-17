// CustomFormatter.hpp
#ifndef CUSTOM_FORMATTER_HPP
#define CUSTOM_FORMATTER_HPP

#include <fmt/format.h>
#include <boost/beast/core/string.hpp>

namespace fmt {
    template <>
    struct formatter<boost::beast::string_view> {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            return ctx.begin();
        }

        // Formats the boost::beast::string_view into the provided output iterator
        template <typename FormatContext>
        auto format(const boost::beast::string_view& sv, FormatContext& ctx) -> decltype(ctx.out()) {
            // Convert to std::string for formatting
            return format_to(ctx.out(), "{}", std::string(sv));
        }
    };
}

#endif