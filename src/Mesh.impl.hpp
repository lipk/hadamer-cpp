#pragma once

#include <Mesh.hpp>
#include <Control.hpp>
#include <Util.hpp>
#include <Util.impl.hpp>

template <typename Config>
typename MeshHelper<Config>::Buffers MeshHelper<Config>::createBuffers(
        const std::array<u64, Config::dimension>& size)
{
    return mapTuple(_tn Config::DataTypes::_tm Wrap<TypeCell>::ToTuple(),
                    [&](auto type) {
        return std::make_shared<_tn Config::_tm BufferKD<
                _tn decltype(type)::Type>>(size);
    });
}

template<typename Config>
Node<Config>::Node(Self *parent, const Buffers &buffers,
                          const u64vec<dim> &position,
                          u64vec<dim> index)
    : isLeaf(true)
    , children(Array<Self*, dim>::createWithBuffer(repeat<dim, u64>(2)))
    , parent(parent)
    , data(mapTuple(buffers, [&](auto buffer) {
        return _tn Config::_tm ArrayKD<_tn decltype(buffer)::element_type::DataType>(
                    buffer, position, Config::nodeSize());
    }))
    , action(NOTHING)
    , level(parent == nullptr ? 0 : parent->level+1)
    , index(index)
    , adjacent(Array<Self*, dim>::createWithBuffer(repeat<dim, u64>(3)))
    , sync(false)
    , propagate(false)
{
}

template<typename Config>
Node<Config>::Node()
    : isLeaf(true)
    , children(Array<Self*, dim>::createWithBuffer(repeat<dim, u64>(2)))
    , parent(nullptr)
    , data(mapTuple(MeshHelper<Config>::createBuffers(Config::nodeSize()), [&](auto buffer) {
        return _tn Config::_tm ArrayKD<_tn decltype(buffer)::element_type::DataType>(
                    buffer, repeat<dim, u64>(0), Config::nodeSize());
    }))
    , action(NOTHING)
    , level(0)
    , index(repeat<dim, u64>(0))
    , adjacent(Array<Self*, dim>::createWithBuffer(repeat<dim, u64>(3)))
    , sync(false)
    , propagate(false)
{
}

template <typename Config>
template <typename Grid, u64 index>
void Node<Config>::upsampleGrid()
{
    ASSERT(!this->isLeaf);
    Loop<dim>(0, 2, [&](const auto& it1) {
        Loop<dim>(0, Config::blockSize/2, [&](const auto& it2) {
            const auto src = it2 + Config::blockSize/2 * it1;
            const auto dst = 2ul * it2;
            Grid::upsample(std::get<index>(this->data).getter(src),
                           std::get<index>(this->children[it1]->data).getter(dst));
        });
    });
}

template <typename Config>
void Node<Config>::upsampleAll()
{
    Config::Grids::forEach([&](auto grid) {
        using Grid = decltype (grid);
        this->upsampleGrid<_tn Grid::Type, Grid::index>();
    });
}

template <typename Config>
template <typename Grid, u64 index>
void Node<Config>::upsampleGridRange(const u64vec<dim>& fromSrc,
                                     const u64vec<dim>& toSrc,
                                     const u64vec<dim>& toDst,
                                     Node<Config>* child)
{
    ASSERT(!this->isLeaf);
    Loop<dim>(fromSrc, toSrc, [&](const auto& it) {
            const auto dst = toDst + (it - fromSrc) * 2;
            Grid::upsample(std::get<index>(this->data).getter(it),
                           std::get<index>(child->data).getter(dst));
    });
}

template <typename Config>
void Node<Config>::upsampleAllRange(const u64vec<dim>& fromSrc,
                                    const u64vec<dim>& toSrc,
                                    const u64vec<dim>& toDst,
                                    const u64vec<dim>& child)
{
    auto childNode = this->children[child];
    Config::Grids::forEach([&](auto grid) {
        using Grid = decltype (grid);
        this->upsampleGridRange<_tn Grid::Type, Grid::index>(fromSrc, toSrc,
                                                             toDst, childNode);
    });
}

template <typename Config>
template <typename Grid, u64 index>
void Node<Config>::downsampleGrid()
{
    ASSERT(!this->isLeaf);
    Loop<dim>(0, 2, [&](const auto& it1) {
        Loop<dim>(0, Config::blockSize/2, [&](const auto& it2) {
            const auto dst = it2 + Config::blockSize/2 * it1;
            const auto src = 2 * it2;
            Grid::downsample(std::get<index>(this->children[it1]->data).getter(src),
                             std::get<index>(this->data).getter(dst));
        });
    });
}

