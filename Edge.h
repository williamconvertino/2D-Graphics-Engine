
#ifndef Edge_DEFINED
#define Edge_DEFINED

#include "GPoint.h"
#include <vector>

struct Edge{
    int yTop;
    int yBottom;
    float yIntercept;
    float slope;
    int orientation;
};

static inline Edge makeEdge(int yTop, int yBottom, float slope, float yIntercept, int orientation) {
    struct Edge edge;
    edge.yTop = yTop;
    edge.yBottom = yBottom;
    edge.slope = slope;
    edge.yIntercept = yIntercept;
    edge.orientation = orientation;
    return(edge);
}

static inline int getX(float& m, float& b, int& y) {
    return(GRoundToInt((m * (y + 0.5)) + b));
}

static inline int getX(Edge& e, int& y) {
    return(getX(e.slope, e.yIntercept, y));
}

bool compareEdgeLessThan(Edge& e1, Edge& e2) {
    if (e1.yTop < e2.yTop) {
        return(true);
    }
    
    if (e1.yTop != e2.yTop) {
        return(false);
    }

    int x1 = getX(e1,e1.yTop);
    int x2 = getX(e2,e2.yTop);

    if (x1 == x2) {    
        x1 = e1.slope;
        x2 = e2.slope;
    }
    return(x1 < x2);
}

bool compareEdgesByX(Edge& e1, Edge& e2, int y) {
    int x1 = getX(e1,y);
    int x2 = getX(e2,y);
    return(x1 < x2);
}


#endif