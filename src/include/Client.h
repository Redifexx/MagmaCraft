#pragma once
#include <enet/enet.h>

#include <iostream>
#include <string.h>	
#include <vector>

namespace Magma
{
	enum class ConnectionState
	{
		DISCONNECTED,
		CONNECTING,
		CONNECTED,
		FAILED
	};

	class Client
	{
		public:
			Client();
			~Client();

			void SetServerHint(const char* hostName, enet_uint16 port);
			bool ConnectToServer();
			bool IsConnected() const;
			ConnectionState GetConnectionState() const { return m_ConnectionState; }
			void SetConnectionState(ConnectionState state) { m_ConnectionState = state; }

			float m_ConnectionTimer = 0.0f;
			const float CONNECTION_TIMEOUT = 5.0f;

		private:
			ENetHost* m_Client;
			ENetPeer* m_Server;
			ENetAddress m_ServerHint;
			ConnectionState m_ConnectionState = ConnectionState::DISCONNECTED;
	};
}