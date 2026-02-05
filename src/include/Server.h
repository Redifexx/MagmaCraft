#pragma once
#include <enet/enet.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <map>

// thank you Sloan Kelly on youtube
// wrapper for enet server
namespace Magma
{
	class Server
	{
		public:
			Server();
			~Server();
			
			void ShutdownServer();

			int GetClientCount() const { return static_cast<int>(m_Clients.size()); }
			int GetMaxClients() const { return m_MaxClients; }
			ENetHost* GetENetHost() const { return m_Server; }
			std::unordered_map<uint16_t, ENetPeer*>& GetClients() { return m_Clients; }

		private:
			ENetHost* m_Server;
			ENetAddress m_ServerHint;
			int m_MaxClients = 16;
			std::unordered_map<uint16_t, ENetPeer*> m_Clients;
	};
}