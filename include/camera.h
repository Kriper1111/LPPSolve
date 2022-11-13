#include <glm/glm.hpp>
#include <string>

class Camera {
    private:
    glm::mat4 projectionMatrix;
    glm::vec3 mDirection;
    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mUp;

    bool isOrthographic = false;

    int viewHeight;
    int viewWidth;
    float fieldOfView;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    void applyRotation();
    void recalcProjection();
    void updateOrbitDepth();

    public:
    float lookInSensitivity = 15.0f;
    float orthographicScale = 1.0;
    float lookDepth = 25.0f;
    Camera(int viewWidth, int viewHeight, float fov = 45.5f, float nearplane = 0.01f, float farplane = 100.0f);
    void changeFOV(float newFOV);
    void setOrtography();
    void setPerspective();

    void setOrthographicScale(float scale); // getter because we need to recalc projection
    float getOrthographicScale();

    void useOrthography(bool useOrthography);
    bool useOrthography();

    void rotate(float rotation);
    void rotate(float pitch, float roll, float yaw);

    void rotateBy(float rotation);
    void rotateBy(float pitch, float roll, float yaw);

    void teleportTo(float positionX, float positionY, float positionZ);
    void moveBy(float positionX, float positionY, float positionZ);
    void walk(float forwards, float backwards, float ascend);

    void orbit(double deltaX, double deltaY);
    void zoom(double deltaX, double deltaY);
    void pan(double deltaX, double deltaY);

    glm::vec3 getCameraRotation();
    glm::vec3 getCameraLocation();
    glm::vec3 getCameraDirection();

    glm::mat4 getView();
    glm::mat4 getProjection();

    float getFOV();
};

std::string glx_toString(glm::vec3 vector);
std::string glx_toString(glm::mat4);