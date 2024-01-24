#pragma once

#include "Walnut/Image.h"

#include "memory"
#include "glm/glm.hpp"

class Renderer
{
public:
		Renderer() = default;

		void OnResize(uint32_t width, uint32_t height);
		void Render();
		void ChangeSphereColor(float colorR, float colorG, float colorB);

		std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
	glm::vec4 UpdatePixel(glm::vec2 coordinate);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	float SphereR = 1.0f;
	float SphereG = 1.0f;
	float SphereB = 1.0f;
	glm::vec3 SphereColor = { SphereR, SphereG, SphereB };
};