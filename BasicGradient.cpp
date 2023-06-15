#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include "GPoint.h"
#include "BlendFunctions.h"
#include <iostream>
#include "GradientFunctions.h"
class BasicGradient : public GShader {

public:

    BasicGradient (GPoint p0, GPoint p1, const GColor color[] , int count, GShader::TileMode tileMode, GradientFunction gf) {
        float xM;
        float yM;
        
        xM = p1.x() - p0.x();
        yM = p1.y() - p0.y();

        this->tileMode = tileMode;
        gradientFunction = gf;
        
        lm = GMatrix(
            xM,yM, p0.x(),
            yM,-xM, p0.y());
        tm = lm;
        colorMap = std::vector<GColor>();
        for (int i = 0; i < count; i++) {
            colorMap.push_back(color[i]);
        }
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
            xMapped = gradientFunction(xMapped, yMapped);
            //float yMapped = pointDst[i].y();

            // if (xMapped >= bm.width()) {
            //     xMapped = bm.width()-1;
            // }
        
            // if (xMapped < 0) {
            //     xMapped = 0;
            // }
           row[i] = getPixel(xMapped); 
           
            //std::cout << "\n" <<  "stop" << std::endl;
        }
        // std::cout << "\n" <<  "fff" << std::endl;
    }

private:

    GMatrix tm;
    GMatrix lm;
    GBitmap bm;
    std::vector<GColor> colorMap;
    float size;
    GShader::TileMode tileMode;

    GradientFunction gradientFunction;

    GPixel getPixel(float x) {
        if (colorMap.size() < 1) {
            return makePremultPixel(GColor::RGBA(0,0,0,0));
        }
        if (colorMap.size() == 1) {
            return makePremultPixel(colorMap.at(0));
        }
        switch (tileMode) {
            default:
            case GShader::kClamp:
                if (x >= 1) {
                    return makePremultPixel(colorMap.at(colorMap.size() -1));
                }
                if (x <= 0) {
                    return makePremultPixel(colorMap.at(0));
                }
                break;
            case GShader::kRepeat:
                x = x-floor(x);
                break;
            case GShader::kMirror:
                x *= 0.5;
                x = x-floor(x);
                x *= 2;
        }
        
        float loc = x * (colorMap.size() - 1);
        int index = GFloorToInt(loc);
        float w = loc - index;
        assert(index < colorMap.size() - 1);
        return interpolate(colorMap.at(index), colorMap.at(index + 1),w);
        
    }
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor color[] , int count) {
    return (std::unique_ptr<GShader>) new BasicGradient(p0,p1,color, count, GShader::kClamp, mapToLinear);
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor color[] , int count, GShader::TileMode tileMode) {
    return (std::unique_ptr<GShader>) new BasicGradient(p0,p1,color, count, tileMode, mapToLinear);
}


std::unique_ptr<GShader> GCreateRadialGradient(GPoint p0, GPoint p1, const GColor color[] , int count, GShader::TileMode tileMode) {
    return (std::unique_ptr<GShader>) new BasicGradient(p0,p1,color, count, tileMode, mapToRadial);
}

std::unique_ptr<GShader> GCreateRayGradient(GPoint p0, GPoint p1, const GColor color[] , int count, GShader::TileMode tileMode) {
    return (std::unique_ptr<GShader>) new BasicGradient(p0,p1,color, count, tileMode, mapToRadialRays);
}