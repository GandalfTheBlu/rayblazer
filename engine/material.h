#pragma once
#include "color.h"
#include "ray.h"
#include "vec3.h"
#include <string>

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
    std::string type = "Lambertian";
    Color color = {0.5f,0.5f,0.5f};
    float roughness = 0.75;

    // this is only needed for dielectric materials.
    float refractionIndex = 1.44;
};

//------------------------------------------------------------------------------
/**
    Scatter ray against material
*/
Ray BSDF(Material const* const material, Ray ray, vec3 point, vec3 normal);