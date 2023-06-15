#ifndef EdgeBuilder_DEFINED
#define EdgeBuilder_DEFINED

#include "Edge.h"
#include "GPoint.h"
#include "GMath.h"
#include <vector>
#include "GPath.h"

class EdgeBuilder {

    public:

        EdgeBuilder(int left, int top, int right, int bottom)
        : clipLeft(left), clipTop(top), clipRight(right), clipBottom(bottom) {
            this->validate();
        }

        void generateAndAddEdge(const GPoint& p1, const GPoint& p2, std::vector<Edge>& edgeList) {
            
            int y1 = GRoundToInt(p1.y());
            int y2 = GRoundToInt(p2.y());
            
            if (y1 == y2 || (y2 < clipTop && y1 < clipTop) || (y2 > clipBottom && y1 > clipBottom)) {
                return;
            }
            int orientation = y2 > y1 ? 1 : -1;
            int x1 = GRoundToInt(p1.x());
            int x2 = GRoundToInt(p2.x());

            if (y2 > y1) {
                std::swap(x1,x2);
                std::swap(y1,y2);
            }
            
            float m = (p2.x() - p1.x())/(p2.y() - p1.y());
            float b = p2.x() - (m*p2.y());
            
            clipTopBottom(x1, y1, x2, y2, m, b);

            if (y1 == y2) {
                return;
            }

            clipRightLeftAndProject(x1, y1, x2, y2, m, b, edgeList, orientation);

        }

        void getClippingLR() {
            printf("[%d,%d]", clipLeft, clipRight);
        }

        void generateQuadraticEdgesAndAdd(GPoint& a, GPoint& b, GPoint& c, float t,std::vector<Edge>& edgeList) {

            float x = a.x() - (2 * b.x()) + c.x();
            float y = a.y() - (2 * b.y()) + c.y();
            int numSeg = sqrt( sqrt(x*x + y*y) / t);
            int nMax = log2(numSeg);
            quadSegmentGenerator(a,b,c,numSeg,edgeList);
            //generateQuadraticEdgesAndAdd(a,b,c,numSeg, 0, nMax, edgeList);
        }

        void generateQuadraticEdgesAndAdd(GPoint& a, GPoint& b, GPoint& c, int numSeg, int n, int nMax, std::vector<Edge>& edgeList) {
            bool aBounded = inTopBottomBounds(a);
            bool bBounded = inTopBottomBounds(b);
            bool cBounded = inTopBottomBounds(c);

            if (!aBounded && !bBounded && !cBounded) {
                return;
            }
            if ( (aBounded && bBounded && cBounded) || n >= nMax) {
                quadSegmentGenerator(a, b, c, numSeg>>n , edgeList);
                return;
            }

            GPoint dst[5];
            GPoint src[3];
            src[0] = a;
            src[1] = b;
            src[2] = c;
            GPath::ChopQuadAt(src,dst,0.5);

            generateQuadraticEdgesAndAdd(src[0],src[1],src[2], numSeg, n+1, nMax, edgeList);
            generateQuadraticEdgesAndAdd(src[2],src[3],src[4], numSeg, n+1, nMax, edgeList);
            
        }

        void generateCubicEdgesAndAdd(GPoint& a, GPoint& b, GPoint& c, GPoint& d, float t,std::vector<Edge>& edgeList) {

            float ux = a.x() - (2*b.x()) + c.x();
            float uy = a.y() - (2*b.y()) + c.y();
            float vx = b.x() - (2*c.x()) + d.x();
            float vy = b.y() - (2*c.y()) + d.y();
            
            float x = std::max(ux, vx);
            float y = std::max(uy, vy);

            float w = sqrt( (x*x) + (y*y) );
            int numSeg = sqrt( (3.0 * w) / (4.0*t) );
            int nMax = log2(numSeg);
            //generateCubicEdgesAndAdd(a,b,c,d,numSeg, 0, nMax, edgeList);
            cubicSegmentGenerator(a,b,c,d,numSeg, edgeList);
        }

        void generateCubicEdgesAndAdd(GPoint& a, GPoint& b, GPoint& c, GPoint& d, int numSeg, int n, int nMax, std::vector<Edge>& edgeList) {
            bool aBounded = inTopBottomBounds(a);
            bool bBounded = inTopBottomBounds(b);
            bool cBounded = inTopBottomBounds(c);
            bool dBounded = inTopBottomBounds(d);

            if (!aBounded && !bBounded && !cBounded && !dBounded) {
                return;
            }

            if ( (aBounded && bBounded && cBounded && dBounded) || n >= nMax) {
                cubicSegmentGenerator(a, b, c, d, numSeg>>n , edgeList);
                return;
            }

            GPoint midPoint = GPoint::Make(
                (a.x() + (2*b.x()) + (3*c.x()) + d.x()) / 8.0,
                (a.y() + (2*b.y()) + (3*c.y()) + d.y()) / 8.0
            );

            GPoint newB1 = GPoint::Make(
                (a.x() + b.x()) / 2.0,
                (a.y() + b.y()) / 2.0
            );
            GPoint newC1 = GPoint::Make(
                (a.x() + (2 * b.x()) + c.x() ) / 4.0,
                (a.y() + (2 * b.y()) + c.y() ) / 4.0
            );

            GPoint newB2 = GPoint::Make(
                (b.x() + (2*c.x()) + d.x()) / 4.0,
                (b.y() + (2*c.y()) + d.y()) / 4.0
            );
            GPoint newC2 = GPoint::Make(
                (c.x() + d.x()) / 2.0,
                (c.y() + d.y()) / 2.0
            );

            generateCubicEdgesAndAdd(a, newB1, newC1, midPoint, numSeg, n+1, nMax, edgeList);
            generateCubicEdgesAndAdd(midPoint, newB2, newC2, c, numSeg, n+1, nMax, edgeList);
            
        }

