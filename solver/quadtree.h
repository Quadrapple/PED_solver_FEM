#pragma once
#include "femmesh.h"
#include <memory>
#include <vector>

class Quadtree;

struct QuadtreeQuad {
    float split;
    bool full;
    int size;
    std::vector<unsigned int> array;
    const Quadtree *parent;

    QuadtreeQuad(const Quadtree *parent);

    std::unique_ptr<QuadtreeQuad> hi;
    std::unique_ptr<QuadtreeQuad> lo;

    void putall(int depth, int maxlen);
    void print();
};

class Quadtree {
    public:
        Quadtree(const Quadtree&) = delete;
        Quadtree operator=(const Quadtree&) = delete;

        Quadtree() {
        }

        const int maxsize = 3;
        void putall(std::shared_ptr<std::vector<Node>> nodes);

        bool compareX(const unsigned int &a, const unsigned int &b) const;
        bool compareY(const unsigned int &a, const unsigned int &b) const;

        std::shared_ptr<std::vector<Node>> nodes;
        std::unique_ptr<QuadtreeQuad> root;
};
