#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include "GPoint.h"
#include "BlendFunctions.h"
#include <iostream>


class TriBitmapShader : public GShader {

public:

    TriBitmapShader (GPoint triPoints[], GPoint texPoints[], GShader* shader) {
        
        this->shader = shader;

        GPoint p0 = triPoints[0];
        GPoint p1 = triPoints[1];
        GPoint p2 = triPoints[2];

        GPoint t0 = texPoints[0];
        GPoint t1 = texPoints[1];
        GPoint t2 = texPoints[2];

        float px1 = p1.x() - p0.x();
        float py1 = p1.y() - p0.y();
        float px2 = p2.x() - p0.x();
        float py2 = p2.y() - p0.y();

        float tx1 = t1.x() - t0.x();
        float ty1 = t1.y() - t0.y();
        float tx2 = t2.x() - t0.x();
        float ty2 = t2.y() - t0.y();

        GMatrix P = GMatrix(
            px1,px2, p0.x(),
            py1,py2, p0.y()
        );

        GMatrix T = GMatrix(
            tx1,tx2, t0.x(),
            ty1,ty2, t0.y()
        );

        GMatrix invT;
        T.invert(&invT);
        lm = GMatrix::Concat(P,invT);

        this->shader->setContext(GMatrix());
    }

    bool isOpaque() {
        return false;
    }

    bool setContext(const GMatrix& ctm) {
        return(this->shader->setContext(GMatrix::Concat(ctm, lm)));
    }

    void shadeRow(int x, int y, int count, GPixel row[]) {
        this->shader->shadeRow(x,y,count,row);
    }
    

private:

    GMatrix lm;
    GShader* shader;

};


std::unique_ptr<GShader> GCreateTriBitmapShader(GPoint triPoints[], GPoint texPoints[], GShader* shader) {
    return (std::unique_ptr<GShader>) new TriBitmapShader(triPoints, texPoints, shader);
}

