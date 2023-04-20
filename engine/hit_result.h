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
    // intersection distance
    float t = FLT_MAX;
};