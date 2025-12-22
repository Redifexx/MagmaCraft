#pragma once

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <WS2tcpip.h> // window sockets

// thank you Sloan Kelly on youtube

namespace Magma
{
	class Server
	{
		public:
			Server();
			~Server();

			void Bind();
			void Update();
			bool GetBound() { return m_Bound; };

		private:
			SOCKET m_In;
			sockaddr_in m_ServerHint;
			bool m_Bound;
	};
}