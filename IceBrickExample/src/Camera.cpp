#include "Camera.hpp"
#include "GLFW/glfw3.h"

// TODO : Delete this when done with abstracting mouse events


#include <Core/InputManager.hpp>
using namespace IceBrick;


Camera::Camera(glm::vec3 position)
    : m_Position(position),
      m_Yaw(90.0f),
      m_Pitch(0.0f),
      m_MovementSpeed(50.0f),
      m_MouseSensitivity(0.1f),
      m_FirstMouse(true),
      m_LastX(0.0f),
      m_LastY(0.0f)
{
    UpdateCameraVectors();
}

void Camera::Update(float deltaTime, void* window)
{
    float velocity = m_MovementSpeed * deltaTime;

    if(InputManager::IsKeyPressed(Key::W)) m_Position += m_Front * velocity;
    if(InputManager::IsKeyPressed(Key::S)) m_Position -= m_Front * velocity;
    if(InputManager::IsKeyPressed(Key::A)) m_Position -= m_Right * velocity;
    if(InputManager::IsKeyPressed(Key::D)) m_Position += m_Right * velocity;

    if(InputManager::IsKeyPressed(Key::Space)) m_Position += m_WorldUp * velocity;
    if(InputManager::IsKeyPressed(Key::LeftShift)) m_Position -= m_WorldUp * velocity;

    double xposIn, yposIn;
    glfwGetCursorPos(static_cast<GLFWwindow*>(window),&xposIn, &yposIn);

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if(m_FirstMouse)
    {
        m_LastX = xpos;
        m_LastY = ypos;
        m_FirstMouse = false;
    }

    float xoffset = xpos - m_LastX;
    float yoffset = m_LastY - ypos;

    m_LastX = xpos;
    m_LastY = ypos;

    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if(m_Pitch > 89.0f) m_Pitch = 89.0f;
    if(m_Pitch < -89.0f) m_Pitch = -89.0f;

    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));


    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
