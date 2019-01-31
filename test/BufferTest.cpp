#include <catch.hpp>
#include <Buffer.hpp>

TEST_CASE("Allocating buffers", "[buffer]")
{
    CHECK_THROWS_AS((Buffer<int, 2>({0, 5})), std::invalid_argument);
    CHECK_THROWS_AS((Buffer<int, 4>({3, 0, 4, 6})), std::invalid_argument);
    CHECK_NOTHROW((Buffer<int, 1>({1})));
    CHECK_NOTHROW((Buffer<int, 1>({10})));
    CHECK_NOTHROW((Buffer<int, 5>({1, 1, 1, 1, 1})));
    CHECK_NOTHROW((Buffer<int, 5>({6, 2, 4, 3, 10})));
}

TEST_CASE("Calculating stride", "[buffer]")
{
    {
        Buffer<int, 3> buf({1, 1, 1});
        CHECK(buf.stride[0] == 1);
        CHECK(buf.stride[1] == 1);
    }
    {
        Buffer<int, 4> buf({4, 5, 6, 7});
        CHECK(buf.stride[0] == 4);
        CHECK(buf.stride[1] == 20);
        CHECK(buf.stride[2] == 120);
    }
}

TEST_CASE("Creating arrays", "[buffer]")
{
    // TODO
}

TEST_CASE("Array indexing", "[buffer]")
{
    // TODO
}
