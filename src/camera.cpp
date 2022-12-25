#include "camera.h"

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

void Camera::applyRotation() {
    glm::vec3 direction;

    // replace with while?
    if (this->mRotation.z > 360) this->mRotation.z -= 360;
    if (this->mRotation.z < 0) this->mRotation.z += 360;

    // XXX: Might cause problems with number precision
    if (this->mRotation.x >  90) this->mRotation.x =  89.9;
    if (this->mRotation.x < -90) this->mRotation.x = -89.9;

    // Yaw:
    direction.x = sin(glm::radians(this->mRotation.z)) * cos(glm::radians(this->mRotation.x));
    direction.y = cos(glm::radians(this->mRotation.z)) * cos(glm::radians(this->mRotation.x));
    // Pitch:
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
    if (this->isOrthographic) {
        this->projectionMatrix = glm::ortho(
                    -aspectRatio * orthographicScale,
                    aspectRatio * orthographicScale,
                    -1.0f * orthographicScale,
                    1.0f * orthographicScale,
                    nearPlane, farPlane
            );
    }
    else {
        this->projectionMatrix = glm::perspective(glm::radians(fieldOfView), aspectRatio, nearPlane, farPlane);
    }
}

void Camera::updateOrbitDepth() {
    this->lookDepth = glm::length(this->mLocation);
}

Camera::Camera(int viewWidth, int viewHeight, float fov, float nearplane, float farplane) {
    this->viewWidth = viewWidth;
    this->viewHeight = viewHeight;
    this->fieldOfView = fov;
    this->aspectRatio = (float)viewWidth/(float)viewHeight;
    this->nearPlane = nearplane;
    this->farPlane = farplane;
    this->isOrthographic = false;
    this->recalcProjection();
    this->mDirection = glm::vec3(0.0f);
    this->mLocation = glm::vec3(0.0, 0.0, 0.0f);
    this->mUp = glm::vec3(0.0f, 0.0f, 1.0f);
    this->rotate(0);
}

void Camera::changeFOV(float FOV) {
    this->fieldOfView = FOV;
    this->recalcProjection();
}

void Camera::changeAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
    this->recalcProjection();
}

void Camera::changeAspectRatio(int viewWidth, int viewHeight) {
    this->viewWidth = viewWidth;
    this->viewHeight = viewHeight;
    this->changeAspectRatio((float)viewWidth / (float)viewHeight);
}

void Camera::changeClipPlanes(float nearplane, float farplane) {
    this->nearPlane = nearplane;
    this->farPlane = farplane;
    this->recalcProjection();
}

void Camera::setOrtography() { this->useOrthography(true); }
void Camera::setPerspective() { this->useOrthography(false); }

void Camera::useOrthography(bool useOrthography) {
    if (useOrthography != this->isOrthographic) {
        this->isOrthographic = useOrthography;
        this->recalcProjection();
    }
}

bool Camera::useOrthography() const { return this->isOrthographic; }

void Camera::setOrthographicScale(float scale) {
    this->orthographicScale = scale;
    if (this->isOrthographic) this->recalcProjection();
}

float Camera::getOrthographicScale() const { return this->orthographicScale; }

void Camera::rotate(float rotation) { rotate(rotation, rotation, rotation); }
void Camera::rotate(float pitch, float roll, float yaw) {
    this->mRotation = glm::vec3(pitch, roll, yaw);
    this->applyRotation();
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
    updateOrbitDepth();
}

void Camera::moveBy(float positionX, float positionY, float positionZ) {
    this->mLocation.x += positionX;
    this->mLocation.y += positionY;
    this->mLocation.z += positionZ;
    updateOrbitDepth();
}

void Camera::walk(float forwards, float sideways, float ascend) {
    glm::vec3 movement = this->mDirection * forwards;
    movement += glm::cross(this->mDirection, this->mUp) * sideways;
    movement.z += ascend;
    this->mLocation += movement;
}

void Camera::walk(float forwards, float sideways, float ascend, glm::vec3 axisMask) {
    glm::vec3 movement = this->mDirection * axisMask * forwards;
    movement += glm::cross(this->mDirection * axisMask, this->mUp) * sideways;
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

float Camera::getFOV() const { return this->fieldOfView; }
