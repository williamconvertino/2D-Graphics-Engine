/*
 *  @author William Convertino
 *  @copyright 2022
 */

#include "GMatrix.h"
#include "GColor.h"
#include "GMath.h"
#include "GPoint.h"
#include "GRect.h"


GMatrix::GMatrix() {
    fMat[0] = 1;    fMat[1] = 0;    fMat[2] = 0;
    fMat[3] = 0;    fMat[4] = 1;    fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1,0,tx,0,1,ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx,0,0,0,sy,0);
}

GMatrix GMatrix::Rotate(float radians) {
    return GMatrix(cosf(radians),-sinf(radians),0,sinf(radians),cosf(radians),0);
}

GMatrix GMatrix::Concat(const GMatrix& m1, const GMatrix& m2) {

    float a = m1[0]*m2[0] + m1[1]*m2[3];
    float b = m1[0]*m2[1] + m1[1]*m2[4];
    float c = m1[0]*m2[2] + m1[1]*m2[5] + m1[2];
    float d = m1[3]*m2[0] + m1[4]*m2[3];
    float e = m1[3]*m2[1] + m1[4]*m2[4];
    float f = m1[3]*m2[2] + m1[4]*m2[5] + m1[5];

    return GMatrix(a,b,c,d,e,f);
}


bool GMatrix::invert(GMatrix* inverse) const {
    float det = ((*this)[0] * (*this)[4]) - ((*this)[1] * (*this)[3]);
    if (det == 0) {
        return false;
    }
    float a = (*this)[0];
    float b = (*this)[1];
    float c = (*this)[2];
    float d = (*this)[3];
    float e = (*this)[4];
    float f = (*this)[5];

    (*inverse)[0] = e / det;
    (*inverse)[1] = -b / det;
    (*inverse)[2] = (b*f - c*e) /det;
    (*inverse)[3] = -d / det;
    (*inverse)[4] = a / det;
    (*inverse)[5] = (c*d - a*f) / det;
    return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {

    for (int i = 0; i < count; i++) {
        GPoint srcPoint = src[i];
        dst[i] = GPoint::Make(
            ((*this)[0] * srcPoint.fX) + ((*this)[1] * srcPoint.fY) + (*this)[2],
            ((*this)[3] * srcPoint.fX) + ((*this)[4] * srcPoint.fY) + (*this)[5]
        );
    }
}