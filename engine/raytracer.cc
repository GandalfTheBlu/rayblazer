#include "raytracer.h"
#include "random.h"

struct WorkArgs
{
    Raytracer* self;
    int pixelX, pixelY;
    size_t pixelCount;
    int workerIndex;
};

void RenderThreadWork(const WorkArgs& args)
{
    size_t rayCount = 0;
    args.self->RaytraceGroup(args.pixelX, args.pixelY, args.pixelCount, &rayCount);
    args.self->rayCounters[args.workerIndex] = rayCount;
}

//------------------------------------------------------------------------------
/**
*/
Raytracer::Raytracer(size_t w, size_t h, std::vector<Color>& frameBuffer, std::vector<Color>& frameBufferCopy, size_t rpp, size_t bounces, int maxSpheres) :
    frameBuffer(frameBuffer),
    frameBufferCopy(frameBufferCopy),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h),
    view(zero_mat()),
    frustum(zero_mat()),
    boundingSpheres(maxSpheres),
    spheres(maxSpheres),
    renderThreads(std::thread::hardware_concurrency())
{
    int x = 0;
    int y = 0;
    size_t pixelCount = width * height / renderThreads.size;
    for (int i = 0; i < renderThreads.size; i++)
    {
        rayCounters.push_back(0);
        renderThreads.InitThread<WorkArgs>(RenderThreadWork, {this, x, y, pixelCount, i}, i);
        x += int(pixelCount);
        while (x >= width)
        {
            y++;
            x -= int(width);
        }
    }
}

Raytracer::~Raytracer()
{

}

void Raytracer::CreateBoundingSpheres()
{
    for (int i = 0; i < spheres.Count(); i++)
    {
        int j = 0;
        for (; j < boundingSpheres.Count(); j++)
        {
            if (boundingSpheres[j]->TryAddSphere(i, spheres[i]->center, spheres[i]->radius))
            {
                break;
            }
        }
        
        if (j == boundingSpheres.Count())
        {
            *boundingSpheres.GetNew() = BoundingSphere();
            boundingSpheres[j]->TryAddSphere(i, spheres[i]->center, spheres[i]->radius);
        }
    }

    int a = 0;
}

void Raytracer::RaytraceGroup(int pixelX, int pixelY, size_t pixelCount, size_t* rayCount)
{
    // just some random stuff that changes over time
    uint32_t seed = 1337420 + (pixelX | frameIndex ^ 45312) * 1234 + (pixelY | frameIndex ^ 31235) * 4321;

    vec3 origin = get_position(view);
    float aspect = (float)width / height;
    int row = pixelY * int(width);

    float two_inv_width = 2.f / width;
    float two_inv_height = 2.f / height;
    float inv_frameIndex = 1.f / frameIndex;
    float inv_rpp = 1.f / rpp;

    for (size_t i = 0; i < pixelCount; i++)
    {
        Color color;
        for (int i = 0; i < rpp; ++i)
        {
            float u = ((float(pixelX + RandomFloat(++seed)) * two_inv_width) - 1.0f) * aspect;
            float v = ((float(pixelY + RandomFloat(++seed)) * two_inv_height) - 1.0f);

            vec3 direction = normalize(transform({ u, v, -1.0f }, frustum));
            color += TracePath(Ray(origin, direction), seed, rayCount);
        }

        // divide by number of samples per pixel, to get the average of the distribution
        color *= inv_rpp;

        int index = row + pixelX;
        Color& res = frameBuffer[index];
        res += color;
        frameBufferCopy[index] = res * inv_frameIndex;

        if (++pixelX >= width)
        {
            if (++pixelY >= height)
            {
                return;
            }
            pixelX = 0;
            row = pixelY * int(width);
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Raytrace()
{
    frameIndex++;
    renderThreads.ExecuteAndWait();
}

//------------------------------------------------------------------------------
/**
*/
inline Color
Raytracer::TracePath(const Ray& ray, uint32_t seed, size_t* rayCount)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Material* hitMaterial = nullptr;
    float distance = FLT_MAX;
    Ray updatedRay = ray;
    Color color = {1.f, 1.f, 1.f};

    for (int i = 0; i < bounces; i++)
    {
        (*rayCount)++;
        if (!Raycast(updatedRay, hitPoint, hitNormal, hitMaterial, distance))
        {
            color = color * Skybox(updatedRay.dir);
            break;
        }

        color = color * hitMaterial->color;

        hitMaterial->BSDF(updatedRay, hitPoint, hitNormal, ++seed);
    }

    return color;
}

//------------------------------------------------------------------------------
/**
*/
inline bool
Raytracer::Raycast(const Ray& ray, vec3& hitPoint, vec3& hitNormal, Material*& hitMaterial, float& distance)
{
    HitResult closestHit;
    int sphereIndex = -1;

    // no bounding spheres
    /*for (int j = 0; j < spheres.Count(); j++)
    {
        if (IntersectSphere(ray, spheres[j]->center, spheres[j]->radius, closestHit.t, closestHit))
        {
            sphereIndex = j;
        }
    }*/

    HitResult boundingSphereHit;

    for (int i = 0; i < boundingSpheres.Count(); i++)
    {
        BoundingSphere* bs = boundingSpheres[i];
        vec3 toCenter = bs->center - ray.origin;
        float distSquared = dot(toCenter, toCenter);
        bool isInside = distSquared < bs->radius* bs->radius;

        if (isInside || IntersectSphere(ray, bs->center, bs->radius, closestHit.t, boundingSphereHit))
        {
            for (int j = 0; j < bs->count; j++)
            {
                int index = bs->containedSphereIndices[j];
                Sphere* s = spheres[index];
                
                if (IntersectSphere(ray, s->center, s->radius, closestHit.t, closestHit))
                {
                    sphereIndex = index;
                }
            }
        }
    }
    
    if (sphereIndex != -1)
    {
        hitPoint = closestHit.p;
        hitNormal = closestHit.normal;
        distance = closestHit.t;
        hitMaterial = spheres[sphereIndex]->material;
        return true;
    }

    return false;
}


//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Clear()
{
    frameIndex = 0;
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
