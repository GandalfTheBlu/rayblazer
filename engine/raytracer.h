#pragma once
#include <vector>
#include "vec3.h"
#include "mat4.h"
#include "color.h"
#include "ray.h"
#include <float.h>
#include "mempool.h"
#include "sphere.h"
#include "threadpool.h"

//------------------------------------------------------------------------------
/**
*/

struct BoundingSphere
{
#define MAX_RADIUS 10.f
#define MAX_SPHERES 20
#define RADIUS_MARGIN 0.1f
    float radius;
    vec3 center;
    int containedSphereIndices[MAX_SPHERES];
    int count;

    BoundingSphere()
    {
        radius = 0.f;
        count = 0;

        for (int i = 0; i < MAX_SPHERES; i++)
            containedSphereIndices[i] = -1;
    }

    bool TryAddSphere(int index, const vec3& _center, float _radius)
    {
        if (count == MAX_SPHERES)
            return false;

        if (count == 0)
        {
            containedSphereIndices[0] = index;
            center = _center;
            radius = _radius + RADIUS_MARGIN;
            count++;
            return true;
        }

        float dist = len(_center - center) + _radius;
        if (dist > MAX_RADIUS)
            return false;

        containedSphereIndices[count] = index;
        radius = radius < dist ? (dist + RADIUS_MARGIN) : radius;
        count++;

        return true;
    }
};

class Raytracer
{
public:
    Raytracer(size_t w, size_t h, std::vector<Color>& frameBuffer, std::vector<Color>& frameBufferCopy, size_t rpp, size_t bounces, int maxSpheres);

    ~Raytracer();

    void CreateBoundingSpheres();

    // start raytracing!
    void Raytrace();

    void RaytraceGroup(int pixelX, int pixelY, size_t pixelCount, size_t* rayCount);

    // add object to scene
    //void AddObject(Object* obj);

    Sphere* GetNewSphere();

    // single raycast, find object
    bool Raycast(const Ray& ray, vec3& hitPoint, vec3& hitNormal, Material*& hitMaterial, float& distance);

    // set camera matrix
    void SetViewMatrix(const mat4& val);

    // clear screen
    void Clear();

    // update matrices. Called automatically after setting view matrix
    void UpdateMatrices();

    // trace a path and return intersection color
    Color TracePath(const Ray& ray, uint32_t seed, size_t* rayCount);

    // get the color of the skybox in a direction
    Color Skybox(vec3 direction);

    std::vector<Color>& frameBuffer;
    std::vector<Color>& frameBufferCopy;
    int frameIndex = 0;
    
    // rays per pixel
    size_t rpp;
    // max number of bounces before termination
    size_t bounces = 5;

    // width of framebuffer
    const size_t width;
    // height of framebuffer
    const size_t height;

    // view matrix
    mat4 view;
    // Go from canonical to view frustum
    mat4 frustum;

    MemoryPool<BoundingSphere> boundingSpheres;

    MemoryPool<Sphere> spheres;
    std::vector<size_t> rayCounters;
    ThreadPool renderThreads;
};

inline Sphere* Raytracer::GetNewSphere()
{
    return this->spheres.GetNew();
}

inline void Raytracer::SetViewMatrix(const mat4& val)
{
    this->view = val;
    this->UpdateMatrices();
}
