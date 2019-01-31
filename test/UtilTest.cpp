#include <catch.hpp>
#include <Util.hpp>
#include <Util.impl.hpp>

TEST_CASE("collect", "[util]")
{
    auto x = collect<4>(4, 3, 2, 1);
    std::array<int, 4> y = {4, 3, 2, 1};
    CHECK(y == x);
}
