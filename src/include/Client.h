#pragma once
#include <enet/enet.h>

#include <iostream>
#include <string.h>	

namespace Magma
{
	class Client
	{
		public:
			Client();
			~Client();

			void SetServerHint(const char* hostName, enet_uint16 port);
			bool ConnectToServer();
			void Update();
			void SendPacket(const char* data, bool isReliable);

		private:
			ENetHost* m_Client;
			ENetPeer* m_Server;
			ENetAddress m_ServerHint;
	};
}