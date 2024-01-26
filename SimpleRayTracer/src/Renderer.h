#pragma once

#include "Walnut/Image.h"

#include "memory"
#include "glm/glm.hpp"

class Renderer
{
public:
		Renderer() = default;

		void OnResize(uint32_t width, uint32_t height);
		void Render(const class Camera& camera);
		void ChangeSphereColor(float colorR, float colorG, float colorB);
		void ChangeLightPosition(float lightPosX, float lightPosY, float lightPosZ);

		std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
	glm::vec4 TraceRay(const class Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	float m_SphereR = 1.0f;
	float m_SphereG = 1.0f;
	float m_SphereB = 1.0f;
	glm::vec3 SphereColor = { m_SphereR, m_SphereG, m_SphereB };

	float m_LightPosX = -1.0f;
	float m_LightPosY = -1.0f;
	float m_LightPosZ = -1.0f;
	glm::vec3 LightPosition = { m_LightPosX, m_LightPosY, m_LightPosZ };
};