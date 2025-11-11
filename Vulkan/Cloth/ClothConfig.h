#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace BinRenderer::Vulkan {

	// GPU 데이터 구조: 옷감 파티클 정보
	struct ClothParticle
	{
		glm::vec4 position;   // xyz: 위치, w: 질량의 역수 (1/mass, 고정점은 0)
		glm::vec4 velocity;   // xyz: 속도, w: padding
		glm::vec4 normal;     // xyz: 노말, w: padding
	};

	// GPU 데이터 구조: 스프링 제약 조건
	struct ClothConstraint
	{
		uint32_t particleA;   // 첫 번째 파티클 인덱스
		uint32_t particleB;   // 두 번째 파티클 인덱스
		float restLength;     // 원래 길이
		float stiffness;   // 강성 (0.0 ~ 1.0)
	};

	// GPU Uniform Buffer: 시뮬레이션 파라미터
	struct ClothSimParams
	{
		glm::vec4 gravity;          // xyz: 중력, w: padding
		glm::vec4 wind;  // xyz: 바람 방향, w: 바람 강도
		float deltaTime;      // 시간 간격
		float damping;              // 감쇠 계수 (0.95 ~ 0.99)
		uint32_t constraintIters;   // 제약 조건 반복 횟수
		uint32_t particleCount;     // 전체 파티클 개수
		float friction;             // 마찰 계수
		float padding[3];// 16바이트 정렬
	};

	// 옷감 설정 클래스 (Builder 패턴)
	class ClothConfig
	{
	public:
		ClothConfig()
			: gridWidth_(32), gridHeight_(32), spacing_(0.1f), gravity_(0.0f, -9.81f, 0.0f),
			damping_(0.98f), constraintIterations_(3), stiffness_(0.8f),
			mass_(1.0f), friction_(0.1f), pinnedCorners_(true)
		{
		}

		// Builder 패턴
		ClothConfig& setGridSize(uint32_t width, uint32_t height)
		{
			gridWidth_ = width;
			gridHeight_ = height;
			return *this;
		}

		ClothConfig& setSpacing(float spacing)
		{
			spacing_ = spacing;
			return *this;
		}

		ClothConfig& setGravity(const glm::vec3& gravity)
		{
			gravity_ = gravity;
			return *this;
		}

		ClothConfig& setDamping(float damping)
		{
			damping_ = damping;
			return *this;
		}

		ClothConfig& setConstraintIterations(uint32_t iterations)
		{
			constraintIterations_ = iterations;
			return *this;
		}

		ClothConfig& setStiffness(float stiffness)
		{
			stiffness_ = stiffness;
			return *this;
		}

		ClothConfig& setMass(float mass)
		{
			mass_ = mass;
			return *this;
		}

		ClothConfig& setFriction(float friction)
		{
			friction_ = friction;
			return *this;
		}

		ClothConfig& setPinnedCorners(bool pinned)
		{
			pinnedCorners_ = pinned;
			return *this;
		}

		ClothConfig& setWindSpeed(float speed)
		{
			windSpeed_ = speed;
			return *this;
		}

		// Getters
		uint32_t getGridWidth() const { return gridWidth_; }
		uint32_t getGridHeight() const { return gridHeight_; }
		float getSpacing() const { return spacing_; }
		const glm::vec3& getGravity() const { return gravity_; }
		float getDamping() const { return damping_; }
		uint32_t getConstraintIterations() const { return constraintIterations_; }
		float getStiffness() const { return stiffness_; }
		float getMass() const { return mass_; }
		float getFriction() const { return friction_; }
		bool isPinnedCorners() const { return pinnedCorners_; }
		float getWindSpeed() const { return windSpeed_; }

		uint32_t getParticleCount() const { return gridWidth_ * gridHeight_; }

	private:
		uint32_t gridWidth_;
		uint32_t gridHeight_;
		float spacing_;
		glm::vec3 gravity_;
		float damping_;
		uint32_t constraintIterations_;
		float stiffness_;
		float mass_;
		float friction_;
		bool pinnedCorners_;
		float windSpeed_{ 0.5f };
	};

} // namespace BinRenderer::Vulkan
