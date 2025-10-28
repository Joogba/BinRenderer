#pragma once
#include <glm/glm.hpp>

namespace Math
{
	static inline glm::vec3 TransformPointNDC(const glm::mat4& M, const glm::vec3& p)
	{
		glm::vec4 q = M * glm::vec4(p, 1.0f);
		if (q.w != 0.0f) q /= q.w;            // 퍼스펙티브 디바이드
		return glm::vec3(q);
	}
}