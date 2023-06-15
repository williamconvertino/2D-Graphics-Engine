#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include "GPoint.h"
#include <iostream>


class BasicShader : public GShader {

public:

    BasicShader (const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode tileMode) {
        lm = localMatrix;
        bm = bitmap;
        this->tileMode = tileMode;
    }

    bool isOpaque() {
        return false;
    }

    bool setContext(const GMatrix& ctm) {
        GMatrix finalMatrix = GMatrix::Concat(
            GMatrix::Concat(ctm,lm),
            GMatrix( bm.width(), 0, 0, 0, bm.height(), 0)
        );
        return(finalMatrix.invert(&tm));
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
            
            switch (tileMode) {
            default:
            case GShader::kClamp:
                if (xMapped > 1) {
                    xMapped = 1;
                }
                if (xMapped < 0) {
                    xMapped = 0;
                }
                if (yMapped > 1) {
                    yMapped = 1;
                }
                if (yMapped < 0) {
                    yMapped = 0;
                }
                break;
            case GShader::kRepeat:
                xMapped = xMapped-floor(xMapped);
                yMapped = yMapped-floor(yMapped);
                break;
            case GShader::kMirror:
                xMapped *= 0.5;
                xMapped = xMapped-floor(xMapped);
                xMapped *= 2;
                yMapped *= 0.5;
                yMapped = yMapped-floor(yMapped);
                yMapped *= 2;
                if (xMapped > 1) {
                    xMapped = 2 - xMapped;
                }
                if (yMapped > 1) {
                    yMapped = 2 - yMapped;
                }
        }   
            
            //printf("<%d,%d>", xMapped, yMapped);
            int xLoc = floor(xMapped * bm.width());
            int yLoc = floor(yMapped * bm.height());
            
            // if (xLoc > bm.width() || yLoc > bm.height()) {
            //     printf("\n<%d,%d>", xLoc, yLoc);
            //     printf("{%d,%d}", bm.width(), bm.height());
            //     std::cout << std::endl;
            // }

            assert(xLoc < bm.width());
            assert(yLoc < bm.height());
            row[i] = *bm.getAddr(xLoc,yLoc);
           
        }
        
    }
    

private:

    GMatrix tm;
    GMatrix lm;
    GBitmap bm;

    GShader::TileMode tileMode;

};


std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& lbm, const GMatrix& localMatrix) {
    return (std::unique_ptr<GShader>) new BasicShader(lbm, localMatrix, GShader::kClamp);
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& lbm, const GMatrix& localMatrix, GShader::TileMode tileMode) {
    return (std::unique_ptr<GShader>) new BasicShader(lbm, localMatrix, tileMode);
}

