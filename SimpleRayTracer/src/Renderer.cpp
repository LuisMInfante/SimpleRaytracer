#include "Renderer.h"

#include "Walnut/Random.h"


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

			// Update the pixel at the coordinate 
			m_ImageData[x + y * m_FinalImage->GetWidth()] = UpdatePixel(coordinate);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::UpdatePixel(glm::vec2 coordinate)
{
	// Scale coordinates to RBGA values
	uint8_t red = (uint8_t)(coordinate.x * 255.0f);
	uint8_t green = (uint8_t)(coordinate.y * 255.0f);
	uint8_t blue = (uint8_t)(0.2f * 255.0f);

	float sphereRadius = 0.5f;

	// Set the camera position
	glm::vec3 CameraPosition(0.0f, 0.0f, 2.0f);

	// Calculate the ray direction from the camera (-1 for forward)
	glm::vec3 rayDirection(coordinate.x, coordinate.y, -1.0f);
	rayDirection = glm::normalize(rayDirection);

	// Calculate the ray distance from the camera to the sphere
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(CameraPosition, rayDirection);
	float c = glm::dot(CameraPosition, CameraPosition) - sphereRadius * sphereRadius;

	float discriminant = b * b - 4.0f * a * c;

	// If the discriminant is negative, the ray does not intersect the sphere
	if (discriminant < 0.0f)
	{
		return 0xff000000;
	}

	// Return the pixel color value
	return 0xff000000 | (blue << 16) | (green << 8) | red;
}
