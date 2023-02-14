#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

#include "Material.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

Material defaultMat;

glm::vec3 bgColor = glm::vec3(0);

float lightRadius = 5.f;
float lightHeight = 5.f;

const int MAX_POINT_LIGHTS = 8;
PointLight pointLights[MAX_POINT_LIGHTS];
int pointLightCount = 0;

DirectionalLight directionalLight;
bool directionalEnabled = false;

SpotLight spotlight;
bool spotlightEnabled = true;

float constantAttenuation = 1.f;
float linearAttenuation = .35f;
float quadraticAttenuation = .44f;

bool manuallyMoveLights = false;	//If true, allows you to move point lights manually

bool phong = true;

bool wireFrame = false;

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;
	ew::Transform lightTransform;

	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	lightTransform.scale = glm::vec3(0.5f);
	lightTransform.position = glm::vec3(0.0f, 5.0f, 0.0f);

	//Light light;
	//light.transform.scale = glm::vec3(0.5f);
	//light.transform.position = glm::vec3(0.0f, 5.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());

		//Attenuation Uniforms
		litShader.setFloat("_Attenuation.constant", constantAttenuation);
		litShader.setFloat("_Attenuation.linear", linearAttenuation);
		litShader.setFloat("_Attenuation.quadratic", quadraticAttenuation);

		//Point Light Uniforms
		litShader.setInt("_UsedPointLights", pointLightCount);
		
		for (int i = 0; i < pointLightCount; i++)
		{
			if (!manuallyMoveLights)
			{
				pointLights[i].pos.x = lightRadius * (cos(2 * glm::pi<float>() * (i / (float)pointLightCount)));
				pointLights[i].pos.y = lightHeight;
				pointLights[i].pos.z = lightRadius * (sin(2 * glm::pi<float>() * (i / (float)pointLightCount)));
			}

			litShader.setVec3("_PointLight[" + std::to_string(i) + "].pos", pointLights[i].pos);
			litShader.setVec3("_PointLight[" + std::to_string(i) + "].color", pointLights[i].color);
			litShader.setFloat("_PointLight[" + std::to_string(i) + "].intensity", pointLights[i].intensity);
		}

		//Directional Light Uniforms
		litShader.setInt("_DirectionalEnabled", directionalEnabled);

		litShader.setVec3("_DirectionalLight.dir", directionalLight.dir);
		litShader.setVec3("_DirectionalLight.color", directionalLight.color);
		litShader.setFloat("_DirectionalLight.intensity", directionalLight.intensity);

		//Spotlight Uniforms
		litShader.setInt("_SpotlightEnabled", spotlightEnabled);

		litShader.setVec3("_Spotlight.pos", spotlight.pos);
		litShader.setVec3("_Spotlight.dir", spotlight.dir);
		litShader.setVec3("_Spotlight.color", spotlight.color);
		litShader.setFloat("_Spotlight.intensity", spotlight.intensity);
		litShader.setFloat("_Spotlight.range", spotlight.range);
		litShader.setFloat("_Spotlight.minAngle", glm::cos(glm::radians(spotlight.innerAngle)));
		litShader.setFloat("_Spotlight.maxAngle", glm::cos(glm::radians(spotlight.outerAngle)));
		litShader.setFloat("_Spotlight.falloff", spotlight.angleFalloff);

		//Material Uniforms
		litShader.setVec3("_Mat.color", defaultMat.color);
		litShader.setFloat("_Mat.ambientCoefficient", defaultMat.ambientK);
		litShader.setFloat("_Mat.diffuseCoefficient", defaultMat.diffuseK);
		litShader.setFloat("_Mat.specularCoefficient", defaultMat.specularK);
		litShader.setFloat("_Mat.shininess", defaultMat.shininess);

		litShader.setVec3("_CamPos", camera.getPosition());

		litShader.setInt("_Phong", phong);

		//Draw cube
		glm::mat4 cubeModel = cubeTransform.getModelMatrix();
		litShader.setMat4("_Model", cubeModel);
		litShader.setMat4("_NormalMatrix", glm::transpose(glm::inverse(cubeModel)));
		cubeMesh.draw();

		//Draw sphere
		glm::mat4 sphereModel = sphereTransform.getModelMatrix();
		litShader.setMat4("_Model", sphereModel);
		litShader.setMat4("_NormalMatrix", glm::transpose(glm::inverse(sphereModel)));
		sphereMesh.draw();

		//Draw cylinder
		glm::mat4 cylinderModel = cylinderTransform.getModelMatrix();
		litShader.setMat4("_Model", cylinderModel);
		litShader.setMat4("_NormalMatrix", glm::transpose(glm::inverse(cylinderModel)));
		cylinderMesh.draw();

		//Draw plane
		glm::mat4 planeModel = planeTransform.getModelMatrix();
		litShader.setMat4("_Model", planeModel);
		litShader.setMat4("_NormalMatrix", glm::transpose(glm::inverse(planeModel)));
		planeMesh.draw();

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());
		for (size_t i = 0; i < pointLightCount; i++)
		{
			unlitShader.setMat4("_Model", glm::translate(glm::mat4(1), pointLights[i].pos));
			unlitShader.setVec3("_Color", pointLights[i].color);
			sphereMesh.draw();
		}

		//Material
		defaultMat.ExposeImGui();

		//General Settings
		ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
		ImGui::Begin("Settings");

		ImGui::Checkbox("Phong Lighting", &phong);
		ImGui::Checkbox("Manually Move Point Lights", &manuallyMoveLights);

		ImGui::Text("GL Falloff Attenuation");
		ImGui::SliderFloat("Linear", &linearAttenuation, .0014f, 1.f);
		ImGui::SliderFloat("Quadratic", &quadraticAttenuation, .000007f, 2.0f);

		ImGui::End();

		//Point Lights
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);	//Size to fit content
		ImGui::Begin("Point Lights");

		ImGui::SliderInt("Light Count", &pointLightCount, 0, MAX_POINT_LIGHTS);
		ImGui::SliderFloat("Light Radius", &lightRadius, 0.f, 100.f);
		ImGui::SliderFloat("Light Height", &lightHeight, -5.f, 30.f);

		for (size_t i = 0; i < pointLightCount; i++)
		{
			ImGui::Text(("Point Light" + std::to_string(i)).c_str());

			ImGui::PushID(i);
			pointLights[i].ExposeImGui(manuallyMoveLights);
			ImGui::PopID();
		}

		ImGui::End();

		//Directional Light
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);	//Size to fit content
		ImGui::Begin("Directional Light");

		ImGui::Checkbox("Enabled", &directionalEnabled);
		directionalLight.ExposeImGui();

		ImGui::End();

		//Spotlight
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);	//Size to fit content
		ImGui::Begin("Spotlight");

		ImGui::Checkbox("Enabled", &spotlightEnabled);
		spotlight.ExposeImGui();

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}
