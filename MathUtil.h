/*
 *  @author William Convertino
 *  @copyright 2022
 */


#ifndef CanvasUtil_DEFINED
#define CanvasUtil_DEFINED

/**
 * @brief Makes a fast approximation of dividing an int by 255.
 * Note: This can only be used for int values do not exceed 16 bits.
 * 
 * @param numerator the number to divide.
 * @return unsigned the quotient of the fast division.
 */
static inline unsigned div255(unsigned numerator) {
    numerator += 0x80;
    numerator += numerator >> 8;
    return(numerator >> 8);
    //return( ( (numerator * 65793) + (1 << 23) ) >> 24);
}

static inline float hornerEvalQuad(float a, float b, float c, float t) {
        float A = a - (2 * b) + c;
        float B = (2*b) - (2*a);
        float C = a;
        return (((A * t) + B) * t) + C;
    }

static inline float hornerEvalCubic(float a, float b, float c, float d, float t) {
        float A = (3*b) + d - (3 * c) - a;
        float B = (3 * c) - (6*b) + (3 * a);
        float C = (3 * b) - (3 * a);
        float D = a;
        return (((((A * t) + B) * t) + C) * t) + D;
    }
static inline float interpolate(float a, float b, float t) {
        return ((1.0-t)*a) + (t * b);
    }

#endif