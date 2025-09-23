#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct CameraData
{
	glm::vec3 cameraPos;
	float pad1;
	glm::vec3 cameraForward;
	float pad2;
	glm::vec3 cameraRight;
	float pad3;
	glm::vec3 cameraUp; // in theory i don't need this cause i can calculate it for now ill leave it TODO remove
	float aspect; // Degrees
};

class Camera
{
public:
	Camera();
	~Camera();
	void CalculateMatricesFromInputs(GLFWwindow* window);
	void Setup(GLFWwindow* window);

	CameraData& getData() { return m_Data; }
private:
	void processKeyboard(GLFWwindow* window, float deltaTime);
	void processMouse(GLFWwindow* window);
	void updateVectors(GLFWwindow* window);
private:
	CameraData m_Data;
	float yaw;    // horizontal angle in degrees
	float pitch;  // vertical angle in degrees
	float sensitivity = 0.2f;
	float speed = 0.001f;

	bool firstMouse;
	double lastX, lastY;
};