#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{


}

Camera::~Camera()
{

}

void Camera::Setup(GLFWwindow* window)
{
    m_Data =
    {
        .cameraPos = glm::vec3(-15.0f,0.0f,32.0f),
        .cameraForward = glm::vec3(0.0f,0.0f,1.0f),
        .cameraRight = glm::vec3(1.0f,0.0f,0.0f),
        .cameraUp = glm::vec3(0.0f,1.0f,0.0f),
        .aspect = 0.0f
    };

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    lastX = xpos;
    lastY = ypos;
    firstMouse = true;
    yaw = -90.0f;
    pitch = 0.0f;
    updateVectors(window);
}

void Camera::processKeyboard(GLFWwindow* window, float deltaTime) {
    float velocity = speed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_Data.cameraPos += m_Data.cameraForward * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_Data.cameraPos -= m_Data.cameraForward * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_Data.cameraPos -= m_Data.cameraRight * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_Data.cameraPos += m_Data.cameraRight * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) m_Data.cameraPos += m_Data.cameraUp * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) m_Data.cameraPos -= m_Data.cameraUp * velocity;
}

void Camera::processMouse(GLFWwindow* window) {

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
    {
        firstMouse = true; // reset on release to avoid jump
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = float(xpos - lastX);
    float yoffset = float(lastY - ypos); // reversed: y ranges bottom->top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void Camera::updateVectors(GLFWwindow* window) {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    m_Data.cameraForward = glm::normalize(f);

    // Recompute right and up vectors
    m_Data.cameraRight = glm::normalize(glm::cross(m_Data.cameraForward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_Data.cameraUp = glm::normalize(glm::cross(m_Data.cameraRight, m_Data.cameraForward));

    int Width, Height;
    glfwGetWindowSize(window, &Width, &Height);
    m_Data.aspect = static_cast<float>(Width) / static_cast<float>(Height);
}

void Camera::CalculateMatricesFromInputs(GLFWwindow* window)
{
    static double lastTime = glfwGetTime();

    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
	processKeyboard(window, deltaTime);
	processMouse(window);
	updateVectors(window);

}