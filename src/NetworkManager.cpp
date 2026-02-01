#include "NetworkManager.h"

using namespace Craft;

void PacketWriter::WriteInt(int value)
{
	// write int as 4 bytes (little-endian)
	buffer.push_back(static_cast<uint8_t>(value & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void PacketWriter::WriteData(const void* data, size_t size)
{
	const uint8_t* bytes = static_cast<const uint8_t*>(data);
	buffer.insert(buffer.end(), bytes, bytes + size);
}

bool NetworkManager::Begin()
{
	m_IsRunning = true;
	switch (m_Role)
	{
		case NetworkRole::SERVER:
			m_Server = std::make_unique<Magma::Server>();
			m_Client = std::make_unique<Magma::Client>();
			m_WorldManager = std::make_shared<WorldManager>();
			return true;

		case NetworkRole::CLIENT:
			m_Client = std::make_unique<Magma::Client>();
			m_WorldManager = std::make_shared<WorldManager>();
			return true;

		default:
			m_IsRunning = false;
			return false;
	}
}

void NetworkManager::End()
{
	if (m_Server) m_Server.reset();
	if (m_Client) m_Client.reset();
	if (m_WorldManager) m_WorldManager.reset();
	NetworkRole m_Role = NetworkRole::NONE;
	m_IsRunning = false;
}	

// Can only be called from a packet
// only from server to client
void NetworkManager::SendChunkData(ENetPeer* peer, int chunkX, int chunkZ)
{
	// get compressed chunk data
	std::vector<uint8_t> chunkData = m_WorldManager->GetChunkDataCompressed(chunkX, chunkZ);

	// write to packet
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::CHUNK_DATA));
	writer.WriteInt(chunkX);
	writer.WriteInt(chunkZ);
	writer.WriteData(chunkData.data(), chunkData.size());

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE // bc chunk data is important
	);

	// send
	enet_peer_send(peer, 0, packet);
}

void NetworkManager::RequestChunkData(ENetPeer* peer, int chunkX, int chunkZ)
{
	// write to packet
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::CHUNK_REQUEST));
	writer.WriteInt(chunkX);
	writer.WriteInt(chunkZ);

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE // bc chunk data is important
	);

	// send
	enet_peer_send(peer, 0, packet);
}

// helper to handle wraparound if that ever happens (probably wont)
bool isSequenceNewer(uint32_t incoming, uint32_t existing)
{
	return ((int32_t)(incoming - existing)) > 0;
}

void NetworkManager::SendPlayerData(EntityWorld& eWorld, uint32_t entityID)
{
	auto& playerRef = eWorld.GetComponent<PlayerComponent>(entityID);
	auto& transformRef = eWorld.GetComponent<TransformComponent>(entityID);
	auto& healthRef = eWorld.GetComponent<HealthComponent>(entityID);
	auto& physicsRef = eWorld.GetComponent<PhysicsComponent>(entityID);

	PlayerPacket pData = {};

	pData.packetType = static_cast<uint8_t>(PacketType::PLAYER_DATA);
	pData.sequenceID = m_PlayerPacketSequence++;
	pData.playerData = {};

	std::strncpy(pData.playerData.username, playerRef.username.c_str(), sizeof(pData.playerData.username));

	pData.playerData.posX = transformRef.localPosition.x;
	pData.playerData.posY = transformRef.localPosition.y;
	pData.playerData.posZ = transformRef.localPosition.z;

	pData.playerData.rotW = transformRef.localRotation.w;
	pData.playerData.rotX = transformRef.localRotation.x;
	pData.playerData.rotY = transformRef.localRotation.y;
	pData.playerData.rotZ = transformRef.localRotation.z;

	pData.playerData.health = healthRef.health;

	pData.playerData.velX = physicsRef.velocity.x;
	pData.playerData.velY = physicsRef.velocity.y;
	pData.playerData.velZ = physicsRef.velocity.z;

	// write to packet
	PacketWriter writer;
	writer.WriteData(&pData.playerData, sizeof(pData.playerData));

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_UNSEQUENCED // bc chunk data is important
	);

	if (m_Role == NetworkRole::SERVER)
	{
		enet_host_broadcast(m_Server->GetENetHost(), 0, packet); // the host
	}
	else if (m_Role == NetworkRole::CLIENT)
	{
		enet_peer_send(m_Client->GetENetPeer(), 0, packet);
	}
}

