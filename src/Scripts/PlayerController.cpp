#include "Scripts/PlayerController.h"
#include <Core/Input.h>
#include <Datatypes/Components/TransformComponent.h>
#include <Datatypes/Components/RelationshipComponent.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

using namespace Craft;

void PlayerController::OnAttach()
{
	// get camera entity from relationship component
	// assuming first child is camera (should be)
	auto& relRef = GetComponent<RelationshipComponent>();
	if (relRef.firstChild != NULL_ENTITY)
	{
		m_CameraEntity = relRef.firstChild;
	}
}

void PlayerController::OnUpdate(float dt)
{
	HandleMovement(dt);
	HandleMouseLook(dt);
}

void PlayerController::HandleMovement(float dt)
{
	auto& transform = GetComponent<TransformComponent>();

	glm::vec3 forward = transform.localRotation * glm::vec3(0.0f, 0.0f, -1.0f); // -z forward
	glm::vec3 right = transform.localRotation *  glm::vec3(1.0f, 0.0f, 0.0f);    // +x right
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);       // +y up

	// only using y rotation for movement
	glm::vec3 moveDir = glm::vec3(0.0f);

	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_W))
	{
		/*
		std::cout << "W HELD" << std::endl;
		std::cout << transform.localPosition.x << " "
				  << transform.localPosition.y << " "
				  << transform.localPosition.z << std::endl;
				  */
		moveDir += forward;
	}
	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_S)) moveDir -= forward;

	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_A)) moveDir -= right;
	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_D)) moveDir += right;

	// add flight mode check at some point
	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_E)) moveDir += up;
	if (Magma::Input::IsKeyHeld(SDL_SCANCODE_Q)) moveDir -= up;

	if (glm::length(moveDir) > 0.0f)
	{
		transform.localPosition += glm::normalize(moveDir) * m_MoveSpeed * dt;
		transform.isDirty = true;
	}
}

void PlayerController::HandleMouseLook(float dt)
{
	//if (!SDL_GetWindowRelativeMouseMode(m_Window)) return;

	float mouseX = Magma::Input::GetMouseDelta().x;
	float mouseY = Magma::Input::GetMouseDelta().y;

	if (mouseX == 0.0f && mouseY == 0.0f) return;

	m_Yaw -= mouseX * m_MouseSensitivity;
	m_Pitch -= mouseY * m_MouseSensitivity;
	m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

	auto& playerTransform = GetComponent<TransformComponent>();
	playerTransform.localRotation = glm::angleAxis(glm::radians(m_Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	playerTransform.isDirty = true;

	// pitch for cam
	if (m_CameraEntity != NULL_ENTITY)
	{
		auto& camTransform = world->GetComponent<TransformComponent>(m_CameraEntity);
		camTransform.localRotation = glm::angleAxis(glm::radians(m_Pitch), glm::vec3(1, 0, 0));
		camTransform.isDirty = true;
	}
}