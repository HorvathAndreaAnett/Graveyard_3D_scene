#include "Camera.hpp"

namespace gps {

	//Camera constructor
	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {

		Camera::cameraPosition = cameraPosition;
		Camera::cameraTarget = cameraTarget;
		Camera::cameraFrontDirection = glm::normalize(-Camera::cameraPosition + Camera::cameraTarget);
		Camera::cameraRightDirection = glm::normalize(glm::cross(cameraUp, Camera::cameraFrontDirection));
		Camera::cameraUpDirection = cameraUp;

	}

	//return the view matrix, using the glm::lookAt() function
	glm::mat4 Camera::getViewMatrix() {
		glm::mat4 view;
		view = glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, glm::vec3(0.0, 1.0, 0.0));

		return view;
	}

	//update the camera internal parameters following a camera move event
	void Camera::move(MOVE_DIRECTION direction, float speed) {

		speed = speed / 5.0f;
		
		if (direction == MOVE_FORWARD)
			cameraPosition += cameraFrontDirection * speed;
		if (direction == MOVE_BACKWARD)
			cameraPosition -= cameraFrontDirection * speed;
		if (direction == MOVE_LEFT)
			cameraPosition -= glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
		if (direction == MOVE_RIGHT)
			cameraPosition += glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
			
		
		
	}

	//update the camera internal parameters following a camera rotate event
	//yaw - camera rotation around the y axis
	//pitch - camera rotation around the x axis
	void Camera::rotate(float pitch, float yaw) {

		this->yawAngle += yaw;
		this->pitchAngle += pitch;

		cameraTarget.z = cameraPosition.z - cos(glm::radians(yawAngle));
		cameraTarget.x = cameraPosition.x + sin(glm::radians(yawAngle));
		cameraTarget.y = cameraPosition.y + sin(glm::radians(pitchAngle));

		this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
	}
}
