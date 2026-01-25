#pragma once

#include <glm/glm.hpp>

// Holds the data representation of a Camera
// Just a big ball of data to be used outside of this class
namespace Magma
{
	class Camera
	{
		public:
			Camera(
				glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
				float yaw = -90.0f,
				float pitch = 0.0f
			);

			void UpdateCameraVectors();

			glm::mat4 GetViewMatrix() const;
			glm::mat4 GetProjectionMatrix() const;
			glm::mat4 GetViewProjectionMatrix() const;

			float GetFOV() const { return m_FOV; }
			float GetAspectRatio() const { return m_AspectRatio; }
			float GetNearPlane() const { return m_NearPlane; }
			float GetFarPlane() const { return m_FarPlane; }
			float GetOrthoSize() const { return m_OrthoSize; }
			float GetYaw() const { return m_Yaw; }
			float GetPitch() const { return m_Pitch; }

			glm::vec3 GetPosition() const { return m_Position; }
			glm::vec3 GetFront() const { return m_Front; }
			glm::vec3 GetUp() const { return m_Up; }
			glm::vec3 GetRight() const { return m_Right; }
			glm::vec3 GetWorldUp() const { return m_WorldUp; }

			bool GetPerspective() const { return m_IsPerspective; }

			void SetFOV(float fov) { m_FOV = fov; }
			void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
			void SetNearPlane(float nearPlane) { m_NearPlane = nearPlane; }	
			void SetFarPlane(float farPlane) { m_FarPlane = farPlane; }
			void SetOrthoSize(float orthoSize) { m_OrthoSize = orthoSize; }
			void SetYaw(float yaw) { m_Yaw = yaw; }
			void SetPitch(float pitch) { m_Pitch = pitch; }

			void SetPosition(const glm::vec3& position) { m_Position = position; }
			void SetPerspective(bool isPerspective) { m_IsPerspective = isPerspective; }

		private:
			float m_FOV;
			float m_AspectRatio;
			float m_NearPlane;
			float m_FarPlane;
			float m_OrthoSize;

			float m_Yaw;
			float m_Pitch;

			glm::vec3 m_Position;
			glm::vec3 m_Front;
			glm::vec3 m_Up;
			glm::vec3 m_Right;
			glm::vec3 m_WorldUp;

			bool m_IsPerspective;
	};
}