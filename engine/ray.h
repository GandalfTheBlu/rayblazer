#pragma once
#include "vec3.h"

//------------------------------------------------------------------------------
/**
*/
class Ray
{
public:
    Ray(vec3 startpoint, vec3 dir) :
        origin(startpoint),
        dir(dir)
    {

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