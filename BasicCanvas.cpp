/*
 *  @author William Convertino
 *  @copyright 2022
 */

#include <iostream>
#include "GCanvas.h"
#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "BlendUtil.h"
#include "EdgeBuilder.h"
#include "Edge.h"
#include "GMatrix.h"
#include <stack>
#include "GShader.h"
#include "GPath.h"
#include <functional>
#include "AdvancedShaders.h"
#include "Debug.h"

class BasicCanvas : public GCanvas {
public:
    BasicCanvas(const GBitmap& device)
        : fDevice(device), eBuilder(0,0,fDevice.width()-1, fDevice.height()-1) {
            transformationStack.push(*new GMatrix());
    }
    

    void blit(const int left, const int right, int y, BlendFunction blendFunction, GShader* shader) {
        
        //debug("start");
        int count = right - left;

        if (count < 1) {
            return;
        }
        
        

        GPixel newPixels[count];
        // debug( left, y,"{","}");
        // debug( count, 0,"{","}");
        // debug(fDevice.width(), fDevice.height(), "[", "]");
        
         shader->shadeRow(left, y, count, newPixels);
        
        for (int i = 0; i < count; i++) {
            
            blendFunction(newPixels[i], *fDevice.getAddr(left + i, y));
        }
    
    }

    void blit(const int left, const int right, int y, BlendFunction blendFunction, GPixel pixel) {
        int count = right - left;
        
        for (int i = 0; i < count; i++) {
            blendFunction(pixel, *fDevice.getAddr(left + i, y));
        }
        
    }


    /**
     * @brief Clears the canvas by replacing every pixel with the given color.
     * 
     * @param color the color to fill the canvas.
     */
    void drawPaint(const GPaint& paint) override {
        GRect screenRect = GRect::MakeLTRB(0,0,fDevice.width(), fDevice.height());
        if (paint.getShader() == nullptr) {
            GPixel newPixel = makePremultPixel(paint.getColor());
            for (int y = 0; y < fDevice.height(); y++) {
                blit(0, fDevice.width(), y,getBlendFunction(paint.getBlendMode()), newPixel);
            }
        } else {
            paint.getShader()->setContext(transformationStack.top());
            for (int y = 0; y < fDevice.height(); y++) {
                blit(0, fDevice.width(), y,getBlendFunction(paint.getBlendMode()), paint.getShader());
            }
        }
    }

    /**
     * @brief Draws a rectangle of the given color to the canvas.
     * 
     * @param rect the rectangle to draw.
     * @param paint the color to draw the rectangle.
     */
    void drawRect(const GRect& rect, const GPaint& paint) override {
        
        GPoint points[4];

        points[0] = GPoint::Make(rect.left(),rect.top());
        points[1] = GPoint::Make(rect.left(),rect.bottom());
        points[2] = GPoint::Make(rect.right(),rect.bottom());
        points[3] = GPoint::Make(rect.right(),rect.top());
        
        drawConvexPolygon(points, 4,paint);

        // GIRect transformedRect;

        // GIRect newRect = clipGIRectToCanvas(rect.round());
        // if (newRect.top() == newRect.bottom()) {
        //     return;
        // }
        // BlendFunction bf = getBlendFunction(paint.getBlendMode());
        // if (paint.getShader() == nullptr) {
        //     GPixel newPixel = makePremultPixel(paint.getColor());
        //     for (int y = newRect.top(); y < newRect.bottom(); y++ ) {
        //       blit(newRect.left(), newRect.right(), y, bf, newPixel);
        //     }
        // } else {
        //     for (int y = newRect.top(); y < newRect.bottom(); y++ ) {
        //         paint.getShader()->setContext(ctm);
        //       blit(newRect.left(), newRect.right(), y, bf, paint.getShader());
        //     }
        // }
    }

    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override {
        
        if (count < 3) {
            return;
        }

        std::vector<Edge> edgeList;
        
        GPoint transformedPoints[count];

        transformationStack.top().mapPoints(transformedPoints, points, count);

        for (int i = 1; i < count; i++) {
            eBuilder.generateAndAddEdge(transformedPoints[i],transformedPoints[i-1], edgeList);
        }
        
        eBuilder.generateAndAddEdge(transformedPoints[0],transformedPoints[count-1], edgeList);
    
        std::sort(edgeList.begin(), edgeList.end(), compareEdgeLessThan);
        
        if (edgeList.size() < 2) {
            return;
        }

        int indexL = 0;
        int indexR = 1;
        int totalIndex = 1;
        int currentY = edgeList[0].yTop;

        BlendFunction bf = getBlendFunction(paint.getBlendMode());

        if (paint.getShader() == nullptr) {
            GPixel newPixel = makePremultPixel(paint.getColor());
         
            while (totalIndex < edgeList.size()) {
                int xL = getX(edgeList[indexL],currentY); 
                int xR = getX(edgeList[indexR],currentY);
                
                blit(xL, xR, currentY, bf, newPixel);
                currentY++;
                if (currentY >= edgeList[indexL].yBottom) {
                    totalIndex++; 
                    indexL = totalIndex;
                }
                if (currentY >= edgeList[indexR].yBottom) {
                    totalIndex++; 
                    indexR = totalIndex;
                }
            }
        } else {
            
            GShader* gs = paint.getShader();
            gs->setContext(transformationStack.top());
            //debug(fDevice.width(), fDevice.height(), "[", "]");
            while (totalIndex < edgeList.size()) {
            int xL = getX(edgeList[indexL],currentY); 
            int xR = getX(edgeList[indexR],currentY);

            // debug(xL, xR, "<", ">");
            //debug("s1");
            blit(xL, xR, currentY, bf, gs);
            //debug("s2");
            //debug(0, 0, "", "");
            currentY++;
            if (currentY >= edgeList[indexL].yBottom) {
                totalIndex++; 
                indexL = totalIndex;
            }
            if (currentY >= edgeList[indexR].yBottom) {
                totalIndex++; 
                indexR = totalIndex;
            }
        } 
        
    }
    }

   

