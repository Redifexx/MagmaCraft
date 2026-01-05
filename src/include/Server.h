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
			void SendPacket(const char* data, bool isReliable);
		private:
			ENetAddress m_ServerHint;
			ENetHost* m_Server;
			int m_MaxClients = 16;
			std::map<UINT16, ENetPeer*> m_Clients;
	};
}