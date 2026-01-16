#pragma once
#include <enet/enet.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// thank you Sloan Kelly on youtube

namespace Magma
{
	class Server
	{
		public:
			Server();
			~Server();

			int GetClientCount() const { return static_cast<int>(m_Clients.size()); }
			int GetMaxClients() const { return m_MaxClients; }

		private:
			ENetAddress m_ServerHint;
			ENetHost* m_Server;
			int m_MaxClients = 16;
			std::map<UINT16, ENetPeer*> m_Clients;
	};
}