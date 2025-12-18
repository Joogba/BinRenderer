#include "RHICamera.h"
#include <cmath>

namespace BinRenderer
{
	using namespace glm;

	RHICamera::RHICamera()
		: fov_(45.0f), znear_(0.01f), zfar_(1000.0f), type_(CameraType::LookAt)
	{
	}

	void RHICamera::updateViewMatrix()
	{
		glm::mat4 currentMatrix = matrices_.view;

		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;

		rotM = glm::rotate(rotM, glm::radians(rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation_.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation_.z), glm::vec3(0.0f, 0.0f, 1.0f));

		transM = glm::translate(glm::mat4(1.0f), position_);

		if (type_ == CameraType::FirstPerson)
		{
			matrices_.view = rotM * transM;
		}
		else
		{
			matrices_.view = transM * rotM;
		}

		viewPos_ = glm::vec4(position_, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

		if (matrices_.view != currentMatrix)
		{
			updated_ = true;
		}
	}

	bool RHICamera::isMoving() const
	{
		return keys_.left || keys_.right || keys_.forward || keys_.backward || keys_.up || keys_.down;
	}

	void RHICamera::setType(CameraType type)
	{
		type_ = type;
	}

	void RHICamera::setPerspective(float fov, float aspect, float znear, float zfar)
	{
		glm::mat4 currentMatrix = matrices_.perspective;
		fov_ = fov;
		znear_ = znear;
		zfar_ = zfar;
		matrices_.perspective = glm::perspectiveRH_ZO(glm::radians(fov), aspect, znear, zfar);
		matrices_.perspective[1][1] *= -1.0f; // Flip Y for Vulkan

		if (matrices_.perspective != currentMatrix)
		{
			updated_ = true;
		}
	}

	void RHICamera::setPosition(const glm::vec3& position)
	{
		position_ = position;
		updateViewMatrix();
	}

	void RHICamera::setRotation(const glm::vec3& rotation)
	{
		rotation_ = rotation;
		updateViewMatrix();
	}

	void RHICamera::setViewPos(const glm::vec3& viewPos)
	{
		viewPos_ = viewPos;
		updateViewMatrix();
	}

	void RHICamera::rotate(const glm::vec3& delta)
	{
		rotation_ += delta;
		updateViewMatrix();
	}

	void RHICamera::setTranslation(const glm::vec3& translation)
	{
		position_ = translation;
		updateViewMatrix();
	}

	void RHICamera::translate(const glm::vec3& delta)
	{
		position_ += delta;
		updateViewMatrix();
	}

	void RHICamera::update(float deltaTime)
	{
		const vec3 upDirection = normalize(vec3(0.0f, 1.0f, 0.0f));

		updated_ = false;
		if (type_ == CameraType::FirstPerson)
		{
			if (isMoving())
			{
				glm::vec3 camFront;
				camFront.x = -cos(glm::radians(rotation_.x)) * sin(glm::radians(rotation_.y));
				camFront.y = sin(glm::radians(rotation_.x));
				camFront.z = cos(glm::radians(rotation_.x)) * cos(glm::radians(rotation_.y));
				camFront = glm::normalize(camFront);

				float moveSpeed = deltaTime * movementSpeed_;

				if (keys_.forward)
					position_ += camFront * moveSpeed;
				if (keys_.backward)
					position_ -= camFront * moveSpeed;
				if (keys_.left)
					position_ -= glm::normalize(glm::cross(camFront, upDirection)) * moveSpeed;
				if (keys_.right)
					position_ += glm::normalize(glm::cross(camFront, upDirection)) * moveSpeed;
				if (keys_.up)
					position_ += upDirection * moveSpeed;
				if (keys_.down)
					position_ -= upDirection * moveSpeed;

				updateViewMatrix();
			}
		}
	}

} // namespace BinRenderer
