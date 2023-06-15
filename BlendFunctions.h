/*
 *  @author William Convertino
 *  @copyright 2022
 */


#ifndef BlendFunctions_DEFINED
#define BlendFunctions_DEFINED

#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "MathUtil.h"
#include <iostream>

/**
 * @brief Turns a GColor into a GPixel containing the pre-multiplied ARGB values, where each non-alpha channel has been multiplied by alpha. 
 * This format makes our blend calculations faster.
 * 
 * @param color 
 * @return GPixel 
 */
static inline GPixel makePremultPixel(const GColor& color) {
        int a = GRoundToInt(color.a * 255);
        int r = GRoundToInt(color.r * color.a * 255);;
        int g = GRoundToInt(color.g * color.a * 255);;
        int b = GRoundToInt(color.b * color.a * 255);;
        return(GPixel_PackARGB(a,r,g,b));
}

static inline GPixel interpolate(GColor c1, GColor c2, float w) {
    //printf("\n{%f, %f, %f}", c1.a, c2.a, w);
    //printf("\n(%f)\n", c1.a + (w * (c1.a - c2.a)) );
    float invW = 1-w;
    float r = invW*c1.r + w*c2.r;
    float g = invW*c1.g + w*c2.g;
    float b = invW*c1.b + w*c2.b;
    float a = invW*c1.a + w*c2.a;
    //printf("<%d, %d, %d, %d, %d>", r,g,b,a);
    //std::cout << "start" << std::endl;
    return makePremultPixel(GColor::RGBA(r,g,b,a));
    //std::cout << "end" << std::endl;
    
}

/**
 * @brief Reassigns the dst pixel to blend with the src pixel using a source-over blending algorithm.
 * 
 * @param src the source pixel.
 * @param dst the destination pixel.
 */
static inline void srcOverBlend(GPixel& src, GPixel& dst) {
    int invAlpha = 255 - GPixel_GetA(src);
    int a = (GPixel_GetA(src) + div255(invAlpha * GPixel_GetA(dst)));
    int r = (GPixel_GetR(src) + div255(invAlpha * GPixel_GetR(dst)));
    int g = (GPixel_GetG(src) + div255(invAlpha * GPixel_GetG(dst)));
    int b = (GPixel_GetB(src) + div255(invAlpha * GPixel_GetB(dst)));

    GPixel newPixel = GPixel_PackARGB(a,r,g,b); 
    dst = newPixel;
        // ((GPixel_GetA(src) + div255(invAlpha * GPixel_GetA(dst))) << GPIXEL_SHIFT_A) |
        // ((GPixel_GetR(src) + div255(invAlpha * GPixel_GetR(dst))) << GPIXEL_SHIFT_R) |
        // ((GPixel_GetG(src) + div255(invAlpha * GPixel_GetG(dst))) << GPIXEL_SHIFT_G) |
        // ((GPixel_GetB(src) + div255(invAlpha * GPixel_GetB(dst))) << GPIXEL_SHIFT_B);
}

/**
 * @brief Reassigns the dst pixel to blend with the src pixel using a source-only blending algorithm.
 * This essentially replaces the dst pixel with the src pixel.
 * 
 * @param src the source pixel. 
 * @param dst the destination pixel.
 */
static inline void srcBlend(GPixel& src, GPixel& dst) {
    dst = src;
}

/**
 * @brief Reassigns the dst pixel to blend with the src pixel using a destination-only blending algorithm.
 * This essentially keeps the destination the same.
 * 
 * @param src the source pixel. 
 * @param dst the destination pixel.
 */
static inline void dstBlend(GPixel& src, GPixel& dst) {
    //Do nothing.
}

static inline void clearBlend(GPixel& src, GPixel& dst) {
    dst = GPixel_PackARGB(0,0,0,0);
}

static inline void dstOverBlend (GPixel& src, GPixel& dst) {

    int invAlpha = 255 - GPixel_GetA(dst);
    int a = (GPixel_GetA(dst) + div255(invAlpha * GPixel_GetA(src)));
    int r = (GPixel_GetR(dst) + div255(invAlpha * GPixel_GetR(src)));
    int g = (GPixel_GetG(dst) + div255(invAlpha * GPixel_GetG(src)));
    int b = (GPixel_GetB(dst) + div255(invAlpha * GPixel_GetB(src)));

    dst = GPixel_PackARGB(a,r,g,b);
}