void NetworkManager::SendPlayerDisconnect(const std::string& username)
{
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::PLAYER_DISCONNECT));

	char usernameChar[32];
	memset(usernameChar, 0, sizeof(usernameChar));
	strncpy(usernameChar, username.c_str(), sizeof(usernameChar));
	writer.WriteData(&usernameChar, sizeof(usernameChar));

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE
	);

	// send
	enet_host_broadcast(m_Server->GetENetHost(), 0, packet);
}

void NetworkManager::LoginRequest(const std::string& username)
{

	m_Client->SetConnectionState(Magma::ConnectionState::AUTHENTICATING);

	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::LOGIN_REQUEST));

	char buffer[32] = { 0 };
	size_t length = std::min(username.length(), sizeof(buffer));
	std::memcpy(buffer, username.data(), length);
	writer.WriteData(buffer, sizeof(buffer));

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE
	);

	// send
	enet_peer_send(m_Client->GetENetPeer(), 0, packet);
}

void NetworkManager::LoginSuccess(ENetPeer* peer, uint8_t networkID, const std::string& username)
{
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::LOGIN_SUCCESS));

	writer.WriteByte(networkID);

	char buffer[32] = { 0 };
	size_t length = std::min(username.length(), sizeof(buffer));
	std::memcpy(buffer, username.data(), length);
	writer.WriteData(buffer, sizeof(buffer));

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE
	);

	// send back to all players
	enet_host_broadcast(m_Server->GetENetHost(), 0, packet);
}

uint8_t NetworkManager::GetAvailableNetworkID()
{
	// 0 & 255 are reserved
	for (uint8_t id = 1; id < 255; ++id)
	{
		if (m_ActiveNetworkIDs.find(id) == m_ActiveNetworkIDs.end())
		{
			return id;
		}
	}

	std::cout << "Server is full." << std::endl;
	return 0;
}

void NetworkManager::Update(float dt)
{
	// Implementation for updating network state and handling incoming packets
	ENetEvent event;

	// Handle Server Events
	if (m_Role == NetworkRole::SERVER)
	{
		while (enet_host_service(m_Server->GetENetHost(), &event, 0) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
					printf("A new client connected from %x:%u.\n",
						event.peer->address.host,
						event.peer->address.port);
					std::cout << "Welcome Player " << event.peer->address.host << "!" << std::endl;
					m_Server->GetClients()[event.peer->incomingPeerID] = event.peer;
					std::cout << "Client Count : [" << m_Server->GetClientCount() << "/" << m_Server->GetMaxClients() << "]" << std::endl;
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					//std::cout << (char*)event.packet->data << std::endl;
					HandlePacket(event.packet, event.peer);
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					std::cout << "Player " << event.peer->address.host << " disconnected." << std::endl;

					std::string username;

					// Remove from entity world and maps
					if (m_PeerToEntityMap.find(event.peer->incomingPeerID) != m_PeerToEntityMap.end())
					{
						uint32_t entityID = m_PeerToEntityMap[event.peer->incomingPeerID];
						if (auto eWorld = m_EntityWorld.lock())
						{
							m_WorldManager->SavePlayerData(*eWorld, entityID);
							auto& playerRef = eWorld->GetComponent<PlayerComponent>(entityID);
							eWorld->m_PlayerEntityMap.erase(playerRef.username);
							username = playerRef.username;
							eWorld->RemoveEntity(entityID);
						}
						m_PeerToEntityMap.erase(event.peer->incomingPeerID);
					}

					// relay player exit
					if (m_Role == NetworkRole::SERVER)
					{
						// send exit packet
						SendPlayerDisconnect(username);
					}

					m_Server->GetClients().erase(event.peer->incomingPeerID);
					event.peer->data = NULL;
					break;
				}
			}
		}
	}

	// Handle Client Events
	if (m_Role == NetworkRole::CLIENT || m_Role == NetworkRole::SERVER)
	{
		while (enet_host_service(m_Client->GetENetHost(), &event, 0) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					std::cout << "Connected to server." << std::endl;
					LoginRequest(m_LocalPlayerUsername);
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
					//std::cout << (char*)event.packet->data << std::endl;
					HandlePacket(event.packet, event.peer);
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					std::cout << "Disconnected from server.\n";
					m_Server = nullptr;
					break;
			}
		}

		if (m_Client->GetConnectionState() == Magma::ConnectionState::CONNECTING)
		{
			if (m_Client->IsConnected())
			{
				m_Client->SetConnectionState(Magma::ConnectionState::CONNECTED);
			}
			else
			{
				m_Client->m_ConnectionTimer -= dt;
				if (m_Client->m_ConnectionTimer <= 0.0f)
				{
					m_Client->SetConnectionState(Magma::ConnectionState::FAILED);
				}
			}
		}
	}
}

