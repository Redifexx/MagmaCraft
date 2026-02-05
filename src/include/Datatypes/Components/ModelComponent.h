#pragma once

#include <memory>
#include "Core/Model.h"

// Holds a model!
namespace Craft
{
	struct ModelComponent
	{
		Magma::Model* model = nullptr;
	};
}