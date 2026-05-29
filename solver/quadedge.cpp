#include "quadedge.h"
#include <glm/ext/vector_float2.hpp>
#include <utility>


Edge* Edge::sym() {
    return (index < 2) ? (this + 2) : (this - 2); 
}

Edge* Edge::rotCCW() {
    return (index < 3) ? (this + 1) : (this - 3);
}

Edge* Edge::rotCW() {
    return (index > 1) ? (this - 1) : (this + 3);
}

Edge* Edge::Onext() {
    return next;
}

Edge* Edge::Oprev() {
    return rotCCW()->Onext()->rotCCW();
}

Edge* Edge::Dnext() {
    return sym()->Onext()->sym();
}

Edge* Edge::Dprev() {
    return rotCW()->Onext()->rotCW();
}

Edge* Edge::Lnext() {
    return rotCW()->Onext()->rotCCW();
}

Edge* Edge::Lprev() {
    return Onext()->sym();
}

Edge* Edge::Rnext() {
    return rotCCW()->Onext()->rotCW();
}

Edge* Edge::Rprev() {
    return sym()->Onext();
}

void Edge::setOnext(Edge *newOnext) {
    this->next = newOnext;
}

Edge* QuadEdge::makeEdge() {
    edgeRecords.push_back(new EdgeRecord());
    return edgeRecords.back()->edges;
}

void QuadEdge::splice(Edge *a, Edge *b) {
    Edge *tmp = a->Onext()->rotCCW()->Onext();
    a->Onext()->rotCCW()->setOnext(b->Onext()->rotCCW()->Onext());
    b->Onext()->rotCCW()->setOnext(tmp);

    tmp = a->Onext();
    a->setOnext(b->Onext());
    b->setOnext(tmp);
}

void Edge::setOrig(int orig) {
    origin = orig;
}
void Edge::setDest(int dest) {
    sym()->origin = dest;
}

int Edge::orig() {
    return origin;
}
int Edge::dest() {
    return sym()->orig();
}

Edge* QuadEdge::connect(Edge *a, Edge *b) {
    Edge *e = makeEdge();

    printf("connected %d and %d\n", a->dest(), b->orig());
    e->setOrig(a->dest());
    e->setDest(b->orig());

    splice(e, a->Lnext());
    splice(e->sym(), b);
    return e;
}

void QuadEdge::deleteEdge(Edge *e) {
    splice(e, e->Oprev());
    splice(e->sym(), e->sym()->Oprev());

    e->setOrig(-1);
    e->setDest(-1);
}

void QuadEdge::swap(Edge *e) {
    Edge *a = e->Oprev();
    Edge *b = e->sym()->Oprev();

    splice(e, a);
    splice(e->sym(), b);
    splice(e, a->Lnext());
    splice(e->sym(), b->Lnext());

    e->setOrig(a->dest());
    e->setDest(b->dest());
}

float triangleDet(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
    return ((a.x - b.x)*(a.y - c.y) - (a.x - c.x)*(a.y - b.y));
}

bool CCW(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
    return triangleDet(a, b, c) > 0;
}

float inCircle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 t) {
    const float a_sq = a.x*a.x + a.y*a.y;
    const float b_sq = b.x*b.x + b.y*b.y;
    const float c_sq = c.x*c.x + c.y*c.y;
    const float t_sq = t.x*t.x + t.y*t.y;

    const float det0 = a_sq * triangleDet(b, c, t);
    const float det1 = b_sq * triangleDet(a, c, t);
    const float det2 = c_sq * triangleDet(a, b, t);
    const float det3 = t_sq * triangleDet(a, b, c);

    return det0 - det1 + det2 - det3;
}

bool QuadEdge::rightOf(Edge *e, glm::vec2 p) {
    return CCW(p, destOf(e).position, origOf(e).position);
}

bool QuadEdge::leftOf(Edge *e, glm::vec2 p) {
    return CCW(p, origOf(e).position, destOf(e).position);
}

bool QuadEdge::isValid(Edge *e, Edge *basel) {
    return rightOf(basel, destOf(e).position);
}