void NetworkManager::HandlePacket(ENetPacket* packet, ENetPeer* peer)
{
	// Implementation for handling received packets

	uint8_t* data = packet->data;
	size_t length = packet->dataLength;

	if (length < 1) return;

	uint8_t packetType = data[0];

	switch (packetType)
	{
		case static_cast<uint8_t>(PacketType::LOGIN_REQUEST):
		{
			std::cout << "Received login request packet." << std::endl;

			// Read username and assign networkID
			std::string username(reinterpret_cast<const char*>(&data[1]), 32);

			size_t firstNull = username.find('\0');
			if (firstNull != std::string::npos)
			{
				username.resize(firstNull);
			}
		
			if (m_NameToNetworkIDMap.contains(username))
			{
				std::cout << "User already logged in." << std::endl;
				return;
			}

			uint8_t networkID = GetAvailableNetworkID();
			m_NameToNetworkIDMap[username] = networkID;
			m_ActiveNetworkIDs[networkID] = peer->incomingPeerID;

			// Create Entity on Server
			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			uint32_t entityID = NULL_ENTITY;

			// add to entity map
			entityID = m_WorldManager->CreatePlayerEntity(*eWorld, username);
			eWorld->m_PlayerIDEntityMap[networkID] = entityID;

			// Add to peer map
			m_PeerToEntityMap[peer->incomingPeerID] = entityID;

			// Send player login success to everyone
			LoginSuccess(peer, networkID, username);

			break;
		}

		case static_cast<uint8_t>(PacketType::LOGIN_SUCCESS):
		{
			// Player Receieves Login Success, Server relays to all other clients
			// this is a client packet, read as if client

			uint8_t networkID = 0;

			std::memcpy(&networkID, &data[1], sizeof(uint8_t));

			// Read username
			std::string username(reinterpret_cast<const char*>(&data[1]), 32);

			size_t firstNull = username.find('\0');
			if (firstNull != std::string::npos)
			{
				username.resize(firstNull);
			}

			// Create Entity on Client
			// If this was our request, we already created our player locally

			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			uint32_t entityID = NULL_ENTITY;

			entityID = m_WorldManager->CreatePlayerEntity(*eWorld, username);

			if (username != m_LocalPlayerUsername) m_NetworkID = networkID;

			eWorld->m_PlayerIDEntityMap[networkID] = entityID;
			m_NetworkIDToNameMap[networkID] = username;

			m_Client->SetConnectionState(Magma::ConnectionState::LOGGED_IN);

			break;
		}

		case static_cast<uint8_t>(PacketType::CHUNK_REQUEST):
		{
			if (length < 9) break; // not enough data (type, int, int)

			int chunkX = 0;
			int chunkZ = 0;

			std::memcpy(&chunkX, &data[1], sizeof(int));
			std::memcpy(&chunkZ, &data[5], sizeof(int));
			// may have to consider endianness here

			//std::cout << "Received CHUNK_REQUEST for chunk (" << chunkX << ", " << chunkZ << ")." << std::endl;

			SendChunkData(peer, chunkX, chunkZ);
			break;
		}

		case static_cast<uint8_t>(PacketType::CHUNK_DATA):
		{
			std::cout << "Received CHUNK_DATA packet." << std::endl;

			if (length < 10) break; // not enough data (type, int, int, data)

			int chunkX = 0;
			int chunkZ = 0;
			size_t dataSize = length;

			std::memcpy(&chunkX, &data[1], sizeof(int));
			dataSize -= sizeof(int);
			std::memcpy(&chunkZ, &data[5], sizeof(int));
			dataSize -= sizeof(int);
			std::vector<uint8_t> chunkData(data + 9, data + length);

			std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
			m_WorldManager->DecompressChunkData(chunkData, *chunk);
			m_WorldManager->AddChunkDataToBuffer(std::move(chunk), chunkX, chunkZ);
			// Handle chunk data logic
			break;
		}

		case static_cast<uint8_t>(PacketType::BLOCK_UPDATE):
			std::cout << "Received BLOCK_UPDATE packet." << std::endl;
			// Handle block update logic
			break;

		case static_cast<uint8_t>(PacketType::PLAYER_DATA):
		{
			std::cout << "Received PLAYER_DATA packet." << std::endl;

			size_t offset = 1;

			// 1 byte - packet type
			// 4 bytes
			uint32_t sequenceID = 0;
			std::memcpy(&sequenceID, &data[offset], sizeof(uint32_t));
			offset += sizeof(uint32_t);

			// 76 Bytes
			SerializedPlayerData pData = {};

            if (length < (offset + sizeof(SerializedPlayerData))) return;

			// Data Copy
			std::memcpy(&pData, &data[offset], sizeof(SerializedPlayerData));

			// check if player is new connection
			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			uint32_t entityID = NULL_ENTITY;

			if (m_Role == NetworkRole::SERVER)
			{
				entityID = m_PeerToEntityMap[peer->incomingPeerID];
			}
			else if (m_Role == NetworkRole::CLIENT)
			{
				entityID = eWorld->m_PlayerEntityMap[static_cast<std::string>(pData.username)];
			}

			// move data to player components

			auto& playerRef = eWorld->GetComponent<PlayerComponent>(entityID);

			if (!isSequenceNewer(sequenceID, playerRef.lastSequenceID)) return;
			playerRef.lastSequenceID = sequenceID;

			auto& transformRef = eWorld->GetComponent<TransformComponent>(entityID);
			auto& healthRef = eWorld->GetComponent<HealthComponent>(entityID);
			auto& physicsRef = eWorld->GetComponent<PhysicsComponent>(entityID);

			transformRef.localPosition.x = pData.posX;
			transformRef.localPosition.y = pData.posY;
			transformRef.localPosition.z = pData.posZ;

			transformRef.localRotation.w = pData.rotW;
			transformRef.localRotation.x = pData.rotX;
			transformRef.localRotation.y = pData.rotY;
			transformRef.localRotation.z = pData.rotZ;

			playerRef.username = pData.username; // pointless to do every tick will fix later

			healthRef.health = pData.health;

			physicsRef.velocity.x = pData.velX;
			physicsRef.velocity.y = pData.velY;
			physicsRef.velocity.z = pData.velZ;

			// relay
			if (m_Role == NetworkRole::SERVER)
			{
				ENetPacket* packet = enet_packet_create(
					data,
					length,
					ENET_PACKET_FLAG_UNSEQUENCED
				);

				for (auto const& [peerID, entityID] : m_PeerToEntityMap)
				{
					if (peerID != peer->incomingPeerID)
					{
						enet_peer_send(m_Server->GetClients()[peerID], 0, packet);
					}
				}
			}

			// Handle block update logic
			break;
		}

		case static_cast<uint8_t>(PacketType::PLAYER_DISCONNECT):
		{
			size_t dataSize = length;

			char usernameChar[32];
			std::memcpy(&usernameChar, &data[1], sizeof(usernameChar));

			std::string username = usernameChar;

			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			uint32_t entityID = eWorld->m_PlayerEntityMap[username];
			m_WorldManager->SavePlayerData(*eWorld, entityID);
			if (auto eWorld = m_EntityWorld.lock())
			{
				eWorld->m_PlayerEntityMap.erase(username);
				eWorld->RemoveEntity(entityID);
			}
		}

		default:
			std::cout << "Received unknown packet type: " << static_cast<int>(packetType) << std::endl;

	}
}