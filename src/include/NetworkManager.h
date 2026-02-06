#pragma once
#include <enet/enet.h>

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "Server.h"
#include "Client.h"
#include "WorldManager.h"
#include "WorldRenderer.h"
#include <Datatypes/EntityWorld.h>

// The core of the entire application
// Sends/Receives/Processes Packets

namespace Craft
{

	class WorldManager; // forward declaration

	enum class NetworkRole
	{
		NONE,
		SERVER,
		CLIENT
	};

	enum class PacketType
	{
		LOGIN_REQUEST,
		LOGIN_SUCCESS,
		CHUNK_REQUEST, //chunk pos
		CHUNK_DATA,
		BLOCK_UPDATE,
		PLAYER_JOIN,
		PLAYER_DATA, //pos, rotation, health, velocity
		PLAYER_DISCONNECT
	};

	struct PacketWriter
	{
		std::vector<uint8_t> buffer;

		void WriteByte(uint8_t value) { buffer.push_back(value); }
		void WriteInt(int value);
		void WriteData(const void* data, size_t size);
	};

	// PACKET DEFINTIONS
	#pragma pack(push, 1)
	struct SerializedPlayerData
	{
		float posX, posY, posZ;
		float rotW, rotX, rotY, rotZ;
		float health;
		float velX, velY, velZ;
	};
	#pragma pack(pop)

	#pragma pack(push, 1)
	struct PlayerPacket
	{
		uint8_t packetType;
		uint32_t sequenceID;
		uint8_t networkID;
		Craft::SerializedPlayerData playerData;
	};
	#pragma pack(pop)

	#pragma pack(push, 1)
	struct PlayerJoinPacket
	{
		uint8_t packetType;
		uint8_t networkID;
		char username[32];
		Craft::SerializedPlayerData playerData;
	};
	#pragma pack(pop)


	class NetworkManager
	{
		public:
			bool Begin();
			void End();

			// Send
			void SendChunkData(ENetPeer* peer, int chunkX, int chunkZ);
			void RequestChunkData(ENetPeer* peer, int chunkX, int chunkZ);

			void SendPlayerData(EntityWorld& eWorld, uint32_t entityID);
			void SendPlayerDisconnect(uint8_t networkID);
			void DisconnectFromServer(bool selfDisconnect = true); // helper for client
			void ShutdownServer();

			void LoginRequestPacket(const std::string& username);
			void LoginSuccessPacket(ENetPeer* peer, uint8_t networkID);
			uint8_t GetAvailableNetworkID();

			// Receive
			void Update(float dt);
			void HandlePacket(ENetPacket* packet, ENetPeer* peer);

			NetworkRole GetNetworkRole() const { return m_Role; }
			void SetNetworkRole(NetworkRole role) { m_Role = role; }

			Magma::Server* GetServer() const { return m_Server.get(); }
			Magma::Client* GetClient() const { return m_Client.get(); }

			void SetEntityWorld(std::shared_ptr<EntityWorld> eWorld) { m_EntityWorld = eWorld; }
			bool IsRunning() const { return m_IsRunning; }
			std::shared_ptr<WorldManager> GetWorldManager() { return m_WorldManager; }

			const std::string& GetLocalPlayerUsername() { return m_LocalPlayerUsername; }
			void SetLocalPlayerUsername(const std::string& username) { m_LocalPlayerUsername = username; }

			uint8_t GetNetworkID() { return m_NetworkID; }

			std::shared_ptr<std::unordered_map <uint8_t, std::string>> GetNetworkIDToNameMap() { return m_NetworkIDToNameMap; }

			// List of Player Names
			std::vector<std::string> m_PlayerNames;

		private:
			std::unique_ptr<Magma::Server> m_Server = nullptr;
			std::unique_ptr<Magma::Client> m_Client = nullptr;
			NetworkRole m_Role = NetworkRole::NONE;
			bool m_IsRunning = false;
			uint32_t m_MaxPacketsProcessedPerFrame = 50;

			uint32_t m_PlayerPacketSequence = 0;

			std::string m_LocalPlayerUsername; // i should maybe move this

			uint8_t m_NetworkID = 255;

			// World Classes
			std::shared_ptr<WorldManager> m_WorldManager = nullptr;
			std::weak_ptr<EntityWorld> m_EntityWorld;

			// NetworkID -> Local Peer ID 
			std::unordered_map <uint8_t, uint16_t> m_ActiveNetworkIDs; // only used by server

			// Local Peer ID -> EntityID
			std::unordered_map<uint16_t, uint32_t> m_PeerToEntityMap; // only used by server

			// Player Username -> NetworkID
			std::unordered_map <std::string, uint8_t> m_NameToNetworkIDMap; // only used by server

			// NetworkID -> Player Username
			std::shared_ptr<std::unordered_map <uint8_t, std::string>> m_NetworkIDToNameMap; // used by server & client
	};
}