std::pair<Edge*, Edge*> QuadEdge::makeLine(unsigned int aInd, unsigned int bInd) {
    Edge *e_ab = makeEdge();

    e_ab->setOrig(aInd);
    e_ab->setDest(bInd);

    return std::make_pair(e_ab, e_ab->sym());
}

std::pair<Edge*, Edge*> QuadEdge::makeTriangle(unsigned int aInd, unsigned int bInd, unsigned int cInd) {
    Edge *e_ab = makeEdge();
    Edge *e_bc = makeEdge();

    e_ab->setOrig(aInd);
    e_ab->setDest(bInd);
    e_bc->setOrig(bInd);
    e_bc->setDest(cInd);

    splice(e_ab->sym(), e_bc);

    Edge *e_ac;

    if(CCW(nodes->at(aInd).position, nodes->at(bInd).position, nodes->at(cInd).position)) {

        printf("triangle %d, %d, %d\n", aInd, bInd, cInd);
        e_ac = connect(e_bc, e_ab);
        return std::make_pair(e_ab, e_bc->sym());

    } else if(CCW(nodes->at(aInd).position, nodes->at(cInd).position, nodes->at(bInd).position)) {

        printf("triangle %d, %d, %d\n", aInd, cInd, bInd);
        e_ac = connect(e_bc, e_ab);
        return std::make_pair(e_ac->sym(), e_ac);

    } else {
        printf("colinear\n");
        return std::make_pair(e_ab, e_bc->sym());
    }
}

std::pair<Edge*, Edge*> QuadEdge::delaunay(unsigned int *nodes, int size) {
    auto &nodeInds = nodes;

    if(size == 2) {
        return makeLine(nodeInds[0], nodeInds[1]);
    } else if(size == 3) {
        return makeTriangle(nodeInds[0], nodeInds[1], nodeInds[2]);
    }

    auto ldo_ldi = delaunay(nodes, size / 2);
    auto ldi = ldo_ldi.second;
    auto ldo = ldo_ldi.first;

    auto rdo_rdi = delaunay(nodes + size / 2, size - size / 2);
    auto rdi = rdo_rdi.first;
    auto rdo = rdo_rdi.second;

    while(true) {
        if(leftOf(ldi, origOf(rdi).position)) {
            ldi = ldi->Lnext();
        } else if(rightOf(rdi, origOf(ldi).position)) {
            rdi = rdi->Rprev();
        } else {
            break;
        }
    }

    Edge *basel = connect(rdi->sym(), ldi);
    if(ldi->orig() == ldo->orig()) {
        ldo = basel->sym();
    }
    if(rdi->orig() == rdo->orig()) {
        rdo = basel;
    }

    Edge *lcand;
    Edge *rcand;
    while(true) {
        lcand = basel->sym()->Onext();
        if(isValid(lcand, basel)) {
            while(inCircle(destOf(basel).position, origOf(basel).position, destOf(lcand).position, destOf(lcand->Onext()).position) > 0) {
                Edge *tmp = lcand->Onext();
                deleteEdge(lcand);
                lcand = tmp;
            }
        }

        rcand = basel->Oprev();
        if(isValid(rcand, basel)) {
            while(inCircle(destOf(basel).position, origOf(basel).position, destOf(rcand).position, destOf(rcand->Oprev()).position) > 0) {
                Edge *tmp = rcand->Oprev();
                deleteEdge(rcand);
                rcand = tmp;
            }
        }

        bool rcandValid = isValid(rcand, basel);
        bool lcandValid = isValid(lcand, basel);

        if(!rcandValid && !lcandValid) {
            break;
        }

        if(!lcandValid || (rcandValid &&
                    inCircle(destOf(lcand).position, origOf(lcand).position, origOf(rcand).position, destOf(rcand).position) > 0)) {
            basel = connect(rcand, basel->sym());
        } else {
            basel = connect(basel->sym(), lcand->sym());
        }
    }
    return std::make_pair(ldo, rdo);
}

const Node& QuadEdge::destOf(Edge *e) {
    return nodes->at(e->dest());
}

const Node& QuadEdge::origOf(Edge *e) {
    return nodes->at(e->orig());
}
