#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->jumping = false;
        this->height = 0.3f;
        this->gravity = 0.02f;
        this->speedJump = 0.0f;

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
    }

    glm::vec3 Camera::getCameraTarget()///
    {
        return cameraTarget;
    }


    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        glm::vec3 forward = glm::normalize(cameraTarget - cameraPosition);
        glm::vec3 right = glm::normalize(glm::cross(forward, cameraUpDirection));

        if (direction == MOVE_FORWARD) {
            cameraPosition += forward * speed;
            cameraTarget += forward * speed;
        }
        else if (direction == MOVE_BACKWARD) {
            cameraPosition -= forward * speed;
            cameraTarget -= forward * speed;
        }
        else if (direction == MOVE_LEFT) {
            cameraPosition -= right * speed;
            cameraTarget -= right * speed;
        }
        else if (direction == MOVE_RIGHT) {
            cameraPosition += right * speed;
            cameraTarget += right * speed;
        }
        if (direction == MOVE_UP && !jumping) {
            jumping = true;
            speedJump = height;  // Apply initial upward velocity
        }
        if (jumping) {
            cameraPosition.y += speedJump;
            cameraTarget.y += speedJump;

            speedJump -= gravity;

            if (cameraPosition.y <= 0.0f) {
                cameraPosition.y = 0.0f;
                cameraTarget.y = 0.0f;
                jumping = false;
                speedJump = 0.0f;
            }
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 direction = glm::normalize(cameraTarget - cameraPosition);

        // Compute the right vector using the cross product of direction and up vector
        glm::vec3 right = glm::normalize(glm::cross(direction, cameraUpDirection));

        // Apply the pitch and yaw rotations
        glm::mat4 rotation = glm::mat4(1.0f); // Identity matrix to start with
        rotation = glm::rotate(rotation, glm::radians(pitch), right); // Rotate around right vector (x-axis)
        rotation = glm::rotate(rotation, glm::radians(yaw), cameraUpDirection); // Rotate around up vector (y-axis)

        // Update camera position and target by applying the rotation
        cameraTarget = glm::vec3(rotation * glm::vec4(cameraTarget - cameraPosition, 0.0f)) + cameraPosition;
    }

}
