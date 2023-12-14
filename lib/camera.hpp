#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "gameObject.hpp"

#include <iostream>

class Camera {
protected:
    // Track the old mouse position
    glm::vec2 m_oldMousePosition;

public:
    Transform transform;
    float sensitivity = 0.001f;
    float fovDegrees = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 25.0f;
    bool fullRange = false;

    void MouseLook(vec2 newMousePosition) {
        // Little hack for our 'mouse look function'
        // We need this so that we can move our camera
        // for the first time.
        static bool firstLook=true;
        if(true == firstLook){
            firstLook=false;
            m_oldMousePosition = newMousePosition;
        }

        // Detect how much the mouse has moved since
        // the last time
        glm::vec2 delta = newMousePosition - m_oldMousePosition;
        delta *= sensitivity;
        if (fullRange && transform.IsUpsideDown()) {
            // This is necessary in case the camera is flipped upside-down (for full range motion)
            // In that case, the camera then needs to rotate clockwise when moving the mouse right
            delta.x *= -1;
        }
        transform.RotateY(-delta.x);
        transform.RotateUp(delta.y, fullRange);

        // Update our old position after we have made changes 
        m_oldMousePosition = newMousePosition;
    }
    void MouseLook(int mouseX, int mouseY){
        // Record our new position as a vector
        glm::vec2 newMousePosition(mouseX, mouseY);
        MouseLook(newMousePosition);
    }

    // Set the position for the camera
    void SetCameraEyePosition(float x, float y, float z){
        transform.position.x = x;
        transform.position.y = y;
        transform.position.z = z;
    }

    float GetEyeXPosition() const {
        return transform.position.x;
    }

    float GetEyeYPosition() const {
        return transform.position.y;
    }

    float GetEyeZPosition() const {
        return transform.position.z;
    }

    float GetViewXDirection() const {
        return transform.GetForwardVector().x;
    }

    float GetViewYDirection() const {
        return transform.GetForwardVector().y;
    }

    float GetViewZDirection() const {
        return transform.GetForwardVector().z;
    }

    Camera(){
        std::cout << "Camera.cpp: (Constructor) Created a Camera!\n";
        // Position us at the origin.
        transform = Transform(
            glm::vec3(0.0f,0.0f, 5.0f), 
            glm::vec3(0.0f,0.0f, -1.0f), // Looking down along the z-axis initially.
            glm::vec3(0.0f, 1.0f, 0.0f)  // For now--our upVector always points up along the y-axis
        );
    }

    glm::mat4 GetViewMatrix() const {
        // Think about the second argument and why that is
        // setup as it is.
        return glm::lookAt( 
            transform.position,
            transform.position + transform.GetForwardVector(),
            transform.GetUpVector()
        );
    }

    glm::mat4 GetProjectionMatrix(const vec2& screenSize) const {
        return glm::perspective(
            glm::radians(fovDegrees),
            screenSize.x / screenSize.y,
            nearPlane, farPlane 
        );
    }

    // Gets the combined View and Projection Matrix
    glm::mat4 GetVPMatrix(const vec2& screenSize) const {
        return GetProjectionMatrix(screenSize) * GetViewMatrix();
    }
};
