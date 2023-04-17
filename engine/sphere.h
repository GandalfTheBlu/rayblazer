#pragma once
#include "object.h"
#include "mat4.h"
#include "pbr.h"
#include "random.h"
#include "ray.h"
#include "material.h"

// returns a random point on the surface of a unit sphere
inline vec3 RandomPointOnUnitSphere()
{
    float x = RandomFloatNTP();
    float y = RandomFloatNTP();
    float z = RandomFloatNTP();
    return normalize({x, y, z});
}

// a spherical object
class Sphere : public Object
{
public:
    float radius;
    vec3 center;
    Material const* const material;

    Sphere(float radius, vec3 center, Material const* const material) : 
        radius(radius),
        center(center),
        material(material)
    {

    }

    ~Sphere() override
    {
    
    }

    Color GetColor()
    {
        return material->color;
    }

    bool Intersect(const Ray& ray, float maxDist, HitResult& outHitInfo) override
    {
        vec3 oc = ray.b - this->center;
        vec3 dir = ray.m;
        float b = dot(oc, dir);
    
        // early out if sphere is "behind" ray
        if (b > 0)
            return false;

        float a = dot(dir, dir);
        float c = dot(oc, oc) - this->radius * this->radius;
        float discriminant = b * b - a * c;

        if (discriminant > 0)
        {
            constexpr float minDist = 0.001f;
            float div = 1.0f / a;
            float sqrtDisc = std::sqrtf(discriminant);
            float dist1 = (-b - sqrtDisc) * div;
            float dist2 = (-b + sqrtDisc) * div;
            float dist = dist1 < dist2 ? dist1 : dist2;
            
            if (dist < minDist)
                dist = dist2;

            if (dist > maxDist)
                return false;

            vec3 p = ray.PointAt(dist);
            outHitInfo.p = p;
            outHitInfo.normal = (p - this->center) * (1.0f / this->radius);
            outHitInfo.t = dist;
            outHitInfo.object = this;

            return true;
        }

        return false;
    }

    void ScatterRay(Ray& inOutRay, const vec3& point, const vec3& normal) override
    {
        this->material->BSDF(inOutRay, point, normal);
    }

};