template <typename Config>
void Node<Config>::downsampleAll()
{
    Config::Grids::forEach([&](auto grid) {
        using Grid = decltype (grid);
        this->downsampleGrid<_tn Grid::Type, Grid::index>();
    });
}

template <typename Config>
void Node<Config>::updateAdjacency()
{
    Loop<dim>(0, 3, [&](const auto& it) {
        this->adjacent[it] = nullptr;
    });
    this->adjacent[repeat<dim, u64>(1)] = this;
    if (this->parent != nullptr) {
        this->sync = false;
        this->propagate = false;
        i64vec<6> t = {0, 1, 1, 1, 1, 2};
        Loop<dim>(0, 3, [&](const auto& it) {
            Self* &adj = this->adjacent[it];
            auto x = this->index * 3 + it;
            for (u8 i = 0; i<dim; ++i) {
                x[i] = t[x[i]];
            }
            adj = this->parent->adjacent[x];
            if (adj != nullptr && !adj->isLeaf) {
                adj = adj->children[(this->index + it) % 2];
            }
            if (adj != nullptr && (adj->isLeaf || this->isLeaf)) {
                this->sync = true;
            }
        });

        if (this->parent->sync || this->parent->propagate) {
            this->propagate = true;
        }
    }

    if (!this->isLeaf) {
        Loop<dim>(0, 2, [&](const auto& it) {
            this->children[it]->updateAdjacency();
        });
    }
}

template <typename Config>
RefinePlan<Config>::RefinePlan(Node<Config>* node)
    : level(node->level)
    , position(repeat<dim, u64>(0))
    , size(repeat<dim, u64>(1))
    , nodes(Array<Node<Config>*, dim>::createWithBuffer(this->size))
    , propagate(true)
{
    this->nodes[this->position] = node;
}

template <typename Config>
RefinePlan<Config>::RefinePlan()
    : level(0u)
    , position(repeat<dim, u64>(0))
    , size(repeat<dim, u64>(0))
    , nodes(Array<Node<Config>*, dim>::createWithBuffer(this->size))
    , propagate(true)
{
}

template <typename Config>
void Node<Config>::calculateRefinePlan()
{
    if (this->isLeaf) {
        if (!(this->action & REFINE)) {
            return;
        }
        this->refinePlan.emplace_back(new RefinePlan<Config>(this));
    } else {
        Loop<dim>(0, 2, [&](const auto& it) {
            auto child = this->children[it];
            for (auto rpit = child->refinePlan.begin();
                 rpit != child->refinePlan.end();) {
                if ((*rpit)->propagate) {
                    (*rpit)->propagateUp(it, this->level);
                    this->refinePlan.push_back(std::move(*rpit));
                    rpit = child->refinePlan.erase(rpit);
                } else {
                    rpit++;
                }
            }
        });
        bool changed;
        do {
            changed = false;
            for (size_t pi = 0; pi<this->refinePlan.size(); ++pi) {
                auto &plan1 = this->refinePlan[pi];
                for (size_t pj = pi+1; pj<this->refinePlan.size(); ++pj) {
                    auto &plan2 = this->refinePlan[pj];
                    if (plan1->merge(plan2, this->level)) {
                        delete this->refinePlan[pj];
                        this->refinePlan.erase(this->refinePlan.begin() + pj);
                    }
                    changed = true;
                    pj--;
                }
            }
        } while (changed);
    }
}

template <typename Config>
void Node<Config>::calculateRefinePlanRecursive()
{
    if (!this->isLeaf) {
        Loop<dim>(0, 2, [&](const auto& it) {
            this->children[it]->calculateRefinePlanRecursive();
        });
    }
    this->calculateRefinePlan();
}

template<typename Config>
void Node<Config>::split(const Buffers& buffers, const u64vec<dim> &position)
{
    ASSERT(this->isLeaf);
    Loop<dim>(0, 2, [&](const auto& it) {
        auto x = it * Config::blockSize;
        auto childPosition = position + it * Config::blockSize;
        this->children[it] = new Node(this, buffers, childPosition, it);
    });
    this->isLeaf = false;
    this->upsampleAll();
}

template<typename Config>
void Node<Config>::merge()
{
    ASSERT(!this->isLeaf);
    this->downsampleAll();
    Loop<dim>(0, 2, [&](const auto& it) {
        ASSERT(this->children[it]->isLeaf);
        delete this->children[it];
        this->children[it] = nullptr;
    });
    this->isLeaf = true;
}

