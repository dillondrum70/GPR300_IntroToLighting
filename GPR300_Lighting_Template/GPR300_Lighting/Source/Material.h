#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include "imgui.h"

struct Material
{
	glm::vec3 color = glm::vec3(1);
	float ambientK = .2f;
	float diffuseK = .5f;
	float specularK = .25f;
	float shininess = 2;

	void ExposeImGui()
	{
		float col[3] = { color.x, color.y, color.z };
		if (ImGui::ColorEdit3("Material Color", col))
		{
			color = glm::vec3(col[0], col[1], col[2]);
		}

		ImGui::SliderFloat("Material Ambient K", &ambientK, 0, 1);
		ImGui::SliderFloat("Material Diffuse K", &diffuseK, 0, 1);
		ImGui::SliderFloat("Material Specular K", &specularK, 0, 1);
		ImGui::SliderFloat("Material Shininess", &shininess, 1, 512);
	}
};

#endif