    /**
     * @brief returns a rectangle representing the intersection of the given rectangle and the canvas size. 
     * 
     * @param rect the rectangle to clip.
     * @return GIRect a rectangle representing the intersection of the given rectangle and the canvas size. 
     */
    GIRect clipGIRectToCanvas(const GIRect& rect) const {
        
        int l = std::max(rect.left(), 0);
        int t = std::max(rect.top(),0);
        int r = std::min(rect.right(), fDevice.width()-1);
        int b = std::min(rect.bottom(),  fDevice.height()-1);
        return(GIRect::MakeLTRB(l,t,r,b));
    }

    void concat(const GMatrix& matrix) {
        transformationStack.top() = GMatrix::Concat(transformationStack.top(),matrix);
    }

    void restore() {
        transformationStack.pop();
    }

    void save() {
        GMatrix ctm = transformationStack.top();
        transformationStack.push(*new GMatrix(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]));
    }

    void drawPath(const GPath& path, const GPaint& paint) {
        
        std::vector<Edge> edgeList;
        auto tPath = path;
        tPath.transform(transformationStack.top());
        
        GPoint pts[GPath::kMaxNextPoints];
        GPath::Edger myEdger(tPath);
        GPath::Verb myVerb;
        while ( (myVerb = myEdger.next(pts)) != GPath::kDone) {
            
            switch (myVerb) {
                case GPath::kLine:
                    eBuilder.generateAndAddEdge(pts[0],pts[1],edgeList);
                    break;
                case GPath::kQuad:
                    eBuilder.generateQuadraticEdgesAndAdd(pts[0],pts[1],pts[2], 0.25, edgeList);
                    break;
                case GPath::kCubic:
                    eBuilder.generateCubicEdgesAndAdd(pts[0],pts[1],pts[2],pts[3], 0.25,edgeList);
                    break;
                default: break;
            }

        }
    

        if (edgeList.size() < 2) {
            return;
        }
        std::sort(edgeList.begin(), edgeList.end(), compareEdgeLessThan);
    
        int yTop = edgeList[0].yTop;
        int yBottom = 0;
        for (Edge e: edgeList) {
            if (e.yBottom > yBottom) {
                yBottom = e.yBottom;
            }
        }
     
        for (int y = yTop; y < yBottom; y++) {
          
            
            std::vector<Edge> activeEdges;
            int wind = 0;
            for (Edge e: edgeList) {
                if (e.yTop <= y + 0.5 && e.yBottom >= y + 0.5) {
                    activeEdges.push_back(e);
                }
            }
            
            //std::sort(activeEdges.begin(),activeEdges.end(),std::bind(compareEdgesByX, std::placeholders::_1, std::placeholders::_2, y));
            std::sort(activeEdges.begin(),activeEdges.end(),[y]( Edge& a,  Edge& b) { 
                return compareEdgesByX(a,b,y); 
            });

            int left;
            int right;
            
            if (paint.getShader() == nullptr) {
                
                for (Edge e: activeEdges) {
                    int x = getX(e, y);
                    //debug(x,y,"(",")"); 
                    // if (x < 0 || x > fDevice.width() - 1) {
                    //     debug(x,y,"(",")");    
                    //     debug(fDevice.width(),fDevice.height(),"<",">");
                    // }
                    
                     if (wind == 0) {
                        left = x;
                    }
                    wind = wind + e.orientation;
                    if (wind == 0) {
                        right = x;

                        if (right > fDevice.width()) {
                            right = fDevice.width()-1;
                        }
                        // debug(left, right, "(", ")");
                        // debug();
                        // debug();
                        blit(left, right, y, getBlendFunction(paint.getBlendMode()), makePremultPixel(paint.getColor()));
                        // debug("end");
                    }
                    
                }
                
            } else {
                //debug("oi");
                GShader* gs = paint.getShader();
                gs->setContext(transformationStack.top());
                for (Edge e: activeEdges) {
                    
                    int x = getX(e, y);
                    if (wind == 0) {
                        left = x;
                    }
                    wind = wind + e.orientation;
                    if (wind == 0) {
                        right = x;
                        blit(left, right, y, getBlendFunction(paint.getBlendMode()), gs);
                    }
                }
            }

        }
        
    }

    void drawTri(GPoint points[], GPaint paint) {
        
        GPath myPath;
        myPath.moveTo(points[0]);
        myPath.lineTo(points[1]);
        myPath.lineTo(points[2]);

        drawPath(myPath, paint);
    }


    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
        
        GPoint p[3];
        GColor c[3];
        GPoint t[3];

        GShader* triangleShader;
        std::unique_ptr<GShader> colorShader;
        std::unique_ptr<GShader> textureShader;
        std::unique_ptr<GShader> combinedShader;
        
        for (int i = 0; i < count; i++) {
            //printf("<%d,%d,%d>\n", indices[3*i   ], indices[(3*i) +1], indices[(3*i) +2]);
            p[0] = verts[indices[3*i]];
            p[1] = verts[indices[(3*i) +1]];
            p[2] = verts[indices[(3*i) +2]];
            
            if (texs != nullptr) {
                t[0] = texs[indices[3*i]];
                t[1] = texs[indices[(3*i)+1]];
                t[2] = texs[indices[(3*i)+2]];
                textureShader = GCreateTriBitmapShader(p,t,paint.getShader());
            }

            if (colors != nullptr) {
                c[0] = colors[indices[3*i]];
                c[1] = colors[indices[(3*i) + 1]];
                c[2] = colors[indices[(3*i) + 2]];
                colorShader = GCreateTriColorShader(p, c);
            }

            if (texs == nullptr) {
                triangleShader = colorShader.get();
            } else if (colors == nullptr) {
                triangleShader = textureShader.get();
            } else {
                combinedShader = GCreateComposedShader(
                        textureShader.get(),
                        colorShader.get()
                    );
                triangleShader = combinedShader.get();
            }
            assert(triangleShader != nullptr);
            // triangleShader->setContext(GMatrix());
            GPaint myPaint = GPaint(triangleShader);
            // myPaint.setColor(myPaint.getColor());
            // myPaint.setBlendMode(myPaint.getBlendMode());
            drawTri(p,myPaint);
        }

    }

    GPoint interpolatePoint(GPoint p1, GPoint p2, float a) {
        return GPoint::Make(
            (p1.x() * a) + (p2.x() * (1-a)),
            (p1.y() * a) + (p2.y() * (1-a))
        );
    }

    GColor interpolateColor(GColor c1, GColor c2, float a) {
        return GColor::RGBA(
            (c1.r * a) + (c2.r * (1-a)),
            (c1.g * a) + (c2.g * (1-a)),
            (c1.b * a) + (c2.b * (1-a)),
            (c1.a * a) + (c2.a * (1-a))
        );
    }

    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
        
        
        int numTriangles = (level+1)*(level+1) * 2;
        int indices[numTriangles * 3];

        int numVerts = (level+2)*(level+2);
        int vertsIndexGrid[level+2][level+2];
        GPoint newVerts[numVerts];
        GColor newColors[numVerts];
        GPoint newTexs[numVerts];
 

        int index = 0;
        
        float a = 0;
        float da = 1.0/(level+1);
        float db = 1.0/(level+1);

        for (int i = 0; i <= level+1; i++) {
            if (i == level+1) {
                a = 1;
            }

            GPoint P1 = interpolatePoint(verts[1], verts[0], a);

            GPoint P2 = interpolatePoint(verts[2], verts[3], a);

            GColor C1;
            GColor C2;

            if (colors != nullptr) {
                C1 = interpolateColor(colors[1], colors[0], a);
                C2 = interpolateColor(colors[2], colors[3], a);
            }

            GPoint TP1;
            GPoint TP2;

            if (texs != nullptr) {
                TP1 = interpolatePoint(texs[1], texs[0], a);
                TP2 = interpolatePoint(texs[2], texs[3], a);
            }

            float b = 0;

            for (int j = 0; j <= level+1; j++) {
                if (j == level+1) {
                    b = 1;
                }

                
                newVerts[index] = interpolatePoint(P2, P1, b);
                vertsIndexGrid[i][j] = index;

                if (colors != nullptr) {
                    newColors[index] = interpolateColor(C2, C1, b);
                }

                if (texs != nullptr) {
                    newTexs[index] =  interpolatePoint(TP2, TP1, b);
                }

                b += db;
                index++;
            }
            a += da;
        }

        int count = 0;
        for (int i = 0; i < level+1; i++) {
            for (int j = 0; j < level+1; j++) {
                indices[count] = vertsIndexGrid[i][j];
                indices[count+1] = vertsIndexGrid[i+1][j];
                indices[count+2] = vertsIndexGrid[i][j+1];
                indices[count+3] = vertsIndexGrid[i+1][j+1];
                indices[count+4] = vertsIndexGrid[i+1][j];
                indices[count+5] = vertsIndexGrid[i][j+1];

                count+=6;
            }
        }
        
        if (colors == nullptr) {
            drawMesh(newVerts, nullptr, newTexs, numTriangles, indices, paint);
        } else if (texs == nullptr) {
             drawMesh(newVerts, newColors, nullptr, numTriangles, indices, paint);
        } else {
            drawMesh(newVerts, newColors, newTexs, numTriangles, indices, paint);
        }
    }

    

    // void drawStroke(GPoint pts[], int count, float thickness, GPaint paint, CapType cap, BendType bend) {
        
