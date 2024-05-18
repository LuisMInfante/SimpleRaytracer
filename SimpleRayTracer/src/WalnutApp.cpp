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
		Sphere sphere;
		sphere.Position = { 0.0f, 0.0f, 0.0f };
		sphere.Radius = 0.5f;
		sphere.Albedo = { 1.0f, 0.0f, 0.0f };
		m_Scene.Spheres.push_back(sphere);

		Light light;
		light.Position = { -1.0f, -1.0f, -1.0f };
		m_Scene.Lights.push_back(light);
	}

	virtual void OnUpdate(float deltaTime) override
	{
		m_Camera.OnUpdate(deltaTime);
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

		/* Scene Controls */
		ImGui::Separator();
		ImGui::Text("Scene");
		ImGui::Separator();

		ImGui::DragFloat3("Sphere Position", glm::value_ptr(m_Scene.Spheres[0].Position), 0.1f, -1.0f, 1.0f);
		ImGui::DragFloat("Sphere Radius", &m_Scene.Spheres[0].Radius, 0.1f, 0.0f, 1.0f);

		/* Color Picker For Sphere */
		ImGui::ColorEdit3("Sphere Albedo", glm::value_ptr(m_Scene.Spheres[0].Albedo), 0.1f);

		/* Light Source Movement */
		ImGui::Separator();
		ImGui::Text("Light Position");
		ImGui::Separator();

		ImGui::DragFloat3("Light Source", glm::value_ptr(m_Scene.Lights[0].Position), 0.1f, -1.0f, 1.0f);

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