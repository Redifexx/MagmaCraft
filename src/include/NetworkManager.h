#pragma once
#include <enet/enet.h>

#include <string>
#include <vector>
#include <cstdint>
#include "Server.h"
#include "Client.h"
#include "WorldManager.h"
#include "WorldRenderer.h"

namespace Craft
{
	enum class NetworkRole
	{
		NONE,
		SOLO,
		SERVER,
		CLIENT
	};

	enum class PacketType
	{
		HANDSHAKE,
		MESSAGE,
		CHUNK_REQUEST,
		CHUNK_DATA,
		BLOCK_UPDATE
	};

	struct PacketWriter
	{
		std::vector<uint8_t> buffer;

		void WriteByte(uint8_t value) { buffer.push_back(value); }
		void WriteInt(int value);
		void WriteData(const void* data, size_t size);
	};


	class NetworkManager
	{
		public:
			bool Begin();
			void End();

			// Send
			void SendChunkData(ENetPeer* peer, int chunkX, int chunkZ);

			// Receive
			void Update(float dt);
			void HandlePacket(ENetPacket* packet, ENetPeer* peer);

			NetworkRole GetNetworkRole() const { return m_Role; }
			void SetNetworkRole(NetworkRole role) { m_Role = role; }

			Server* GetServer() const { return m_Server; }
			Client* GetClient() const { return m_Client; }
			bool IsRunning() const { return m_IsRunning; }
			WorldManager* GetWorldManager() const { return m_WorldManager; }
			WorldRenderer* GetWorldRenderer() const { return m_WorldRenderer; }

		private:
			Server* m_Server = nullptr;
			Client* m_Client = nullptr;
			NetworkRole m_Role = NetworkRole::NONE;
			bool m_IsRunning = false;

			// World Classes
			WorldManager* m_WorldManager = nullptr;
			WorldRenderer* m_WorldRenderer = nullptr;
	};
}

// next up: implement SendChunkData and HandlePacket methods