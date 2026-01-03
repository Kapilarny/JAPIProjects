// Stolen from LearnOpenGL (https://learnopengl.com/Getting-started/Camera)

#pragma once

// #include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Config camera settings, with defaults.
static inline float YAW         = -90.0f;
static inline float PITCH       =  0.0f;
static inline float SPEED       =  2.5f;
static inline float SENSITIVITY =  0.1f;
static inline float ZOOM        =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f)) : Front(glm::vec3(0.0f, 0.0f, -1.0f))
    {
        Position = position;
        WorldUp = up;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ) : Front(glm::vec3(0.0f, 0.0f, -1.0f))
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ChangeX(int direction) {
        Position += Right * SPEED * (float)direction;
    }
    void ChangeY(int direction) {
        Position.z += SPEED * (float)direction;
    }
    void ChangeZ(int direction) {
        Position += Front * SPEED * (float)direction;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= SENSITIVITY;
        yoffset *= SENSITIVITY;

        YAW   += xoffset;
        PITCH += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped.
            float upper_bound = 89.0f;
            float lower_bound = -upper_bound;

            if (constrainPitch) {
                if (PITCH > upper_bound)
                    PITCH = upper_bound;
                if (PITCH < lower_bound)
                    PITCH = lower_bound;
            }

        // Update Front, Right and Up Vectors using the updated Euler angles.
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        ZOOM -= (float)yoffset;
        if (ZOOM < 1.0f)
            ZOOM = 1.0f;
        if (ZOOM > 45.0f)
            ZOOM = 45.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(YAW)) * cos(glm::radians(PITCH));
        front.z = sin(glm::radians(-PITCH));
        front.y = sin(glm::radians(YAW)) * cos(glm::radians(PITCH));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};