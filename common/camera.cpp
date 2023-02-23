#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include <iostream>

#define LOG(x) std::cout << x << std::endl

using namespace glm;

Camera::Camera(GLFWwindow* window) : window(window)
{
    position = glm::vec3(0, 0, 5);
    horizontalAngle = 3.14f;
    verticalAngle = 0.0f;
    FoV = 45.0f;
    speed = 6.0f;
    mouseSpeed = 0.001f;
    fovSpeed = 2.0f;
}


void Camera::onMouseMove(double xPos, double yPos) {

    static double lastxPos = xPos;
    static double lastyPos = yPos;

    if (active) {
        horizontalAngle += mouseSpeed * (lastxPos - xPos);
        verticalAngle += mouseSpeed * (lastyPos - yPos);
    }
    lastxPos = xPos;
    lastyPos = yPos;
}

void Camera::update()
{

    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);


    // Task 5.4: right and up vectors of the camera coordinate system
    // use spherical coordinates
	vec3 direction(
		cos(verticalAngle)*sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle)*cos(horizontalAngle)
    );

    // Right vector
	vec3 right(
        sin(horizontalAngle - 3.14f/2.0f),
        0.0f,
        cos(horizontalAngle - 3.14f / 2.0f)
    );

    // Up vector
    vec3 up = cross(right, direction);

    // Task 5.5: update camera position using the direction/right vectors
    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position += up * deltaTime * speed;
        //std::cout << up.x << up.y << up.z << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position -= up * deltaTime * speed;
    }

    // Task 5.6: handle zoom in/out effects
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		FoV -= fovSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		FoV += fovSpeed;
	}

    // Task 5.7: construct projection and view matrices
	projectionMatrix = perspective(radians(FoV), 4.0f / 3.0f, 0.1f, 200.0f);
    viewMatrix = lookAt(
		position,
        position + direction,
		up
    );
    //*/

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}