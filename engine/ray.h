#pragma once
#include "vec3.h"

//------------------------------------------------------------------------------
/**
*/
class Ray
{
public:

    // --debug purposes--
    static size_t spawnedCount;
    // --

    Ray(vec3 startpoint, vec3 dir) :
        b(startpoint),
        m(dir)
    {
        spawnedCount++;
    }

    ~Ray()
    {

    }

    vec3 PointAt(float t) const
    {
        return {b + m * t};
    }

    // beginning of ray
    vec3 b;
    // magnitude and direction of ray
    vec3 m;
};

inline size_t Ray::spawnedCount = 0;