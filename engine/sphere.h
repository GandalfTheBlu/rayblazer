#pragma once
#include "hit_result.h"
#include "mat4.h"
#include "random.h"
#include "ray.h"
#include "material.h"

// a spherical object
class Sphere
{
public:
    float radius;
    vec3 center;
    Material* material;

    Sphere() : 
        radius(0.f),
        material(nullptr)
    {}

    Sphere(float radius, vec3 center, Material* material) : 
        radius(radius),
        center(center),
        material(material)
    {

    }

    ~Sphere()
    {
    
    }

    Color GetColor()
    {
        return material->color;
    }

    bool Intersect(const Ray& ray, float maxDist, HitResult& outHitInfo)
    {
        vec3 oc = ray.origin - this->center;
        vec3 dir = ray.dir;
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
            vec3 normal = (p - this->center) * (1.0f / this->radius);

            outHitInfo.p = p;
            outHitInfo.normal = normal;
            outHitInfo.t = dist;
            outHitInfo.material = this->material;

            return true;
        }

        return false;
    }
};