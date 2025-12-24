#pragma once

#include <iostream>
#include <string.h>	

namespace Magma
{
	class Client
	{
		public:
			Client();
			~Client();

			void Bind();
			bool SendServerMessage(std::string msg);
			bool GetBound() { return m_Bound; };

		private:
			bool m_Bound;
	};
}