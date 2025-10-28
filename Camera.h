#pragma once

#ifndef FORCEINLINE
#define FORCEINLINE inline
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>
#include <map>

#include "Math/Plane.h"
#include "Math/VectorUtil.h"
#include "Math/MathUtility.h"
#include "Math/MatrixUtil.h"



namespace Camera
{
	struct FrustumPlane
	{
		bool IsInFrustum(const glm::vec3& pos, float radius) const
		{
			for (const auto& plane : Planes)
			{
				const float r = glm::dot(pos, plane.n) - plane.d + radius;
				if (r < 0.0f)
					return false;
			}
			return true;
		}

		bool IsInFrustumWithDirection(const glm::vec3& pos, const glm::vec3& direction, float radius) const
		{
			for (const auto& plane : Planes)
			{
				const float r = glm::dot(pos, plane.n) - plane.d + radius;
				if (r < 0.0f)
				{
					if (glm::dot(direction, plane.n) <= 0.0)
						return false;
				}
			}
			return true;
		}

		std::array<Math::Plane, 6> Planes;
	};

	class Camera
	{
	public:
		enum class ECameraType
		{
			PERSPECTIVE = 0,
			ORTHOGONAL,
			REVERSE_Z_PERSPECTIVE
		};

		static std::map<int32_t, Camera*> CameraMap;
		// Consider changing to force inline
		FORCEINLINE static void AddCamera(int32_t id, Camera* camera)
		{
			CameraMap[id] = camera;
		}
		FORCEINLINE static Camera* GetCamera(int32_t id)
		{
			auto it = CameraMap.find(id);
			if (it != CameraMap.end())
				return it->second;
			return nullptr;
		}
		FORCEINLINE static void RemoveCamera(int32_t id)
		{
			CameraMap.erase(id);
		}
		FORCEINLINE static Camera* GetMainCamera()
		{
			if (CameraMap.empty())
				return nullptr;
			return GetCamera(0); // First camera is considered main camera
		}
		FORCEINLINE static Camera* CreateCamera(int32_t id, ECameraType type = ECameraType::PERSPECTIVE)
		{
			Camera* camera = new Camera();
			//SetCamera
			return camera;
		}
		FORCEINLINE static void GetForwardRightUpFromEulerAngle(glm::vec3& outForward, glm::vec3& outRight, glm::vec3& outUp, const glm::vec3& inEulerAngle)
		{
			outForward = glm::normalize(Math::DirectionFromEuler_PitchYaw(inEulerAngle));

			const bool isInvert = (inEulerAngle.x < 0.0f || glm::pi<float>() < inEulerAngle.x);

			const glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
			const glm::vec3 WorldFwd = glm::vec3(0.0f, 0.0f, 1.0f);
			const glm::vec3 UpForCross = isInvert ? -WorldUp : WorldUp;
			const glm::vec3 FwdForCross = isInvert ? -WorldFwd : WorldFwd;

			outRight = glm::normalize(glm::cross(UpForCross, outForward));
			if (Math::IsNearlyZeroVec2(outRight))
				outRight = glm::normalize(glm::cross(FwdForCross, outForward));

			outUp = glm::normalize(glm::cross(outForward, outRight));
		}
		FORCEINLINE static void SetCamera(Camera* OutCamera, const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up,
			float fovRad, float nearDist, float farDist, float width, float height, bool isPerspective = true, float distance = 300.0f)
		{
			const auto toTarget = (target - pos);
			OutCamera->pos = pos;
			OutCamera->target = target;
			OutCamera->up = up;
			OutCamera->distance = distance;
			OutCamera->SetEulerAngle(Math::GetEulerAngleFromDirection(toTarget));

			OutCamera->fovRad = fovRad;
			OutCamera->near = nearDist;
			OutCamera->far = farDist;
			OutCamera->width = static_cast<int32_t>(width);
			OutCamera->height = static_cast<int32_t>(height);
			OutCamera->isPerspectiveProjection = isPerspective;
			// Create View and Projection matrix
			/*OutCamera->ViewMatrix = CameraUtil::CreateViewMatrix(pos, target, up);
			switch (OutCamera->Type)
			{
				case ECameraType::PERSPECTIVE:
					OutCamera->ProjectionMatrix = CameraUtil::CreatePerspectiveMatrix(width, height, fov, nearDist, farDist);
					break;
				case ECameraType::ORTHOGONAL:
					OutCamera->ProjectionMatrix = CameraUtil::CreateOrthogonalMatrix(width, height, nearDist, farDist);
					break;
				case ECameraType::REVERSE_Z_PERSPECTIVE:
					OutCamera->ProjectionMatrix = CameraUtil::CreateReverseZPerspectiveMatrix(width, height, fov, nearDist, farDist);
					break;
			}*/
		}

		Camera();
		virtual ~Camera();

		virtual glm::mat4x4 CreateView() const;
		virtual glm::mat4x4 CreateProjection() const;

		//virtual void BindCamera(const Shader* shader) const;

		void UpdateCameraFrustrum();
		void UpdateCamera();

		FORCEINLINE void UpdateCameraParameters()
		{
			glm::vec3 forwardDir;
			glm::vec3 rightDir;
			glm::vec3 upDir;
			GetForwardRightUpFromEulerAngle(forwardDir, rightDir, upDir, eulerAngle);
			target = pos + forwardDir * distance;
			up = pos + upDir;
		}

		FORCEINLINE void SetEulerAngle(const glm::vec3& InEulerAngle)
		{
			if(!Math::IsNearlyEqualEuler(eulerAngle, InEulerAngle, 1e-4f))
			{
				eulerAngle = InEulerAngle;
				UpdateCameraParameters();
			}
		}

