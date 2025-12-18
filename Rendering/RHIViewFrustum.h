#pragma once

#include <array>
#include <glm/glm.hpp>

namespace BinRenderer
{
	/**
	 * @brief 평면 (Plane)
	 */
	struct Plane
	{
		glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
		float distance = 0.0f;

		Plane() = default;
		Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
		Plane(const glm::vec3& normal, const glm::vec3& point);

		float distanceToPoint(const glm::vec3& point) const;
	};

	/**
	 * @brief Axis-Aligned Bounding Box
	 */
	struct AABB
	{
		glm::vec3 min = glm::vec3(0.0f);
		glm::vec3 max = glm::vec3(0.0f);

		AABB() = default;
		AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

		glm::vec3 getCenter() const { return (min + max) * 0.5f; }
		glm::vec3 getExtents() const { return (max - min) * 0.5f; }

		// Transform AABB by matrix
		AABB transform(const glm::mat4& matrix) const;
	};

	/**
	 * @brief 플랫폼 독립적 View Frustum (절두체)
	 */
	class RHIViewFrustum
	{
	public:
		enum PlaneIndex
		{
			Left = 0,
			Right = 1,
			Bottom = 2,
			Top = 3,
			Near = 4,
			Far = 5
		};

		RHIViewFrustum() = default;

		// ViewProjection 행렬에서 Frustum Plane 추출
		void extractFromViewProjection(const glm::mat4& viewProjection);

		// AABB가 Frustum 안에 있는지 또는 교차하는지 테스트
		bool intersects(const AABB& aabb) const;

		// 점이 Frustum 안에 있는지 테스트
		bool contains(const glm::vec3& point) const;

		// Plane 접근
		const Plane& getPlane(PlaneIndex index) const { return planes_[index]; }

	private:
		std::array<Plane, 6> planes_;
	};

} // namespace BinRenderer
