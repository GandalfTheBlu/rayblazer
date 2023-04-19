#pragma once
#include "vec3.h"
#include "material.h"
#include <float.h>

//------------------------------------------------------------------------------
/**
*/
struct HitResult
{
    // hit point
    vec3 p;
    // normal
    vec3 normal;
    // hit material, or nullptr
    Material* material = nullptr;
    // intersection distance
    float t = FLT_MAX;
};