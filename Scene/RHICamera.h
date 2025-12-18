#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace BinRenderer
{
	/**
	 * @brief 플랫폼 독립적 카메라 클래스
	 */
	class RHICamera
	{
	public:
		enum class CameraType
		{
			LookAt,
			FirstPerson
		};

		RHICamera();
		~RHICamera() = default;

		// 카메라 설정
		void setPerspective(float fov, float aspect, float znear, float zfar);
		void setPosition(const glm::vec3& position);
		void setRotation(const glm::vec3& rotation);
		void setViewPos(const glm::vec3& viewPos);
		void setType(CameraType type);

		// 이동 및 회전
		void rotate(const glm::vec3& delta);
		void translate(const glm::vec3& delta);
		void setTranslation(const glm::vec3& translation);

		// 속도 설정
		void setRotationSpeed(float speed) { rotationSpeed_ = speed; }
		void setMovementSpeed(float speed) { movementSpeed_ = speed; }

		// 업데이트
		void update(float deltaTime);
		void updateViewMatrix();

		// 상태 쿼리
		bool isMoving() const;
		float getNearClip() const { return znear_; }
		float getFarClip() const { return zfar_; }
		CameraType getType() const { return type_; }

		// 행렬 접근
		const glm::mat4& getViewMatrix() const { return matrices_.view; }
		const glm::mat4& getProjectionMatrix() const { return matrices_.perspective; }
		glm::mat4 getViewProjectionMatrix() const { return matrices_.perspective * matrices_.view; }

		// 위치/회전 접근
		const glm::vec3& getPosition() const { return position_; }
		const glm::vec3& getRotation() const { return rotation_; }
		const glm::vec3& getViewPos() const { return viewPos_; }

		// 키 입력 (간단한 FPS 카메라)
		struct KeyState
		{
			bool left = false;
			bool right = false;
			bool forward = false;
			bool backward = false;
			bool up = false;
			bool down = false;
		};
		KeyState& getKeys() { return keys_; }

		bool isUpdated() const { return updated_; }
		void setUpdated(bool updated) { updated_ = updated; }

	private:
		// 카메라 파라미터
		float fov_ = 60.0f;
		float znear_ = 0.1f;
		float zfar_ = 1000.0f;
		CameraType type_ = CameraType::FirstPerson;

		// Transform
		glm::vec3 rotation_ = glm::vec3(-1.888507f, -0.764950f, -0.725987f);
		glm::vec3 position_ = glm::vec3(6.0f, -62.0f, 0.0f);
		glm::vec3 viewPos_ = glm::vec3(1.888507f, -0.764950f, 0.725987f);

		// 이동 속도
		float rotationSpeed_ = 0.1f;
		float movementSpeed_ = 10.0f;

		// 행렬
		struct
		{
			glm::mat4 perspective = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);
		} matrices_;

		// 키 입력 상태
		KeyState keys_;

		bool updated_ = true;
	};

} // namespace BinRenderer
