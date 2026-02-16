#include "NetworkManager.h"
#include "WorldManager.h"

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
	if (m_IsRunning) return false;

	m_IsRunning = true;
	switch (m_Role)
	{
		case NetworkRole::SERVER:
			m_Server = std::make_unique<Magma::Server>();
			m_Client = std::make_unique<Magma::Client>();
			m_WorldManager = std::make_shared<WorldManager>();
			m_NetworkIDToNameMap = std::make_shared<std::unordered_map<uint8_t, std::string>>();
			m_WorldManager->SetNetworkIDToNameMap(m_NetworkIDToNameMap);
			return true;

		case NetworkRole::CLIENT:
			m_Client = std::make_unique<Magma::Client>();
			m_WorldManager = std::make_shared<WorldManager>();
			m_NetworkIDToNameMap = std::make_shared<std::unordered_map<uint8_t, std::string>>();
			m_WorldManager->SetNetworkIDToNameMap(m_NetworkIDToNameMap);
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
void NetworkManager::SendChunkData(ENetPeer* peer, int chunkX, int chunkY, int chunkZ)
{
	// get compressed chunk data
	std::vector<uint8_t> chunkData = m_WorldManager->GetChunkDataCompressed(chunkX, chunkY, chunkZ);

	// write to packet
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::CHUNK_DATA));
	writer.WriteInt(chunkX);
	writer.WriteInt(chunkY);
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

void NetworkManager::RequestChunkData(ENetPeer* peer, int chunkX, int chunkY, int chunkZ)
{
	// write to packet
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::CHUNK_REQUEST));
	writer.WriteInt(chunkX);
	writer.WriteInt(chunkY);
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
	pData.networkID = playerRef.networkID;

	//std::cout << "My NID " << (int)m_NetworkID << " & Sending nID: " << (int)playerRef.networkID;
	if (playerRef.networkID != m_NetworkID) std::cout << "SENDING WRONG DATA" << std::endl;

	pData.playerData = {};

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
	writer.WriteData(&pData, sizeof(pData));

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

void NetworkManager::SendPlayerDisconnect(uint8_t networkID)
{
	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::PLAYER_DISCONNECT));
	writer.WriteByte(networkID);

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE
	);

	// send
	enet_host_broadcast(m_Server->GetENetHost(), 0, packet);
}

void NetworkManager::DisconnectFromServer(bool selfDisconnect)
{
	if (!m_Client) return;
	m_PlayerNames.clear();
	m_NetworkIDToNameMap->clear();
	m_NameToNetworkIDMap.clear();
	m_Client->DisconnectFromServer(selfDisconnect);
}

void NetworkManager::ShutdownServer()
{
	if (!m_Server) return;
	m_NameToNetworkIDMap.clear();
	m_NetworkIDToNameMap->clear();
	m_PeerToEntityMap.clear();
	m_ActiveNetworkIDs.clear();
	DisconnectFromServer(false);
	m_Server->ShutdownServer();
}

// Writes a packet to the server with username
void NetworkManager::LoginRequestPacket(const std::string& username)
{
	std::cout << "Sending Login Request Packet..." << std::endl;
	m_Client->SetConnectionState(Magma::ConnectionState::AUTHENTICATING);

	PacketWriter writer;
	writer.WriteByte(static_cast<uint8_t>(PacketType::LOGIN_REQUEST));

	char buffer[32] = { 0 };
	std::memcpy(buffer, username.c_str(), 31);
	writer.WriteData(buffer, sizeof(buffer));

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE
	);

	// send
	enet_peer_send(m_Client->GetENetPeer(), 0, packet);
	std::cout << "Login Request Packet Sent." << std::endl;
}

