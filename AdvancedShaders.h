#include "GColor.h"
#include "GPoint.h"

std::unique_ptr<GShader> GCreateTriBitmapShader(GPoint triPoints[], GPoint texPoints[], GShader* shader);

std::unique_ptr<GShader> GCreateTriColorShader(GPoint points[], GColor color[]);

std::unique_ptr<GShader> GCreateComposedShader(GShader* sh1, GShader* sh2);

std::unique_ptr<GShader> GCreateRadialGradient(GPoint p0, GPoint p1, const GColor color[] , int count, GShader::TileMode tileMode);

std::unique_ptr<GShader> GCreateRayGradient(GPoint p0, GPoint p1, const GColor color[] , int count, GShader::TileMode tileMode);

std::unique_ptr<GShader> GCreateFinalRadialGradient(GPoint center, float radius, const GColor color[] , int count, GShader::TileMode tileMode);