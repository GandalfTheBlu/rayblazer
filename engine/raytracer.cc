#include "raytracer.h"
#include "random.h"

struct WorkArgs
{
    Raytracer* self;
    int pixelX, pixelY, pixelCount;
};

void RenderThreadWork(const WorkArgs& args)
{
    args.self->RaytraceGroup(args.pixelX, args.pixelY, args.pixelCount);
}

//------------------------------------------------------------------------------
/**
*/
Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, std::vector<Color>& frameBufferCopy, unsigned rpp, unsigned bounces, int maxSpheres) :
    frameBuffer(frameBuffer),
    frameBufferCopy(frameBufferCopy),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h),
    view(zero_mat()),
    frustum(zero_mat()),
    spheres(maxSpheres),
    renderThreads(std::thread::hardware_concurrency())
{
    int x = 0;
    int y = 0;
    int pixelCount = width * height / renderThreads.size;
    for (int i = 0; i < renderThreads.size; i++)
    {
        renderThreads.InitThread<WorkArgs>(RenderThreadWork, {this, x, y, pixelCount}, i);
        x += pixelCount;
        while (x >= width)
        {
            y++;
            x -= width;
        }
    }
}

Raytracer::~Raytracer()
{
    
}

void Raytracer::RaytraceGroup(int pixelX, int pixelY, size_t pixelCount)
{
    static uint32_t seed = 1337420;
    vec3 origin = get_position(view);
    float aspect = (float)(width / height);
    int row = pixelY * width;

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
            color += TracePath(Ray(origin, direction));
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
            row = pixelY * width;
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
Raytracer::TracePath(const Ray& ray)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Material* hitMaterial = nullptr;
    float distance = FLT_MAX;
    Ray updatedRay = ray;
    Color color = {1.f, 1.f, 1.f};

    for (int i = 0; i < bounces; i++)
    {
        if (!Raycast(updatedRay, hitPoint, hitNormal, hitMaterial, distance))
        {
            color = color * Skybox(updatedRay.dir);
            break;
        }

        color = color * hitMaterial->color;

        hitMaterial->BSDF(updatedRay, hitPoint, hitNormal);
    }

    return color;
}

//------------------------------------------------------------------------------
/**
*/
inline bool
Raytracer::Raycast(const Ray& ray, vec3& hitPoint, vec3& hitNormal, Material*& hitMaterial, float& distance)
{
    bool isHit = false;
    HitResult closestHit;

    for(int i=0; i<this->spheres.Count(); i++)
    {
        if (this->spheres[i]->Intersect(ray, closestHit.t, closestHit))
        {
            isHit = true;
        }
    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitMaterial = closestHit.material;
    distance = closestHit.t;
    
    return isHit;
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
