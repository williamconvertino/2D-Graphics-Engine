#include "GShader.h"
#include "GBitmap.h"
#include "GMatrix.h"
#include "GPoint.h"
#include "BlendFunctions.h"
#include <iostream>
#include "Debug.h"


class ComposedShader : public GShader {

public:

    ComposedShader (GShader* sh1, GShader* sh2) {
        shader_1 = sh1;
        shader_2 = sh2;
        assert(shader_1 != nullptr && shader_2 != nullptr);
    }

    bool isOpaque() {
        return shader_1->isOpaque() && shader_2->isOpaque();
    }

    bool setContext(const GMatrix& ctm) {
        //debug("StartContext");
        bool cont = shader_1->setContext(ctm) && shader_2->setContext(ctm);
        //debug("EndContext");
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) {
        //debug("StartShade");
        GPixel sRow1[count];
        GPixel sRow2[count];
        shader_1->shadeRow(x,y,count,sRow1);
        shader_2->shadeRow(x,y,count,sRow2);
        //debug("StopShade");

        for (int i = 0; i < count; i++) {
            
            row[i] = GPixel_PackARGB(
                div255(GPixel_GetA(sRow1[i]) * GPixel_GetA(sRow2[i])),
                div255(GPixel_GetR(sRow1[i]) * GPixel_GetR(sRow2[i])),
                div255(GPixel_GetG(sRow1[i]) * GPixel_GetG(sRow2[i])),
                div255(GPixel_GetB(sRow1[i]) * GPixel_GetB(sRow2[i]))
            );

        }
    }
    

private:

    GShader* shader_1;
    GShader* shader_2;

};


std::unique_ptr<GShader> GCreateComposedShader(GShader* sh1, GShader* sh2) {
    return (std::unique_ptr<GShader>) new ComposedShader(sh1,sh2);
}

