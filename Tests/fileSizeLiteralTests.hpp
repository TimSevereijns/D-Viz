#ifndef FILESIZELITERALS_HPP
#define FILESIZELITERALS_HPP

#include <catch2/catch.hpp>

#include <literals.h>

TEST_CASE("User-defined size literals")
{
    using namespace Literals::Numeric::Binary;

    STATIC_REQUIRE(22_KiB == 22 * 1'024);
    STATIC_REQUIRE(10_MiB == 10 * 1'024 * 1'024);
    STATIC_REQUIRE(1_GiB == 1 * 1'024 * 1'024 * 1'024);
}

#endif // FILESIZELITERALS_HPP
