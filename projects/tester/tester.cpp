#include <iostream>
#include <string>
#include <chrono>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "vec3.h"
#include "raytracer.h"
#include "sphere.h"

bool IsDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool IsUnsignedInt(const char* str)
{
	size_t length = std::strlen(str);
	for (size_t i = 0; i < length; i++)
		if (!IsDigit(str[i]))
			return false;

	return true;
}

float Clamp01(float v)
{
	return (v < 0.f ? 0.f : (v > 1.f ? 1.f : v));
}

void SaveDataToPNG(const char* filename, int width, int height, float* colorData)
{
	int numberOfChannels = 3;// rgb
	char* imageData = new char[(size_t)width * height * numberOfChannels];

	// read colorData and map to char values in range 0-255
	// the colorData starts in the bottom left corner and is in column major order but
	// the imageData needs to start in the top left corner and be in row major order
	int index = 0;
	for (int y = height - 1; y >= 0; y--)
	{
		for (int x = 0; x < width; x++)
		{
			int colorIndex = (y * width + x) * numberOfChannels;
			float r = Clamp01(colorData[colorIndex]);
			float g = Clamp01(colorData[colorIndex + 1]);
			float b = Clamp01(colorData[colorIndex + 2]);

			imageData[index++] = (char)(255.99f * r);
			imageData[index++] = (char)(255.99f * g);
			imageData[index++] = (char)(255.99f * b);
		}
	}

	stbi_write_png(filename, width, height, numberOfChannels, imageData, width * numberOfChannels);

	delete[] imageData;
}

struct Timer
{
private:
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point endTime;

public:
	Timer(){}

	void Start()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	void Stop()
	{
		endTime = std::chrono::high_resolution_clock::now();
	}

	double GetMillisecondDuration()
	{
		std::chrono::duration<double, std::milli> duration = endTime - startTime;
		return duration.count();
	}
};

float MyRandomFloat01()
{
	static uint32_t x = 1337420;
	uint32_t max = -1;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	return (float)x / max;
}

int main(int argc, char* argv[])
{
	// verify arguments
	if (argc < 5 || 
		!IsUnsignedInt(argv[1]) || 
		!IsUnsignedInt(argv[2]) ||
		!IsUnsignedInt(argv[3]) ||
		!IsUnsignedInt(argv[4]))
	{
		std::cout << "incorrect arguments, arguments are: width, height, raysPerPixel, numberOfSpheres, imageFile(optional)" << std::endl;
		return 1;
	}

	// parse arguments
	size_t width = (size_t)std::stoi(argv[1]);
	size_t height = (size_t)std::stoi(argv[2]);
	int raysPerPixel = std::stoi(argv[3]);
	int numberOfSpheres = std::stoi(argv[4]);
	const char* imageFilename = (argc == 6 ? argv[5] : nullptr);

	// setup-code for raytracer
	std::vector<Color> framebuffer;
	framebuffer.resize(width * height);

	std::vector<Color> framebufferCopy;
	framebufferCopy.resize(width * height);

	int maxBounces = 5;

	Raytracer rt = Raytracer(width, height, framebuffer, raysPerPixel, maxBounces);

	{
		Material* mat = new Material();
		mat->type = "Lambertian";
		mat->color = { 0.5,0.5,0.5 };
		mat->roughness = 0.3;
		Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);
		rt.AddObject(ground);
	}

	for (int i = 0; i < numberOfSpheres; i++)
	{
		Material* mat = new Material();
		mat->type = "Lambertian";
		float r = MyRandomFloat01();
		float g = MyRandomFloat01();
		float b = MyRandomFloat01();
		mat->color = { r,g,b };
		mat->roughness = MyRandomFloat01();
		const float span = 10.0f;
		Sphere* ground = new Sphere(
			MyRandomFloat01() * 0.7f + 0.2f,
			{
				(-1.f + 2.f * MyRandomFloat01()) * span,
				MyRandomFloat01() * span + 0.2f,
				(-1.f + 2.f * MyRandomFloat01())* span
			},
			mat
		);
		rt.AddObject(ground);
	}
	
	vec3 camPos = { 0,1.0f,10.0f };
	vec3 moveDir = { 0,0,0 };
	float pitch = 0;
	float yaw = 0;
	float oldx = 0;
	float oldy = 0;
	float rotx = 0;
	float roty = 0;
	int frameIndex = 0;

	std::cout << "starting performance test..." << std::endl;
	Timer timer;
	timer.Start();

	// render "loop"
	const int numberOfIterations = 300;
	for (int i = 0; i < numberOfIterations; i++)
	{
		moveDir = normalize(moveDir);

		mat4 xMat = (rotationx(rotx));
		mat4 yMat = (rotationy(roty));
		mat4 cameraTransform = multiply(yMat, xMat);

		camPos = camPos + transform(moveDir * 0.2f, cameraTransform);

		cameraTransform.m30 = camPos.x;
		cameraTransform.m31 = camPos.y;
		cameraTransform.m32 = camPos.z;

		rt.SetViewMatrix(cameraTransform);

		rt.Raytrace();
		frameIndex++;

		// Get the average distribution of all samples
		{
			size_t p = 0;
			for (Color const& pixel : framebuffer)
			{
				framebufferCopy[p] = pixel;
				framebufferCopy[p].r /= frameIndex;
				framebufferCopy[p].g /= frameIndex;
				framebufferCopy[p].b /= frameIndex;
				p++;
			}
		}
	}

	timer.Stop();
	float duration = timer.GetMillisecondDuration() / numberOfIterations;
	float raysSpawned = (float)Ray::spawnedCount / numberOfIterations;
	std::cout << "test completed:" << std::endl;
	std::cout << "\taverage time per frame: " << duration << " ms" << std::endl;
	std::cout << "\taverage number of rays spawned per frame: " << raysSpawned << std::endl;
	std::cout << "\taverage MegaRays/s: " << ((raysSpawned / 1000000.f) / (duration / 1000.f)) << std::endl;

	// save result image
	if (imageFilename != nullptr)
	{
		std::cout << "storing image result to '" << imageFilename << "'" << std::endl;;
		SaveDataToPNG(imageFilename, width, height, &framebufferCopy[0].r);
	}

	return 0;
}