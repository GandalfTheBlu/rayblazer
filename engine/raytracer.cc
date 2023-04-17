#include "raytracer.h"
#include "random.h"
#include <cassert>

//------------------------------------------------------------------------------
/**
*/
Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces) :
    frameBuffer(frameBuffer),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Raytrace()
{
    static float aspect = (float)(this->width / this->height);

    for (int y = 0; y < this->height; y++)
    {
        const int row = y * this->width;
        for (int x = 0; x < this->width; x++)
        {
            Color color;
            for (int i = 0; i < this->rpp; ++i)
            {
                float u = (((float(x + RandomFloat()) * (1.0f / this->width)) * 2.0f) - 1.0f) * aspect;
                float v = (((float(y + RandomFloat()) * (1.0f / this->height)) * 2.0f) - 1.0f);

                vec3 direction(u, v, -1.0f);
                direction = transform(direction, this->frustum);
                
                Ray ray(get_position(this->view), direction);
                color += this->TracePath(ray);
            }

            // divide by number of samples per pixel, to get the average of the distribution
            color.r /= this->rpp;
            color.g /= this->rpp;
            color.b /= this->rpp;

            this->frameBuffer[row + x] += color;
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
Color
Raytracer::TracePath(const Ray& ray)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Object* hitObject = nullptr;
    float distance = FLT_MAX;
    Ray updatedRay = ray;
    Color color = {1.f, 1.f, 1.f};

    for (int i = 0; i < bounces; i++)
    {
        if (!Raycast(updatedRay, hitPoint, hitNormal, hitObject, distance))
        {
            color = color * Skybox(updatedRay.m);
            break;
        }

        color = color * hitObject->GetColor();

        hitObject->ScatterRay(updatedRay, hitPoint, hitNormal);
    }

    return color;
}

//------------------------------------------------------------------------------
/**
*/
bool
Raytracer::Raycast(const Ray& ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, float& distance)
{
    bool isHit = false;
    HitResult closestHit;

    for(int i=0; i<this->objects.size(); i++)
    {
        Object* object = this->objects[i];

        if (object->Intersect(ray, closestHit.t, closestHit))
        {
            isHit = true;
        }
    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    distance = closestHit.t;
    
    return isHit;
}


//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Clear()
{
    for (auto& color : this->frameBuffer)
    {
        color.r = 0.0f;
        color.g = 0.0f;
        color.b = 0.0f;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::UpdateMatrices()
{
    this->frustum = transpose(inverse(this->view));
}

//------------------------------------------------------------------------------
/**
*/
Color
Raytracer::Skybox(vec3 direction)
{
    float t = 0.5f*(direction.y + 1.f);
    float it = 1.f - t;
    return {it + 0.5f*t, it + 0.7f*t, it + 1.f*t};
}
