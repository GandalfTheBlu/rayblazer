#include <stdio.h>
#include "window.h"
#include "vec3.h"
#include "raytracer.h"

#define degtorad(angle) angle * MPI / 180

int main()
{ 
    Display::Window wnd;
    
    wnd.SetTitle("TrayRacer");
    
    if (!wnd.Open())
        return 1;

    std::vector<Color> framebuffer;

    const unsigned w = 1000;
    const unsigned h = 500;
    framebuffer.resize(w * h);

    std::vector<Color> framebufferCopy;
    framebufferCopy.resize(w * h);

    int pixelsSize = 1;
    wnd.SetSize(w*pixelsSize, h*pixelsSize);
    
    int raysPerPixel = 1;
    int maxBounces = 5;
    int maxSpheres = 500;

    Raytracer rt = Raytracer(w, h, framebuffer, framebufferCopy, raysPerPixel, maxBounces, maxSpheres);
    MemoryPool<Material> materials(maxSpheres);

    uint32_t seed = 1337420;

    // Create some objects
    int matType = 0;
    for (int i = 0; i < maxSpheres; i++)
    {
        Material* mat = materials.GetNew();
        switch (matType++)
        {
        case 0:
            mat->type = MaterialType::Lambertian;
            break;
        case 1:
            mat->type = MaterialType::Conductor;
            break;
        case 2:
            mat->type = MaterialType::Dielectric;
            matType = 0;
            break;
        }
        float r = RandomFloat(++seed);
        float g = RandomFloat(++seed);
        float b = RandomFloat(++seed);
        mat->color = { r,g,b };
        mat->roughness = RandomFloat(++seed);

        const vec3 minPos(-50.f, 0.f, -100.f);
        const vec3 maxPos(50, 50, 20);
        const vec3 span = maxPos - minPos;

        float radius = RandomFloat(++seed) * 1.5f + 0.5f;
        vec3 pos(
            minPos.x + span.x * RandomFloat(++seed),
            minPos.y + span.y * RandomFloat(++seed),
            minPos.z + span.z * RandomFloat(++seed)
        );

        *rt.GetNewSphere() = Sphere(radius, pos, mat);
    }

    rt.CreateBoundingSpheres();
    
    bool exit = false;

    // camera
    bool resetFramebuffer = false;
    vec3 camPos = { 0,1.0f,10.0f };
    vec3 moveDir = { 0,0,0 };

    wnd.SetKeyPressFunction([&exit, &moveDir, &resetFramebuffer](int key, int scancode, int action, int mods)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            exit = true;
            break;
        case GLFW_KEY_W:
            moveDir.z -= 1.0f;
            resetFramebuffer = true;
            break;
        case GLFW_KEY_S:
            moveDir.z += 1.0f;
            resetFramebuffer = true;
            break;
        case GLFW_KEY_A:
            moveDir.x -= 1.0f;
            resetFramebuffer = true;
            break;
        case GLFW_KEY_D:
            moveDir.x += 1.0f;
            resetFramebuffer = true;
            break;
        case GLFW_KEY_Q:
            moveDir.y += 1.0f;
            resetFramebuffer = true;
            break;
        case GLFW_KEY_E:
            moveDir.y -= 1.0f;
            resetFramebuffer = true;
            break;
        default:
            break;
        }
    });

    float pitch = 0;
    float yaw = 0;
    float oldx = 0;
    float oldy = 0;
    bool firstUpdate = true;

    wnd.SetMouseMoveFunction([&firstUpdate, &pitch, &yaw, &oldx, &oldy, &resetFramebuffer](double x, double y)
    {
        float fx = (float)x;
        float fy = (float)y;

        if (firstUpdate)
        {
            firstUpdate = false;
        }
        else
        {
            yaw += 0.1f * (fx - oldx);
            pitch += 0.1f * (fy - oldy);
            resetFramebuffer = true;
        }
        
        oldx = fx;
        oldy = fy;
    });

    // rendering loop
    while (wnd.IsOpen() && !exit)
    {
        resetFramebuffer = false;
        moveDir = {0,0,0};

        // poll input
        wnd.Update();

        moveDir = normalize(moveDir);

        mat4 xMat = (rotationx(pitch));
        mat4 yMat = (rotationy(yaw));
        mat4 cameraTransform = multiply(yMat, xMat);

        camPos = camPos + transform(moveDir * 0.2f, cameraTransform);
        
        cameraTransform.m30 = camPos.x;
        cameraTransform.m31 = camPos.y;
        cameraTransform.m32 = camPos.z;

        rt.SetViewMatrix(cameraTransform);
        
        if (resetFramebuffer)
        {
            rt.Clear();
        }

        rt.Raytrace();

        glClearColor(0, 0, 0, 1.0);
        glClear( GL_COLOR_BUFFER_BIT );

        wnd.Blit((float*)&framebufferCopy[0], w, h);
        wnd.SwapBuffers();
    }

    if (wnd.IsOpen())
        wnd.Close();

    return 0;
}