#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Layer.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) {}

	virtual void OnUpdate(float deltaTime) override
	{
		m_Camera.OnUpdate(deltaTime);
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		/* Color Picker For Sphere */
		ImGui::Separator();
		ImGui::Text("Color Picker");
		ImGui::Separator();

		if(ImGui::SliderFloat("Red", &m_colorR, 0.0f, 1.0f))
		{
			UpdateSphereColor();
		}
		if (ImGui::SliderFloat("Green", &m_colorG, 0.0f, 1.0f))
		{
			UpdateSphereColor();
		}
		if (ImGui::SliderFloat("Blue", &m_colorB, 0.0f, 1.0f))
		{
			UpdateSphereColor();
		}

		/* Light Source Movement */
		ImGui::Separator();
		ImGui::Text("Light Position");
		ImGui::Separator();

		if (ImGui::SliderFloat("Light X", &m_lightPosX, -1.0f, 1.0f))
		{
			UpdateLightPosition();
		}
		if(ImGui::SliderFloat("Light Y", &m_lightPosY, -1.0f, 1.0f))
		{
			UpdateLightPosition();
		}
		if (ImGui::SliderFloat("Light Z", &m_lightPosZ, -1.0f, 1.0f))
		{
			UpdateLightPosition();
		}


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
		m_Renderer.Render(m_Camera);

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