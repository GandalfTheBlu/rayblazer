#pragma once
#include "ray.h"
#include "color.h"
#include <float.h>

class Object;

//------------------------------------------------------------------------------
/**
*/
struct HitResult
{
    // hit point
    vec3 p;
    // normal
    vec3 normal;
    // hit object, or nullptr
    Object* object = nullptr;
    // intersection distance
    float t = FLT_MAX;
};

//------------------------------------------------------------------------------
/**
*/
class Object
{
public:
    Object() 
    {}

    virtual ~Object()
    {}

    virtual bool Intersect(const Ray& ray, float maxDist, HitResult& outHitInfo) = 0;
    virtual Color GetColor() = 0;
    virtual void ScatterRay(Ray& inOutRay, const vec3& point, const vec3& normal) = 0;
};