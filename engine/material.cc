#include "material.h"
#include "pbr.h"
#include "mat4.h"
#include "sphere.h"
#include "random.h"

//------------------------------------------------------------------------------
/**
*/
void
Material::BSDF(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    switch (type)
    {
    case MaterialType::Lambertian:
        BSDF_Lambertial(inOutRay, point, normal);
        break;
    case MaterialType::Dielectric:
        BSDF_Dielectric(inOutRay, point, normal);
        break;
    case MaterialType::Conductor:
        BSDF_Conductor(inOutRay, point, normal);
        break;
    }
}

void Material::BSDF_Lambertial(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    float cosTheta = -dot(normalize(inOutRay.m), normalize(normal));

    // probability that a ray will reflect on a microfacet
    float F = FresnelSchlick(cosTheta, 0.04f, this->roughness);
    float r = RandomFloat();

    if (r < F)
    {
        // importance sample with brdf specular lobe
        vec3 H = ImportanceSampleGGX_VNDF(RandomFloat(), RandomFloat(), this->roughness, inOutRay.m, TBN(normal));
        vec3 reflected = reflect(inOutRay.m, H);
        inOutRay = { point, normalize(reflected) };
    }
    else
    {
        inOutRay = { point, normalize(normalize(normal) + RandomPointOnUnitSphere()) };
    }
}
void Material::BSDF_Dielectric(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    float cosTheta = -dot(normalize(inOutRay.m), normalize(normal));

    // probability that a ray will reflect on a microfacet
    float F = FresnelSchlick(cosTheta, 0.95f, this->roughness);
    float r = RandomFloat();

    if (r < F)
    {
        // importance sample with brdf specular lobe
        vec3 H = ImportanceSampleGGX_VNDF(RandomFloat(), RandomFloat(), this->roughness, inOutRay.m, TBN(normal));
        vec3 reflected = reflect(inOutRay.m, H);
        inOutRay = { point, normalize(reflected) };
    }
    else
    {
        inOutRay = { point, normalize(normalize(normal) + RandomPointOnUnitSphere()) };
    }
}
void Material::BSDF_Conductor(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    float cosTheta = -dot(normalize(inOutRay.m), normalize(normal));

    vec3 outwardNormal;
    float niOverNt;
    vec3 refracted;
    float reflect_prob;
    float cosine;
    vec3 rayDir = inOutRay.m;

    if (cosTheta <= 0)
    {
        outwardNormal = -normal;
        niOverNt = this->refractionIndex;
        cosine = cosTheta * niOverNt / len(rayDir);
    }
    else
    {
        outwardNormal = normal;
        niOverNt = 1.0 / this->refractionIndex;
        cosine = cosTheta / len(rayDir);
    }

    if (Refract(normalize(rayDir), outwardNormal, niOverNt, refracted))
    {
        // fresnel reflectance at 0 deg incidence angle
        float F0 = std::powf(this->refractionIndex - 1, 2) / std::powf(this->refractionIndex + 1, 2);
        reflect_prob = FresnelSchlick(cosine, F0, this->roughness);
    }
    else
    {
        reflect_prob = 1.0;
    }

    if (RandomFloat() < reflect_prob)
    {
        vec3 reflected = reflect(rayDir, normal);
        inOutRay = { point, reflected };
    }
    else
    {
        inOutRay = { point, refracted };
    }
}