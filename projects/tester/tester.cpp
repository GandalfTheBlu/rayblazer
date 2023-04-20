#include <iostream>
#include <string>
#include <chrono>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "raytracer.h"

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

void SaveDataToPNG(const char* filename, size_t width, size_t height, float* colorData)
{
	size_t numberOfChannels = 3;// rgb
	char* imageData = new char[width * height * numberOfChannels];

	// read colorData and map to char values in range 0-255
	// the colorData starts in the bottom left corner and is in column major order but
	// the imageData needs to start in the top left corner and be in row major order
	int index = 0;
	for (int y = int(height) - 1; y >= 0; y--)
	{
		for (int x = 0; x < int(width); x++)
		{
			size_t colorIndex = (y * width + x) * numberOfChannels;
			float r = Clamp01(colorData[colorIndex]);
			float g = Clamp01(colorData[colorIndex + 1]);
			float b = Clamp01(colorData[colorIndex + 2]);

			imageData[index++] = (char)(255.99f * r);
			imageData[index++] = (char)(255.99f * g);
			imageData[index++] = (char)(255.99f * b);
		}
	}

	stbi_write_png(filename, int(width), int(height), int(numberOfChannels), imageData, int(width * numberOfChannels));

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

	float GetMillisecondDuration()
	{
		std::chrono::duration<float, std::milli> duration = endTime - startTime;
		return duration.count();
	}
};

int main(int argc, char* argv[])
{
	// verify arguments
	if (argc < 5 || 
		!IsUnsignedInt(argv[1]) || 
		!IsUnsignedInt(argv[2]) ||
		!IsUnsignedInt(argv[3]) ||
		!IsUnsignedInt(argv[4]) ||
		!IsUnsignedInt(argv[5]))
	{
		std::cout << "incorrect arguments, arguments are: width, height, raysPerPixel, numberOfSpheres, maxBounces, imageFile(optional)" << std::endl;
		return 1;
	}

	// parse arguments
	size_t width = (size_t)std::stoi(argv[1]);
	size_t height = (size_t)std::stoi(argv[2]);
	int raysPerPixel = std::stoi(argv[3]);
	int numberOfSpheres = std::stoi(argv[4]);
	int maxBounces = std::stoi(argv[5]);
	const char* imageFilename = (argc == 7 ? argv[6] : nullptr);

	// setup-code for raytracer
	std::vector<Color> framebuffer;
	framebuffer.resize(width * height);

	std::vector<Color> framebufferCopy;
	framebufferCopy.resize(width * height);

	Raytracer rt = Raytracer(width, height, framebuffer, framebufferCopy, raysPerPixel, maxBounces, numberOfSpheres);
	MemoryPool<Material> materials(numberOfSpheres);

	// create some spheres
	uint32_t seed = 1337420;

	int matType = 0;
	for (int i = 0; i < numberOfSpheres; i++)
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
	std::cout << "number of bounding spheres: " << rt.boundingSpheres.Count() << std::endl;
	
	vec3 camPos = { 0.f, 10.0f, 0.f };
	float rotx = 180.f;
	float roty = 0.f;

	mat4 xMat = rotationx(rotx);
	mat4 yMat = rotationy(roty);
	mat4 cameraTransform = multiply(yMat, xMat);

	cameraTransform.m30 = camPos.x;
	cameraTransform.m31 = camPos.y;
	cameraTransform.m32 = camPos.z;

	rt.SetViewMatrix(cameraTransform);

	std::cout << "starting performance test..." << std::endl;
	Timer timer;
	timer.Start();

	// render "loop"
	const int numberOfIterations = 1;

	for (int i = 0; i < numberOfIterations; i++)
	{
		rt.Raytrace();
	}

	timer.Stop();
	float duration = timer.GetMillisecondDuration() / numberOfIterations;
	size_t rayCount = 0;
	for (int i = 0; i < rt.rayCounters.size(); i++)
	{
		rayCount += rt.rayCounters[i];
	}
	std::cout << "test completed:" << std::endl;
	std::cout << "\taverage time per frame: " << duration << " ms" << std::endl;
	std::cout << "\tnumber of rays spawned last frame: " << rayCount << std::endl;
	std::cout << "\taverage MegaRays/s: " << (((float)rayCount / 1000000.f) / (duration / 1000.f)) << std::endl;

	// save result image
	if (imageFilename != nullptr)
	{
		std::cout << "storing image result to '" << imageFilename << "'" << std::endl;;
		SaveDataToPNG(imageFilename, width, height, &framebufferCopy[0].r);
	}

	return 0;
}