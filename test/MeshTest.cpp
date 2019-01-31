#include <catch.hpp>
#include <Mesh.hpp>
#include <Mesh.impl.hpp>
#include <Util.impl.hpp>
#include <iostream>

struct GridI64 : public GridConfig<i64>
{
    static void upsample(Array<i64, 2>::Getter src, Array<i64, 2>::Getter dst) {
        dst(0l, 0l) = dst(0l, 1l) = dst(1l, 0l) = dst(1l, 1l) = src(0l, 0l);
    }
    static void downsample(Array<i64, 2>::Getter src, Array<i64, 2>::Getter dst) {
        dst(0l, 0l) = (src(0l, 0l) + src(1l, 0l) + src(0l, 1l) + src(1l, 1l)) / 4;
    }
};
using Config2D = MeshConfig<2, 8, GridI64>;

TEST_CASE("Create Node", "[mesh]")
{
    auto buffers = MeshHelper<Config2D>::createBuffers(repeat<2, u64>(16));
    Node<Config2D> node(nullptr, buffers, repeat<2, u64>(0), repeat<2, u64>(0));
    auto buffers2 = MeshHelper<Config2D>::createBuffers(repeat<2, u64>(16));
    node.split(buffers2, repeat<2, u64>(0));
}

TEST_CASE("Create Tree", "[mesh]")
{
    Tree<Config2D> tree;
}

TEST_CASE("Restructure Tree", "[mesh]")
{
    Tree<Config2D> tree;
    tree.restructure();

    tree.root->action = REFINE;
    tree.restructure();
    CHECK(!tree.root->isLeaf);
    CHECK(tree.root->children[{0, 0}]->isLeaf);
    CHECK(tree.root->children[{0, 1}]->isLeaf);
    CHECK(tree.root->children[{1, 0}]->isLeaf);
    CHECK(tree.root->children[{1, 1}]->isLeaf);

    tree.root->children[{0, 0}]->action = DEREFINE;
    tree.restructure();
    CHECK(!tree.root->isLeaf);
    CHECK(tree.root->children[{0, 0}]->isLeaf);
    CHECK(tree.root->children[{0, 1}]->isLeaf);
    CHECK(tree.root->children[{1, 0}]->isLeaf);
    CHECK(tree.root->children[{1, 1}]->isLeaf);

    tree.root->children[{0, 0}]->action = DEREFINE;
    tree.root->children[{0, 1}]->action = DEREFINE;
    tree.root->children[{1, 0}]->action = DEREFINE;
    tree.root->children[{1, 1}]->action = DEREFINE;
    tree.restructure();
    CHECK(tree.root->isLeaf);
}

TEST_CASE("Synchronize Tree", "[mesh]")
{
    Tree<Config2D> tree;
    tree.root->action = REFINE;
    tree.restructure();
    tree.synchronize();
}
