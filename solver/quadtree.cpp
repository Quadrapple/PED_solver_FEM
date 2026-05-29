#include "quadtree.h"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <random>

bool Quadtree::compareX(const unsigned int &a, const unsigned int &b) const {
    return nodes->at(a).position.x < nodes->at(b).position.x;
}

bool Quadtree::compareY(const unsigned int &a, const unsigned int &b) const {
    return nodes->at(a).position.y < nodes->at(b).position.y;
}

QuadtreeQuad::QuadtreeQuad(const Quadtree *parent) : hi(nullptr), lo(nullptr), array(), parent(parent), full(false) {
}

void QuadtreeQuad::print() {
    if(!full) {
        printf("(");
        for(auto n : array) {
            printf("%d, ", n);
        }
        printf(")");
        return;
    } else {
        hi->print();
        lo->print();
    }
}

void QuadtreeQuad::putall(int depth, int maxlen) {
    printf("depth %d\n", depth);
    size = array.size();
    if(this->array.size() <= maxlen) {
        std::sort(array.begin(), array.end(),
                [this] (const unsigned int &a, const unsigned int &b) { return parent->compareY(a,b); } );
        std::stable_sort(array.begin(), array.end(),
                [this] (const unsigned int &a, const unsigned int &b) { return parent->compareX(a,b); } );
        return;
    }
//  printf("tested size %d to maxsize %d\n", size, maxlen);

    int guessamount = glm::min((size_t)200, this->array.size());

    std::vector<float> medianBuf;
    medianBuf.reserve(guessamount);

    if(this->array.size() > 200) {
        //randomness gibberish
        auto distribution = std::uniform_int_distribution<>(0, guessamount - 1);
        std::mt19937 gen(rand());

        if(depth % 2 == 0) {
            for(int i = 0; i < guessamount; i++) {
                const int index = distribution(gen);
                medianBuf.push_back(parent->nodes->at(array[index]).position.x);
            }
        } else {
            for(int i = 0; i < guessamount; i++) {
                const int index = distribution(gen);
                medianBuf.push_back(parent->nodes->at(array[index]).position.y);
            }
        }

        std::sort(medianBuf.begin(), medianBuf.end());
        split = medianBuf[guessamount / 2];
    } else {
        if(depth % 2 == 0) {
            for(int i = 0; i < guessamount; i++) {
                medianBuf.push_back(parent->nodes->at(array[i]).position.x);
            }
        } else {
            for(int i = 0; i < guessamount; i++) {
                medianBuf.push_back(parent->nodes->at(array[i]).position.y);
            }
        }
        std::sort(medianBuf.begin(), medianBuf.end());
        split = medianBuf[guessamount / 2];
    }


    printf("median is %f \n", split);
    hi = std::make_unique<QuadtreeQuad>(parent);
    lo = std::make_unique<QuadtreeQuad>(parent);

    if(depth % 2 == 0) {
        for(auto nodeInd : this->array) {
            if(parent->nodes->at(nodeInd).position.x > split) {
                hi->array.push_back(nodeInd);
            } else if(parent->nodes->at(nodeInd).position.x < split) {
                lo->array.push_back(nodeInd);
            } else {
                //prevent one-point quadrants
                if(hi->array.size() < 2) {
                    hi->array.push_back(nodeInd);
                } else {
                    lo->array.push_back(nodeInd);
                }
            }
        }
    } else {
        for(auto nodeInd : this->array) {
            if(parent->nodes->at(nodeInd).position.y > split) {
                hi->array.push_back(nodeInd);
            } else if(parent->nodes->at(nodeInd).position.y < split) {
                lo->array.push_back(nodeInd);

            } else {
                //prevent one-point quadrants
                if(hi->array.size() < 2) {
                    hi->array.push_back(nodeInd);
                } else {
                    lo->array.push_back(nodeInd);
                }
            }
        }
    }
    printf("lo %zu, hi %zu\n", lo->array.size(), hi->array.size());
    if(lo->array.size() == 0) {
        printf("(");
        for(auto nodeInd : this->array) {
            float n = parent->nodes->at(nodeInd).position.x;
            printf("%f, ", n);
        }
        printf(")\n");
    }

    this->array.clear();
    this->full = true;
    hi->putall(depth + 1, maxlen);
    lo->putall(depth + 1, maxlen);
}

void Quadtree::putall(std::shared_ptr<std::vector<Node>> nodes) {
    this->nodes = nodes;
    printf("set nodes\n");
    root = std::make_unique<QuadtreeQuad>(this);

    for(unsigned int i = 0; i < nodes->size(); i++) {
        root->array.push_back(i);
    }
    printf("pushed into root arr\n");
    root->putall(0, maxsize);
    root->print();
}
