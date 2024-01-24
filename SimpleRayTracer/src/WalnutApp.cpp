#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Layer.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::SliderFloat("Red", &colorR, 0.0f, 1.0f);
		ImGui::SliderFloat("Green", &colorG, 0.0f, 1.0f);
		ImGui::SliderFloat("Blue", &colorB, 0.0f, 1.0f);

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
		UpdateSphereColor();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}

	void UpdateSphereColor() 
	{
		m_Renderer.ChangeSphereColor(colorR, colorG, colorB);
	}


private:
	Renderer m_Renderer;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;

	float colorR = 1.0f; 
	float colorG = 1.0f; 
	float colorB = 1.0f; 

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