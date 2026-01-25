#pragma once

#include <string>


// Layers are used to control render order
// and allow for modular systems
// very cool stuff :)
namespace Magma
{
	class Layer
	{
		public:
			Layer(const std::string& name = "Layer") : m_DebugName(name) {}
			virtual ~Layer() = default;

			virtual void OnAttach() {}
			virtual void OnDetach() {}
			virtual void OnUpdate(float dt) {}
			virtual void OnImGuiRender() {}

		protected:
			std::string m_DebugName;
	};
}