#include <cmath>
#include <math.h>

typedef float (*GradientFunction) (float, float);

static float mapToLinear(float x, float y) {
    return x;
}

static float mapToRadial(float x, float y) {
    return std::sqrt(x*x + y*y);
}

static float mapToRadialRays(float x, float y) {
    return (std::atan2(x,y) + M_PI) / (2.0 * M_PI);
}