// Sends Incoming Player assigned network ID
void NetworkManager::LoginSuccessPacket(ENetPeer* peer, uint8_t networkID)
{
	std::cout << "Sending Login Success Packet..." << std::endl;
	auto eWorld = m_EntityWorld.lock();
	if (!eWorld) return;

	uint32_t entityID = eWorld->m_PlayerIDEntityMap[networkID];

	auto& playerRef = eWorld->GetComponent<PlayerComponent>(entityID);
	auto& transformRef = eWorld->GetComponent<TransformComponent>(entityID);
	auto& healthRef = eWorld->GetComponent<HealthComponent>(entityID);
	auto& physicsRef = eWorld->GetComponent<PhysicsComponent>(entityID);

	PlayerPacket pData = {};

	pData.packetType = static_cast<uint8_t>(PacketType::LOGIN_SUCCESS);
	pData.sequenceID = 0; // sending once
	pData.networkID = playerRef.networkID;


	pData.playerData = {};

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
	writer.WriteData(&pData, sizeof(pData));

	// create ENet packet
	ENetPacket* packet = enet_packet_create(
		writer.buffer.data(),
		writer.buffer.size(),
		ENET_PACKET_FLAG_RELIABLE
	);

	// send back to player
	enet_peer_send(peer, 0, packet);

	std::cout << "Login Success Packet Sent." << std::endl;
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
		int packetsProcessed = 0;
		m_StaleChunkTimer -= dt;
		
		if (m_StaleChunkTimer <= 0.0f)
		{
			m_StaleChunkTimer = m_StaleChunkRate;

			std::vector<glm::vec3> playerPositions;
			
			auto eWorld = m_EntityWorld.lock();
			if (eWorld)
			{
				auto transformPool = eWorld->GetComponentPool<TransformComponent>();
				auto playerPool = eWorld->GetComponentPool<PlayerComponent>();
				auto& entities = playerPool->GetAllEntities();

				for (auto entity : entities)
				{
					if (!transformPool->Contains(entity)) return;
					auto& transformRef = eWorld->GetComponent<TransformComponent>(entity);
					playerPositions.push_back(transformRef.localPosition);
				}
			}

			// remove stale chunks, make the distance large to avoid lag spikes
			m_WorldManager->UnloadStaleChunks(playerPositions, m_WorldManager->GetServerRenderDistance() * 2);
		}

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
					HandlePacket(event.packet, event.peer);
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					std::cout << "Received server disconnect packet" << std::endl;
					std::string username;
					uint8_t networkID = 0;


					// Remove from entity world and maps
					auto itr = m_PeerToEntityMap.find(event.peer->incomingPeerID);
					if (itr != m_PeerToEntityMap.end())
					{
						uint32_t entityID = m_PeerToEntityMap[event.peer->incomingPeerID];

						if (auto eWorld = m_EntityWorld.lock())
						{
							if (eWorld->Contains<PlayerComponent>(entityID))
							{
								auto& playerRef = eWorld->GetComponent<PlayerComponent>(entityID);

								if (!m_NetworkIDToNameMap->contains(playerRef.networkID)) return;

								username = (*m_NetworkIDToNameMap)[playerRef.networkID];
								networkID = m_NameToNetworkIDMap[username]; // optimize

								m_WorldManager->SavePlayerData(*eWorld, entityID);

								// Remove from list
								auto& playerNames = m_PlayerNames;
								const std::string& nameToRemove = (*m_NetworkIDToNameMap)[networkID];
								auto it = std::find(playerNames.begin(), playerNames.end(), nameToRemove);
								if (it != playerNames.end()) {
									playerNames.erase(it);
								}


								m_NameToNetworkIDMap.erase(username);
								m_NetworkIDToNameMap->erase(networkID);
								eWorld->m_PlayerIDEntityMap.erase(networkID);
								eWorld->RemoveEntity(entityID);
							}
						}
						m_PeerToEntityMap.erase(itr);
					}
					else
					{
						return;
					}

					// Clean active network ids
					m_ActiveNetworkIDs.erase(networkID);
					std::cout << username << " - " << (int)networkID << " disconnected." << std::endl;


					// relay player exit
					SendPlayerDisconnect(networkID);

					m_Server->GetClients().erase(event.peer->incomingPeerID);
					event.peer->data = NULL;
					break;
				}
			}

			packetsProcessed = 0;
			if (packetsProcessed > m_MaxPacketsProcessedPerFrame)
			{
				break;
			}
		}
	}

	// Handle Client Events
	if (m_Role == NetworkRole::CLIENT || m_Role == NetworkRole::SERVER)
	{
		int packetsProcessed = 0;

		while (enet_host_service(m_Client->GetENetHost(), &event, 0) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					std::cout << "Connected to server." << std::endl;
					LoginRequestPacket(m_LocalPlayerUsername);
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
					HandlePacket(event.packet, event.peer);
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					m_Client->DisconnectFromServer(true);
					break;
			}

			packetsProcessed = 0;
			if (packetsProcessed > m_MaxPacketsProcessedPerFrame)
			{
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
			// SERVER
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
			(*m_NetworkIDToNameMap)[networkID] = username;
			m_ActiveNetworkIDs[networkID] = peer->incomingPeerID;

			// Create Entity on Server
			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			uint32_t entityID = NULL_ENTITY;

			// add to entity map
			entityID = m_WorldManager->CreatePlayerEntity(*eWorld, networkID);
			eWorld->m_PlayerIDEntityMap[networkID] = entityID;

			auto& transformRef = eWorld->GetComponent<TransformComponent>(entityID);
			auto& healthRef = eWorld->GetComponent<HealthComponent>(entityID);
			auto& physicsRef = eWorld->GetComponent<PhysicsComponent>(entityID);

			// Add to peer map
			m_PeerToEntityMap[peer->incomingPeerID] = entityID;

			// Send player login success to peer
			LoginSuccessPacket(peer, networkID);
			std::cout << "Server added new NetworkID: " << (int)networkID << std::endl;


			// Introduce the player to everyone currently in server
			// create player join packet
			// please create helper function bro
			PlayerJoinPacket jData = {};
			jData.packetType = static_cast<uint8_t>(PacketType::PLAYER_JOIN);
			jData.networkID = networkID;

			// writing username safely to packet
			std::memset(jData.username, 0, sizeof(jData.username));
			std::strncpy(jData.username, username.c_str(), sizeof(jData.username) - 1);
			jData.username[31] = '\0';

			SerializedPlayerData pData = {};
			pData.posX = transformRef.localPosition.x;
			pData.posY = transformRef.localPosition.y;
			pData.posZ = transformRef.localPosition.z;

			pData.rotX = transformRef.localRotation.w;
			pData.rotX = transformRef.localRotation.x;
			pData.rotY = transformRef.localRotation.y;
			pData.rotZ = transformRef.localRotation.z;

			pData.health = healthRef.health;

			pData.velX = physicsRef.velocity.x;
			pData.velY = physicsRef.velocity.y;
			pData.velZ = physicsRef.velocity.z;

			jData.playerData = pData;

			PacketWriter writer;
			writer.WriteData(&jData, sizeof(PlayerJoinPacket));

			// create ENet packet
			ENetPacket* packet = enet_packet_create(
				writer.buffer.data(),
				writer.buffer.size(),
				ENET_PACKET_FLAG_RELIABLE
			);

			// broadcast to all except the new player
			for (auto const& [id, peerID] : m_ActiveNetworkIDs)
			{
				if (id != networkID)
				{
					if (m_Server->GetClients().count(peerID))
					{
						enet_peer_send(m_Server->GetClients()[peerID], 0, packet);
					}
				}
			}

			// Send player list to newcomer
			for (auto const& [name, netID] : m_NameToNetworkIDMap)
			{
				if (netID == networkID) continue; // skip itself

				PlayerJoinPacket jData = {};
				jData.packetType = static_cast<uint8_t>(PacketType::PLAYER_JOIN);
				jData.networkID = netID;
				strncpy(jData.username, name.c_str(), 32);

				// search up the other current players' info
				entityID = eWorld->m_PlayerIDEntityMap[netID];

				transformRef = eWorld->GetComponent<TransformComponent>(entityID);
				healthRef = eWorld->GetComponent<HealthComponent>(entityID);
				physicsRef = eWorld->GetComponent<PhysicsComponent>(entityID);

				SerializedPlayerData pData = {};
				pData.posX = transformRef.localPosition.x;
				pData.posY = transformRef.localPosition.y;
				pData.posZ = transformRef.localPosition.z;

				pData.rotX = transformRef.localRotation.w;
				pData.rotX = transformRef.localRotation.x;
				pData.rotY = transformRef.localRotation.y;
				pData.rotZ = transformRef.localRotation.z;

				pData.health = healthRef.health;

				pData.velX = physicsRef.velocity.x;
				pData.velY = physicsRef.velocity.y;
				pData.velZ = physicsRef.velocity.z;

				jData.playerData = pData;

				PacketWriter writer;
				writer.WriteData(&jData, sizeof(PlayerJoinPacket));

				// create ENet packet
				ENetPacket* jPacket = enet_packet_create(
					writer.buffer.data(),
					writer.buffer.size(),
					ENET_PACKET_FLAG_RELIABLE
				);
				enet_peer_send(peer, 0, jPacket);
			}
			std::cout << "Login Request Packet Processed." << std::endl;
			break;
		}

		case static_cast<uint8_t>(PacketType::LOGIN_SUCCESS):
		{
			std::cout << "Received Login Success Packet..." << std::endl;
			// CLIENT
			uint8_t networkID = 0;

			// read packet
			PlayerPacket pData;

			// memory aligned so should work
			memcpy(&pData, data, sizeof(PlayerPacket));

			networkID = pData.networkID;
			m_NetworkID = networkID;

			std::cout << "My NetworkID: " << (int)networkID << std::endl;

			if (m_Role == NetworkRole::CLIENT)
			{
				auto eWorld = m_EntityWorld.lock();
				if (!eWorld) return;

				// Create Myself
				uint32_t entityID = m_WorldManager->CreatePlayerEntity(*eWorld, networkID);
				eWorld->m_PlayerIDEntityMap[networkID] = entityID;


				// set spawn point from file
				auto& transformRef = eWorld->GetComponent<TransformComponent>(entityID);
				auto& healthRef = eWorld->GetComponent<HealthComponent>(entityID);
				auto& physicsRef = eWorld->GetComponent<PhysicsComponent>(entityID);

				transformRef.localPosition.x = pData.playerData.posX;
				transformRef.localPosition.y = pData.playerData.posY;
				transformRef.localPosition.z = pData.playerData.posZ;

				std::cout << "Spawning at...";
				std::cout << transformRef.localPosition.x << " ";
				std::cout << transformRef.localPosition.y << " ";
				std::cout << transformRef.localPosition.z << std::endl;

				transformRef.localRotation.w = pData.playerData.rotW;
				transformRef.localRotation.x = pData.playerData.rotX;
				transformRef.localRotation.y = pData.playerData.rotY;
				transformRef.localRotation.z = pData.playerData.rotZ;

				transformRef.isDirty = true;

				healthRef.health = pData.playerData.health;

				physicsRef.velocity.x = pData.playerData.velX;
				physicsRef.velocity.y = pData.playerData.velY;
				physicsRef.velocity.z = pData.playerData.velZ;
			}
			m_Client->SetConnectionState(Magma::ConnectionState::LOGGED_IN);
			std::cout << "Login Success Packet Processed." << std::endl;
			break;
		}

		case static_cast<uint8_t>(PacketType::CHUNK_REQUEST):
		{
			// deny requests from players we dont know
			auto itr = m_PeerToEntityMap.find(peer->incomingPeerID);
			if (itr == m_PeerToEntityMap.end()) return;

			// SERVER
			if (length < 13) break; // not enough data (type, int, int, int)

			int chunkX = 0;
			int chunkY = 0;
			int chunkZ = 0;

			std::memcpy(&chunkX, &data[1], sizeof(int));
			std::memcpy(&chunkY, &data[5], sizeof(int));
			std::memcpy(&chunkZ, &data[9], sizeof(int));

			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			// check who it is and deny them a chunk if it's outside their render distance
			auto& transformRef = eWorld->GetComponent<TransformComponent>(itr->second);

			int playerChunkX = WorldToChunkPos(static_cast<int>(transformRef.localPosition.x));
			int playerChunkY = WorldToChunkPos(static_cast<int>(transformRef.localPosition.y));
			int playerChunkZ = WorldToChunkPos(static_cast<int>(transformRef.localPosition.z));

			// using chessboard / chebyshev distance algo here

			int dx = std::abs(chunkX - playerChunkX);
			int dy = std::abs(chunkY - playerChunkY);
			int dz = std::abs(chunkZ - playerChunkZ);

			int dist = std::max({ dx, dy, dz });

			// added a small buffer
			if (dist > (m_WorldManager->GetServerRenderDistance() + 2)) return;

			if (chunkY < 0 || chunkY > 15)
			{
				return; // prevents the server from crashing sending chunks from all directions
			}

			SendChunkData(peer, chunkX, chunkY, chunkZ);
			break;
		}

		case static_cast<uint8_t>(PacketType::CHUNK_DATA):
		{
			// CLIENT

			if (length < 17)
			{
				std::cout << "Recieved Bad Chunk Data." << std::endl;
				break;
			}

			int chunkX = 0;
			int chunkY = 0;
			int chunkZ = 0;
			size_t dataSize = length;

			std::memcpy(&chunkX, &data[1], sizeof(int));
			dataSize -= sizeof(int);
			std::memcpy(&chunkY, &data[5], sizeof(int));
			dataSize -= sizeof(int);
			std::memcpy(&chunkZ, &data[9], sizeof(int));
			dataSize -= sizeof(int);


			if (data + 13 > data + length) break;
			std::vector<uint8_t> chunkData(data + 13, data + length);
			if (chunkData.size() < 4) break;
			std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
			m_WorldManager->DecompressChunkData(chunkData, *chunk);
			m_WorldManager->AddChunkDataToBuffer(std::move(chunk), chunkX, chunkY, chunkZ);
			// Handle chunk data logic
			break;
		}

		case static_cast<uint8_t>(PacketType::BLOCK_UPDATE):
			std::cout << "Received BLOCK_UPDATE packet." << std::endl;
			// Handle block update logic
			// not implemented yet :(
			break;

		case static_cast<uint8_t>(PacketType::PLAYER_JOIN):
		{
			std::cout << "Received Player Join Packet..." << std::endl;
			// CLIENT
			if (length < sizeof(PlayerJoinPacket)) return;

			// read packet
			PlayerJoinPacket jData;

			// memory aligned so should work
			memcpy(&jData, data, sizeof(PlayerJoinPacket));

			if (jData.networkID == m_NetworkID)
			{
				return;
			}

			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			std::string username = std::string(jData.username);
			m_NameToNetworkIDMap[username] = jData.networkID;
			(*m_NetworkIDToNameMap)[jData.networkID] = username;
			m_PlayerNames.push_back((*m_NetworkIDToNameMap)[jData.networkID]);

			// Check if we already added this player as server-client combo
			if (eWorld->m_PlayerIDEntityMap.contains(jData.networkID)) return; 

			uint32_t entityID = m_WorldManager->CreatePlayerEntity(*eWorld, jData.networkID);
			std::cout << "New NetworkID Joined: " << (int)jData.networkID << std::endl;

			eWorld->m_PlayerIDEntityMap[jData.networkID] = entityID;

			auto& transformRef = eWorld->GetComponent<TransformComponent>(entityID);
			auto& healthRef = eWorld->GetComponent<HealthComponent>(entityID);
			auto& physicsRef = eWorld->GetComponent<PhysicsComponent>(entityID);

			transformRef.localPosition.x = jData.playerData.posX;
			transformRef.localPosition.y = jData.playerData.posY;
			transformRef.localPosition.z = jData.playerData.posZ;

			transformRef.localRotation.w = jData.playerData.rotW;
			transformRef.localRotation.x = jData.playerData.rotX;
			transformRef.localRotation.y = jData.playerData.rotY;
			transformRef.localRotation.z = jData.playerData.rotZ;

			transformRef.isDirty = true;

			healthRef.health = jData.playerData.health;

			physicsRef.velocity.x = jData.playerData.velX;
			physicsRef.velocity.y = jData.playerData.velY;
			physicsRef.velocity.z = jData.playerData.velZ;

			std::cout << "Player Join Packet Processed. Welcome " << username << " at ";
			std::cout << transformRef.localPosition.x << " ";
			std::cout << transformRef.localPosition.y << " ";
			std::cout << transformRef.localPosition.z << std::endl;

			break;
		}

		case static_cast<uint8_t>(PacketType::PLAYER_DATA):
		{
			// SERVER / CLIENT

			size_t offset = 1;

			// 1 byte - packet type
			// 4 bytes
			uint32_t sequenceID = 0;
			std::memcpy(&sequenceID, &data[offset], sizeof(uint32_t));
			offset += sizeof(uint32_t);

			uint8_t networkID = 0;
			std::memcpy(&networkID, &data[offset], sizeof(uint8_t));
			offset += sizeof(uint8_t);

			// 76 Bytes
			SerializedPlayerData pData;

            if (length < (offset + sizeof(SerializedPlayerData))) return;

			// Data Copy
			std::memcpy(&pData, &data[offset], sizeof(SerializedPlayerData));

			// check if player is new connection
			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			uint32_t entityID = NULL_ENTITY;

			if (m_Role == NetworkRole::SERVER)
			{
				if (m_PeerToEntityMap.find(peer->incomingPeerID) == m_PeerToEntityMap.end())
				{
					std::cout << "unknown player's data" << std::endl;
					// ignore unverified players
					return;
				}
				entityID = m_PeerToEntityMap[peer->incomingPeerID]; // possible optimization here

			}
			else if (m_Role == NetworkRole::CLIENT)
			{
				if (eWorld->m_PlayerIDEntityMap.find(networkID) == eWorld->m_PlayerIDEntityMap.end())
				{
					// didn't know player
					std::cout << "unknown player's data" << std::endl;
					return;
				}
				entityID = eWorld->m_PlayerIDEntityMap[networkID];
			}

			auto& playerComp = eWorld->GetComponent<PlayerComponent>(entityID);
			if (playerComp.isLocalPlayer)
			{
				return;
			}

			// move data to player components

			auto& playerRef = eWorld->GetComponent<PlayerComponent>(entityID);

			if (!isSequenceNewer(sequenceID, playerRef.lastSequenceID)) return;
			playerRef.lastSequenceID = sequenceID;

			auto& transformRef = eWorld->GetComponent<TransformComponent>(entityID);
			auto& healthRef = eWorld->GetComponent<HealthComponent>(entityID);
			auto& physicsRef = eWorld->GetComponent<PhysicsComponent>(entityID);

			// begin interpolation
			playerRef.startPos = transformRef.localPosition;
			playerRef.startRot = transformRef.localRotation;

			playerRef.targetPos = glm::vec3(pData.posX, pData.posY, pData.posZ);
			playerRef.targetRot = glm::quat(pData.rotW, pData.rotX, pData.rotY, pData.rotZ);

			playerRef.interpolationTime = 0.0f;

			if (glm::distance(playerRef.startPos, playerRef.targetPos) > 10.0f) // rubberbanding solution
			{
				transformRef.localPosition = playerRef.targetPos;
				playerRef.startPos = playerRef.targetPos;
			}

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
			break;
		}

		case static_cast<uint8_t>(PacketType::PLAYER_DISCONNECT):
		{
			// CLIENT
			size_t dataSize = length;

			uint8_t networkID = 0;

			std::memcpy(&networkID, &data[1], sizeof(uint8_t));

			if (!m_NetworkIDToNameMap->contains(networkID)) return; // we dont have that player

			auto eWorld = m_EntityWorld.lock();
			if (!eWorld) return;

			uint32_t entityID = eWorld->m_PlayerIDEntityMap[networkID];
			if (auto eWorld = m_EntityWorld.lock())
			{
				eWorld->m_PlayerIDEntityMap.erase(networkID);
				eWorld->RemoveEntity(entityID);
			}

            auto& playerNames = m_PlayerNames;
            const std::string& nameToRemove = (*m_NetworkIDToNameMap)[networkID];
            auto it = std::find(playerNames.begin(), playerNames.end(), nameToRemove);
            if (it != playerNames.end()) {
                playerNames.erase(it);
            }

			std::string username = (*m_NetworkIDToNameMap)[networkID];
			m_NameToNetworkIDMap.erase(username);
			(*m_NetworkIDToNameMap).erase(networkID);
			break;
		}

		default:
			std::cout << "Received unknown packet type: " << static_cast<int>(packetType) << std::endl;

	}
}