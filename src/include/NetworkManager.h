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
		SERVER,
		CLIENT
	};

	enum class PacketType
	{
		HANDSHAKE,
		MESSAGE,
		CHUNK_REQUEST, //chunk pos, add or remove
		CHUNK_DATA,
		BLOCK_UPDATE,
		PLAYER_DATA //pos, rotation, health, velocity
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
			void RequestChunkData(ENetPeer* peer, int chunkX, int chunkZ, bool chunkAdd);

			// Receive
			void Update(float dt);
			void HandlePacket(ENetPacket* packet, ENetPeer* peer);

			NetworkRole GetNetworkRole() const { return m_Role; }
			void SetNetworkRole(NetworkRole role) { m_Role = role; }

			Magma::Server* GetServer() const { return m_Server.get(); }
			Magma::Client* GetClient() const { return m_Client.get(); }
			bool IsRunning() const { return m_IsRunning; }
			std::shared_ptr<WorldManager> GetWorldManager() const { return m_WorldManager; }

		private:
			std::unique_ptr<Magma::Server> m_Server = nullptr;
			std::unique_ptr<Magma::Client> m_Client = nullptr;
			NetworkRole m_Role = NetworkRole::NONE;
			bool m_IsRunning = false;

			// World Classes
			std::shared_ptr<WorldManager> m_WorldManager = nullptr;
	};
}