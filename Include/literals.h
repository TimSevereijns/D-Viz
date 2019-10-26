#ifndef LITERALS_H
#define LITERALS_H

#include <cstddef>

namespace Literals::Numeric
{
    namespace Binary
    {
        constexpr auto operator""_KiB(unsigned long long value) noexcept -> std::size_t
        {
            return value * 1'024;
        }

        constexpr auto operator""_MiB(unsigned long long value) noexcept
        {
            return value * 1'024 * 1_KiB;
        }

        constexpr auto operator""_GiB(unsigned long long value) noexcept
        {
            return value * 1'024 * 1_MiB;
        }

        constexpr auto operator""_TiB(unsigned long long value) noexcept
        {
            return value * 1'024 * 1_GiB;
        }
    } // namespace Binary

    namespace Decimal
    {
        constexpr auto operator""_KB(unsigned long long value) noexcept -> std::size_t
        {
            return value * 1'000;
        }

        constexpr auto operator""_MB(unsigned long long value) noexcept
        {
            return value * 1'000 * 1_KB;
        }

        constexpr auto operator""_GB(unsigned long long value) noexcept
        {
            return value * 1'000 * 1_MB;
        }

        constexpr auto operator""_TB(unsigned long long value) noexcept
        {
            return value * 1'000 * 1_GB;
        }
    } // namespace Decimal
} // namespace Literals::Numeric

#endif // LITERALS_H
