#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include "GPoint.h"
#include "BlendFunctions.h"
#include <iostream>


class TriColorShader : public GShader {

public:

    TriColorShader (GPoint points[], GColor color[]) {
        GPoint p0 = points[0];
        GPoint p1 = points[1];
        GPoint p2 = points[2];
        float x1 = p1.x() - p0.x();
        float y1 = p1.y() - p0.y();
        float x2 = p2.x() - p0.x();
        float y2 = p2.y() - p0.y();

        lm = tm = GMatrix();
        lm = GMatrix(
            x1,x2, p0.x(),
            y1,y2, p0.y());
        tm=lm;
        c0 = color[0];
        c1 = color[1];
        c2 = color[2];
    }

    bool isOpaque() {
        return false;
    }

    bool setContext(const GMatrix& ctm) {
        return(GMatrix::Concat(ctm,lm).invert(&tm));
    }

    void shadeRow(int x, int y, int count, GPixel row[]) {
        
        GPoint pointSrc[count];
        GPoint pointDst[count];
        for (int i = 0; i < count; i++) {
            pointSrc[i] = GPoint::Make(x + i + 0.5, y + 0.5);
        }  
        tm.mapPoints(pointDst,pointSrc,count);

       for (int i = 0; i < count; i++) {
            float xMapped = pointDst[i].x();
            float yMapped = pointDst[i].y();
            
            row[i] = makePremultPixel(triInterpolate(xMapped, yMapped).pinToUnit());
        }

        
    }
    

private:

    GMatrix tm;
    GMatrix lm;
    GColor c0;
    GColor c1;
    GColor c2;

    float triInterpolateValue(float x, float y, float v0, float v1, float v2) {
        float value = (x*v1) + (y*v2) + ((1.0-x-y)*v0);
        return value;
    } 

    GColor triInterpolate(float x, float y) {
        return GColor::RGBA(
            triInterpolateValue(x,y,c0.r,c1.r, c2.r),
            triInterpolateValue(x,y,c0.g,c1.g, c2.g),
            triInterpolateValue(x,y,c0.b,c1.b, c2.b),
            triInterpolateValue(x,y,c0.a,c1.a, c2.a)
        ).pinToUnit();
    }

};


std::unique_ptr<GShader> GCreateTriColorShader(GPoint points[], GColor color[]) {
    return (std::unique_ptr<GShader>) new TriColorShader(points, color);
}

