#pragma once

#include <vector>
#include <glm/glm.hpp>

typedef struct Sphere Sphere;
typedef struct Scene Scene;
typedef struct Light Light;

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Light> Lights;
};

struct Sphere
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	float Radius = 0.5f;

	glm::vec3 Albedo{ 1.0f }; // Non-lit color
};

struct Light
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
};
