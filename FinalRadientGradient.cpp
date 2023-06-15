#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include "GPoint.h"
#include "BlendFunctions.h"


class FinalRadialGradient : public GShader {

public:

    FinalRadialGradient (GPoint center, float radius, const GColor color[] , int count, GShader::TileMode tileMode) {
        
        GPoint p0 = center;

        GPoint p1 = GPoint::Make(center.x() + radius, center.y());

        float xM;
        float yM;
        
        xM = p1.x() - p0.x();
        yM = p1.y() - p0.y();

        this->tileMode = tileMode;
        
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
            xMapped = std::sqrt(xMapped*xMapped + yMapped*yMapped);
            
           row[i] = getPixel(xMapped); 
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
                if (x >= 0.9999f) {
                    //return makePremultPixel(colorMap.at(colorMap.size() -1));
                    x = 0.9999f;
                }
                if (x <= 0) {
                    //return makePremultPixel(colorMap.at(0));
                    x=0;
                }
                break;
            case GShader::kRepeat:
                x = x-floor(x);
                break;
            case GShader::kMirror:
                x *= 0.5;
                x = x-floor(x);
                x *= 2.0;

                if (x >= 1) {
                    x = 2.0-x;
                }
                break;
        }
        
        float loc = x * (colorMap.size() - 1);
        int index = GFloorToInt(loc);
        float w = loc - index;
        //assert(index < colorMap.size() - 1);
        // if (index >= colorMap.size() - 1) {
        //   //printf("%d, %d\n",index, colorMap.size());
        //     //index = colorMap.size() - 1;
        //     return makePremultPixel(colorMap.at(colorMap.size() -1));
        // }
        //assert(index < colorMap.size() - 1);
        return interpolate(colorMap.at(index), colorMap.at(index + 1),w);
        
    }
};

std::unique_ptr<GShader> GCreateFinalRadialGradient(GPoint center, float radius, const GColor color[] , int count, GShader::TileMode tileMode) {
    return (std::unique_ptr<GShader>) new FinalRadialGradient(center,radius,color, count, tileMode);
}