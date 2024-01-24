#include "Renderer.h"

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

void Renderer::Render()
{
	/*
		* More efficient by rendering horizontally instead of vertically
		* by accessing contiguous memory
		* also by potential cache hits
	*/

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{ 
			// Calculate the coordinate of the pixel in the range [0, 1]
			glm::vec2 coordinate = { (float)x / (float)m_FinalImage->GetWidth() , (float)y / (float)m_FinalImage->GetHeight() };
			coordinate = coordinate * 2.0f - 1.0f; // Scale to [-1, 1]

			float aspectRatio = (float)m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();
			coordinate.x *= aspectRatio; // Scale the x coordinate by the aspect ratio

			// Update the pixel at the coordinate 
			glm::vec4 color = UpdatePixel(coordinate);
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

glm::vec4 Renderer::UpdatePixel(glm::vec2 coordinate)
{
	// Scale coordinates to RBGA values
	uint8_t red = (uint8_t)(coordinate.x * 255.0f);
	uint8_t green = (uint8_t)(coordinate.y * 255.0f);
	uint8_t blue = (uint8_t)(0.2f * 255.0f);
	uint8_t alpha = (uint8_t)(1.0f * 255.0f);
	uint32_t rgba = ((alpha << 24) | (blue << 16) | (green << 8) | red);

	// Define the sphere
	glm::vec3 sphereOrigin(0.0f, 0.0f, 0.0f);
	float sphereRadius = 0.5f;

	// Define the light
	glm::vec3 lightPosition = glm::normalize(glm::vec3( - 1.0f, -1.0f, -1.0f));

	// Set the camera position
	glm::vec3 CameraPosition(0.0f, 0.0f, 1.0f);

	// Calculate the ray direction from the camera (-1 for forward)
	glm::vec3 rayDirection(coordinate.x, coordinate.y, -1.0f);
	// rayDirection = glm::normalize(rayDirection);

	// Calculate the ray distance from the camera to the sphere
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(CameraPosition, rayDirection);
	float c = glm::dot(CameraPosition, CameraPosition) - sphereRadius * sphereRadius;

	float discriminant = b * b - 4.0f * a * c;

	// If the discriminant is negative, the ray does not intersect the sphere
	if (discriminant < 0.0f)
	{
		return glm::vec4(0, 0, 0, 1);
	}

	// Calculate the distance from the camera to the sphere
	float distance[2] = {(-b - sqrt(discriminant)) / (2.0f * a), (-b + sqrt(discriminant)) / (2.0f * a)};


	// Calculate the intersection point
	glm::vec3 hitPosition = CameraPosition + rayDirection * distance[0];
	glm::vec3 normal = glm::normalize(hitPosition - sphereOrigin); 
	float lightIntensity = glm::max(glm::dot(normal, -lightPosition), 0.0f); // Equiv to cos(theta)
	glm::vec3 litColor = SphereColor * lightIntensity;

	return glm::vec4(litColor, 1.0f);


}
