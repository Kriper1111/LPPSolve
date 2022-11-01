#include "camera.h"

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

void Camera::applyRotation() {
    glm::vec3 direction;
    // Yaw:
    direction.x = sin(glm::radians(this->mRotation.z)) * cos(glm::radians(this->mRotation.x));
    direction.y = cos(glm::radians(this->mRotation.z)) * cos(glm::radians(this->mRotation.x));
    // Pitch:
    // XXX: weirdly flips the image when passes through 180
    direction.z = sin(glm::radians(this->mRotation.x));
    this->mDirection = glm::normalize(direction);

    #ifdef ALLOW_CAMERA_MOVEMENT
    // Roll:
    // That we don't really need, but will use
    this->mUp.x = sin(glm::radians(this->mRotation.y));
    this->mUp.z = cos(glm::radians(this->mRotation.y));
    this->mUp = glm::normalize(this->mUp);
    #endif
}

void Camera::recalcProjection() {
    this->projectionMatrix = glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
}

Camera::Camera(int viewWidth, int viewHeight, float fov, float nearplane, float farplane) {
    fieldOfView = fov;
    aspectRatio = (float)viewWidth/(float)viewHeight;
    nearPlane = nearplane;
    farPlane = farplane;
    this->recalcProjection();
    this->mDirection = glm::vec3(0.0f);
    this->mLocation = glm::vec3(0.0, 0.0, 0.0f);
    this->mUp = glm::vec3(0.0f, 0.0f, 1.0f);
    this->rotate(0);
}

void Camera::rotate(float rotation) { rotate(rotation, rotation, rotation); }
void Camera::rotate(float pitch, float roll, float yaw) {
    this->mRotation = glm::vec3(pitch, roll, yaw);
    this->applyRotation();
}

void Camera::changeFOV(float FOV) {
    fieldOfView = FOV;
    this->recalcProjection();
}

void Camera::rotateBy(float rotation) { rotateBy(rotation, rotation, rotation); }
void Camera::rotateBy(float pitch, float roll, float yaw) {
    this->mRotation.x += pitch;
    this->mRotation.y += roll;
    this->mRotation.z += yaw;
    this->applyRotation();
}

void Camera::orbit(double deltaX, double deltaY) {
    glm::vec3 previousDirection(this->mDirection.x, this->mDirection.y, this->mDirection.z);
    // ...
    this->mRotation.z += deltaX / this->lookInSensitivity;
    this->mRotation.x += deltaY / this->lookInSensitivity;
    // ...
    applyRotation();
    this->mLocation -= (this->mDirection - previousDirection) * this->lookDepth;
}
void Camera::zoom(double deltaX, double deltaY) {
    double zoomAmount = glm::length(glm::vec2(deltaX, deltaY) / this->lookInSensitivity);
    this->lookDepth -= glm::sign(deltaY) * zoomAmount;
    this->walk(glm::sign(deltaY) * zoomAmount, 0, 0);
}
void Camera::pan(double deltaX, double deltaY) {
    this->walk(0, deltaX / this->lookInSensitivity, -deltaY / this->lookInSensitivity);
}

void Camera::teleportTo(float positionX, float positionY, float positionZ) {
    this->mLocation.x = positionX;
    this->mLocation.y = positionY;
    this->mLocation.z = positionZ;
}

void Camera::moveBy(float positionX, float positionY, float positionZ) {
    this->mLocation.x += positionX;
    this->mLocation.y += positionY;
    this->mLocation.z += positionZ;
}

void Camera::walk(float forwards, float sideways, float ascend) {
    glm::vec3 movement = this->mDirection * forwards;
    movement += glm::cross(this->mDirection, this->mUp) * sideways;
    movement.z += ascend;
    this->mLocation += movement;
}

#ifdef DEBUG
std::string glx_toString(glm::vec3 vector) { return glm::to_string(vector); }
std::string glx_toString(glm::mat4 matrix) { return glm::to_string(matrix); }
#else
std::string glx_toString(glm::vec3 vector) { return ""; }
std::string glx_toString(glm::mat4 matrix) { return ""; }
#endif

glm::vec3 Camera::getCameraLocation() { return this->mLocation; }
glm::vec3 Camera::getCameraRotation() { return this->mRotation; }
glm::vec3 Camera::getCameraDirection() { return this->mDirection; }

glm::mat4 Camera::getView() { return glm::lookAt(this->mLocation, this->mLocation + this->mDirection, this->mUp); }
glm::mat4 Camera::getProjection() { return this->projectionMatrix; }

float Camera::getFOV() { return this->fieldOfView; }