//         GPath strokePath;
        
//         GVector prev_v;
//         GVector prev_u;
        
//         float half_t = thickness/2.0;

//         for (int i = 0; i < count-1; i++) {
//             GPoint p0 = pts[i];
//             GPoint p1 = pts[i+1];
//             GVector v;
//             v.set(p1.x() - p0.x(), p1.y() - p0.y());
//             GVector vPerp;
//             vPerp.set(-v.y(), v.x());

//             GVector u;
//             float lenScale = half_t / vPerp.length();
//             u.set(vPerp.x() * lenScale, vPerp.y() * lenScale);

//             if (i == 0) {
//                 GVector capVector;
//                 capVector.set(-v.x() * lenScale, -v.y() * lenScale);

//                 if (cap == square) {
//                     strokePath.moveTo(p0 + u + capVector);
//                     strokePath.lineTo(p0 + u);
//                     strokePath.lineTo(p0 - u);
//                     strokePath.lineTo(p0 - u + capVector);
//                 }
//                 if (cap == round) {
//                     strokePath.moveTo(p0);
//                     strokePath.addCircle(p0, capVector.length(), GPath::kCCW_Direction);
//                 }
                
//             }

//             strokePath.moveTo(p0 + u);
//             strokePath.lineTo(p1 + u);            
//             strokePath.lineTo(p1 - u);            
//             strokePath.lineTo(p0 - u);

