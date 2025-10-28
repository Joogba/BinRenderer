#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Camera
{
	Camera::Camera() : Type(ECameraType::PERSPECTIVE)
	{
	}
	Camera::~Camera()
	{
	}
	glm::mat4x4 Camera::CreateView() const
	{
		return CameraUtil::CreateViewMatrix(pos, target, up);
	}
	glm::mat4x4 Camera::CreateProjection() const
	{
		if (isPerspectiveProjection)
		{
			if (isInfinityFar)
			{
				return CameraUtil::CreatePerspectiveMatrixFarAtInfinity(static_cast<float>(width), static_cast<float>(height), fovRad, near);
			}
			else
			{
				return CameraUtil::CreatePerspectiveMatrix(static_cast<float>(width), static_cast<float>(height), fovRad, near, far);
			}
		}
		else
		{
			return CameraUtil::CreateOrthogonalMatrix(static_cast<float>(width), static_cast<float>(height), near, far);
		}
	}
	void Camera::UpdateCameraFrustrum()
	{
		auto toTarget = glm::normalize(target - pos);
		const auto length = tanf(fovRad) * far;
		auto toRight = GetRightVector() * length;
		auto toUp = GetUpVector() * length;

		auto rightUp = glm::normalize(toTarget * far + toRight + toUp);
		auto leftUp = glm::normalize(toTarget * far - toRight + toUp);
		auto rightDown = glm::normalize(toTarget * far + toRight - toUp);
		auto leftDown = glm::normalize(toTarget * far - toRight - toUp);

		auto far_lt = pos + leftUp * far;
		auto far_rt = pos + rightUp * far;
		auto far_lb = pos + leftDown * far;
		auto far_rb = pos + rightDown * far;

		auto near_lt = pos + leftUp * near;
		auto near_rt = pos + rightUp * near;
		auto near_lb = pos + leftDown * near;
		auto near_rb = pos + rightDown * near;

		Frustum.Planes[0] = Math::Plane::CreateFrustumFromThreePoints(near_lb, far_lb, near_lt);		// left
		Frustum.Planes[1] = Math::Plane::CreateFrustumFromThreePoints(near_rt, far_rt, near_rb);		// right
		Frustum.Planes[2] = Math::Plane::CreateFrustumFromThreePoints(near_lt, far_lt, near_rt);		// top
		Frustum.Planes[3] = Math::Plane::CreateFrustumFromThreePoints(near_rb, far_rb, near_lb);		// bottom
		Frustum.Planes[4] = Math::Plane::CreateFrustumFromThreePoints(near_lb, near_lt, near_rb);	// near
		Frustum.Planes[5] = Math::Plane::CreateFrustumFromThreePoints(far_rb, far_rt, far_lb);		// far
	}

	void Camera::UpdateCamera()
	{
		preViewProjection = projection * view;

		view = CreateView();
		projection = CreateProjection();
		viewProjection = projection * view;
		inverseViewProjection = glm::inverse(viewProjection);

		UpdateCameraFrustrum();
		if (isPerspectiveProjection && isInfinityFar)
		{
			reverseZProjection = CameraUtil::CreateReverseZPerspectiveMatrix(static_cast<float>(width), static_cast<float>(height), fovRad, near, far);
		}
		else
		{
			reverseZProjection = glm::mat4(1.0f); // Identity matrix if not reverse Z perspective
		}
		
		UpdateCameraParameters();
	}

	void Camera::GetRectInNDCSpace(glm::vec3& outMin, glm::vec3& outMax, const glm::mat4& inViewPort) const
	{
		glm::vec3 far_lt, far_rt, far_lb, far_rb;
		glm::vec3 near_lt, near_rt, near_lb, near_rb;

		const auto origin = pos;
		const float n = near;
		const float f = far;

		const glm::vec3 forward = glm::normalize(GetForwardVector());
		const glm::vec3 rightN = glm::normalize(GetRightVector());
		const glm::vec3 upN = glm::normalize(GetUpVector());

		if (isPerspectiveProjection)
		{
			const float aspect = static_cast<float>(width) / static_cast<float>(height);
			const float t = tanf(fovRad * 0.5f);

			const glm::vec3 right = rightN * (t * aspect);
			const glm::vec3 up = upN * t;

			const glm::vec3 ru = forward + right + up;
			const glm::vec3 lu = forward - right + up;
			const glm::vec3 rd = forward + right - up;
			const glm::vec3 ld = forward - right - up;

			far_lt = origin + lu * f;  far_rt = origin + ru * f;
			far_lb = origin + ld * f;  far_rb = origin + rd * f;

			near_lt = origin + lu * n;  near_rt = origin + ru * n;
			near_lb = origin + ld * n;  near_rb = origin + rd * n;
		}
		else
		{
			// Width/Height in world units (not pixels)
			const float halfW = static_cast<float>(width) * 0.5f;
			const float halfH = static_cast<float>(height) * 0.5f;

			far_lt = origin + forward * f - rightN * halfW + upN * halfH;
			far_rt = origin + forward * f + rightN * halfW + upN * halfH;
			far_lb = origin + forward * f - rightN * halfW - upN * halfH;
			far_rb = origin + forward * f + rightN * halfW - upN * halfH;

			near_lt = origin + forward * n - rightN * halfW + upN * halfH;
			near_rt = origin + forward * n + rightN * halfW + upN * halfH;
			near_lb = origin + forward * n - rightN * halfW - upN * halfH;
			near_rb = origin + forward * n + rightN * halfW - upN * halfH;
		}

		{ // World -> NDC transform
			far_lt = Math::TransformPointNDC(inViewPort, far_lt);
			far_rt = Math::TransformPointNDC(inViewPort, far_rt);
			far_lb = Math::TransformPointNDC(inViewPort, far_lb);
			far_rb = Math::TransformPointNDC(inViewPort, far_rb);

			near_lt = Math::TransformPointNDC(inViewPort, near_lt);
			near_rt = Math::TransformPointNDC(inViewPort, near_rt);
			near_lb = Math::TransformPointNDC(inViewPort, near_lb);
			near_rb = Math::TransformPointNDC(inViewPort, near_rb);
		}

		outMin = glm::vec3(FLT_MAX);
		outMax = glm::vec3(-FLT_MAX);

		auto acc = [&](const glm::vec3& p) {
			outMin = glm::min(outMin, p);
			outMax = glm::max(outMax, p);
		};

		acc(far_lt);  acc(far_rt);  acc(far_lb);  acc(far_rb);
		acc(near_lt); acc(near_rt); acc(near_lb); acc(near_rb);

		// Optional: Clamp x,y to [-1,1] for screen clipping
		// OutPosMin = glm::clamp(OutPosMin, glm::vec3(-1.f), glm::vec3(1.f));
		// OutPosMax = glm::clamp(OutPosMax, glm::vec3(-1.f), glm::vec3(1.f));
	}
	void Camera::GetRectInScreenSpace(glm::vec3& outMin, glm::vec3& outMax, const glm::mat4& inViewPort, const glm::vec2& inScreenSize) const
	{
		GetRectInNDCSpace(outMin, outMax, inViewPort);

		outMin = glm::max(outMin, glm::vec3(-1.0f));
		outMax = glm::min(outMax, glm::vec3(1.0f));

		outMin.x = (outMin.x * 0.5f + 0.5f) * inScreenSize.x;
		outMin.y = (outMin.y * 0.5f + 0.5f) * inScreenSize.y;

		outMax.x = (outMax.x * 0.5f + 0.5f) * inScreenSize.x;
		outMax.y = (outMax.y * 0.5f + 0.5f) * inScreenSize.y;
	}
	void Camera::GetFrustumVertexInWorld(glm::vec3* outVertexArray) const
	{
		glm::vec3 far_lt, far_rt, far_lb, far_rb;
		glm::vec3 near_lt, near_rt, near_lb, near_rb;

		const auto origin = pos;
		const float n = near;
		const float f = far;

		const glm::vec3 forward = glm::normalize(GetForwardVector());
		const glm::vec3 rightN = glm::normalize(GetRightVector());
		const glm::vec3 upN = glm::normalize(GetUpVector());

		if (isPerspectiveProjection)
		{
			const float aspect = static_cast<float>(width) / static_cast<float>(height);
			const float t = tanf(fovRad * 0.5f);

			const glm::vec3 right = rightN * (t * aspect);
			const glm::vec3 up = upN * t;

			const glm::vec3 ru = forward + right + up;
			const glm::vec3 lu = forward - right + up;
			const glm::vec3 rd = forward + right - up;
			const glm::vec3 ld = forward - right - up;

			far_lt = origin + lu * f;  far_rt = origin + ru * f;
			far_lb = origin + ld * f;  far_rb = origin + rd * f;

			near_lt = origin + lu * n;  near_rt = origin + ru * n;
			near_lb = origin + ld * n;  near_rb = origin + rd * n;
		}
		else
		{
			// Width/Height in world units (not pixels)
			const float halfW = static_cast<float>(width) * 0.5f;
			const float halfH = static_cast<float>(height) * 0.5f;

			far_lt = origin + forward * f - rightN * halfW + upN * halfH;
			far_rt = origin + forward * f + rightN * halfW + upN * halfH;
			far_lb = origin + forward * f - rightN * halfW - upN * halfH;
			far_rb = origin + forward * f + rightN * halfW - upN * halfH;

			near_lt = origin + forward * n - rightN * halfW + upN * halfH;
			near_rt = origin + forward * n + rightN * halfW + upN * halfH;
			near_lb = origin + forward * n - rightN * halfW - upN * halfH;
			near_rb = origin + forward * n + rightN * halfW - upN * halfH;
		}

		outVertexArray[0] = far_lt;
		outVertexArray[1] = far_rt;
		outVertexArray[2] = far_lb;
		outVertexArray[3] = far_rb;

		outVertexArray[4] = near_lt;
		outVertexArray[5] = near_rt;
		outVertexArray[6] = near_lb;
		outVertexArray[7] = near_rb;
		
	}
}

