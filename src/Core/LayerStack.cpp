#include "Core/LayerStack.h"

using namespace Magma;

LayerStack::~LayerStack()
{
	for (Layer* layer : m_Layers)
	{
		layer->OnDetach();
		delete layer;
	}
}

void LayerStack::PushLayer(Layer* layer)
{
	m_Layers.emplace_back(layer);
	layer->OnAttach();
}