// /*
// Begin joints
// */


//             if (i != 0 && bend == flat) {
//                 strokePath.moveTo(p0 + prev_u);
//                 strokePath.lineTo(p0 + u);
//                 strokePath.lineTo(p0);
//                 strokePath.moveTo(p0 - prev_u);
//                 strokePath.lineTo(p0 - u);
//                 strokePath.lineTo(p0);
                
//             }

//             if (i != 0 && bend == rounded) {
//                 strokePath.moveTo(p0);
//                 strokePath.addCircle(p0, u.length(), GPath::kCCW_Direction);
//             }

//             if (i != 0 && bend == miter) {

//                 GVector Qn;
//                 Qn.set(u.x() + prev_u.x(), u.y() + prev_u.y());
//                 Qn = normalize(Qn);
                
//                 // printf("%f, %f\n", Q.x(), Q.y());
//                 float denom = sqrt( (1.0 - (dotProduct(u, prev_u) / (u.length() * prev_u.length())) ) / 2.0 );

//                 if (thickness/denom > 1) {
                
//                     float q = half_t / denom;

//                     GVector Q;

//                     Q.set(Qn.x() * q, Qn.y() * q);

//                     if (crossProduct(u, prev_u)>0) {

//                         strokePath.moveTo(p0 + u);
//                         strokePath.lineTo(p0 + Q);
//                         strokePath.lineTo(p0 + prev_u);
//                         strokePath.lineTo(p0);

