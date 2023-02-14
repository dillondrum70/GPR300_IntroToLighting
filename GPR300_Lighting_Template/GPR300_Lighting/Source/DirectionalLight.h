#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "glm/glm.hpp"

struct DirectionalLight
{
	float intensity = 1.f;

	glm::vec3 color = glm::vec3(1);

	glm::vec3 dir = glm::vec3(0, -1, 0);

	void ExposeImGui();
};

#endif
