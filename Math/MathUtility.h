#pragma once
#include <glm/gtx/norm.hpp>

namespace Math
{
	static inline bool IsNearlyZero(float v, float eps = 1e-8f) { return std::abs(v) <= eps; }
	static inline bool IsNearlyZeroVec2(const glm::vec3& v, float eps = 1e-8f) { return glm::length2(v) <= eps; }
}