template <typename Config>
void executeRefinePlan(const RefinePlan<Config>& plan)
{
    auto size = plan.size * Config::blockSize * 2ul + repeat<Config::dimension, u64>(2);
    auto buffers = MeshHelper<Config>::createBuffers(size);
    Loop<Config::dimension>(repeat<Config::dimension, u64>(0),
                             plan.size, [&](const auto& it) {
        plan.nodes[it]->split(buffers, it);
    });
}

template <typename Config>
void Node<Config>::restructure()
{
    if (!this->isLeaf) {
        int action = DEREFINE;
        Loop<dim>(0, 2, [&](const auto& it) {
            action &= this->children[it]->action;
        });
        if (action & DEREFINE) {
            this->merge();
            return;
        }
        Loop<dim>(0, 2, [&](const auto& it) {
            action &= this->children[it]->action;
        });
    }
    for (size_t i = 0; i<this->refinePlan.size(); ++i) {
        executeRefinePlan<Config>(*this->refinePlan[i]);
    }
    this->action = NOTHING;
    this->refinePlan.clear();
}

template <typename Config>
template <u64 index_>
void Node<Config>::exchangeHaloGrid(Node<Config> *node1,
                                Node<Config> *node2,
                                const u64vec<dim>& fromSrc,
                                const u64vec<dim>& toSrc,
                                const u64vec<dim>& fromDst)
{
    if (node1 == nullptr || node2 == nullptr) {
        // TODO: boundary conditions
        return;
    }
    if (isInSameBuffer(node1, node2)) {
        return;
    }
    Loop<dim>(fromSrc, toSrc, [&](auto& it) {
        std::get<index_>(node1->data)[fromDst + it - fromSrc] = std::get<index_>(node2->data)[it];
    });
}

template <typename Config>
bool Node<Config>::isInSameBuffer(Node<Config>* node1, Node<Config>* node2)
{
    return std::get<0>(node1->data).buffer == std::get<0>(node2->data).buffer;
}

template <typename Config>
void Node<Config>::exchangeHaloAll(Node<Config> *node1,
                                Node<Config> *node2,
                                const u64vec<dim>& fromSrc,
                                const u64vec<dim>& toSrc,
                                const u64vec<dim>& fromDst)
{
    Config::Grids::forEach([&](auto grid) {
        using Grid = decltype (grid);
        exchangeHaloGrid<Grid::index>(node1, node2, fromSrc, toSrc, fromDst);
    });
}

template <typename Config>
void Node<Config>::synchronize()
{
    Loop<dim>(0, 3, [&](auto& it) {
        u64vec<dim> fromSrc, toSrc, fromDst;
        for (u8 i = 0; i<dim; ++i) {
            fromSrc[i] = it[i] == 0 ? Config::blockSize : 1;
            toSrc[i] = it[i] == 2 ? 2 : Config::blockSize+1;
            fromDst[i] = it[i] == 0 ? 1 : Config::blockSize;
        }
        exchangeHaloAll(this, this->adjacent[it], fromSrc, toSrc, fromDst);
    });
}

template <typename Config>
void Node<Config>::synchronizeRecursive()
{
    if (this->sync) {
        this->synchronize();
    }
    if (!this->isLeaf) {
        Loop<dim>(0, 2, [&](auto& it) {
            this->children[it]->synchronize();
        });
    }
}

template <typename Config>
template <typename Function>
void Node<Config>::applyKernel(const Function& func)
{
    if (this->isLeaf) {
        this->action = DEREFINE;
        Loop<dim>(1, Config::blockSize, [&](auto& it) {
            func(DataView<Config>(*this, it));
        });
    } else {
        Loop<dim>(0, 2, [&](auto& it) {
            this->children[it]->applyKernel(func);
        });
    }
}

template <typename Config>
void Node<Config>::propagateUp()
{
    if (!this->isLeaf) {
        Loop<dim>(0, 2, [&](auto& it) {
            this->children[it]->propagateUp();
        });
    }
    if (this->propagate) {
        this->downsampleAll();
    }
}

template <typename Config>
void Node<Config>::propagateDown()
{
    if (this->isLeaf) {
        return;
    }
    if (this->propagate || this->sync) {
        Loop<dim>(0, 3, [&](auto& it) {
            u64vec<dim> fromSrc, toSrc, fromDst;
            for (u8 i = 0; i<dim; ++i) {
                fromSrc[i] = it[i] == 0 ? Config::blockSize + 1 : -1;
                toSrc[i] = it[i] == 2 ? 1 : Config::blockSize + 2;
                fromDst[i] = it[i] == 0 ? -1 : Config::blockSize + 1;
                this->upsampleAllRange(fromSrc, toSrc, fromDst, repeat<dim, u64>(0));
            }
        });
    }
}

