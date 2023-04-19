#include "material.h"
#include "pbr.h"
#include "mat4.h"
#include "random.h"

inline vec3 RandomPointInUnitCube(uint32_t seed)
{
    return { 0.5f - RandomFloat(++seed), 0.5f - RandomFloat(++seed), 0.5f - RandomFloat(++seed) };
}

//------------------------------------------------------------------------------
/**
*/

void
Material::BSDF(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    switch (type)
    {
    case MaterialType::Lambertian:
        BSDF_Lambertian(inOutRay, point, normal);
        break;
    case MaterialType::Dielectric:
        BSDF_Dielectric(inOutRay, point, normal);
        break;
    case MaterialType::Conductor:
        BSDF_Conductor(inOutRay, point, normal);
        break;
    }
}

void Material::BSDF_Lambertian(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    static uint32_t seed = 1337420;
    float cosTheta = -dot(inOutRay.dir, normal);

    // probability that a ray will reflect on a microfacet
    float F = FresnelSchlick(cosTheta, 0.04f, this->roughness);
    float r = RandomFloat(++seed);

    if (r < F)
    {
        // importance sample with brdf specular lobe
        vec3 H = ImportanceSampleGGX_VNDF(RandomFloat(++seed), RandomFloat(++seed), this->roughness, inOutRay.dir, TBN(normal));
        inOutRay = {point, reflect(inOutRay.dir, H) };
    }
    else
    {
        inOutRay = {point, normalize(normal + RandomPointInUnitCube(++seed)) };
    }
}
void Material::BSDF_Dielectric(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    static uint32_t seed = 1337420;
    float cosTheta = -dot(inOutRay.dir, normal);

    // probability that a ray will reflect on a microfacet
    float F = FresnelSchlick(cosTheta, 0.95f, this->roughness);
    float r = RandomFloat(++seed);

    if (r < F)
    {
        // importance sample with brdf specular lobe
        vec3 H = ImportanceSampleGGX_VNDF(RandomFloat(++seed), RandomFloat(++seed), this->roughness, inOutRay.dir, TBN(normal));
        vec3 reflected = reflect(inOutRay.dir, H);
        inOutRay = { point, reflect(inOutRay.dir, H) };

    }
    else
    {
        inOutRay = { point, normalize(normal + RandomPointInUnitCube(++seed)) };
    }
}
void Material::BSDF_Conductor(Ray& inOutRay, const vec3& point, const vec3& normal) const
{
    static uint32_t seed = 1337420;
    float cosTheta = -dot(inOutRay.dir, normal);

    vec3 outwardNormal;
    float niOverNt;
    vec3 refracted;
    float reflect_prob;
    float cosine;
    vec3 rayDir = inOutRay.dir;

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

    if (RandomFloat(++seed) < reflect_prob)
    {
        inOutRay = { point, reflect(rayDir, normal) };
    }
    else
    {
        inOutRay = { point, refracted };
    }
}