
#include "GFinal.h"
#include "GMatrix.h"
#include "AdvancedShaders.h"

class BasicFinal: public GFinal {

    public:

    BasicFinal() {

    }

    std::unique_ptr<GShader> createRadialGradient (GPoint center, float radius, const GColor colors[], int count, GShader::TileMode mode) {
        
        return GCreateFinalRadialGradient(center, radius, colors, count, mode);
    }

    
    void addLine(GPath* path, GPoint p0, GPoint p1, float width, CapType cap) {

        float half_width = width/2.0;

        //Make v and u vectors

        GVector v = GVector();

        v.set(p1.x() - p0.x(), p1.y() - p0.y());

        GVector vPerp;
        vPerp.set(-v.y(), v.x());

        GVector u;
        float uScale = half_width / vPerp.length();
        u.set(vPerp.x() * uScale, vPerp.y() * uScale);

        //Make line w thickness

        path->moveTo(p0 + u);
        path->lineTo(p1 + u);            
        path->lineTo(p1 - u);            
        path->lineTo(p0 - u);

        //Add caps

        GVector capVector;
        capVector.set(-v.x() * uScale, -v.y() * uScale);

        if (cap == CapType::kSquare) {
            path->moveTo(p0 + u + capVector);
            path->lineTo(p0 + u);
            path->lineTo(p0 - u);
            path->lineTo(p0 - u + capVector);

            path->moveTo(p1 + u - capVector);
            path->lineTo(p1 + u);
            path->lineTo(p1 - u);
            path->lineTo(p1 - u - capVector);
        }
        if (cap == CapType::kRound) {
            path->moveTo(p0);
            path->addCircle(p0, capVector.length(), GPath::kCCW_Direction);

            path->moveTo(p1);
            path->addCircle(p1, capVector.length(), GPath::kCCW_Direction);
        }



    }


};

std::unique_ptr<GFinal> GCreateFinal() {
    return std::unique_ptr<GFinal>(new BasicFinal());
}