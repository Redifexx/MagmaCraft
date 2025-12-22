#pragma once

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <WS2tcpip.h> // window sockets
#include <string.h>	

namespace Magma
{
	class Client
	{
		public:
			Client();
			~Client();

			void Bind();
			bool SendMessage(std::string msg);
			bool GetBound() { return m_Bound; };

		private:
			SOCKET m_Out;
			sockaddr_in m_ServerHint;
			bool m_Bound;
	};
}