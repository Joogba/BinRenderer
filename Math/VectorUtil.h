#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>   
#include <glm/gtx/norm.hpp>        
#include <cmath>

namespace Math
{
	static inline glm::vec3 DirectionFromEuler_PitchYaw(const glm::vec3& euler)
	{
        const float cp = std::cos(euler.x);
        const float sp = std::sin(euler.x);
        const float cy = std::cos(euler.y);
        const float sy = std::sin(euler.y);

        // yaw=0, pitch=0 -> (0,0,1)
        return glm::vec3(cp * sy, sp, cp * cy);
	}

    static inline float WrapRad(float x)
    {
        x = std::fmod(x + glm::pi<float>(), glm::two_pi<float>());
        if (x < 0.0f) x += glm::two_pi<float>();
        return x - glm::pi<float>();
    }

        static inline bool IsNearlyEqualEuler(const glm::vec3 & a, const glm::vec3 & b, float eps = 1e-5f)
        {
            glm::vec3 d = a - b;
            d.x = WrapRad(d.x);
            d.y = WrapRad(d.y);
            d.z = WrapRad(d.z);
            return glm::all(glm::lessThanEqual(glm::abs(d), glm::vec3(eps)));
        }

        static inline glm::vec3 GetEulerAngleFromDirection(const glm::vec3& direction)
        {
            glm::vec3 euler(0.0f);
            euler.x = std::asin(direction.y); // Pitch
            euler.y = std::atan2(direction.x, direction.z); // Yaw
            euler.z = 0.0f; // Roll is not calculated here
            return euler;
		}
}