		FORCEINLINE glm::vec3 GetForwardVector() const
		{
			return glm::vec3(glm::column(view, 2));
		}
		FORCEINLINE glm::vec3 GetUpVector() const
		{
			return glm::vec3(glm::column(view, 1));
		}
		FORCEINLINE glm::vec3 GetRightVector() const
		{
			return glm::vec3(glm::column(view, 0));
		}
		FORCEINLINE void MoveShift(float dist)
		{
			auto toRight = GetRightVector() * dist;
			pos += toRight;
			target += toRight;
			up += toRight;	
		}
		FORCEINLINE void MoveForward(float dist)
		{
			auto toForward = GetForwardVector() * dist;
			pos += toForward;
			target += toForward;
			up += toForward;
		}
		FORCEINLINE void RotateCameraAxis(const glm::vec3& axis, float radian)
		{
			const glm::mat4 T_pos = glm::translate(glm::mat4(1.0f), pos);   // == MakeTranslate(Pos)
			const glm::mat4 T_npos = glm::translate(glm::mat4(1.0f), -pos);   // == MakeTranslate(-Pos)
			const glm::mat4 R = glm::rotate(glm::mat4(1.0f), radian, glm::normalize(axis));

			const glm::mat4 M = T_pos * R * T_npos; // (column-vector convention)


			pos = glm::vec3(M * glm::vec4(pos, 1.0f));
			target = glm::vec3(M * glm::vec4(target, 0.0f));
			up = glm::vec3(M * glm::vec4(up, 0.0f));   // Direction vector uses w=0 for rotation only
		}
		FORCEINLINE void RotateForwardAxis(float radian)
		{
			RotateCameraAxis(GetForwardVector(), radian);
		}

		FORCEINLINE void RotateUpAxis(float radian)
		{
			RotateCameraAxis(GetUpVector(), radian);
		}

		FORCEINLINE void RotateRightAxis(float radian)
		{
			RotateCameraAxis(GetRightVector(), radian);
		}

		FORCEINLINE void RotateXAxis(float radian)
		{
			RotateCameraAxis(glm::vec3(1.0f, 0.0f, 0.0f), radian);
		}
		FORCEINLINE void RotateYAxis(float radian)
		{
			RotateCameraAxis(glm::vec3(0.0f, 1.0f, 0.0f), radian);
		}

		FORCEINLINE void RotateZAxis(float radian)
		{
			RotateCameraAxis(glm::vec3(0.0f, 0.0f, 1.0f), radian);
		}
		FORCEINLINE glm::mat4 GetViewProjectionMatrix() const
		{
			return viewProjection;
		}

		FORCEINLINE glm::mat4 GetInverseViewProjectionMatrix() const
		{
			return inverseViewProjection;
		}
		FORCEINLINE bool IsInFrustum(const glm::vec3& pos, float radius) const
		{
			for(auto& i : Frustum.Planes)
			{
				const float d = glm::dot(pos, i.n) - i.d + radius;
				if (d < 0.0f)
					return false;
			}
			return true;
		}
		FORCEINLINE bool IsInFrustumWithDirection(const glm::vec3& pos,const glm::vec3& dir, float radius) const
		{
			for (auto& i : Frustum.Planes)
			{
				const float d = glm::dot(pos, i.n) - i.d + radius;
				if (d < 0.0f)
				{
					if(glm::dot(dir, i.n) <= 0.0f) // When direction is same or opposite to plane normal
						return false; // Camera is behind the plane
				}
			}
			return true;
		}

		FORCEINLINE glm::vec3 GetEulerAngle() const
		{
			return eulerAngle;
		}

		void GetRectInNDCSpace(glm::vec3& outMin, glm::vec3& outMax, const glm::mat4& inViewPort) const;
		void GetRectInScreenSpace(glm::vec3& outMin, glm::vec3& outMax, const glm::mat4& inViewPort, const glm::vec2& inScreenSize = glm::vec2(1.0f,1.0f)) const;
		void GetFrustumVertexInWorld(glm::vec3* outVertexArray) const;

		//Variables
		ECameraType Type = ECameraType::PERSPECTIVE;

		glm::vec3 pos;
		glm::vec3 target;
		glm::vec3 up;

		glm::vec3 eulerAngle = glm::vec3(0.0f, 0.0f, 0.0f); // Pitch, Yaw, Roll
		float distance = 300.0f; // Distance between camera and target

		glm::mat4x4 view;
		glm::mat4x4 projection;
		glm::mat4x4 viewProjection;
		glm::mat4x4 inverseViewProjection;
		glm::mat4x4 preViewProjection;
		glm::mat4x4 reverseZProjection;
		bool isPerspectiveProjection = true;
		bool isInfinityFar = false;

		float fovRad = 0.0f;
		float near = 0.0f;
		float far = 0.0f;

		FrustumPlane Frustum;
		int32_t width = 0;
		int32_t height = 0;

	};
} // namespace Camera

namespace CameraUtil
{
	glm::mat4 CreateViewMatrix(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up, bool rightHanded = false);
	glm::mat4 CreatePerspectiveMatrix(float width, float height, float fov, float nearDist, float farDist, bool rightHanded = false);
	glm::mat4 CreatePerspectiveMatrixFarAtInfinity(float width, float height, float fov, float nearDist, bool rightHanded = false);
	glm::mat4 CreateReverseZPerspectiveMatrix(float width, float height, float fov, float nearDist, float farDist, bool rightHanded = false);
	glm::mat4 CreateOrthogonalMatrix(float width, float height, float nearDist, float farDist);
	glm::mat4 CreateOrthogonalMatrix(float left, float right, float top, float bottom, float nearDist, float farDist);
}