#include "Camera.h"

#include <iostream>
#include <cmath>

namespace cgCourse
{
	void Camera::create(const glm::uvec2 & _extent,
						const glm::vec3 & _position,
						const glm::vec3 & _up,
						float _pitch, float _yaw)
	{
		position = _position;
		viewPortSize = _extent;
		Up = _up;
		Pitch = _pitch;
		Yaw = _yaw;
		MovementSpeed = MOVEMENT_SPEED;
		MouseSensitivity = MOUSE_SENSITIVITY;
		Fov = FOV; 
		WorldUp = WORLD_UP;


		AspectRatio = float(_extent.x)/_extent.y;
		NearPlane =NEAR_PLANE;
		FarPlane=FAR_PLANE;
		LeftPlane=LEFT_PLANE;
		RightPlane=RIGHT_PLANE;
		TopPlane=TOP_PLANE;
		BottomPlane=BOTTOM_PLANE;
		initialY = _position.y;

		computeViewMatrix();
		updateCameraVectors();
		setViewport(_extent);
	}

	void Camera::create(const glm::uvec2 & _extent,
						const glm::vec3 & _position,
						const glm::vec3 & _origin,
						const glm::vec3 & _up
						)
	{
		position = _position;
		viewPortSize = _extent;
		origin = _origin;
		Up = _up;
		WorldUp = WORLD_UP;

		// 计算front向量
		glm::vec3 front = glm::normalize(_origin - _position);

		// 计算yaw和pitch
		Yaw = glm::degrees(glm::atan(front.z, front.x)) - 90.0f;
		Pitch = glm::degrees(glm::asin(front.y));

		MovementSpeed = MOVEMENT_SPEED;
		MouseSensitivity = MOUSE_SENSITIVITY;
		Fov = FOV; 

		AspectRatio = float(_extent.x)/_extent.y;
		NearPlane =NEAR_PLANE;
		FarPlane=FAR_PLANE;
		LeftPlane=LEFT_PLANE;
		RightPlane=RIGHT_PLANE;
		TopPlane=TOP_PLANE;
		BottomPlane=BOTTOM_PLANE;

		computeViewMatrix();
		updateCameraVectors();
		setViewport(_extent);
	}


	void Camera::updateCameraData(){
		computeViewMatrix();
		updateCameraVectors();
		setViewport(viewPortSize);
	}

	void Camera::setViewport(const glm::uvec2 & _extent)
	{
		viewPortSize = _extent;
		glViewport(0, 0, int(viewPortSize.x), int(viewPortSize.y));
		computeProjectionMatrix(viewPortSize);
		computeViewProjectionMatrix();
	}

	void Camera::computeViewProjectionMatrix()
	{
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}

	void Camera::computeViewMatrix()
	{
		viewMatrix = glm::lookAt(position, position + Front, WorldUp);
	}

	void Camera::updateCameraVectors(){
		// Front = glm::normalize(origin - position);
		// Right = glm::normalize(glm::cross(Front, up));

		glm::vec3 front;
		front.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
		front.y = sin(glm::radians(Pitch));
		front.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));

		Front = glm::normalize(front);
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
		// std::cout<<"Front:"<<Front.x<<", "<<Front.y<<", "<<Front.z<<","<<std::endl;

	}

	
	void Camera::computeProjectionMatrix(const glm::uvec2 & _extent)
	{
		projectionMatrix = glm::perspective(glm::radians(Fov), AspectRatio, NearPlane, FarPlane);
		// std::cout<<projectionMatrix[0][0]<<","<<projectionMatrix[0][1]<<","<<projectionMatrix[0][2]<<","<<projectionMatrix[0][3]<<std::endl;
		// std::cout<<projectionMatrix[1][0]<<","<<projectionMatrix[1][1]<<","<<projectionMatrix[1][2]<<","<<projectionMatrix[1][3]<<std::endl;
		// std::cout<<projectionMatrix[2][0]<<","<<projectionMatrix[2][1]<<","<<projectionMatrix[2][2]<<","<<projectionMatrix[2][3]<<std::endl;
		// std::cout<<projectionMatrix[3][0]<<","<<projectionMatrix[3][1]<<","<<projectionMatrix[3][2]<<","<<projectionMatrix[3][3]<<std::endl;
	}

	const glm::vec3 & Camera::getPosition() const
	{
		return position;
	}

	const glm::mat4 & Camera::getViewMatrix() const
	{
		return viewMatrix;
	}

	const glm::mat4 & Camera::getProjectionMatrix() const
	{
		return projectionMatrix;
	}

	const glm::mat4 & Camera::getViewProjectionMatrix() const
	{
		return viewProjectionMatrix;
	}

	void Camera::ProcessKeyboard(CameraDirection direction, float deltaTime) {
		float velocity = MovementSpeed * deltaTime;
		
		if (direction == FORWORD){
			position += velocity * Front;
		}
		if (direction == BACKWORD)
			position -= velocity * Front;
		if (direction == LEFT){
			position -= velocity * Right;
		}
		if (direction == RIGHT){
			position += velocity * Right;
		}
		position.y = initialY;
		computeViewMatrix();
		setViewport(viewPortSize);
	}

	void Camera::ProcessMouseMovement(float x_offset, float y_offset) {

		x_offset *= MouseSensitivity;
		y_offset *= MouseSensitivity;

		Yaw += x_offset;
		Pitch += y_offset;

		if (Pitch > 89.0f)
			Pitch = 89.0f;
		else if (Pitch < -89.0f)
			Pitch = -89.0f;
		updateCameraVectors();
	}
}

