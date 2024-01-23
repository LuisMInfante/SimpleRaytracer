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

			// Update the pixel at the coordinate 
			m_ImageData[x + y * m_FinalImage->GetWidth()] = UpdatePixel(coordinate);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::UpdatePixel(glm::vec2 coordinate)
{
	// Scale coordinates to RBGA values
	uint8_t r = (uint8_t)(coordinate.x * 255.0f);
	uint8_t g = (uint8_t)(coordinate.y * 255.0f);
	uint8_t b = (uint8_t)(0.2f * 255.0f);

	// Return the pixel color value
	return 0xff000000 | (b << 16) | (g << 8) | r;
}
