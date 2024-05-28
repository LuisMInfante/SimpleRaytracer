#pragma once

#include "Walnut/Image.h"

#include <memory>
#include <glm/glm.hpp>
#include <vector>

typedef struct Settings Settings;

class Renderer
{
public:
		Renderer() = default;

		void OnResize(uint32_t width, uint32_t height);
		void Render(const class Camera& camera, const class Scene& scene);
		void ChangeSphereColor(float colorR, float colorG, float colorB);
		void ChangeLightPosition(float lightPosX, float lightPosY, float lightPosZ);

		std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }
		void ResetFrameCount() { m_FrameCount = 1; }

		struct Settings
		{
			bool Accumulate = true;
		};
		Settings& GetSettings() { return m_Settings; }

private:
	struct HitEvent
	{
		bool Hit = false;
		uint32_t HitObjectIndex;
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;
	};

	glm::vec4 RayGen(uint32_t x, uint32_t y);
	HitEvent TraceRay(const class Ray& ray);
	HitEvent ClosestHit(const class Ray& ray, uint32_t objectIndex, float hitDistance);
	HitEvent Miss(const class Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationBuffer = nullptr;
	uint32_t m_FrameCount = 1;

	std::vector<uint32_t> m_HorizontalPixelIterator;
	std::vector<uint32_t> m_VerticalPixelIterator;

	Settings m_Settings;

	const Scene* m_CurrentScene = nullptr;
	const Camera* m_CurrentCamera = nullptr;

	float m_SphereR = 1.0f;
	float m_SphereG = 1.0f;
	float m_SphereB = 1.0f;
	glm::vec3 SphereColor = { m_SphereR, m_SphereG, m_SphereB };

	float m_LightPosX = -1.0f;
	float m_LightPosY = -1.0f;
	float m_LightPosZ = -1.0f;
	glm::vec3 LightPosition = { m_LightPosX, m_LightPosY, m_LightPosZ };
};