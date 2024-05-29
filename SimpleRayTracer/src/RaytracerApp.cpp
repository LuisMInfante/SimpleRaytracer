#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Layer.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
		Material& ground = m_Scene.Materials.emplace_back();
		ground.Albedo = { 0.1f, 0.1f, 0.1f };
		ground.Roughness = 0.1f;

		Material& floatingSphere = m_Scene.Materials.emplace_back();
		floatingSphere.Albedo = { 0.2f, 0.55f, 0.6f };
		floatingSphere.Roughness = 0.0f;

		Material& EmissiveSphere = m_Scene.Materials.emplace_back();
		EmissiveSphere.Albedo = { 0.9f, 0.6f, 0.4f };
		EmissiveSphere.Roughness = 0.1f;
		EmissiveSphere.EmissionColor = { 0.9f, 0.6f, 0.4f };
		EmissiveSphere.EmissionStrength = 2.0f;

		Material& SideSphere = m_Scene.Materials.emplace_back();
		SideSphere.Albedo = { 0.8f, 0.3f, 0.2f };
		SideSphere.Roughness = 0.0f;

		Material& BackSphere = m_Scene.Materials.emplace_back();
		BackSphere.Albedo = { 0.2f, 0.8f, 0.1f };
		BackSphere.Roughness = 0.0f;

		Sphere FloorSphere;
		FloorSphere.Position = { 0.0f, -100.5f, 0.0f };
		FloorSphere.Radius = 100.0f;
		FloorSphere.MaterialIndex = 0;
		m_Scene.Spheres.push_back(FloorSphere); 

		Sphere sphere;
		sphere.Position = { 0.0f, 0.0f, 0.0f };
		sphere.Radius = 0.5f;
		sphere.MaterialIndex = 1;
		m_Scene.Spheres.push_back(sphere);

		Sphere sphere2;
		sphere2.Position = { 2.0f, 0.49f, 0.0f };
		sphere2.Radius = 1.0f;
		sphere2.MaterialIndex = 2;
		m_Scene.Spheres.push_back(sphere2);

		Sphere sphere3;
		sphere3.Position = { -2.0f, 0.49f, 1.0f };
		sphere3.Radius = 1.0f;
		sphere3.MaterialIndex = 3;
		m_Scene.Spheres.push_back(sphere3);

		Sphere sphere4;
		sphere4.Position = {-1.5f, 0.31f, -6.0f };
		sphere4.Radius = 1.0f;
		sphere4.MaterialIndex = 4;
		m_Scene.Spheres.push_back(sphere4);

		Light light;
		light.Position = { -1.0f, -1.0f, -1.0f };
		m_Scene.Lights.push_back(light);
	}

	virtual void OnUpdate(float deltaTime) override
	{
		if (m_Camera.OnUpdate(deltaTime))
		{
			m_Renderer.ResetFrameCount();
		}
	}
	virtual void OnUIRender() override
	{
		/* Settings */
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		/* Accumulation */
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameCount();
		}

		/* Scene Controls */
		ImGui::Separator();
		ImGui::Text("Scene Controls");
		ImGui::Separator();

		/* Object Controls */
		ImGui::Text("Objects");
		ImGui::Separator();
		for (size_t i = 1; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);
			/* Sphere Position and Radius */
			if(ImGui::DragFloat3("Position", glm::value_ptr(m_Scene.Spheres[i].Position), 0.1f)) { m_Renderer.ResetFrameCount(); }
			if(ImGui::DragFloat("Radius", &m_Scene.Spheres[i].Radius, 0.1f, 0.0f, 500.0f)) { m_Renderer.ResetFrameCount(); }

			/* Material Index */
			if(ImGui::DragInt("Material Select", &m_Scene.Spheres[i].MaterialIndex, 1.0f, 0.0f, (int)m_Scene.Materials.size() - 1)) { m_Renderer.ResetFrameCount(); }

			ImGui::Separator();
			ImGui::PopID();
		}

		/* Material Controls */
		ImGui::Text("Materials");
		ImGui::Separator();
		for (size_t i = 1; i < m_Scene.Materials.size(); i++)
		{
			ImGui::PushID(i);

			/* Material Properties */
			if (ImGui::ColorEdit3("Albedo", glm::value_ptr(m_Scene.Materials[i].Albedo), 0.1f)) { m_Renderer.ResetFrameCount(); }
			if(ImGui::DragFloat("Roughness", &m_Scene.Materials[i].Roughness, 0.001f, 0.0f, 1.0f)) { m_Renderer.ResetFrameCount(); }
			if(ImGui::DragFloat("Metallic", &m_Scene.Materials[i].Metallic, 0.001f, 0.0f, 1.0f)) { m_Renderer.ResetFrameCount(); }
			if(ImGui::ColorEdit3("Emission Color", glm::value_ptr(m_Scene.Materials[i].EmissionColor), 0.1f)) { m_Renderer.ResetFrameCount(); }
			if(ImGui::DragFloat("Emission Strength", &m_Scene.Materials[i].EmissionStrength, 0.05f, 0.0f, FLT_MAX)) { m_Renderer.ResetFrameCount(); }

			ImGui::Separator();
			ImGui::PopID();
		}

		/* Light Source Movement */
		/*
		ImGui::Separator();
		ImGui::Text("Light Position");
		ImGui::Separator();

		if(ImGui::DragFloat3("Light Source", glm::value_ptr(m_Scene.Lights[0].Position), 0.1f, -5.0f, 5.0f)) { m_Renderer.ResetFrameCount(); }
		*/

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
		{
			ImGui::Image(image->GetDescriptorSet(), 
						{ (float)image->GetWidth(), 
						(float)image->GetHeight() }, 
						ImVec2(0,1), 
						ImVec2(1,0)); // Flipping image vertically
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Camera, m_Scene);

		m_LastRenderTime = timer.ElapsedMillis();
	}

	void UpdateSphereColor() 
	{
		m_Renderer.ChangeSphereColor(m_colorR, m_colorG, m_colorB);
	}

	void UpdateLightPosition()
	{
		m_Renderer.ChangeLightPosition(m_lightPosX, m_lightPosY, m_lightPosZ);
	}


private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;

	// Color sliders
	float m_colorR = 1.0f;
	float m_colorG = 1.0f;
	float m_colorB = 1.0f;

	// Light source sliders
	float m_lightPosX = -1.0f;
	float m_lightPosY = -1.0f;
	float m_lightPosZ = -1.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Simple RayTracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}
				ImGui::EndMenu();
			}
		});
	return app;
}