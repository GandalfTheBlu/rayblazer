#pragma once
#include "vec3.h"
#include "mat4.h"
#include <math.h>

//------------------------------------------------------------------------------
/**
*/
inline float
FresnelSchlick(float cosTheta, float F0, float roughness)
{
    return F0 + (std::fmax(1.0f - roughness, F0) - F0) * std::powf(2.f, ((-5.55473f*cosTheta - 6.98316f) * cosTheta));
}

//------------------------------------------------------------------------------
/**
*/
inline vec3
ImportanceSampleGGX_VNDF(float u1, float u2, float roughness, vec3 const& V, mat4 const& basis)
{
    float alpha = roughness * roughness;

    vec3 Ve = -vec3(dot(V, get_row0(basis)), dot(V, get_row2(basis)), dot(V, get_row1(basis)));

    vec3 Vh = normalize(vec3(alpha * Ve.x, alpha * Ve.y, Ve.z));

    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;

    vec3 T1 = lensq > 0.0f ? vec3(-Vh.y, Vh.x, 0.0f) * (1 / std::sqrtf(lensq)) : vec3(1.0f, 0.0f, 0.0f);
    vec3 T2 = cross(Vh, T1);

    float r = std::sqrtf(u1);
    float phi = 2.0f * 3.141592f * u2;
    float cs = std::cos(phi);
    float sn = std::sqrtf(1.f - cs * cs);
    float t1 = r * cs;
    float t2 = r * sn;
    float s = 0.5f * (1.0f + Vh.z);
    float t1sq = (t1 * t1);
    t2 = (1.0f - s) * std::sqrtf(1.0f - t1sq) + s * t2;

    vec3 Nh = T1 * t1 + T2 * t2 + Vh * std::sqrtf(std::fmaxf(0.0f, 1.0f - t1sq - (t2 * t2)));

    // Tangent space H
    vec3 Ne = vec3(alpha * Nh.x, std::fmaxf(0.0f, Nh.z), alpha * Nh.y);

    // World space H
    return normalize(transform(Ne, basis));
}

//------------------------------------------------------------------------------
/**
*/
inline bool
Refract(vec3 v, vec3 n, float niOverNt, vec3& refracted)
{
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0f - niOverNt * niOverNt * (1.0f - dt * dt);
    if (discriminant > 0)
    {
        refracted = ((uv - n * dt) * niOverNt) - (n * std::sqrtf(discriminant));
        return true;
    }

    return false;
}