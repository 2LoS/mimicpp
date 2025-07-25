//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_SOURCE_LOCATION_HPP
#define MIMICPP_UTILITIES_SOURCE_LOCATION_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/PathPrinter.hpp"
#include "mimic++/printing/type/PrintType.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <cstddef>
    #include <string_view>
    #include <type_traits>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util::source_location
{
    /**
     * \defgroup SOURCE_LOCATION source-location
     * \brief Contains source-location related functionalities.
     * \details By default, `mimic++` uses the C++20 `std::source_location`.
     * However, this feature is not available on all platforms or with all compilers.
     * To support older environments, users can configure an alternative by setting the framework option \ref MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND.
     *
     * Additionally, users must provide an appropriate trait specialization for `mimicpp::util::source_location::backend_traits`,
     * which will be checked by the `mimicpp::util::source_location::backend` concept.
     * Such a specialization requires at least the following (compatible) definitions:
     * ```cpp
     * template <>
     * struct mimicpp::source_location::backend_traits<MySourceLocationBackend>
     * {
     *    static MySourceLocationBackend current() noexcept;
     *    static std::string_view function_name(MySourceLocationBackend const& backend);
     *    static std::string_view file_name(MySourceLocationBackend const& backend);
     *    static std::size_t line(MySourceLocationBackend const& backend);
     *    static std::size_t column(MySourceLocationBackend const& backend);
     * };
     * ```
     *
     * \{
     */

    template <typename Backend>
    struct backend_traits;

    template <typename T>
    concept backend =
        std::copyable<T>
        && requires(backend_traits<std::remove_cvref_t<T>> traits, std::remove_cvref_t<T> const& backend) {
               { decltype(traits)::current() } noexcept -> std::convertible_to<std::remove_cvref_t<T>>;
               { decltype(traits)::file_name(backend) } -> std::convertible_to<std::string_view>;
               { decltype(traits)::function_name(backend) } -> std::convertible_to<std::string_view>;
               { decltype(traits)::line(backend) } -> std::convertible_to<std::size_t>;
           };

    /**
     * \}
     */
}

#ifdef MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND

namespace mimicpp::util::source_location
{
    static_assert(backend<MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND>, "The provided source-location backend does not satisfy the backend requirements.");
    using InstalledBackend = MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND;
}

#else

    #ifndef __cpp_lib_source_location
        #error \
            "std::source_location is not available in this environment." \
            "mimic++ requires a working source location implementation, "\
            "which can be provided by defining the MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND macro."
    #endif

    #ifndef MIMICPP_DETAIL_IS_MODULE
        #include <source_location>
    #endif

namespace mimicpp::util::source_location
{
    template <>
    struct backend_traits<std::source_location>
    {
        [[nodiscard]]
        static constexpr std::source_location current(std::source_location const loc = std::source_location::current()) noexcept
        {
            return loc;
        }

        [[nodiscard]]
        static constexpr std::string_view file_name(std::source_location const& loc) noexcept
        {
            return loc.file_name();
        }

        [[nodiscard]]
        static constexpr std::string_view function_name(std::source_location const& loc) noexcept
        {
            return loc.function_name();
        }

        [[nodiscard]]
        static constexpr std::size_t line(std::source_location const& loc) noexcept
        {
            return std::size_t{loc.line()};
        }
    };

    static_assert(backend<std::source_location>, "std::source_location isn't suitable as a source-location backend. Blame the mimic++ maintainer!");

    using InstalledBackend = std::source_location;
}

#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief A thin wrapper around the currently installed source-location backend with additional ``operator ==``.
     * \ingroup SOURCE_LOCATION
     */
    class SourceLocation
    {
        // note: The `template <typename... Canary, typename Backend = source_location::InstalledBackend>` hack is done,
        // so that we can declare all functions as `constexpr`, even when the currently installed backend is clearly not
        // `constexpr`.

    public:
        ~SourceLocation() = default;

        template <typename... Canary, std::same_as<source_location::InstalledBackend> Backend = source_location::InstalledBackend>
        [[nodiscard]]
        explicit(false) constexpr SourceLocation(Backend loc = source_location::backend_traits<Backend>::current()) noexcept
            : m_SourceLocation{std::move(loc)}
        {
        }

        SourceLocation(SourceLocation const&) = default;
        SourceLocation& operator=(SourceLocation const&) = default;
        SourceLocation(SourceLocation&&) = default;
        SourceLocation& operator=(SourceLocation&&) = default;

        template <typename... Canary, typename Backend = source_location::InstalledBackend>
        [[nodiscard]]
        constexpr std::string_view file_name() const noexcept
        {
            return source_location::backend_traits<Backend>::file_name(m_SourceLocation);
        }

        template <typename... Canary, typename Backend = source_location::InstalledBackend>
        [[nodiscard]]
        constexpr std::string_view function_name() const noexcept
        {
            return source_location::backend_traits<Backend>::function_name(m_SourceLocation);
        }

        template <typename... Canary, typename Backend = source_location::InstalledBackend>
        [[nodiscard]]
        constexpr std::size_t line() const noexcept
        {
            return source_location::backend_traits<Backend>::line(m_SourceLocation);
        }

        [[nodiscard]]
        friend constexpr bool operator==(SourceLocation const& lhs, SourceLocation const& rhs) noexcept
        {
            return lhs.line() == rhs.line()
                && lhs.file_name() == rhs.file_name()
                && lhs.function_name() == rhs.function_name();
        }

    private:
        static_assert(source_location::backend<source_location::InstalledBackend>);
        source_location::InstalledBackend m_SourceLocation;
    };
}

template <>
struct mimicpp::printing::detail::state::common_type_printer<mimicpp::util::SourceLocation>
{
    template <print_iterator OutIter>
    static constexpr OutIter print(OutIter out, util::SourceLocation const& loc)
    {
        out = format::format_to(std::move(out), "`");
        out = print_path(std::move(out), loc.file_name());
        out = format::format_to(std::move(out), "`");

        out = format::format_to(
            std::move(out),
            "#L{}, `",
            loc.line());
        out = type::prettify_function(std::move(out), StringT{loc.function_name()});
        out = format::format_to(std::move(out), "`");

        return out;
    }
};

#endif
