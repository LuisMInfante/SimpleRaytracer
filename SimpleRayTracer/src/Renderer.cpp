#include "Renderer.h"
#include "Camera.h"
#include "Ray.h"

#include "Walnut/Random.h"

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
}

void Renderer::Render(const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	/*
		* More efficient by rendering horizontally instead of vertically
		* by accessing contiguous memory
		* also by potential cache hits
	*/

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{ 
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];

			// Update the pixel at the coordinate 
			glm::vec4 color = TraceRay(ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f)); // Clamp the color to the range [0, 1]
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utility::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

void Renderer::ChangeSphereColor(float colorR, float colorG, float colorB)
{
	SphereColor = { colorR, colorG, colorB };
}

void Renderer::ChangeLightPosition(float lightPosX, float lightPosY, float lightPosZ)
{
	LightPosition = { lightPosX, lightPosY, lightPosZ };
}

glm::vec4 Renderer::TraceRay(const Ray& ray)
{
	// Define the sphere
	glm::vec3 sphereOrigin(0.0f, 0.0f, 0.0f);
	float sphereRadius = 0.5f;

	// Define the light
	glm::normalize(LightPosition);

	// Calculate the ray distance from the camera to the sphere
	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(ray.Origin, ray.Direction);
	float c = glm::dot(ray.Origin, ray.Origin) - sphereRadius * sphereRadius;

	float discriminant = b * b - 4.0f * a * c;

	// If the discriminant is negative, the ray does not intersect the sphere
	if (discriminant < 0.0f)
	{
		return glm::vec4(0, 0, 0, 1);
	}

	// Calculate the distance from the camera to the sphere
	float distance[2] = {(-b - sqrt(discriminant)) / (2.0f * a), (-b + sqrt(discriminant)) / (2.0f * a)};

	// Calculate the intersection point
	glm::vec3 hitPosition = ray.Origin + ray.Direction * distance[0];
	glm::vec3 normal = glm::normalize(hitPosition - sphereOrigin); 
	float lightIntensity = glm::max(glm::dot(normal, -LightPosition), 0.0f); // Equiv to cos(theta)
	glm::vec3 litColor = SphereColor * lightIntensity;

	return glm::vec4(litColor, 1.0f);
}
