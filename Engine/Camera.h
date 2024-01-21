/*
 * Basic camera class
 *
 * Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */
#pragma once

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Core/IntrusivePtr.h>
#include <RHI/Buffer.h>
#include <RHI/MutableBuffer.h>
#include <RHI/RHIRuntime.h>

class Camera : public IntrusiveCounter<Camera>
{
private:
	// uniform Buffer for camera matrix
	IntrusivePtr<MutableBuffer> uniformBuffer;
	IntrusivePtr<RHIRuntime> rhiRuntime;
	glm::vec2 mousePos = {};

	float fov;
	float aspect;
	float zNear, zFar;
	float yaw = 90.0f;
	float pitch = 0.0f;

	void updateViewMatrix()
	{
		glm::vec3 direction = {};
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction = glm::normalize(direction);

		ubo.viewMatrix = glm::lookAtLH(position,
									   position + direction,
									   glm::vec3(0.0f, 1.0f, 0.0f));
	};

public:
	struct UBO
	{
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
	};

	UBO ubo;

	IntrusivePtr<MutableBuffer> GetUBOBuffer()
	{
		return this->uniformBuffer;
	}

	Camera(IntrusivePtr<RHIRuntime> rhiRuntime);
	void Allocate();

	bool EventCallback(UpdateInput inputs);

	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();
	glm::vec3 center = glm::vec3();

	float rotationSpeed = 1.0f;
	float movementSpeed = 1.0f;

	bool updated = false;
	bool flipY = false;

	struct
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
	} keys;

	bool moving()
	{
		return keys.left || keys.right || keys.up || keys.down;
	}

	void setPerspective(float fov, float aspect, float zNear, float zFar)
	{
		this->fov = fov;
		this->aspect = aspect;
		this->zNear = zNear;
		this->zFar = zFar;
		ubo.projectionMatrix = glm::perspective(glm::radians(fov), aspect, zFar, zNear);
	};

	void updatePerspective()
	{
		ubo.projectionMatrix = glm::perspective(glm::radians(fov), aspect, zFar, zNear);
	}

	void setPosition(glm::vec3 position)
	{
		this->position = position;
		updateViewMatrix();
	}

	void setRotation(glm::vec3 rotation)
	{
		this->rotation = rotation;
		updateViewMatrix();
	}

	// rotate around x and y
	// align with view coord, x to right, y up, z front
	void rotate(glm::vec3 delta)
	{
		yaw -= delta.x;
		pitch += delta.y;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		updateViewMatrix();
	}

	void setTranslation(glm::vec3 translation)
	{
		this->position = translation;
		updateViewMatrix();
	};

	void translate(glm::vec3 delta)
	{
		this->position += delta;
		updateViewMatrix();
	}

	void setRotationSpeed(float rotationSpeed)
	{
		this->rotationSpeed = rotationSpeed;
	}

	void setMovementSpeed(float movementSpeed)
	{
		this->movementSpeed = movementSpeed;
	}

	void update(float deltaTime)
	{
		updated = false;
		if (moving())
		{
			glm::vec3 camFront;
			camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
			camFront.y = sin(glm::radians(rotation.x));
			camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
			camFront = glm::normalize(camFront);

			float moveSpeed = deltaTime * movementSpeed;
			if (keys.up)
				position += camFront * moveSpeed;
			if (keys.down)
				position -= camFront * moveSpeed;
			if (keys.left)
				position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
			if (keys.right)
				position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

			updateViewMatrix();
		}
	};
};