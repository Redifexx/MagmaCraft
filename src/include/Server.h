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
			ENetHost* GetENetHost() const { return m_Server; }
			std::map<UINT16, ENetPeer*>& GetClients() { return m_Clients; }

		private:
			ENetHost* m_Server;
			ENetAddress m_ServerHint;
			int m_MaxClients = 16;
			std::map<UINT16, ENetPeer*> m_Clients;
	};
}