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
				std::cout << (char*)event.packet->data << std::endl;
				HandlePacket(event.packet, event.peer);
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				std::cout << "Player " << event.peer->address.host << " disconnected." << std::endl;
				m_Server->GetClients().erase(event.peer->incomingPeerID);
				break;
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
					std::cout << "Connected to server." << std::endl;
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					std::cout << (char*)event.packet->data << std::endl;
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
		case static_cast<uint8_t>(PacketType::HANDSHAKE):
			std::cout << "Received HANDSHAKE packet." << std::endl;
			// Handle handshake logic
			break;

		case static_cast<uint8_t>(PacketType::MESSAGE):
			break;

		case static_cast<uint8_t>(PacketType::CHUNK_REQUEST):
		{
			if (length < 9) break; // not enough data (type, int, int)

			int chunkX = 0;
			int chunkZ = 0;

			std::memcpy(&chunkX, &data[1], sizeof(int));
			std::memcpy(&chunkZ, &data[5], sizeof(int));
			// may have to consider endianness here

			std::cout << "Received CHUNK_REQUEST for chunk (" << chunkX << ", " << chunkZ << ")." << std::endl;
			
			SendChunkData(peer, chunkX, chunkZ);
			break;
		}

		case static_cast<uint8_t>(PacketType::CHUNK_DATA):
			std::cout << "Received CHUNK_DATA packet." << std::endl;


			// Handle chunk data logic
			break;

		case static_cast<uint8_t>(PacketType::BLOCK_UPDATE):
			std::cout << "Received BLOCK_UPDATE packet." << std::endl;
			// Handle block update logic
			break;

		default:
			std::cout << "Received unknown packet type: " << static_cast<int>(packetType) << std::endl;

	}
}