    private:

        const int clipLeft;
        const int clipTop;
        const int clipRight;
        const int clipBottom;

        void validate() const {
            assert(clipLeft <= clipRight);
            assert(clipTop <= clipBottom);
        }

        //Assumes y1 > y2
        void clipTopBottom(int& x1, int& y1, int& x2, int& y2, float& m, float& b) {
            
            if (y1 > clipBottom) {
                y1 = clipBottom;
                x1 =  getX(m, b, y1); //GRoundToInt((m * y1) + b);
            }
            if (y2 < clipTop) {
                y2 = clipTop;
                x2 = getX(m,b,y2);
            }
        }

        //Assumes y1 > y2
        void clipRightLeftAndProject(int& x1, int& y1, int& x2, int& y2, float& m, float& b, std::vector<Edge>& edgeList, int orientation) {
            // printf("\n--%d,%d--", clipBottom, clipTop);
            // if (y1 <= clipTop || y2 <= clipTop ) {
            //     printf("\n<%d,%d>",y1,y2);
            // }

            // if (y1 >= clipBottom || y2 >= clipBottom ) {
            //     printf("\n<%d,%d>",y1,y2);
            // }

            if (x1 >= clipRight && x2 >= clipRight) {
                edgeList.push_back(makeEdge(y2,y1,0,clipRight, orientation));
                return;
            }
            if (x1 <= clipLeft && x2 <= clipLeft) {
                edgeList.push_back(makeEdge(y2,y1,0,clipLeft, orientation));
                return;
            }

            if (x1 > clipRight) {
                int y3 = y1;
                x1 = clipRight;
                y1 = GRoundToInt(( x1 - b)/m); //(( x1 - .5 - b)/m)
                edgeList.push_back(makeEdge(y1,y3,0,clipRight, orientation));
            } else if (x1 < clipLeft) {
                int y3 = y1;
                x1 = clipLeft;
                y1 = GRoundToInt((x1 - b)/m); //((x1 + 0.5 - b)/m)
                edgeList.push_back(makeEdge(y1,y3,0,clipLeft, orientation));
            }

            if (x2 > clipRight) {
                int y4 = y2;
                x2 = clipRight;
                y2 = GRoundToInt((x2 - b)/m); //((x2 - .5 - b)/m)
                edgeList.push_back(makeEdge(y4,y2,0,clipRight, orientation));
            } else if (x2 < clipLeft) {
                int y4 = y2;
                x2 = clipLeft;
                y2 = GRoundToInt((x2 - b)/m);
                edgeList.push_back(makeEdge(y4,y2,0,clipLeft, orientation));
            }
            edgeList.push_back(makeEdge(y2,y1,m,b, orientation));
        }


    bool inTopBottomBounds(GPoint& p) {
        return p.y() < clipBottom && p.y() > clipTop;
    }

    void quadSegmentGenerator (GPoint& a, GPoint& b, GPoint& c, int numSeg, std::vector<Edge>& edgeList) {
        
        GPoint prev = a;

        float t = 1.0/numSeg;
        float dt = t;

        for (int i = 1; i < numSeg-1; i++) {
            GPoint current = GPoint::Make(hornerEvalQuad(a.x(), b.x(), c.x(), t), hornerEvalQuad(a.y(), b.y(), c.y(), t));
            generateAndAddEdge(prev, current, edgeList);
            prev = current;
            t += dt;
        }
        generateAndAddEdge(prev, c, edgeList);
        
    }

    void cubicSegmentGenerator (GPoint& a, GPoint& b, GPoint& c, GPoint& d, int numSeg, std::vector<Edge>& edgeList) {
        
        if (numSeg == 0) {
            numSeg = 1;
        }

        GPoint prev = a;

        float t = 1.0/numSeg;
        float dt = t;

        for (int i = 1; i < numSeg-1; i++) {
            GPoint current = GPoint::Make(hornerEvalCubic(a.x(), b.x(), c.x(), d.x(), t), hornerEvalCubic(a.y(), b.y(), c.y(), d.y(), t));
            generateAndAddEdge(prev, current, edgeList);
            prev = current;
            t += dt;
        }
        generateAndAddEdge(prev, d, edgeList);

    }

    

};

#endif