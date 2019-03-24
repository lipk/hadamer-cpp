#pragma once

#include <Types.hpp>
#include <Buffer.hpp>
#include <Control.hpp>
#include <Util.hpp>
#include <vector>

const int NOTHING = 0;
const int REFINE = (1<<0);
const int DEREFINE = (1<<1);

TYPEFUNC_GETTER(DataType);

template<u8 dimension_, u64 blockSize_, typename ...Grids_>
struct MeshConfig
{
    static constexpr const u8 dimension = dimension_;
    static constexpr const u64 blockSize = blockSize_;
    using Grids = TypeList<Grids_...>;
    using DataTypes = _tn Grids::_tm Map<GetDataType>;

    static u64vec<dimension> nodeSize() {
        return repeat<dimension>(blockSize);
    }

    template<typename T>
    using ArrayKD = Array<T, dimension>;
    using Arrays = _tn DataTypes::_tm Wrap<ArrayKD>::ToTuple;

    template<typename T>
    using BufferKD = Buffer<T, dimension>;
    template<typename T>
    using BufferKDPtr = std::shared_ptr<BufferKD<T>>;
    using Buffers = _tn DataTypes::_tm Wrap<BufferKDPtr>::ToTuple;
};

template <typename Config>
struct MeshHelper
{
    using Arrays = typename Config::Arrays;
    using Buffers = typename Config::Buffers;
    static _tn Config::Buffers createBuffers(
            const std::array<u64, Config::dimension>& size);
};

template<typename DataType_>
struct GridConfig
{
    using DataType = DataType_;
};

template<typename Config>
struct Mesh;

template<typename Config>
struct RefinePlan;

template<typename Config>
struct Node
{
    static constexpr const u8 dim = Config::dimension;

    NOT_COPYABLE(Node)
    NOT_MOVABLE(Node)

    using Self = Node<Config>;
    using Arrays = _tn Config::Arrays;
    using Buffers = _tn Config::Buffers;

    bool isLeaf;
    Array<Self*, dim> children;
    Self *parent;
    Arrays data;
    int action;
    u32 level;
    u64vec<dim> index;
    std::vector<RefinePlan<Config>*> refinePlan;
    Array<Self*, dim> adjacent;
    bool sync, propagate;

    Node(Self *parent, const Buffers& buffers, const u64vec<dim>& position, u64vec<dim> index);
    Node();

    void split(const Buffers& buffers, const u64vec<dim>& position);
    void merge();

    template <typename Grid, u64 index>
    void upsampleGrid();
    void upsampleAll();

    template <typename Grid, u64 index>
    void downsampleGrid();
    void downsampleAll();

    template <typename Grid, u64 index>
    void upsampleGridRange(const u64vec<dim>& fromSrc, const u64vec<dim>& toSrc,
                           const u64vec<dim>& toDst, Node<Config>* child);
    void upsampleAllRange(const u64vec<dim>& fromSrc, const u64vec<dim>& toSrc,
                          const u64vec<dim>& toDst, const u64vec<dim>& child);

    void updateAdjacency();
    void calculateRefinePlan();
    void calculateRefinePlanRecursive();

    void restructure();

    void propagateUp();
    void propagateDown();

    static bool isInSameBuffer(Node<Config>* node1, Node<Config>* node2);
    template <u64 index>
    static void exchangeHaloGrid(Node<Config> *node1, Node<Config> *node2,
                      const u64vec<dim>& fromSrc, const u64vec<dim>& toSrc,
                      const u64vec<dim>& fromDst);
    static void exchangeHaloAll(Node<Config> *node1, Node<Config> *node2,
                      const u64vec<dim>& fromSrc, const u64vec<dim>& toSrc,
                      const u64vec<dim>& fromDst);
    void synchronize();
    void synchronizeRecursive();

    template <typename Function>
    void applyKernel(const Function& func);
};

template<typename Config>
struct DataView
{
    Node<Config>& node;
    u64vec<Config::dimension> base;
    bool refine, derefine;

    DataView(Node<Config>& node, u64vec<Config::dimension> base);

    template<uint32_t index, typename ...XS>
    _tn Config::DataTypes::_tm Get<index>& get(i64 x1, XS... xs) {
        auto offset = collect<Config::dimension, i64>(x1, xs...);
        return std::get<index>(this->node.data)[this->base + offset];
    }

    ~DataView() {
        if (this->refine) {
            node.action = REFINE;
        } else if (this->derefine) {
            node.action = node.action & DEREFINE;
        }
    }
};

template<typename Config>
struct Tree
{
    NOT_COPYABLE(Tree)
    NOT_MOVABLE(Tree)

    Node<Config> *root;
    Tree();

    void restructure();
    void synchronize();

    template <typename Function>
    void applyKernel(const Function& func);
};

template<typename Config>
struct Mesh
{
    Array<Tree<Config>, Config::dimension> trees;

    Mesh(const u64vec<Config::dimension>& size);

    void updateStructure();

    template <typename Function>
    void applyKernel(const Function& func);
};

template<typename Config>
struct RefinePlan
{
    static constexpr const u8 dim = Config::dimension;
    using Self = RefinePlan<Config>;
    u32 level;
    u64vec<dim> position, size;
    Array<Node<Config>*, dim> nodes;
    bool propagate;

    void propagateUp(const u64vec<dim>& index, u32 currentLevel);
    RefinePlan(Node<Config> *node);
    RefinePlan();

    bool merge(Self *other, u32 level);
};