namespace CameraUtil
{

	// rightHanded = true  -> OpenGL/RH lookAt style
	// rightHanded = false -> D3D-style LH lookAt
	glm::mat4 CreateViewMatrix(const glm::vec3& pos, const glm::vec3& target, glm::vec3& up, bool rightHanded)
	{
		glm::vec3 f = glm::normalize(target - pos);  // forward
		up = glm::normalize(up);

		// Adjust if f and up are nearly parallel
		if (glm::abs(glm::dot(f, up)) > 0.999f) {
			up = (glm::abs(f.y) < 0.999f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
		}

		glm::mat4 m(1.0f);

		if (rightHanded) {
			// RH: Camera view -Z forward
			const glm::vec3 s = glm::normalize(glm::cross(f, up)); // right
			const glm::vec3 u = glm::cross(s, f);           // true up

			m[0][0] = s.x;  m[1][0] = s.y;  m[2][0] = s.z;  m[3][0] = -glm::dot(s, pos);
			m[0][1] = u.x;  m[1][1] = u.y;  m[2][1] = u.z;  m[3][1] = -glm::dot(u, pos);
			m[0][2] = -f.x; m[1][2] = -f.y; m[2][2] = -f.z; m[3][2] = glm::dot(f, pos);
			// Last row is [0,0,0,1]
		}
		else {
			// LH: Camera view +Z forward
			const glm::vec3 s = glm::normalize(glm::cross(up, f)); // right
			const glm::vec3 u = glm::cross(f, s);      // true up

			m[0][0] = s.x;  m[1][0] = s.y;  m[2][0] = s.z;  m[3][0] = -glm::dot(s, pos);
			m[0][1] = u.x;  m[1][1] = u.y;  m[2][1] = u.z;  m[3][1] = -glm::dot(u, pos);
			m[0][2] = f.x;  m[1][2] = f.y;  m[2][2] = f.z;  m[3][2] = -glm::dot(f, pos);
		}
		return m;
	}

	glm::mat4 CreatePerspectiveMatrix(float width, float height, float fov, float nearDist, float farDist, bool rightHanded)
	{
		const float aspect = width / height;
		return rightHanded
			? glm::perspectiveRH_ZO(fov, aspect, nearDist, farDist)  // RH + ZO
			: glm::perspectiveLH_ZO(fov, aspect, nearDist, farDist); // LH + ZO
	}

	glm::mat4 CreatePerspectiveMatrixFarAtInfinity(float width, float height, float fov, float nearDist, bool rightHanded)
	{
		const float aspect = width / height;
		return rightHanded
			? glm::infinitePerspectiveRH_ZO(fov, aspect, nearDist) // RH + ZO
			: glm::infinitePerspectiveLH_ZO(fov, aspect, nearDist); // LH + ZO
	}

	glm::mat4 CreateReverseZPerspectiveMatrix(float width, float height, float fov, float nearDist, float farDist, bool rightHanded)
	{
		return glm::mat4();
	}

}