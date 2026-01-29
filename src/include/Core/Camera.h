#pragma once

#include <glm/glm.hpp>

// Holds the data representation of a Camera
// Just a big ball of data to be used outside of this class
// refactored and slimed down for Craft's ECS
namespace Magma
{
	class Camera
	{
		public:
			Camera() { UpdateProjectionMatrix(); }

			void UpdateProjectionMatrix();

			const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
			const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
			glm::mat4 GetViewProjectionMatrix() const { return m_ViewMatrix * m_ProjectionMatrix; }

			float GetFOV() const { return m_FOV; }
			float GetAspectRatio() const { return m_AspectRatio; }
			float GetNearPlane() const { return m_NearPlane; }
			float GetFarPlane() const { return m_FarPlane; }
			float GetOrthoSize() const { return m_OrthoSize; }
			bool GetPerspective() const { return m_IsPerspective; }

			void SetFOV(float fov) { m_FOV = fov; UpdateProjectionMatrix(); }
			void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; UpdateProjectionMatrix(); }
			void SetNearPlane(float nearPlane) { m_NearPlane = nearPlane; UpdateProjectionMatrix(); }
			void SetFarPlane(float farPlane) { m_FarPlane = farPlane; UpdateProjectionMatrix(); }
			void SetOrthoSize(float orthoSize) { m_OrthoSize = orthoSize; UpdateProjectionMatrix(); }
			void SetPerspective(bool isPerspective) { m_IsPerspective = isPerspective; UpdateProjectionMatrix(); }

			void SetViewMatrix(const glm::mat4& viewMatrix) { m_ViewMatrix = viewMatrix; }
			void SetProjectionMatrix(const glm::mat4& projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

		private:
			float m_FOV = 90.0f;
			float m_AspectRatio = 16.0f / 9.0f;
			float m_NearPlane = 0.1f;
			float m_FarPlane = 500.0f;
			float m_OrthoSize = 10.0f;

			glm::mat4 m_ViewMatrix{ 1.0f };
			glm::mat4 m_ProjectionMatrix{ 1.0f };

			bool m_IsPerspective = true;
	};
}