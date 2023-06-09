#pragma once
#include "color.h"
#include "ray.h"
#include "vec3.h"
#include <stdint.h>

enum class MaterialType
{
    Lambertian,
    Dielectric,
    Conductor
};

//------------------------------------------------------------------------------
/**
*/
struct Material
{
    
    /*
        type can be "Lambertian", "Dielectric" or "Conductor".
        Obviously, "lambertian" materials are dielectric, but we separate them here
        just because figuring out a good IOR for ex. plastics is too much work
    */
    MaterialType type = MaterialType::Lambertian;
    Color color = {0.5f,0.5f,0.5f};
    float roughness = 0.75;

    // this is only needed for dielectric materials.
    float refractionIndex = 1.44f;

    /**
        Scatter ray against material
    */
    void BSDF(Ray& inOutRay, const vec3& point, const vec3& normal, uint32_t seed) const;

private:
    void BSDF_Lambertian(Ray& inOutRay, const vec3& point, const vec3& normal, uint32_t seed) const;
    void BSDF_Dielectric(Ray& inOutRay, const vec3& point, const vec3& normal, uint32_t seed) const;
    void BSDF_Conductor(Ray& inOutRay, const vec3& point, const vec3& normal, uint32_t seed) const;
};