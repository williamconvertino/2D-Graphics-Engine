/*
 *  @author William Convertino
 *  @copyright 2022
 */

#ifndef BlendUtil_DEFINED
#define BlendUtil_DEFINED

#include "GBlendMode.h"
#include "BlendFunctions.h"

#include <map>

typedef void (*BlendFunction) (GPixel&, GPixel&);

static std::map<GBlendMode,BlendFunction> blendFunctionMap = {
    
    
    {GBlendMode::kClear,clearBlend},
    {GBlendMode::kSrc,srcBlend},     
    {GBlendMode::kDst,dstBlend},      
    {GBlendMode::kSrcOver,srcOverBlend},  
    {GBlendMode::kDstOver,dstOverBlend}, 
    {GBlendMode::kSrcIn,srcInBlend},
    {GBlendMode::kDstIn,dstInBlend},
    {GBlendMode::kSrcOut,srcOutBlend},
    {GBlendMode::kDstOut,dstOutBlend},
    {GBlendMode::kSrcATop,srcATopBlend},
    {GBlendMode::kDstATop,dstATopBlend},
    {GBlendMode::kXor,xorBlend}
};

/**
 * @brief Returns the blending function associated with the given blend mode.
 * 
 * @param blendMode 
 * @return BlendFunction 
 */
static inline BlendFunction getBlendFunction(GBlendMode blendMode) {
    auto pair = blendFunctionMap.find(blendMode);
    if (pair == blendFunctionMap.end()) {
        printf("This blend mode hasn't been implemented yet...");
        return(srcBlend);
    }
    return(pair->second);
}

#endif