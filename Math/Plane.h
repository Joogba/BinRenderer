#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>    
#include <glm/gtx/norm.hpp>       

namespace Math
{
    struct Plane
    {
        glm::vec3 n = glm::vec3(0.0f); // Normal vector
        float d = 0.0f;                // Distance (Ax + By + Cz + D = 0)

        Plane() = default;

        Plane(float nx, float ny, float nz, float distance)
            : n(nx, ny, nz), d(distance)
        {
        }

        Plane(const glm::vec3& normal, float distance)
            : n(glm::normalize(normal)), d(distance)
        {
        }

        // Construct a plane from 3 points (counter-clockwise order recommended)
        static Plane CreateFrustumFromThreePoints(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
        {
            glm::vec3 direction0 = p1 - p0;
            glm::vec3 direction1 = p2 - p0;
            glm::vec3 normal = glm::normalize(glm::cross(direction0, direction1));
            // Distance d: choose either of these, depending on sign convention
            // (Ax + By + Cz + D = 0) <=> dot(n, p) + d = 0
            float distance = -glm::dot(normal, p0);   
            return Plane(normal, distance);
        }

        // Dot product with normal (ignores d)
        float DotNormal(const glm::vec3& v) const
        {
            return glm::dot(n, v);
        }

        // Dot product with point (평면 방정식 결과)
        float DotCoord(const glm::vec3& p) const
        {
            return glm::dot(n, p) + d;
        }

        // 점이 평면 위에 있는지(부동소수점 오차 고려)
        bool IsOnPlane(const glm::vec3& p, float epsilon = 1e-5f) const
        {
            return glm::abs(DotCoord(p)) < epsilon;
        }
    };
}