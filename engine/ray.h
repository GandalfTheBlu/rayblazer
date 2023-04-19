#pragma once
#include "vec3.h"
#include <atomic>

//------------------------------------------------------------------------------
/**
*/
class Ray
{
public:

    // --debug purposes--
    static std::atomic<int> spawnedCount;
    // --

    Ray(vec3 startpoint, vec3 dir) :
        origin(startpoint),
        dir(dir)
    {
        spawnedCount++;
    }

    ~Ray()
    {

    }

    vec3 PointAt(float t) const
    {
        return {origin + dir * t};
    }

    // beginning of ray
    vec3 origin;
    // magnitude and direction of ray
    vec3 dir;
};

inline std::atomic<int> Ray::spawnedCount = 0;