template<typename Config>
DataView<Config>::DataView(Node<Config> &node, u64vec<Config::dimension> base)
    : node(node)
    , base(base)
    , refine(false)
    , derefine(false)
{
}

template<typename Config>
void RefinePlan<Config>::propagateUp(const u64vec<dim>& index, u32 currentLevel)
{
    // TODO: set propagate
    ASSERT(this->propagate);
    u64 offset = 1 << (this->level - currentLevel - 1);
    this->position = index * offset;
}

template<typename Config>
bool RefinePlan<Config>::merge(Self *other, u32 level)
{
    if (this->level != other->level) {
        return false;
    }

    RefinePlan merged;
    merged.level = this->level;
    merged.propagate = false;
    int diffCoord = -1;
    for (u8 i = 0; i<dim; ++i) {
        if (this->position[i] == other->position[i] &&
                this->size[i] == other->size[i]) {
            merged.position[i] = this->position[i];
            merged.size[i] = this->size[i];
        } else if (diffCoord != -1) {
            return false;
        } else if (this->position[i] + this->size[i] == other->position[i]) {
            merged.position[i] = this->position[i];
            merged.size[i] = this->size[i] + other->size[i];
            diffCoord = i;
        } else if (other->position[i] + other->size[i] == this->position[i]){
            merged.position[i] = other->position[i];
            merged.size[i] = this->size[i] + other->size[i];
            diffCoord = i;
        } else {
            return false;
        }
        if (merged.position[i] + merged.size[i] == 1 << (merged.level - level)
                || merged.position[i] == 0) {
            merged.propagate = true;
        }
    }
    ASSERT(diffCoord != -1);
    auto nodesBuffer = std::make_shared<Buffer<Node<Config>*, dim>>(merged.size);
    auto pos1 = repeat<dim, u64>(0);
    auto pos2 = repeat<dim, u64>(0);
    for (u8 i = 0; i<dim; ++i) {
        if (i == diffCoord) {
            if (this->position[i] > other->position[i]) {
                pos1[i] = other->size[i];
            } else {
                pos2[i] = this->size[i];
            }
        }
    }
    Array<Node<Config>*, dim> array1(nodesBuffer, this->size, pos1);
    Array<Node<Config>*, dim> array2(nodesBuffer, other->size, pos2);
    Loop<dim>(repeat<dim, u64>(0), this->size, [&](auto& it) {
       array1[it] = this->nodes[it];
    });
    Loop<dim>(repeat<dim, u64>(0), other->size, [&](auto& it) {
       array2[it] = other->nodes[it];
    });
    merged.nodes = Array<Node<Config>*, dim>(nodesBuffer,
                                             merged.size, repeat<dim, u64>(0));
    *this = merged;
    return true;
}

template<typename Config>
Tree<Config>::Tree()
    : root(new Node<Config>())
{
}

template <typename Config>
void Tree<Config>::restructure()
{
    this->root->calculateRefinePlanRecursive();
    this->root->restructure();
    this->root->updateAdjacency();
}

template <typename Config>
void Tree<Config>::synchronize()
{
    this->root->propagateUp();
    this->root->synchronize();
}

template <typename Config>
template<typename Function>
void Tree<Config>::applyKernel(const Function &func)
{
    this->root->applyKernel(func);
}

template<typename Config>
Mesh<Config>::Mesh(const u64vec<Config::dimension> &size)
    : trees(Array<Tree<Config>, Config::dimension>::createWithBuffer(size))
{
    Loop<Config::dimension>(repeat<Config::dimension>(0UL),
                            this->trees.size, [&](auto& it) {
        new (&this->trees[it]) Tree<Config>();
    });
    this->updateStructure();
}

template <typename Config>
void Mesh<Config>::updateStructure()
{
    Loop<Config::dimension>(repeat<Config::dimension>(0UL),
                            this->trees.size, [&](auto& it) {
        this->trees[it].restructure();
        this->trees[it].synchronize();
    });
}

template <typename Config>
template<typename Function>
void Mesh<Config>::applyKernel(const Function &func)
{
    Loop<Config::dimension>(repeat<Config::dimension>(0UL),
                            this->trees.size, [&](auto& it) {
        this->trees[it].applyKernel(func);
    });
}
