#ifndef CAMERA_H
#define CAMERA_H

#include "GLIncludes.h"
#include <glm/gtc/matrix_transform.hpp>

enum CameraDirection
{
	FORWORD,
	BACKWORD,
	LEFT,
	RIGHT
};
//default camera settings
const glm::vec3 CAMERA_POS(0.0f, 0.0f, 5.0f);
const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);
const float PITCH = 0.0f;
const float YAW = -89.0f;

const float MOVEMENT_SPEED = 4.0f;
const float MOUSE_SENSITIVITY = 0.2f;
const float FOV = 45.0f;
const float ASPECT_RATIO = 4 / 3;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 100.0f;
const float LEFT_PLANE = 0.0f;
const float RIGHT_PLANE = 1600.0f;
const float BOTTOM_PLANE = 0.0f;
const float TOP_PLANE = 1200.f;

namespace cgCourse
{
	class Camera
	{
	public:
	
		Camera() = default;

		void create(const glm::uvec2 & _extent,
					const glm::vec3 & _position,
					const glm::vec3 & _up,
					float pitch, float yaw
					);
		void create(const glm::uvec2 & _extent,
							const glm::vec3 & _position,
							const glm::vec3 & _origin,
							const glm::vec3 & _up
							);

		void setViewport(const glm::uvec2 & _extent);

		const glm::vec3 & getPosition() const;
		const glm::mat4 & getViewMatrix() const;
		const glm::mat4 & getProjectionMatrix() const;
		const glm::mat4 & getViewProjectionMatrix() const;
		void updateCameraVectors();
		void updateCameraData();
		void ProcessKeyboard(CameraDirection direction, float deltaTime);
		void ProcessMouseMovement(float x_offset, float y_offset);
		float MovementSpeed = 1.0f;

		float Fov;
		glm::uvec2 viewPortSize;
		float NearPlane;
		float FarPlane;
		float LeftPlane;
		float RightPlane;
		float TopPlane;
		float BottomPlane;
		float AspectRatio;
		float Pitch;
		float Yaw;
		float initialY;
		

	private:
		void computeViewProjectionMatrix();
		void computeProjectionMatrix(const glm::uvec2 & _extent);
		void computeViewMatrix();

		glm::vec3 position;
		glm::vec3 origin;
		glm::vec3 Up;
		glm::vec3 Front;
		glm::vec3 Right;
		glm::vec3 WorldUp;



		float MouseSensitivity = 0.2f;

		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 viewProjectionMatrix;
	};
}

#endif // CAMERA_H

