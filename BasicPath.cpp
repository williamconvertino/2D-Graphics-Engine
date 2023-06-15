#include "GPath.h"
#include "MathUtil.h"
#include "GMatrix.h"

GPath& GPath::addRect(const GRect& r, Direction dir) {
    
    moveTo(r.left(), r.top());
    if (dir == kCCW_Direction) {
        lineTo(r.left(), r.bottom());
        lineTo(r.right(), r.bottom());
        lineTo(r.right(), r.top());
    } else {
        lineTo(r.right(), r.top());
        lineTo(r.right(), r.bottom());
        lineTo(r.left(), r.bottom());
    }
    
    return *this;
}

GPath& GPath::addPolygon(const GPoint pts[], int count) {
    moveTo(pts[0]);
    for (int i = 1; i < count; i++) {
        lineTo(pts[i]);
    }
    return *this;
}

GRect GPath::bounds() const {
    float xMin = FLT_MAX;
    float xMax = FLT_MIN;
    float yMin = FLT_MAX;
    float yMax = FLT_MIN;

    for (GPoint point: fPts) {
       
        xMin = std::min(xMin, point.x());
        xMax = std::max(xMax, point.x());
        yMin = std::min(yMin, point.y());
        yMax = std::max(yMax, point.y());
    }
    return GRect::MakeLTRB(xMin, yMin, xMax, yMax);
    
    //return GRect::LTRB(0,0,0,0);
}

void GPath::transform(const GMatrix& m) {
    int count = fPts.size();
    GPoint pts[count];
    GPoint transformed[count];
    std::copy(fPts.begin(), fPts.end(), pts);
    m.mapPoints(transformed, pts, count);
    fPts.clear();
    for (GPoint point: transformed) {
        fPts.push_back(point);
    }
}

float circleOffset = 0.55191502449351;

GPath& GPath::addCircle(GPoint center, float radius, Direction direction) {

    GPoint a = GPoint::Make(center.x(),center.y() - radius);
    GPoint b = GPoint::Make(a.x() + (radius * circleOffset), a.y());
    GPoint c = GPoint::Make(center.x() + radius, center.y() - (radius * circleOffset));
    GPoint d = GPoint::Make(center.x() + radius, center.y());
    GPoint e = GPoint::Make(center.x() + radius, center.y() + (radius * circleOffset));
    GPoint f = GPoint::Make(center.x() + (radius * circleOffset), center.y() + radius);
    GPoint g = GPoint::Make(center.x(), center.y() + radius);
    GPoint h = GPoint::Make(center.x() - (radius * circleOffset), center.y() + radius);
    GPoint i = GPoint::Make(center.x() - radius, center.y() + (radius * circleOffset));
    GPoint j = GPoint::Make(center.x() - radius, center.y());
    GPoint k = GPoint::Make(center.x() - radius, center.y() - (radius * circleOffset));
    GPoint l = GPoint::Make(center.x() - (radius * circleOffset), center.y() - radius);

    if (direction == kCW_Direction) {
        moveTo(a);
        cubicTo(b,c,d);
        cubicTo(e,f,g);
        cubicTo(h,i,j);
        cubicTo(k,l,a);
    } else {
        moveTo(a);
        cubicTo(l,k,j);
        cubicTo(i,h,g);
        cubicTo(f,e,d);
        cubicTo(c,b,a);
    }
    

    return *this;
}
            
void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    GPoint a = src[0];
    GPoint b = src[1];
    GPoint c = src[2];
    
    GPoint tPoint = GPoint::Make(
        hornerEvalQuad(a.x(),b.x(),c.x(),t),
        hornerEvalQuad(a.y(),b.y(),c.y(),t)
    );

    GPoint newB1 = GPoint::Make(
        interpolate(a.x(),b.x(),t),
        interpolate(a.y(),b.y(),t)
    );

    GPoint newB2 = GPoint::Make(
        interpolate(b.x(),c.x(),t),
        interpolate(b.y(),c.y(),t)
    );

    dst[0] = a;
    dst[1] = newB1;
    dst[2] = tPoint;
    dst[3] = newB2;
    dst[4] = c;
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    GPoint a = src[0];
    GPoint b = src[1];
    GPoint c = src[2];
    GPoint d = src[3];
    
    GPoint midPoint = GPoint::Make(
        hornerEvalCubic(a.x(),b.x(),c.x(), d.x(), t),
        hornerEvalCubic(a.y(),b.y(),c.y(),d.y(),t)
    );

    GPoint aP1 = GPoint::Make(
        interpolate(a.x(),b.x(),t),
        interpolate(a.y(),b.y(),t)
    );

    GPoint bP2 = GPoint::Make(
        interpolate(c.x(),d.x(),t),
        interpolate(c.y(),d.y(),t)
    );

    GPoint tempPoint = GPoint::Make(
        interpolate(b.x(),c.x(),t),
        interpolate(b.y(),c.y(),t)
    );
    GPoint aP2 = GPoint::Make(
        interpolate(aP1.x(),tempPoint.x(),t),
        interpolate(aP1.y(),tempPoint.y(),t)
    );
    GPoint bP1 = GPoint::Make(
        interpolate(tempPoint.x(),bP2.x(),t),
        interpolate(tempPoint.y(),bP2.y(),t)
    );

    dst[0] = a;
    dst[1] = aP1;
    dst[2] = aP2;
    dst[3] = midPoint;
    dst[4] = bP1;
    dst[5] = bP2;
    dst[6] = d;
}