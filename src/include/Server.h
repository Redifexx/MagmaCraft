#pragma once
#include <enet/enet.h>
#include <iostream>
#include <map>

// thank you Sloan Kelly on youtube

namespace Magma
{
	class Server
	{
		public:
			Server();
			~Server();

			void Update();

		private:
			ENetAddress m_ServerHint;
			ENetHost* m_Host;
			int m_MaxClients = 10;
			std::map<UINT16, ENetPeer*> m_Clients;
	};
}