#include "Renderer.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include "Walnut/Random.h"

#include <execution>
#include <chrono>

namespace Utility
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t red = (uint8_t)(color.r * 255.0f);
		uint8_t green = (uint8_t)(color.g * 255.0f);
		uint32_t blue = (uint8_t)(color.b * 255.0f);
		uint32_t alpha = (uint8_t)(color.a * 255.0f);

		return ((alpha << 24) | (blue << 16) | (green << 8) | red);
	}

	static glm::vec4 ConvertToVec4(const uint32_t& rgba)
	{
		float red = (float)(rgba & 0xFF) / 255.0f;
		float green = (float)((rgba >> 8) & 0xFF) / 255.0f;
		float blue = (float)((rgba >> 16) & 0xFF) / 255.0f;
		float alpha = (float)((rgba >> 24) & 0xFF) / 255.0f;

		return glm::vec4(red, green, blue, alpha);
	}

	/*
	*					PCG Hash Function
	*		* https://jcgt.org/published/0009/03/02/
	*       * https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
	*/

	static uint32_t pcg_hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	static float RandomFloat(uint32_t& seed)
	{
		seed = pcg_hash(seed);
		const uint32_t mantissa_mask = (1 << 23) - 1; // 23 bits for mantissa
		uint32_t mantissa_bits = seed & mantissa_mask; // Extract mantissa bits from the seed
		const uint32_t one_as_int = 127 << 23; // 1.0 in IEEE 754 as an integer

		uint32_t random_bits = (mantissa_bits | one_as_int); // Combine the mantissa bits with the exponent for 1.0f
		float random_float;
		memcpy(&random_float, &random_bits, sizeof(random_float)); // Convert the integer to a float

		return random_float - 1.0f; // Get a random float in the range [0, 1]
	}

	static glm::vec3 RandomInUnitSphere(uint32_t& seed)
	{
		return glm::normalize(glm::vec3(
			RandomFloat(seed) * 2.0f - 1.0f, 
			RandomFloat(seed) * 2.0f - 1.0f, 
			RandomFloat(seed) * 2.0f - 1.0f)
		);
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// Exit if the image is already the correct size
		if(m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	
	// Delete the old image data and allocate new memory
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	// Delete the old accumulation buffer and allocate new memory
	delete[] m_AccumulationBuffer;
	m_AccumulationBuffer = new glm::vec4[width * height];

	m_HorizontalPixelIterator.resize(width);
	for (uint32_t i = 0; i < width; i++)
	{
		m_HorizontalPixelIterator[i] = i;
	}

	m_VerticalPixelIterator.resize(height);
	for (uint32_t i = 0; i < height; i++)
	{
		m_VerticalPixelIterator[i] = i;
	}

	ResetFrameCount();
}

void Renderer::Render(const Camera& camera, const Scene& scene)
{
	m_CurrentScene = &scene;
	m_CurrentCamera = &camera;

	if (m_FrameCount == 1)
	{
		memset(m_AccumulationBuffer, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));
	}

	/*
		* More efficient by rendering horizontally instead of vertically
		* by accessing contiguous memory
		* also by potential cache hits
	*/
	
#define MT_RENDER 1
#if MT_RENDER

	// Parallelize the rendering process
	std::for_each(std::execution::par, m_VerticalPixelIterator.begin(), m_VerticalPixelIterator.end(), [this](uint32_t y)
		{
			std::for_each(std::execution::par, m_HorizontalPixelIterator.begin(), m_HorizontalPixelIterator.end(), [this, y](uint32_t x)
				{
					// Calculate the color of the pixel at the coordinate and Update
					glm::vec4 color = RayGen(x, y);
					m_AccumulationBuffer[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 finalColor = m_AccumulationBuffer[x + y * m_FinalImage->GetWidth()];
					finalColor /= (float)m_FrameCount;

					finalColor = glm::clamp(finalColor, glm::vec4(0.0f), glm::vec4(1.0f)); // Clamp the color to the range [0, 1]
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utility::ConvertToRGBA(finalColor);
				});
		});

#else

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{ 
			// Calculate the color of the pixel at the coordinate and Update
			glm::vec4 color = RayGen(x, y);
			m_AccumulationBuffer[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 finalColor = m_AccumulationBuffer[x + y * m_FinalImage->GetWidth()];
			finalColor /= (float)m_FrameCount;

			finalColor = glm::clamp(finalColor, glm::vec4(0.0f), glm::vec4(1.0f)); // Clamp the color to the range [0, 1]
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utility::ConvertToRGBA(finalColor);
		}
	}

#endif

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
	{
		// Increment the frame count
		m_FrameCount++;
	}
	else
	{
		// Reset the frame count
		m_FrameCount = 1;
	}
}

void Renderer::ChangeSphereColor(float colorR, float colorG, float colorB)
{
	SphereColor = { colorR, colorG, colorB };
}

void Renderer::ChangeLightPosition(float lightPosX, float lightPosY, float lightPosZ)
{
	LightPosition = { lightPosX, lightPosY, lightPosZ };
}

glm::vec4 Renderer::RayGen(uint32_t x, uint32_t y)
{
	// Define the ray
	Ray ray;
	ray.Origin = m_CurrentCamera->GetPosition();
	ray.Direction = m_CurrentCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	// Define the light(s)
	const Light& light = m_CurrentScene->Lights[0];
	glm::normalize(light.Position);

	glm::vec3 litColor = { 0.0f, 0.0f, 0.0f };
	glm::vec3 throughput(1.0f);
	int numBounces = 10;

	uint32_t seed = x + y * m_FinalImage->GetWidth() * m_FrameCount;

	for (int i = 0; i < numBounces; i++)
	{
		seed += i;

		// Trace the ray
		Renderer::HitEvent hitEvent = TraceRay(ray);

		// If the ray did not hit anything, return background color
		if (!hitEvent.Hit || hitEvent.HitDistance < 0)
		{
			glm::vec3 skyColor = { 0.6f, 0.7f, 0.9f };
			litColor += skyColor * throughput;
			break;
		}

		// Define the closest sphere
		const Sphere& sphere = m_CurrentScene->Spheres[hitEvent.HitObjectIndex];
		const Material& material = m_CurrentScene->Materials[sphere.MaterialIndex];
		glm::vec3 sphereColor = material.Albedo;

		/*
		// Calculate the light intensity and color
		float lightIntensity = glm::max(glm::dot(hitEvent.WorldNormal, -light.Position), 0.0f); // Equiv to cos(theta)
		sphereColor *= lightIntensity;
		litColor += sphereColor * throughput * glm::vec3(0.5f, 0.5f, 0.5f);
		*/

		// Absorb color for each bounce
		throughput *= material.Albedo;
		litColor += material.GetEmittingColor() * throughput;

		// Calculate the new ray direction
		ray.Origin = hitEvent.WorldPosition + hitEvent.WorldNormal * 0.001f;

		// Calculate Reflection
		glm::vec3 reflectDirection = glm::reflect(ray.Direction, hitEvent.WorldNormal);
		glm::vec3 randomDirection; 

		if (m_Settings.FastRandom)
		{
			randomDirection = glm::normalize(hitEvent.WorldNormal + Utility::RandomInUnitSphere(seed));
		}
		else
		{
			randomDirection = glm::normalize(hitEvent.WorldNormal + Walnut::Random::InUnitSphere());
		}

		// Mix reflection and random direction based on roughness
		ray.Direction = glm::normalize(glm::mix(reflectDirection, randomDirection, material.Roughness));

		// Mix reflected color based on metallic value
		throughput *= glm::mix(glm::vec3(1.0f), material.Albedo, material.Metallic);

	}

	return glm::vec4(litColor, 1.0f);
}

Renderer::HitEvent Renderer::TraceRay(const Ray& ray)
{
	// No Sphere in scene
	if (m_CurrentScene->Spheres.size() == 0)
	{
		return Miss(ray);
	}

	uint32_t closestSphere = std::numeric_limits<uint32_t>::max();
	float closestDistance = std::numeric_limits<float>::max();

	// Loop through all the spheres in the scene
	for (size_t i = 0; i < m_CurrentScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_CurrentScene->Spheres[i];

		// Calculate translated ray origin (based on Sphere Origin)
		glm::vec3 origin = ray.Origin - sphere.Position;

		// Calculate the ray distance from the camera to the sphere
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction); 
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;

		// If the discriminant is negative, the ray does not intersect the sphere
		if (discriminant < 0.0f)
		{
			continue;
		}

		// Calculate the distance from the camera to the sphere
		float distance[2] = {(-b - sqrt(discriminant)) / (2.0f * a), (-b + sqrt(discriminant)) / (2.0f * a)};

		// Update the closest sphere
		if (distance[0] > 0.0f && distance[0] < closestDistance)
		{
			closestDistance = distance[0];
			closestSphere = i;
		}
	}

	// No sphere was hit
	if (closestSphere == std::numeric_limits<uint32_t>::max())
	{
		return Miss(ray);
	}

	return ClosestHit(ray, closestSphere, closestDistance);
}


Renderer::HitEvent Renderer::ClosestHit(const class Ray& ray, uint32_t objectIndex, float hitDistance)
{
	// Create a hit event
	Renderer::HitEvent hitEvent;
	hitEvent.Hit = true;
	hitEvent.HitObjectIndex = objectIndex;
	hitEvent.HitDistance = hitDistance;

	// Get the closest sphere
	const Sphere& closestSphere = m_CurrentScene->Spheres[objectIndex];

	// Calculate the intersection point
	glm::vec3 origin = ray.Origin - closestSphere.Position; // Translate to sphere space
	hitEvent.WorldPosition = origin + ray.Direction * hitDistance;
	hitEvent.WorldNormal = glm::normalize(hitEvent.WorldPosition);

	// Translate back to world space
	hitEvent.WorldPosition += closestSphere.Position;

	// Return the hit event
	return hitEvent;
}

Renderer::HitEvent Renderer::Miss(const class Ray& ray)
{
	Renderer::HitEvent hitEvent;
	hitEvent.Hit = false; // No hit (to be safe)
	hitEvent.HitDistance = -1.0f; // No hit distance
	return hitEvent;
}
