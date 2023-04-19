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

    const unsigned w = 1280;
    const unsigned h = 720;
    framebuffer.resize(w * h);

    std::vector<Color> framebufferCopy;
    framebufferCopy.resize(w * h);

    int pixelsSize = 1;
    wnd.SetSize(w*pixelsSize, h*pixelsSize);
    
    int raysPerPixel = 1;
    int maxBounces = 5;
    int maxSpheres = 100;

    Raytracer rt = Raytracer(w, h, framebuffer, framebufferCopy, raysPerPixel, maxBounces, maxSpheres);
    MemoryPool<Material> materials(100);

    // Create some objects
    Material* mat = materials.GetNew();
    mat->type = MaterialType::Lambertian;
    mat->color = { 0.5,0.5,0.5 };
    mat->roughness = 0.3;
    Sphere* ground = rt.GetNewSphere();
    *ground = Sphere(1000, { 0,-1000, -1 }, mat);

    uint32_t seed = 1337420;

    for (int it = 0; it < 12; it++)
    {
        {
            Material* mat = materials.GetNew();
                mat->type = MaterialType::Lambertian;
                float r = RandomFloat(++seed);
                float g = RandomFloat(++seed);
                float b = RandomFloat(++seed);
                mat->color = { r,g,b };
                mat->roughness = RandomFloat(++seed);
                const float span = 10.0f;
                Sphere* sphere = rt.GetNewSphere();
                *sphere = Sphere(
                    RandomFloat(++seed) * 0.7f + 0.2f,
                    {
                        RandomFloatNTP(++seed) * span,
                        RandomFloat(++seed) * span + 0.2f,
                        RandomFloatNTP(++seed) * span
                    },
                    mat);
        }{
            Material* mat = materials.GetNew();
            mat->type = MaterialType::Conductor;
            float r = RandomFloat(++seed);
            float g = RandomFloat(++seed);
            float b = RandomFloat(++seed);
            mat->color = { r,g,b };
            mat->roughness = RandomFloat(++seed);
            const float span = 30.0f;
            Sphere* sphere = rt.GetNewSphere();
            *sphere = Sphere(
                RandomFloat(++seed) * 0.7f + 0.2f,
                {
                    RandomFloatNTP(++seed) * span,
                    RandomFloat(++seed) * span + 0.2f,
                    RandomFloatNTP(++seed) * span
                },
                mat);
        }{
            Material* mat = materials.GetNew();
            mat->type = MaterialType::Dielectric;
            float r = RandomFloat(++seed);
            float g = RandomFloat(++seed);
            float b = RandomFloat(++seed);
            mat->color = { r,g,b };
            mat->roughness = RandomFloat(++seed);
            mat->refractionIndex = 1.65;
            const float span = 25.0f;
            Sphere* sphere = rt.GetNewSphere();
            *sphere = Sphere(
                RandomFloat(++seed) * 0.7f + 0.2f,
                {
                    RandomFloatNTP(++seed) * span,
                    RandomFloat(++seed) * span + 0.2f,
                    RandomFloatNTP(++seed) * span
                },
                mat);
        }
    }
    
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
            yaw += 0.1 * (fx - oldx);
            pitch += 0.1 * (fy - oldy);
            resetFramebuffer = true;
        }
        
        oldx = fx;
        oldy = fy;
    });

    float rotx = 0;
    float roty = 0;

    // number of accumulated frames
    int frameIndex = 0;

    // rendering loop
    while (wnd.IsOpen() && !exit)
    {
        resetFramebuffer = false;
        moveDir = {0,0,0};
        //pitch = 0;
        //yaw = 0;

        // poll input
        wnd.Update();

        //rotx -= pitch;
        //roty -= yaw;

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