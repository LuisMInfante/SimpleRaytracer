#pragma once

#include <vector>
#include <glm/glm.hpp>

typedef struct Sphere Sphere;
typedef struct Scene Scene;
typedef struct Light Light;
typedef struct Material Material;

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Light> Lights;
	std::vector<Material> Materials;
};

struct Material
{
	glm::vec3 Albedo{ 1.0f }; // Non-lit color (0 = darker, 1 = lighter)
	float Roughness = 1.0f; // Glossiness (0 = mirror, 1 = matte)
	float Metallic = 0.0f; // Reflectivity (0 = dielectric, 1 = metal)
};

struct Sphere
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	float Radius = 0.5f;

	int MaterialIndex;
};

struct Light
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
};
