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
class Raytracer
{
public:
    Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, std::vector<Color>& frameBufferCopy, unsigned rpp, unsigned bounces, int maxSpheres);

    ~Raytracer();

    // start raytracing!
    void Raytrace();

    void RaytraceGroup(int pixelX, int pixelY, size_t pixelCount);

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
    Color TracePath(const Ray& ray);

    // get the color of the skybox in a direction
    Color Skybox(vec3 direction);

    std::vector<Color>& frameBuffer;
    std::vector<Color>& frameBufferCopy;
    int frameIndex = 0;
    
    // rays per pixel
    unsigned rpp;
    // max number of bounces before termination
    unsigned bounces = 5;

    // width of framebuffer
    const unsigned width;
    // height of framebuffer
    const unsigned height;

    // view matrix
    mat4 view;
    // Go from canonical to view frustum
    mat4 frustum;

    MemoryPool<Sphere> spheres;
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
