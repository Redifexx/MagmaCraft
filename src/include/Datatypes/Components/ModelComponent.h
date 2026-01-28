#pragma once

#include <memory>
#include "Core/Model.h"

namespace Craft
{
	struct ModelComponent
	{
		std::unique_ptr<Magma::Model> model = nullptr;
	};
}