//                     } else {
//                         strokePath.moveTo(p0 - u);
//                         strokePath.lineTo(p0 - Q);
//                         strokePath.lineTo(p0 - prev_u);
//                         strokePath.lineTo(p0);
//                     }
//                 }
                


//                 // GVector Q;
//                 // Q.set(u.x() + prev_u.x(), u.y() + prev_u.y());

//                 // float qDenom = sqrt((1.0 - dotProduct(u,prev_u)) / 2.0);
//                 // printf("%f\n", dotProduct(u,prev_u));
//                 // float q = half_t / qDenom;

//                 // Q.set(Q.x() * q, Q.y() * q);

//                 // //printf("%f, %f\n", Q.x(), Q.y());

//                 // strokePath.moveTo(p0 + prev_u);
//                 // strokePath.lineTo(p0 + Q);
//                 // strokePath.lineTo(p0 + u);
//                 // strokePath.lineTo(p0);
//             }

// /*
// End joints
// */

//             if (i == count-2) {
//                 GVector capVector;
//                 capVector.set(-v.x() * lenScale, -v.y() * lenScale);

//                 if (cap == square) {
//                     strokePath.moveTo(p1 + u - capVector);
//                     strokePath.lineTo(p1 + u);
//                     strokePath.lineTo(p1 - u);
//                     strokePath.lineTo(p1 - u - capVector);
//                 }
//                 if (cap == round) {
//                     strokePath.moveTo(p1);
//                     strokePath.addCircle(p1, capVector.length(), GPath::kCCW_Direction);
//                 }

//             }
//             prev_v = v;
//             prev_u = u;
//         }

//         drawPath(strokePath, paint);

//    }


private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;

    EdgeBuilder eBuilder;

    std::stack<GMatrix> transformationStack;

    float crossProduct(GVector a, GVector b) {
        return a.x()*b.y() - a.y()*b.x();
    }

    float dotProduct(GVector a, GVector b) {
        return (a.x()*b.x() + a.y()*b.y() ) ;
    }

    GVector normalize(GVector a) {
        GVector ret;
        ret.set(a.x()/a.length(), a.y()/a.length());
        return(ret);
    }
    
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new BasicCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    
    // GColor c = GColor::RGBA(1.0,1.0,1.0,1.0);
    // GPaint back = GPaint(c);
    // canvas->drawPaint(back);

    // // GPoint points[] = {GPoint::Make(0,0), GPoint::Make(0,100),GPoint::Make(100,100), GPoint::Make(100,0) };
    // // GColor colors[] = {GColor::RGBA(1,1,1,1), GColor::RGBA(0.4,0,0.5,0), GColor::RGBA(0.1,0.8,0.3,0), GColor::RGBA(0.2,0.1,1,1)};
    // GColor mycolors[4] = {

    //     GColor::RGB(0.9,0.9,0.2),
    //     GColor::RGB(0.7,0.4,0.6),
    //     GColor::RGB(0.2,0.9,0.1),
    //     GColor::RGB(0.5,0.5,0.9)

    // };

    // std::unique_ptr<GShader> grad = GCreateRayGradient(
    //      GPoint::Make(130,127),
    //      GPoint::Make(255,255),
    //      mycolors,
    //      4,
    //      GShader::kClamp
    //  );
    //  GPaint paint = GPaint(grad.get());

    //  GPoint myPoints[] = {
    //      GPoint::Make(27,80),
    //      GPoint::Make(114,58),
    //      GPoint::Make(224,140)
    //  };
    // //canvas->drawStroke(myPoints, 3, 10.0, paint, GCanvas::CapType::square, GCanvas::BendType::miter);

    // GPoint myPoints2[] = {
    //     GPoint::Make(23,175),
    //      GPoint::Make(85,225),
    //      GPoint::Make(181,162),
    //      GPoint::Make(234,232)
    // };
    
    // //canvas->drawStroke(myPoints2, 4, 20.0, GPaint(), GCanvas::CapType::round, GCanvas::BendType::miter);

    // GPoint myPoints3[] = {
    //     GPoint::Make(10,200),
    //     GPoint::Make(20,20),
    //      GPoint::Make(40,190),
    //      GPoint::Make(120,40)
    // };

    // canvas->drawStroke(myPoints3, 4, 20.0, GPaint(), GCanvas::CapType::round, GCanvas::BendType::miter);


    // //canvas->drawPaint(paint);

    // // canvas->drawQuad(points, colors, nullptr, 2, paint);
    // //canvas->drawConvexPolygon(points, 4, paint);


    return "tears in rain";
}