static inline void srcInBlend (GPixel& src, GPixel& dst) {

    int a = div255(GPixel_GetA(dst) * GPixel_GetA(src));
    int r = div255(GPixel_GetA(dst) * GPixel_GetR(src));
    int g = div255(GPixel_GetA(dst) * GPixel_GetG(src));
    int b = div255(GPixel_GetA(dst) * GPixel_GetB(src));

    dst = GPixel_PackARGB(a,r,g,b);
}


static inline void dstInBlend (GPixel& src, GPixel& dst) {

    int a = div255(GPixel_GetA(src) * GPixel_GetA(dst));
    int r = div255(GPixel_GetA(src) * GPixel_GetR(dst));
    int g = div255(GPixel_GetA(src) * GPixel_GetG(dst));
    int b = div255(GPixel_GetA(src) * GPixel_GetB(dst));

    dst = GPixel_PackARGB(a,r,g,b);
}

static inline void srcOutBlend (GPixel& src, GPixel& dst) {

    int invAlpha = 255 - GPixel_GetA(dst);
    int a = div255(invAlpha * GPixel_GetA(src));
    int r = div255(invAlpha * GPixel_GetR(src));
    int g = div255(invAlpha * GPixel_GetG(src));
    int b = div255(invAlpha * GPixel_GetB(src));

    dst = GPixel_PackARGB(a,r,g,b);

}

static inline void dstOutBlend (GPixel& src, GPixel& dst) {

    int invAlpha = 255 - GPixel_GetA(src);
    int a = div255(invAlpha * GPixel_GetA(dst));
    int r = div255(invAlpha * GPixel_GetR(dst));
    int g = div255(invAlpha * GPixel_GetG(dst));
    int b = div255(invAlpha * GPixel_GetB(dst));

    dst = GPixel_PackARGB(a,r,g,b);

}

static inline void srcATopBlend (GPixel& src, GPixel& dst) {
    
    int invAlpha = 255 - GPixel_GetA(src);
    int a = div255((GPixel_GetA(dst)*GPixel_GetA(src)) + (invAlpha * GPixel_GetA(dst)));
    int r = div255((GPixel_GetA(dst)*GPixel_GetR(src)) + (invAlpha * GPixel_GetR(dst)));
    int g = div255((GPixel_GetA(dst)*GPixel_GetG(src)) + (invAlpha * GPixel_GetG(dst)));
    int b = div255((GPixel_GetA(dst)*GPixel_GetB(src)) + (invAlpha * GPixel_GetB(dst)));

    dst = GPixel_PackARGB(a,r,g,b);
}

static inline void dstATopBlend (GPixel& src, GPixel& dst) {

    int invAlpha = 255 - GPixel_GetA(dst);
    int a = div255((GPixel_GetA(src)*GPixel_GetA(dst)) + (invAlpha * GPixel_GetA(src)));
    int r = div255((GPixel_GetA(src)*GPixel_GetR(dst)) + (invAlpha * GPixel_GetR(src)));
    int g = div255((GPixel_GetA(src)*GPixel_GetG(dst)) + (invAlpha * GPixel_GetG(src)));
    int b = div255((GPixel_GetA(src)*GPixel_GetB(dst)) + (invAlpha * GPixel_GetB(src)));

    dst = GPixel_PackARGB(a,r,g,b);
}

static inline void xorBlend (GPixel& src, GPixel& dst) {

    int invAlpha = 255 - GPixel_GetA(dst);
    int invSrcAlpha = 255 - GPixel_GetA(src);
    int a = div255((invSrcAlpha*GPixel_GetA(dst)) + (invAlpha * GPixel_GetA(src)));
    int r = div255((invSrcAlpha*GPixel_GetR(dst)) + (invAlpha * GPixel_GetR(src)));
    int g = div255((invSrcAlpha*GPixel_GetG(dst)) + (invAlpha * GPixel_GetG(src)));
    int b = div255((invSrcAlpha*GPixel_GetB(dst)) + (invAlpha * GPixel_GetB(src)));

    dst = GPixel_PackARGB(a,r,g,b);
}

#endif