#include "PointLight.h"

#include "imgui.h"

void PointLight::ExposeImGui()
{
	ImGui::DragFloat3("Light Position", &pos.x);

	ImGui::SliderFloat("Light Intensity", &intensity, 0, 1);

	ImGui::ColorEdit3("Light Color", &color.x);
}